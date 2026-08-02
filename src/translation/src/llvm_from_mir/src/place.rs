use core::panic;

use crate::context::CodegenCtx;
use crate::ty::gen_ty;
use inkwell::values::PointerValue;
use nitrate_mir::prelude as mir;

/// Compute the address (PointerValue) of a MIR Place.
pub fn gen_place<'ctx>(ctx: &mut CodegenCtx<'ctx, '_>, place: &mir::Place) -> PointerValue<'ctx> {
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
            let ptr_val = crate::operand::gen_operand(ctx, &place_op);
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
            let idx_val = crate::operand::gen_operand(ctx, &idx_op);
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
        mir::Place::Downcast { base, variant_name: _ } => gen_place(ctx, base),
    }
}

/// Determine the MirType for a place so we know what LLVM type to load.
pub fn get_place_type_for_load<'ctx>(ctx: &CodegenCtx<'ctx, '_>, place: &mir::Place) -> mir::MirTypeId {
    match place {
        mir::Place::Local(local_id) => {
            let idx = local_id.as_usize() as u32;
            if (idx as usize) < ctx.mir_func.locals.len() {
                ctx.mir_func.locals[idx as usize].ty.clone()
            } else {
                panic!("local index out of bounds: {}", idx);
            }
        }
        mir::Place::Static(_name) => mir::MirType::Unit.into(),
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
