use crate::{
    rvalue::{CodegenCtx, gen_rval},
    ty::gen_ty,
};

use inkwell::values::PointerValue;
use nitrate_hir::{StructMemoryLayoutCell, prelude as hir};
use nitrate_hir_get_type::HirGetType;
use nitrate_nstring::NString;
use std::ops::Deref;

fn gen_place_field_access<'ctx>(
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    struct_value: &hir::Value,
    field_name: &NString,
) -> PointerValue<'ctx> {
    // If the value is behind a reference/pointer, first dereference it
    let resolved_value = if let hir::Type::Reference { to, .. } | hir::Type::Pointer { to, .. } =
        struct_value.determine_type(ctx.tab).expect("Failed to get type")
    {
        // Dereference the reference to get the underlying struct
        let deref_value = hir::Value::Deref {
            place: struct_value.clone().into(),
        };
        gen_place(ctx, &deref_value)
    } else {
        gen_place(ctx, struct_value)
    };

    let value_type = struct_value.determine_type(ctx.tab).expect("Failed to get type");
    // Resolve through references to find the actual struct type
    let actual_type = match &value_type {
        hir::Type::Reference { to, .. } | hir::Type::Pointer { to, .. } => to.deref().clone(),
        _ => value_type.clone(),
    };
    let hir_struct_def = actual_type.as_struct().expect("expected struct type").borrow();

    let field_index = hir_struct_def
        .layout
        .iter()
        .position(|cell| {
            cell == &StructMemoryLayoutCell::Field {
                field_name: field_name.clone(),
            }
        })
        .expect("Field not found in struct");

    let llvm_struct_value = resolved_value;
    let llvm_struct_ty = gen_ty(&actual_type, &mut ctx.into());

    let index = ctx.llvm.i32_type().const_int(field_index as u64, false);

    let gep = unsafe {
        // SAFETY: ** I don't know if this is safe or not
        ctx.bb.build_in_bounds_gep(
            llvm_struct_ty,
            llvm_struct_value,
            &[ctx.llvm.i32_type().const_int(0, false), index],
            "field_access_gep",
        )
    }
    .unwrap();

    gep
}

fn gen_place_deref<'ctx>(ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>, place: &hir::Value) -> PointerValue<'ctx> {
    let llvm_value = gen_rval(ctx, place);
    let ptr_ty = llvm_value.get_type();

    if !ptr_ty.is_pointer_type() {
        panic!("Cannot dereference non-pointer type");
    }

    let pointee_ty = match place.determine_type(ctx.tab).unwrap() {
        hir::Type::Pointer { to, .. } => to.deref().clone(),
        hir::Type::Reference { to, .. } => to.deref().clone(),
        _ => unreachable!(),
    };

    let llvm_pointee_ty = gen_ty(&pointee_ty, &mut ctx.into());

    // Load the struct value from the pointer
    let loaded_value = ctx
        .bb
        .build_load(llvm_pointee_ty, llvm_value.into_pointer_value(), "deref_load")
        .unwrap();

    // Store it into a temporary alloca so we have a pointer to the struct
    let alloca = ctx.bb.build_alloca(llvm_pointee_ty, "derefed").unwrap();
    ctx.bb.build_store(alloca, loaded_value).unwrap();
    alloca
}

pub(crate) fn gen_place<'ctx>(
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    hir_value: &hir::Value,
) -> PointerValue<'ctx> {
    match hir_value {
        hir::Value::InferredInteger(_)
        | hir::Value::InferredFloat(_)
        | hir::Value::Assign { .. }
        | hir::Value::Borrow { .. }
        | hir::Value::If { .. }
        | hir::Value::While { .. }
        | hir::Value::Loop { .. }
        | hir::Value::Break { .. }
        | hir::Value::Continue { .. }
        | hir::Value::Return { .. } => panic!("Value is not a place"),

        hir::Value::Unit
        | hir::Value::Bool(_)
        | hir::Value::I8(_)
        | hir::Value::I16(_)
        | hir::Value::I32(_)
        | hir::Value::I64(_)
        | hir::Value::I128(_)
        | hir::Value::U8(_)
        | hir::Value::U16(_)
        | hir::Value::U32(_)
        | hir::Value::U64(_)
        | hir::Value::U128(_)
        | hir::Value::F32(_)
        | hir::Value::F64(_)
        | hir::Value::USize32(_)
        | hir::Value::USize64(_)
        | hir::Value::StringLit(_)
        | hir::Value::BStringLit(_)
        | hir::Value::List { .. }
        | hir::Value::Binary { .. }
        | hir::Value::Unary { .. }
        | hir::Value::Tuple { .. }
        | hir::Value::Block { .. }
        | hir::Value::Call { .. }
        | hir::Value::MethodCall { .. }
        | hir::Value::EnumVariant { .. }
        | hir::Value::StructObject { .. }
        | hir::Value::Cast { .. } => {
            let tmp_ty = gen_ty(
                &hir_value.determine_type(ctx.tab).expect("unable to get bool type"),
                &mut ctx.into(),
            );

            let alloca = ctx.bb.build_alloca(tmp_ty, "").unwrap();
            let llvm_value = gen_rval(ctx, hir_value);
            ctx.bb.build_store(alloca, llvm_value).unwrap();
            alloca
        }

        hir::Value::FieldAccess { expr, field_name } => gen_place_field_access(ctx, &expr.borrow(), field_name),

        hir::Value::Deref { place } => gen_place_deref(ctx, &place.borrow()),

        hir::Value::FunctionSymbol { id } => match ctx.module.get_function(&id.borrow().mangled_name) {
            Some(func) => func.as_global_value().as_pointer_value(),
            None => panic!("Function symbol not found in module"),
        },

        hir::Value::GlobalVariableSymbol { id } => match ctx.globals.get(&id.borrow().mangled_name) {
            Some(ptr) => ptr.0,
            None => panic!("Global variable symbol not found"),
        },

        hir::Value::LocalVariableSymbol { id } => match ctx.locals.get(&id.borrow().name) {
            Some(ptr) => ptr.0,
            None => panic!("Local variable symbol not found"),
        },

        hir::Value::ParameterSymbol { id } => match ctx.parameters.get(&id.borrow().name) {
            Some(ptr) => ptr.0,
            None => panic!("Parameter symbol not found"),
        },
    }
}
