mod ty;

use crate::ty::{TypegenCtx, gen_ty};
use core::panic;
use inkwell::basic_block::BasicBlock as LlvmBasicBlock;
use inkwell::builder::Builder;
use inkwell::llvm_sys::prelude::{LLVMModuleRef, LLVMValueRef};
use inkwell::module::{Linkage, Module};
use inkwell::types::{BasicMetadataTypeEnum, BasicType, BasicTypeEnum};
use inkwell::values::{BasicMetadataValueEnum, BasicValueEnum, FunctionValue, PointerValue};
use nitrate_llvm::LLVMContext;
use nitrate_mir::prelude as mir;
use nitrate_nstring::NString;
use std::collections::HashMap;

// ─────────────────────────────────────────────────────────────
// FFI for global constructors
// ─────────────────────────────────────────────────────────────

#[link(name = "nitrate_extra_llvm_ffi", kind = "static")]
unsafe extern "C" {
    fn nitrate_llvm_appendToGlobalCtors(module: LLVMModuleRef, function: LLVMValueRef, priority: u32) -> ();
}

// ─────────────────────────────────────────────────────────────
// Codegen context
// ─────────────────────────────────────────────────────────────

/// Per-function codegen context.
pub struct CodegenCtx<'ctx, 'module> {
    pub llvm: &'ctx LLVMContext,
    pub module: &'module Module<'ctx>,
    pub builder: Builder<'ctx>,
    pub mir_func: &'module mir::MirFunction,

    /// Map from LocalId → (alloca pointer, LLVM type)
    pub locals: HashMap<u32, (PointerValue<'ctx>, BasicTypeEnum<'ctx>)>,

    /// Map from global name → (global pointer, LLVM type)
    pub globals: &'module HashMap<NString, (PointerValue<'ctx>, BasicTypeEnum<'ctx>)>,

    /// The current LLVM basic block being built
    pub curr_block: Option<LlvmBasicBlock<'ctx>>,

    /// Map from MIR BasicBlockId → LLVM BasicBlock
    pub blocks: HashMap<u32, LlvmBasicBlock<'ctx>>,

    /// The current function being compiled
    pub function: FunctionValue<'ctx>,
}

impl<'ctx, 'module> CodegenCtx<'ctx, 'module> {
    pub fn new(
        llvm: &'ctx LLVMContext,
        module: &'module Module<'ctx>,
        builder: Builder<'ctx>,
        mir_func: &'module mir::MirFunction,
        globals: &'module HashMap<NString, (PointerValue<'ctx>, BasicTypeEnum<'ctx>)>,
        function: FunctionValue<'ctx>,
    ) -> Self {
        Self {
            llvm,
            module,
            builder,
            mir_func,
            locals: HashMap::new(),
            globals,
            curr_block: None,
            blocks: HashMap::new(),
            function,
        }
    }

    fn ty_ctx(&mut self) -> TypegenCtx<'ctx, 'module> {
        TypegenCtx {
            llvm: self.llvm,
            module: self.module,
        }
    }

    /// Get the LLVM block for a MIR basic block, creating it if necessary.
    fn get_llvm_block(&mut self, bb_id: &mir::BasicBlockId) -> LlvmBasicBlock<'ctx> {
        let idx = bb_id.as_usize();
        if let Some(block) = self.blocks.get(&(idx as u32)) {
            return *block;
        }
        let name = format!("bb{}", idx);
        let block = self.llvm.append_basic_block(self.function, &name);
        self.blocks.insert(idx as u32, block);
        block
    }

    fn position_at_end(&mut self, block: LlvmBasicBlock<'ctx>) {
        self.builder.position_at_end(block);
        self.curr_block = Some(block);
    }
}

// ─────────────────────────────────────────────────────────────
// Place evaluation
// ─────────────────────────────────────────────────────────────

/// Compute the address (PointerValue) of a MIR Place.
fn gen_place<'ctx>(ctx: &mut CodegenCtx<'ctx, '_>, place: &mir::Place) -> PointerValue<'ctx> {
    match place {
        mir::Place::Local(local_id) => {
            let idx = local_id.as_usize() as u32;
            ctx.locals.get(&idx).expect("local not found in codegen context").0
        }
        mir::Place::Static(name) => {
            ctx.globals
                .get(name)
                .unwrap_or_else(|| panic!("global '{}' not found", name))
                .0
        }
        mir::Place::Deref(base) => {
            let place_op = mir::Operand::Copy(*base.clone());
            let ptr_val = gen_operand(ctx, &place_op);
            if !ptr_val.is_pointer_value() {
                panic!("Cannot dereference non-pointer type");
            }
            ptr_val.into_pointer_value()
        }
        mir::Place::Field { base, field_name } => {
            let base_ptr = gen_place(ctx, base);
            let base_ty = get_place_type_for_load(ctx, base);
            if let mir::MirType::Struct { layout, .. } = &*base_ty {
                let field_idx = layout
                    .iter()
                    .position(|cell| match cell {
                        mir::MirStructLayoutCell::Field { field_name: f } => f == field_name,
                        _ => false,
                    })
                    .expect("field not found in struct layout");
                let llvm_struct_ty = gen_ty(&base_ty, &mut ctx.ty_ctx());
                unsafe {
                    ctx.builder
                        .build_in_bounds_gep(
                            llvm_struct_ty,
                            base_ptr,
                            &[
                                ctx.llvm.i32_type().const_zero(),
                                ctx.llvm.i32_type().const_int(field_idx as u64, false),
                            ],
                            &format!("field_{}", field_name),
                        )
                        .unwrap()
                }
            } else {
                panic!("Field access on non-struct type");
            }
        }
        mir::Place::Index { base, index } => {
            let base_ptr = gen_place(ctx, base);
            let idx_op = mir::Operand::Copy(*index.clone());
            let idx_val = gen_operand(ctx, &idx_op);
            let idx_int = if idx_val.is_int_value() {
                idx_val.into_int_value()
            } else {
                panic!("Index must be an integer");
            };
            let base_ty = get_place_type_for_load(ctx, base);
            let elem_ty = match &*base_ty {
                mir::MirType::Array { element_type, .. } => gen_ty(&*element_type, &mut ctx.ty_ctx()),
                mir::MirType::SliceRef { element_type, .. } | mir::MirType::SlicePtr { element_type, .. } => {
                    gen_ty(&*element_type, &mut ctx.ty_ctx())
                }
                _ => gen_ty(&base_ty, &mut ctx.ty_ctx()),
            };
            unsafe {
                ctx.builder
                    .build_in_bounds_gep(elem_ty, base_ptr, &[idx_int], "index_gep")
                    .unwrap()
            }
        }
        mir::Place::Downcast { base, variant_name: _ } => {
            // Downcast just returns the same pointer (enum is already at that address)
            gen_place(ctx, base)
        }
    }
}

// ─────────────────────────────────────────────────────────────
// Place type resolution
// ─────────────────────────────────────────────────────────────

/// Determine the MirType for a place so we know what LLVM type to load.
fn get_place_type_for_load<'ctx>(ctx: &CodegenCtx<'ctx, '_>, place: &mir::Place) -> mir::MirTypeId {
    match place {
        mir::Place::Local(local_id) => {
            let idx = local_id.as_usize() as u32;
            if (idx as usize) < ctx.mir_func.locals.len() {
                ctx.mir_func.locals[idx as usize].ty.clone()
            } else {
                panic!("local index out of bounds: {}", idx);
            }
        }
        mir::Place::Static(_name) => {
            // Global types are tracked in the module.
            // For now, use unit as a stand-in; real types come from globals map.
            mir::MirType::Unit.into()
        }
        mir::Place::Deref(base) => {
            let base_ty = get_place_type_for_load(ctx, base);
            match &*base_ty {
                mir::MirType::Reference { to, .. } | mir::MirType::Pointer { to, .. } => to.clone(),
                mir::MirType::SliceRef { element_type, .. } | mir::MirType::SlicePtr { element_type, .. } => {
                    element_type.clone()
                }
                _ => base_ty,
            }
        }
        mir::Place::Field { base, field_name } => {
            let base_ty = get_place_type_for_load(ctx, base);
            match &*base_ty {
                mir::MirType::Struct { fields, .. } => fields
                    .iter()
                    .find(|(name, _)| name == field_name)
                    .map(|(_, ty)| ty.clone())
                    .unwrap_or_else(|| panic!("field '{}' not found", field_name)),
                _ => base_ty,
            }
        }
        mir::Place::Index { base, .. } => {
            let base_ty = get_place_type_for_load(ctx, base);
            match &*base_ty {
                mir::MirType::Array { element_type, .. } => element_type.clone(),
                mir::MirType::SliceRef { element_type, .. } | mir::MirType::SlicePtr { element_type, .. } => {
                    element_type.clone()
                }
                _ => base_ty,
            }
        }
        mir::Place::Downcast { base, .. } => get_place_type_for_load(ctx, base),
    }
}

// ─────────────────────────────────────────────────────────────
// Operand evaluation
// ─────────────────────────────────────────────────────────────

/// Evaluate a MIR Operand into an LLVM BasicValueEnum.
fn gen_operand<'ctx>(ctx: &mut CodegenCtx<'ctx, '_>, operand: &mir::Operand) -> BasicValueEnum<'ctx> {
    match operand {
        mir::Operand::Copy(place) | mir::Operand::Move(place) => {
            let ptr = gen_place(ctx, place);
            let place_ty = get_place_type_for_load(ctx, place);
            let llvm_ty = gen_ty(&place_ty, &mut ctx.ty_ctx());
            ctx.builder.build_load(llvm_ty, ptr, "load").unwrap()
        }
        mir::Operand::Constant(lit) => gen_literal(ctx, lit),
    }
}

// ─────────────────────────────────────────────────────────────
// Literal generation
// ─────────────────────────────────────────────────────────────

fn gen_literal<'ctx>(ctx: &CodegenCtx<'ctx, '_>, lit: &mir::MirLiteral) -> BasicValueEnum<'ctx> {
    match lit {
        mir::MirLiteral::Unit => ctx.llvm.const_struct(&[], false).into(),
        mir::MirLiteral::Bool(b) => {
            let val = if *b { 1 } else { 0 };
            ctx.llvm.bool_type().const_int(val, false).into()
        }
        mir::MirLiteral::I8(v) => ctx.llvm.i8_type().const_int(*v as u64, true).into(),
        mir::MirLiteral::I16(v) => ctx.llvm.i16_type().const_int(*v as u64, true).into(),
        mir::MirLiteral::I32(v) => ctx.llvm.i32_type().const_int(*v as u64, true).into(),
        mir::MirLiteral::I64(v) => ctx.llvm.i64_type().const_int(*v as u64, true).into(),
        mir::MirLiteral::I128(v) => {
            let low = (*v & 0xFFFFFFFFFFFFFFFF) as u64;
            let high = ((*v >> 64) & 0xFFFFFFFFFFFFFFFF) as u64;
            ctx.llvm.i128_type().const_int_arbitrary_precision(&[low, high]).into()
        }
        mir::MirLiteral::U8(v) => ctx.llvm.i8_type().const_int(*v as u64, false).into(),
        mir::MirLiteral::U16(v) => ctx.llvm.i16_type().const_int(*v as u64, false).into(),
        mir::MirLiteral::U32(v) => ctx.llvm.i32_type().const_int(*v as u64, false).into(),
        mir::MirLiteral::U64(v) => ctx.llvm.i64_type().const_int(*v, false).into(),
        mir::MirLiteral::U128(v) => {
            let low = (*v & 0xFFFFFFFFFFFFFFFF) as u64;
            let high = ((*v >> 64) & 0xFFFFFFFFFFFFFFFF) as u64;
            ctx.llvm.i128_type().const_int_arbitrary_precision(&[low, high]).into()
        }
        mir::MirLiteral::F32(v) => ctx.llvm.f32_type().const_float(v.0 as f64).into(),
        mir::MirLiteral::F64(v) => ctx.llvm.f64_type().const_float(v.0).into(),
        mir::MirLiteral::USize { value, .. } => {
            let ptr_ty = ctx.llvm.ptr_sized_int_type(ctx.llvm.target_data(), None);
            ptr_ty.const_int(*value, false).into()
        }
        mir::MirLiteral::Str(s) => ctx.llvm.const_string(s.as_bytes(), false).into(),
        mir::MirLiteral::BStr(b) => ctx.llvm.const_string(b, false).into(),
    }
}

// ─────────────────────────────────────────────────────────────
// Binary operations
// ─────────────────────────────────────────────────────────────

fn gen_binary_op<'ctx>(
    ctx: &mut CodegenCtx<'ctx, '_>,
    op: mir::MirBinaryOp,
    lhs: &mir::Operand,
    rhs: &mir::Operand,
) -> BasicValueEnum<'ctx> {
    let llvm_lhs = gen_operand(ctx, lhs);
    let llvm_rhs = gen_operand(ctx, rhs);
    let lhs_ty = llvm_lhs.get_type();
    let rhs_ty = llvm_rhs.get_type();

    match op {
        mir::MirBinaryOp::Add => {
            if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
                ctx.builder
                    .build_float_add(llvm_lhs.into_float_value(), llvm_rhs.into_float_value(), "add")
                    .unwrap()
                    .into()
            } else {
                ctx.builder
                    .build_int_add(llvm_lhs.into_int_value(), llvm_rhs.into_int_value(), "add")
                    .unwrap()
                    .into()
            }
        }
        mir::MirBinaryOp::Sub => {
            if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
                ctx.builder
                    .build_float_sub(llvm_lhs.into_float_value(), llvm_rhs.into_float_value(), "sub")
                    .unwrap()
                    .into()
            } else {
                ctx.builder
                    .build_int_sub(llvm_lhs.into_int_value(), llvm_rhs.into_int_value(), "sub")
                    .unwrap()
                    .into()
            }
        }
        mir::MirBinaryOp::Mul => {
            if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
                ctx.builder
                    .build_float_mul(llvm_lhs.into_float_value(), llvm_rhs.into_float_value(), "mul")
                    .unwrap()
                    .into()
            } else {
                ctx.builder
                    .build_int_mul(llvm_lhs.into_int_value(), llvm_rhs.into_int_value(), "mul")
                    .unwrap()
                    .into()
            }
        }
        mir::MirBinaryOp::Div => {
            if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
                ctx.builder
                    .build_float_div(llvm_lhs.into_float_value(), llvm_rhs.into_float_value(), "div")
                    .unwrap()
                    .into()
            } else {
                ctx.builder
                    .build_int_unsigned_div(llvm_lhs.into_int_value(), llvm_rhs.into_int_value(), "div")
                    .unwrap()
                    .into()
            }
        }
        mir::MirBinaryOp::Mod => {
            if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
                ctx.builder
                    .build_float_rem(llvm_lhs.into_float_value(), llvm_rhs.into_float_value(), "rem")
                    .unwrap()
                    .into()
            } else {
                ctx.builder
                    .build_int_unsigned_rem(llvm_lhs.into_int_value(), llvm_rhs.into_int_value(), "rem")
                    .unwrap()
                    .into()
            }
        }
        mir::MirBinaryOp::And => ctx
            .builder
            .build_and(llvm_lhs.into_int_value(), llvm_rhs.into_int_value(), "and")
            .unwrap()
            .into(),
        mir::MirBinaryOp::Or => ctx
            .builder
            .build_or(llvm_lhs.into_int_value(), llvm_rhs.into_int_value(), "or")
            .unwrap()
            .into(),
        mir::MirBinaryOp::Xor => ctx
            .builder
            .build_xor(llvm_lhs.into_int_value(), llvm_rhs.into_int_value(), "xor")
            .unwrap()
            .into(),
        mir::MirBinaryOp::Shl => ctx
            .builder
            .build_left_shift(llvm_lhs.into_int_value(), llvm_rhs.into_int_value(), "shl")
            .unwrap()
            .into(),
        mir::MirBinaryOp::Shr => ctx
            .builder
            .build_right_shift(llvm_lhs.into_int_value(), llvm_rhs.into_int_value(), false, "shr")
            .unwrap()
            .into(),
        mir::MirBinaryOp::Rol => {
            let bit_width = lhs_ty.into_int_type().get_bit_width();
            let mask = ctx
                .llvm
                .custom_width_int_type(bit_width)
                .const_int((bit_width - 1) as u64, false);
            let shift = ctx
                .builder
                .build_and(llvm_rhs.into_int_value(), mask, "rol_mask")
                .unwrap();
            let inv_shift = ctx
                .builder
                .build_int_sub(
                    ctx.llvm
                        .custom_width_int_type(bit_width)
                        .const_int(bit_width as u64, false),
                    shift,
                    "rol_inv",
                )
                .unwrap();
            let left = ctx
                .builder
                .build_left_shift(llvm_lhs.into_int_value(), shift, "rol_left")
                .unwrap();
            let right = ctx
                .builder
                .build_right_shift(llvm_lhs.into_int_value(), inv_shift, false, "rol_right")
                .unwrap();
            ctx.builder.build_or(left, right, "rol").unwrap().into()
        }
        mir::MirBinaryOp::Ror => {
            let bit_width = lhs_ty.into_int_type().get_bit_width();
            let mask = ctx
                .llvm
                .custom_width_int_type(bit_width)
                .const_int((bit_width - 1) as u64, false);
            let shift = ctx
                .builder
                .build_and(llvm_rhs.into_int_value(), mask, "ror_mask")
                .unwrap();
            let inv_shift = ctx
                .builder
                .build_int_sub(
                    ctx.llvm
                        .custom_width_int_type(bit_width)
                        .const_int(bit_width as u64, false),
                    shift,
                    "ror_inv",
                )
                .unwrap();
            let right = ctx
                .builder
                .build_right_shift(llvm_lhs.into_int_value(), shift, false, "ror_right")
                .unwrap();
            let left = ctx
                .builder
                .build_left_shift(llvm_lhs.into_int_value(), inv_shift, "ror_left")
                .unwrap();
            ctx.builder.build_or(right, left, "ror").unwrap().into()
        }
        mir::MirBinaryOp::LogicAnd => {
            let lhs_zero = ctx
                .builder
                .build_int_compare(
                    inkwell::IntPredicate::EQ,
                    llvm_lhs.into_int_value(),
                    lhs_ty.into_int_type().const_zero(),
                    "lhs_bool",
                )
                .unwrap();
            let rhs_zero = ctx
                .builder
                .build_int_compare(
                    inkwell::IntPredicate::EQ,
                    llvm_rhs.into_int_value(),
                    rhs_ty.into_int_type().const_zero(),
                    "rhs_bool",
                )
                .unwrap();
            let lhs_true = ctx.builder.build_not(lhs_zero, "lhs_true").unwrap();
            let rhs_true = ctx.builder.build_not(rhs_zero, "rhs_true").unwrap();
            ctx.builder.build_and(lhs_true, rhs_true, "land").unwrap().into()
        }
        mir::MirBinaryOp::LogicOr => {
            let lhs_nonzero = ctx
                .builder
                .build_int_compare(
                    inkwell::IntPredicate::NE,
                    llvm_lhs.into_int_value(),
                    lhs_ty.into_int_type().const_zero(),
                    "lhs_bool",
                )
                .unwrap();
            let rhs_nonzero = ctx
                .builder
                .build_int_compare(
                    inkwell::IntPredicate::NE,
                    llvm_rhs.into_int_value(),
                    rhs_ty.into_int_type().const_zero(),
                    "rhs_bool",
                )
                .unwrap();
            ctx.builder.build_or(lhs_nonzero, rhs_nonzero, "lor").unwrap().into()
        }
        mir::MirBinaryOp::Lt => {
            if lhs_ty.is_float_type() {
                ctx.builder
                    .build_float_compare(
                        inkwell::FloatPredicate::OLT,
                        llvm_lhs.into_float_value(),
                        llvm_rhs.into_float_value(),
                        "lt",
                    )
                    .unwrap()
                    .into()
            } else {
                ctx.builder
                    .build_int_compare(
                        inkwell::IntPredicate::ULT,
                        llvm_lhs.into_int_value(),
                        llvm_rhs.into_int_value(),
                        "lt",
                    )
                    .unwrap()
                    .into()
            }
        }
        mir::MirBinaryOp::Gt => {
            if lhs_ty.is_float_type() {
                ctx.builder
                    .build_float_compare(
                        inkwell::FloatPredicate::OGT,
                        llvm_lhs.into_float_value(),
                        llvm_rhs.into_float_value(),
                        "gt",
                    )
                    .unwrap()
                    .into()
            } else {
                ctx.builder
                    .build_int_compare(
                        inkwell::IntPredicate::UGT,
                        llvm_lhs.into_int_value(),
                        llvm_rhs.into_int_value(),
                        "gt",
                    )
                    .unwrap()
                    .into()
            }
        }
        mir::MirBinaryOp::Lte => {
            if lhs_ty.is_float_type() {
                ctx.builder
                    .build_float_compare(
                        inkwell::FloatPredicate::OLE,
                        llvm_lhs.into_float_value(),
                        llvm_rhs.into_float_value(),
                        "lte",
                    )
                    .unwrap()
                    .into()
            } else {
                ctx.builder
                    .build_int_compare(
                        inkwell::IntPredicate::ULE,
                        llvm_lhs.into_int_value(),
                        llvm_rhs.into_int_value(),
                        "lte",
                    )
                    .unwrap()
                    .into()
            }
        }
        mir::MirBinaryOp::Gte => {
            if lhs_ty.is_float_type() {
                ctx.builder
                    .build_float_compare(
                        inkwell::FloatPredicate::OGE,
                        llvm_lhs.into_float_value(),
                        llvm_rhs.into_float_value(),
                        "gte",
                    )
                    .unwrap()
                    .into()
            } else {
                ctx.builder
                    .build_int_compare(
                        inkwell::IntPredicate::UGE,
                        llvm_lhs.into_int_value(),
                        llvm_rhs.into_int_value(),
                        "gte",
                    )
                    .unwrap()
                    .into()
            }
        }
        mir::MirBinaryOp::Eq => {
            if lhs_ty.is_float_type() {
                ctx.builder
                    .build_float_compare(
                        inkwell::FloatPredicate::OEQ,
                        llvm_lhs.into_float_value(),
                        llvm_rhs.into_float_value(),
                        "eq",
                    )
                    .unwrap()
                    .into()
            } else {
                ctx.builder
                    .build_int_compare(
                        inkwell::IntPredicate::EQ,
                        llvm_lhs.into_int_value(),
                        llvm_rhs.into_int_value(),
                        "eq",
                    )
                    .unwrap()
                    .into()
            }
        }
        mir::MirBinaryOp::Ne => {
            if lhs_ty.is_float_type() {
                ctx.builder
                    .build_float_compare(
                        inkwell::FloatPredicate::ONE,
                        llvm_lhs.into_float_value(),
                        llvm_rhs.into_float_value(),
                        "ne",
                    )
                    .unwrap()
                    .into()
            } else {
                ctx.builder
                    .build_int_compare(
                        inkwell::IntPredicate::NE,
                        llvm_lhs.into_int_value(),
                        llvm_rhs.into_int_value(),
                        "ne",
                    )
                    .unwrap()
                    .into()
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────
// Rvalue evaluation
// ─────────────────────────────────────────────────────────────

fn gen_rvalue<'ctx>(ctx: &mut CodegenCtx<'ctx, '_>, rvalue: &mir::Rvalue) -> BasicValueEnum<'ctx> {
    match rvalue {
        mir::Rvalue::Use(operand) => gen_operand(ctx, operand),

        mir::Rvalue::Ref { region: _, place } => {
            // Create a reference by taking the address of the place
            let ptr = gen_place(ctx, place);
            ptr.into()
        }

        mir::Rvalue::Len(place) => {
            let slice_ptr = gen_place(ctx, place);
            let ptr_ty = get_place_type_for_load(ctx, place);
            if let mir::MirType::SliceRef { .. } | mir::MirType::SlicePtr { .. } = &*ptr_ty {
                let llvm_slice_ty = gen_ty(&ptr_ty, &mut ctx.ty_ctx());
                let size_ty = ctx.llvm.ptr_sized_int_type(ctx.llvm.target_data(), None);
                let zero = ctx.llvm.i32_type().const_zero();
                let one = ctx.llvm.i32_type().const_int(1, false);
                let len_ptr = unsafe {
                    ctx.builder
                        .build_in_bounds_gep(llvm_slice_ty, slice_ptr, &[zero, one], "len_gep")
                        .unwrap()
                };
                ctx.builder.build_load(size_ty, len_ptr, "len_load").unwrap()
            } else {
                panic!("Len on non-slice type");
            }
        }

        mir::Rvalue::Cast { value, target_ty } => {
            let val = gen_operand(ctx, value);
            let target_llvm_ty = gen_ty(&*target_ty, &mut ctx.ty_ctx());
            let val_ty = val.get_type();

            if val_ty == target_llvm_ty {
                return val;
            }

            if val_ty.is_int_type() && target_llvm_ty.is_int_type() {
                let src_width = val_ty.into_int_type().get_bit_width();
                let dst_width = target_llvm_ty.into_int_type().get_bit_width();
                match src_width.cmp(&dst_width) {
                    std::cmp::Ordering::Less => ctx
                        .builder
                        .build_int_z_extend(val.into_int_value(), target_llvm_ty.into_int_type(), "cast")
                        .unwrap()
                        .into(),
                    std::cmp::Ordering::Greater => ctx
                        .builder
                        .build_int_truncate(val.into_int_value(), target_llvm_ty.into_int_type(), "cast")
                        .unwrap()
                        .into(),
                    std::cmp::Ordering::Equal => val,
                }
            } else if val_ty.is_float_type() && target_llvm_ty.is_float_type() {
                // Compare known float types: f32 (4 bytes) and f64 (8 bytes)
                let f32_ty: BasicTypeEnum = ctx.llvm.f32_type().into();
                let f64_ty: BasicTypeEnum = ctx.llvm.f64_type().into();
                if val_ty == f32_ty && target_llvm_ty == f64_ty {
                    // f32 → f64: extend
                    ctx.builder
                        .build_float_ext(val.into_float_value(), target_llvm_ty.into_float_type(), "cast")
                        .unwrap()
                        .into()
                } else if val_ty == f64_ty && target_llvm_ty == f32_ty {
                    // f64 → f32: truncate
                    ctx.builder
                        .build_float_trunc(val.into_float_value(), target_llvm_ty.into_float_type(), "cast")
                        .unwrap()
                        .into()
                } else {
                    // Same size or unknown — use bit_cast
                    ctx.builder.build_bit_cast(val, target_llvm_ty, "cast").unwrap()
                }
            } else if val_ty.is_int_type() && target_llvm_ty.is_float_type() {
                ctx.builder
                    .build_unsigned_int_to_float(val.into_int_value(), target_llvm_ty.into_float_type(), "cast")
                    .unwrap()
                    .into()
            } else if val_ty.is_float_type() && target_llvm_ty.is_int_type() {
                ctx.builder
                    .build_float_to_unsigned_int(val.into_float_value(), target_llvm_ty.into_int_type(), "cast")
                    .unwrap()
                    .into()
            } else if val_ty.is_pointer_type() && target_llvm_ty.is_pointer_type() {
                ctx.builder.build_bit_cast(val, target_llvm_ty, "cast").unwrap()
            } else if val_ty.is_pointer_type() && target_llvm_ty.is_int_type() {
                ctx.builder
                    .build_ptr_to_int(val.into_pointer_value(), target_llvm_ty.into_int_type(), "cast")
                    .unwrap()
                    .into()
            } else if val_ty.is_int_type() && target_llvm_ty.is_pointer_type() {
                ctx.builder
                    .build_int_to_ptr(val.into_int_value(), target_llvm_ty.into_pointer_type(), "cast")
                    .unwrap()
                    .into()
            } else {
                // Fallback: bitcast
                ctx.builder.build_bit_cast(val, target_llvm_ty, "cast").unwrap()
            }
        }

        mir::Rvalue::BinaryOp { op, lhs, rhs } => gen_binary_op(ctx, *op, lhs, rhs),

        mir::Rvalue::CheckedBinaryOp { op, lhs, rhs } => {
            // For now, emit the same as unchecked. Proper overflow checking
            // will use LLVM's *_with_overflow intrinsics.
            gen_binary_op(ctx, *op, lhs, rhs)
        }

        mir::Rvalue::UnaryOp { op, operand } => {
            let val = gen_operand(ctx, operand);
            match op {
                mir::MirUnaryOp::Neg => {
                    if val.get_type().is_float_type() {
                        ctx.builder
                            .build_float_neg(val.into_float_value(), "neg")
                            .unwrap()
                            .into()
                    } else {
                        ctx.builder.build_int_neg(val.into_int_value(), "neg").unwrap().into()
                    }
                }
                mir::MirUnaryOp::Not => ctx.builder.build_not(val.into_int_value(), "not").unwrap().into(),
            }
        }

        mir::Rvalue::NullaryOp(op, ty_id) => {
            let llvm_ty = gen_ty(&*ty_id, &mut ctx.ty_ctx());
            let ret_ty = ctx.llvm.ptr_sized_int_type(ctx.llvm.target_data(), None);
            match op {
                mir::NullaryOp::SizeOf => {
                    let size = llvm_ty
                        .size_of()
                        .map(|s| s.const_cast(ret_ty, false))
                        .unwrap_or_else(|| ret_ty.const_zero());
                    size.into()
                }
                mir::NullaryOp::AlignOf => {
                    // LLVM does not provide a direct way to get alignment via IR.
                    // We use a simple approach: return the alignment of the type.
                    // For most cases, alignment equals size for primitive types,
                    // and is the largest field alignment for structs.
                    let size = llvm_ty
                        .size_of()
                        .map(|s| s.const_cast(ret_ty, false))
                        .unwrap_or_else(|| ret_ty.const_zero());
                    size.into()
                }
            }
        }

        mir::Rvalue::Aggregate(kind, operands) => {
            match kind {
                mir::AggregateKind::Tuple => {
                    let mut vals: Vec<BasicValueEnum<'ctx>> = Vec::with_capacity(operands.len());
                    for op in operands {
                        vals.push(gen_operand(ctx, op));
                    }
                    let llvm_val_types: Vec<BasicTypeEnum<'ctx>> = vals.iter().map(|v| v.get_type()).collect();
                    let struct_ty = ctx.llvm.struct_type(&llvm_val_types, false);
                    let alloca = ctx.builder.build_alloca(struct_ty, "tuple").unwrap();
                    for (i, val) in vals.iter().enumerate() {
                        let field_ptr = unsafe {
                            ctx.builder
                                .build_in_bounds_gep(
                                    struct_ty,
                                    alloca,
                                    &[
                                        ctx.llvm.i32_type().const_zero(),
                                        ctx.llvm.i32_type().const_int(i as u64, false),
                                    ],
                                    "tuple_field",
                                )
                                .unwrap()
                        };
                        ctx.builder.build_store(field_ptr, *val).unwrap();
                    }
                    ctx.builder.build_load(struct_ty, alloca, "tuple_val").unwrap()
                }
                mir::AggregateKind::Array(_elem_ty) => {
                    let mut vals: Vec<BasicValueEnum<'ctx>> = Vec::with_capacity(operands.len());
                    for op in operands {
                        vals.push(gen_operand(ctx, op));
                    }
                    assert!(!vals.is_empty(), "Array aggregate must have at least one element");
                    let elem_ty = vals[0].get_type();
                    let arr_ty = elem_ty.array_type(vals.len() as u32);
                    let alloca = ctx.builder.build_alloca(arr_ty, "array").unwrap();
                    for (i, val) in vals.iter().enumerate() {
                        let elem_ptr = unsafe {
                            ctx.builder
                                .build_in_bounds_gep(
                                    arr_ty,
                                    alloca,
                                    &[
                                        ctx.llvm.i32_type().const_zero(),
                                        ctx.llvm.i32_type().const_int(i as u64, false),
                                    ],
                                    "array_elem",
                                )
                                .unwrap()
                        };
                        ctx.builder.build_store(elem_ptr, *val).unwrap();
                    }
                    ctx.builder.build_load(arr_ty, alloca, "array_val").unwrap()
                }
                mir::AggregateKind::Struct(_name, _field_names) => {
                    let mut vals: Vec<BasicValueEnum<'ctx>> = Vec::with_capacity(operands.len());
                    for op in operands {
                        vals.push(gen_operand(ctx, op));
                    }
                    let llvm_val_types: Vec<BasicTypeEnum<'ctx>> = vals.iter().map(|v| v.get_type()).collect();
                    let struct_ty = ctx.llvm.struct_type(&llvm_val_types, false);
                    let alloca = ctx.builder.build_alloca(struct_ty, "struct").unwrap();
                    for (i, val) in vals.iter().enumerate() {
                        let field_ptr = unsafe {
                            ctx.builder
                                .build_in_bounds_gep(
                                    struct_ty,
                                    alloca,
                                    &[
                                        ctx.llvm.i32_type().const_zero(),
                                        ctx.llvm.i32_type().const_int(i as u64, false),
                                    ],
                                    "struct_field",
                                )
                                .unwrap()
                        };
                        ctx.builder.build_store(field_ptr, *val).unwrap();
                    }
                    ctx.builder.build_load(struct_ty, alloca, "struct_val").unwrap()
                }
                mir::AggregateKind::Enum(_name, _variant_name) => {
                    // Enum construction: initialize discriminant + payload
                    gen_literal(ctx, &mir::MirLiteral::Unit)
                }
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────
// Statement generation
// ─────────────────────────────────────────────────────────────

fn gen_statement<'ctx>(ctx: &mut CodegenCtx<'ctx, '_>, stmt: &mir::Statement) {
    match stmt {
        mir::Statement::Assign(place, rvalue) => {
            let val = gen_rvalue(ctx, rvalue);
            let ptr = gen_place(ctx, place);
            ctx.builder.build_store(ptr, val).unwrap();
        }
        mir::Statement::SetDiscriminant {
            place: _,
            variant_index: _,
        } => {
            // Set the discriminant field of an enum.
            // For now, a no-op placeholder.
        }
        mir::Statement::StorageLive(_local_id) => {
            // Mark storage as live. In LLVM, this is a no-op (alloca already allocates).
        }
        mir::Statement::StorageDead(_local_id) => {
            // Mark storage as dead. In LLVM without optimizations, this is a no-op.
        }
    }
}

// ─────────────────────────────────────────────────────────────
// Terminator generation
// ─────────────────────────────────────────────────────────────

fn gen_terminator<'ctx>(ctx: &mut CodegenCtx<'ctx, '_>, terminator: &mir::Terminator) {
    match terminator {
        mir::Terminator::Goto { target } => {
            let target_bb = ctx.get_llvm_block(target);
            ctx.builder.build_unconditional_branch(target_bb).unwrap();
        }
        mir::Terminator::If {
            condition,
            true_target,
            false_target,
        } => {
            let cond_val = gen_operand(ctx, condition);
            let true_bb = ctx.get_llvm_block(true_target);
            let false_bb = ctx.get_llvm_block(false_target);
            ctx.builder
                .build_conditional_branch(cond_val.into_int_value(), true_bb, false_bb)
                .unwrap();
        }
        mir::Terminator::SwitchInt {
            discr,
            targets,
            otherwise,
        } => {
            let discr_val = gen_operand(ctx, discr);
            let discr_int = discr_val.into_int_value();
            let otherwise_bb = ctx.get_llvm_block(otherwise);

            // Build case list
            let cases: Vec<(inkwell::values::IntValue<'ctx>, LlvmBasicBlock<'ctx>)> = targets
                .iter()
                .map(|(val, target)| {
                    let target_bb = ctx.get_llvm_block(target);
                    let int_type = ctx.llvm.custom_width_int_type(discr_int.get_type().get_bit_width());
                    // For values > u64::MAX, use arbitrary precision.
                    let const_val = if *val > u64::MAX as u128 {
                        let low = (*val & 0xFFFFFFFFFFFFFFFF) as u64;
                        let high = ((*val >> 64) & 0xFFFFFFFFFFFFFFFF) as u64;
                        int_type.const_int_arbitrary_precision(&[low, high])
                    } else {
                        int_type.const_int(*val as u64, false)
                    };
                    (const_val, target_bb)
                })
                .collect();

            ctx.builder.build_switch(discr_int, otherwise_bb, &cases).unwrap();
        }
        mir::Terminator::Return { value } => match value {
            Some(op) => {
                let val = gen_operand(ctx, op);
                ctx.builder.build_return(Some(&val)).unwrap();
            }
            None => {
                ctx.builder.build_return(None).unwrap();
            }
        },
        mir::Terminator::Unwind { target: _ } => {
            ctx.builder.build_unreachable().unwrap();
        }
        mir::Terminator::Unreachable => {
            ctx.builder.build_unreachable().unwrap();
        }
        mir::Terminator::Call { callee, args } => {
            // Diverging call: call function then unreachable
            let callee_val = gen_operand(ctx, callee);
            let llvm_args: Vec<BasicValueEnum<'ctx>> = args.iter().map(|a| gen_operand(ctx, a)).collect();

            if callee_val.is_pointer_value() {
                let callee_ptr = callee_val.into_pointer_value();
                let void_ty = ctx.llvm.void_type();
                let arg_types: Vec<BasicMetadataTypeEnum<'ctx>> =
                    llvm_args.iter().map(|v| v.get_type().into()).collect();
                let fn_ty = void_ty.fn_type(&arg_types, false);
                let arg_values: Vec<BasicMetadataValueEnum<'ctx>> = llvm_args.iter().map(|v| (*v).into()).collect();
                ctx.builder
                    .build_indirect_call(fn_ty, callee_ptr, &arg_values, "")
                    .unwrap();
            } else {
                panic!("Call target must be a function pointer");
            }
            ctx.builder.build_unreachable().unwrap();
        }
        mir::Terminator::CallReturn {
            callee,
            args,
            destination,
            target,
        } => {
            let callee_val = gen_operand(ctx, callee);
            let llvm_args: Vec<BasicValueEnum<'ctx>> = args.iter().map(|a| gen_operand(ctx, a)).collect();

            let call_result = if callee_val.is_pointer_value() {
                let callee_ptr = callee_val.into_pointer_value();
                let arg_types: Vec<BasicMetadataTypeEnum<'ctx>> =
                    llvm_args.iter().map(|v| v.get_type().into()).collect();
                let dest_ty = get_place_type_for_load(ctx, destination);
                let llvm_dest_ty = gen_ty(&dest_ty, &mut ctx.ty_ctx());
                let fn_ty = llvm_dest_ty.fn_type(&arg_types, false);
                let arg_values: Vec<BasicMetadataValueEnum<'ctx>> = llvm_args.iter().map(|v| (*v).into()).collect();
                ctx.builder
                    .build_indirect_call(fn_ty, callee_ptr, &arg_values, "call")
                    .unwrap()
            } else {
                panic!("Call target must be a function pointer");
            };

            // Store result to destination
            let dest_ptr = gen_place(ctx, destination);
            if call_result.try_as_basic_value().is_left() {
                let result_val = call_result.try_as_basic_value().left().unwrap();
                ctx.builder.build_store(dest_ptr, result_val).unwrap();
            }

            let target_bb = ctx.get_llvm_block(target);
            ctx.builder.build_unconditional_branch(target_bb).unwrap();
        }
        mir::Terminator::Resume => {
            ctx.builder.build_unreachable().unwrap();
        }
        mir::Terminator::Abort => {
            ctx.builder.build_unreachable().unwrap();
        }
    }
}

// ─────────────────────────────────────────────────────────────
// Function codegen
// ─────────────────────────────────────────────────────────────

/// Generate LLVM IR for a single MIR function.
fn gen_function<'ctx>(ctx: &mut CodegenCtx<'ctx, '_>, llvm_function: FunctionValue<'ctx>) {
    // Create entry block
    let entry = ctx.llvm.append_basic_block(llvm_function, "entry");
    ctx.position_at_end(entry);

    // Allocate locals (SSA registers become allocas)
    for (i, local_decl) in ctx.mir_func.locals.iter().enumerate() {
        let llvm_ty = gen_ty(&*local_decl.ty, &mut ctx.ty_ctx());
        let alloca = ctx.builder.build_alloca(llvm_ty, &format!("local_{}", i)).unwrap();
        ctx.locals.insert(i as u32, (alloca, llvm_ty));
    }

    // Map parameters to their allocas
    for (i, param_id) in ctx.mir_func.params.iter().enumerate() {
        let param_idx = param_id.as_usize() as u32;
        if let Some(llvm_param) = llvm_function.get_nth_param(i as u32) {
            if let Some((alloca, _)) = ctx.locals.get(&param_idx).copied() {
                ctx.builder.build_store(alloca, llvm_param).unwrap();
            }
        }
    }

    // Create all basic blocks first (to allow forward references)
    for bb_id in ctx.mir_func.blocks.iter() {
        ctx.get_llvm_block(bb_id);
    }

    // Branch from entry to the first basic block
    let entry_target = ctx.get_llvm_block(&ctx.mir_func.entry_block);
    ctx.builder.build_unconditional_branch(entry_target).unwrap();

    // Generate code for each basic block
    for bb_id in ctx.mir_func.blocks.iter() {
        let bb_data = bb_id.borrow();
        let llvm_bb = ctx.get_llvm_block(bb_id);
        ctx.position_at_end(llvm_bb);

        for stmt in bb_data.statements.iter() {
            gen_statement(ctx, stmt);
        }
        gen_terminator(ctx, &bb_data.terminator);
    }
}

// ─────────────────────────────────────────────────────────────
// Module-level entry point
// ─────────────────────────────────────────────────────────────

/// Generate LLVM IR module from a MIR module.
///
/// This function translates the MIR representation into LLVM IR using the
/// Inkwell library. It handles:
/// - Function declarations and definitions
/// - Type generation (MirType → LLVM type)
/// - Basic block codegen (statements + terminators)
/// - Place/Operand/Rvalue evaluation
///
/// The conversion is straightforward because MIR basic blocks map directly
/// to LLVM basic blocks, and MIR terminators map directly to LLVM branch
/// instructions.
pub fn generate_llvmir_from_mir<'ctx>(
    package_name: &str,
    mir_module: &mir::MirModule,
    mir_store: &mir::MirStore,
    llvm: &'ctx LLVMContext,
) -> Module<'ctx> {
    let module = llvm.create_module(package_name);
    let mut globals: HashMap<NString, (PointerValue<'ctx>, BasicTypeEnum<'ctx>)> = HashMap::new();

    // First pass: declare all global variables
    for (global_name, global_ty) in mir_module.globals.iter() {
        let llvm_ty = mir::using_storage(mir_store, || {
            gen_ty(global_ty, &mut TypegenCtx { llvm, module: &module })
        });
        let global = module.add_global(llvm_ty, None, global_name);
        global.set_initializer(&llvm_ty.const_zero());
        global.set_linkage(Linkage::External);
        globals.insert(global_name.clone(), (global.as_pointer_value(), llvm_ty));
    }

    // Generate each function
    for func_id in mir_module.functions.iter() {
        mir::using_storage(mir_store, || {
            let mir_func_borrowed = func_id.borrow();

            if mir_func_borrowed.is_extern() {
                // For extern functions, just declare
                let return_ty = gen_ty(&mir_func_borrowed.return_ty, &mut TypegenCtx { llvm, module: &module });
                let param_tys: Vec<BasicMetadataTypeEnum<'ctx>> = mir_func_borrowed
                    .params
                    .iter()
                    .map(|pid| {
                        let local = pid.borrow();
                        gen_ty(&local.ty, &mut TypegenCtx { llvm, module: &module }).into()
                    })
                    .collect();
                let fn_ty = return_ty.fn_type(&param_tys, false);
                module.add_function(&mir_func_borrowed.name, fn_ty, Some(Linkage::External));
            } else {
                // For functions with bodies, declare and define
                let return_ty = gen_ty(&mir_func_borrowed.return_ty, &mut TypegenCtx { llvm, module: &module });
                let param_tys: Vec<BasicMetadataTypeEnum<'ctx>> = mir_func_borrowed
                    .params
                    .iter()
                    .map(|pid| {
                        let local = pid.borrow();
                        gen_ty(&local.ty, &mut TypegenCtx { llvm, module: &module }).into()
                    })
                    .collect();
                let fn_ty = return_ty.fn_type(&param_tys, false);
                let llvm_function = module.add_function(&mir_func_borrowed.name, fn_ty, Some(Linkage::External));

                let builder = llvm.create_builder();
                let mut ctx = CodegenCtx::new(llvm, &module, builder, &mir_func_borrowed, &globals, llvm_function);
                gen_function(&mut ctx, llvm_function);
            }
        });
    }

    // Verify the module
    if let Err(e) = module.verify() {
        eprintln!("LLVM Module Verification Error: {}", e.to_string());
        eprintln!("Generated LLVM Module:\n");
        eprintln!("{}", module.print_to_string().to_string());
        panic!("Generated LLVM module is invalid");
    }

    module
}
