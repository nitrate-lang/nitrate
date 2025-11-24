use inkwell::values::PointerValue;
use nitrate_hir_get_type::HirGetType;

use crate::{
    rvalue::{CodegenCtx, gen_rval},
    ty::gen_ty,
};
use nitrate_hir::{StructMemoryLayoutCell, prelude as hir};
use nitrate_nstring::NString;

fn gen_place_field_access<'ctx>(
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_, '_>,
    struct_value: &hir::Value,
    field_name: &NString,
) -> PointerValue<'ctx> {
    let hir_struct_def = &ctx.store[struct_value
        .get_type(ctx.store, ctx.tab)
        .expect("Failed to get type")
        .as_struct()
        .expect("expected struct type")]
    .borrow();

    let field_index = hir_struct_def
        .layout
        .iter()
        .position(|cell| {
            cell == &StructMemoryLayoutCell::Field {
                field_name: field_name.clone(),
            }
        })
        .expect("Field not found in struct");

    let llvm_struct_value = gen_place(ctx, struct_value);
    let llvm_struct_ty = gen_ty(
        &struct_value
            .get_type(ctx.store, ctx.tab)
            .expect("unable to get struct type"),
        &mut ctx.into(),
    );

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

fn gen_place_deref<'ctx>(
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_, '_>,
    place: &hir::Value,
) -> PointerValue<'ctx> {
    let llvm_value = gen_rval(ctx, place);
    let ptr_ty = llvm_value.get_type();

    if !ptr_ty.is_pointer_type() {
        panic!("Cannot dereference non-pointer type");
    }

    let pointee_ty = match place.get_type(ctx.store, ctx.tab).unwrap() {
        hir::Type::Pointer { to, .. } => &ctx.store[&to],
        hir::Type::Reference { to, .. } => &ctx.store[&to],
        _ => unreachable!(),
    };

    let load = ctx
        .bb
        .build_load(
            gen_ty(&pointee_ty, &mut ctx.into()),
            llvm_value.into_pointer_value(),
            "deref_load",
        )
        .unwrap();

    match load {
        inkwell::values::BasicValueEnum::PointerValue(ptr) => return ptr,
        _ => panic!("Dereferenced value is not a pointer"),
    }
}

pub(crate) fn gen_place<'ctx>(
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_, '_>,
    hir_value: &hir::Value,
) -> PointerValue<'ctx> {
    match hir_value {
        hir::Value::InferredInteger(_)
        | hir::Value::InferredFloat(_)
        | hir::Value::StructObject { .. }
        | hir::Value::EnumVariant { .. }
        | hir::Value::Binary { .. }
        | hir::Value::Unary { .. }
        | hir::Value::Assign { .. }
        | hir::Value::Cast { .. }
        | hir::Value::Borrow { .. }
        | hir::Value::List { .. }
        | hir::Value::Tuple { .. }
        | hir::Value::If { .. }
        | hir::Value::While { .. }
        | hir::Value::Loop { .. }
        | hir::Value::Break { .. }
        | hir::Value::Continue { .. }
        | hir::Value::Return { .. }
        | hir::Value::Block { .. }
        | hir::Value::Closure { .. }
        | hir::Value::Call { .. }
        | hir::Value::MethodCall { .. } => panic!("Value is not a place"),

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
        | hir::Value::BStringLit(_) => {
            let tmp_ty = gen_ty(
                &hir_value
                    .get_type(ctx.store, ctx.tab)
                    .expect("unable to get bool type"),
                &mut ctx.into(),
            );

            let alloca = ctx.bb.build_alloca(tmp_ty, "").unwrap();
            let llvm_value = gen_rval(ctx, hir_value);
            ctx.bb.build_store(alloca, llvm_value).unwrap();
            alloca
        }

        hir::Value::FieldAccess { expr, field_name } => {
            let expr = ctx.store[expr].borrow();
            gen_place_field_access(ctx, &expr, field_name)
        }

        hir::Value::Deref { place } => {
            let place = ctx.store[place].borrow();
            gen_place_deref(ctx, &place)
        }

        hir::Value::FunctionSymbol { id } => {
            let function = ctx.store[id].borrow();
            match ctx.module.get_function(&function.mangled_name) {
                Some(func) => func.as_global_value().as_pointer_value(),
                None => panic!("Function symbol not found in module"),
            }
        }

        hir::Value::GlobalVariableSymbol { id } => {
            let global_var = ctx.store[id].borrow();
            match ctx.globals.get(&global_var.mangled_name) {
                Some(ptr) => ptr.0,
                None => panic!("Global variable symbol not found"),
            }
        }

        hir::Value::LocalVariableSymbol { id } => {
            let local_var = ctx.store[id].borrow();
            match ctx.locals.get(&local_var.name) {
                Some(ptr) => ptr.0,
                None => panic!("Local variable symbol not found"),
            }
        }

        hir::Value::ParameterSymbol { id } => {
            let parameter = ctx.store[id].borrow();
            match ctx.parameters.get(&parameter.name) {
                Some(ptr) => ptr.0,
                None => panic!("Parameter symbol not found"),
            }
        }
    }
}
