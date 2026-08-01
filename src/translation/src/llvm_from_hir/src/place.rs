use crate::{
    rvalue::{CodegenCtx, gen_rval},
    ty::gen_ty,
};

use core::panic;
use inkwell::values::PointerValue;
use nitrate_hir::{StructMemoryLayoutCell, prelude as hir};
use nitrate_hir_get_type::HirGetType;
use nitrate_nstring::NString;
use std::ops::Deref;

/// GEP helper with consistent naming — computes `ptr + indices` where the
/// first index operates on the outermost aggregate.
fn build_gep<'ctx>(
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    base: PointerValue<'ctx>,
    pointee_ty: inkwell::types::BasicTypeEnum<'ctx>,
    indices: &[inkwell::values::IntValue<'ctx>],
    name: &str,
) -> PointerValue<'ctx> {
    unsafe {
        // SAFETY: Indices are validated to be within the bounds of the
        // aggregate by the HIR validator. `build_in_bounds_gep` requires
        // that the resulting pointer stay within the allocated object.
        ctx.bb.build_in_bounds_gep(pointee_ty, base, indices, name)
    }
    .unwrap()
}

fn gen_place_field_access<'ctx>(
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    struct_value: &hir::Value,
    field_name: &NString,
) -> PointerValue<'ctx> {
    // Resolve through references to find the actual struct type.
    let value_type = struct_value.determine_type(ctx.tab).expect("Failed to get type");
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

    let llvm_struct_ty = gen_ty(&actual_type, &mut ctx.into());

    // Compute a pointer to the struct:
    // - If the value is itself a reference/pointer, evaluating it as an
    //   rvalue yields the struct address directly — GEP on that.
    // - Otherwise the value is a place — GEP on its address.
    let llvm_struct_ptr = if matches!(&value_type, hir::Type::Reference { .. } | hir::Type::Pointer { .. }) {
        let ref_value = gen_rval(ctx, struct_value);
        debug_assert!(
            ref_value.get_type().is_pointer_type(),
            "reference/pointer rvalue must be a pointer"
        );
        ref_value.into_pointer_value()
    } else {
        gen_place(ctx, struct_value)
    };

    let zero = ctx.llvm.i32_type().const_zero();
    let index = ctx.llvm.i32_type().const_int(field_index as u64, false);
    build_gep(ctx, llvm_struct_ptr, llvm_struct_ty, &[zero, index], "field_access_gep")
}

/// Compute the address of the underlying data pointer for a slice stored at
/// `slice_ptr` (a pointer to a `{ptr, len}` fat pointer). Returns the loaded
/// data pointer — the base of the contiguous element storage.
fn gen_slice_data_ptr<'ctx>(
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    slice_ptr: PointerValue<'ctx>,
    slice_ty: inkwell::types::BasicTypeEnum<'ctx>,
) -> PointerValue<'ctx> {
    let ptr_ty = ctx.llvm.ptr_type(inkwell::AddressSpace::default());
    let zero = ctx.llvm.i32_type().const_zero();
    let data_ptr_field = build_gep(ctx, slice_ptr, slice_ty, &[zero, zero], "slice_data_ptr_gep");
    ctx.bb
        .build_load(ptr_ty, data_ptr_field, "slice_data_ptr_load")
        .unwrap()
        .into_pointer_value()
}

fn gen_place_index_access<'ctx>(
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    collection: &hir::Value,
    index: &hir::Value,
) -> PointerValue<'ctx> {
    let collection_type = collection
        .determine_type(ctx.tab)
        .expect("Failed to get collection type");

    // Generate an integer index value.
    let index_val = gen_rval(ctx, index);
    let index_int = if index_val.is_int_value() {
        index_val.into_int_value()
    } else {
        panic!("Index must be an integer");
    };

    let actual_type = match &collection_type {
        hir::Type::Reference { to, .. } | hir::Type::Pointer { to, .. } => to.deref().clone(),
        _ => collection_type.clone(),
    };

    match &actual_type {
        hir::Type::Array { .. } => {
            // `collection` may be behind a reference: use its address directly.
            let collection_ptr = if matches!(
                &collection_type,
                hir::Type::Reference { .. } | hir::Type::Pointer { .. }
            ) {
                let ref_value = gen_rval(ctx, collection);
                ref_value.into_pointer_value()
            } else {
                gen_place(ctx, collection)
            };

            let llvm_collection_ty = gen_ty(&actual_type, &mut ctx.into());
            let zero = ctx.llvm.i32_type().const_zero();
            build_gep(
                ctx,
                collection_ptr,
                llvm_collection_ty,
                &[zero, index_int],
                "index_access_gep",
            )
        }

        hir::Type::SliceRef { element_type, .. } | hir::Type::SlicePtr { element_type, .. } => {
            // A slice is a fat pointer `{data_ptr, len}`. Indexing must:
            //   1. Obtain a pointer to the fat pointer.
            //   2. Extract the data pointer field.
            //   3. GEP the *element* pointer by `index`.
            let slice_ptr = if matches!(
                &collection_type,
                hir::Type::Reference { .. } | hir::Type::Pointer { .. }
            ) {
                // Reference-to-slice: loading the reference gives the fat pointer value.
                // We need its address, so store it in a temporary alloca.
                let slice_value = gen_rval(ctx, collection);
                let llvm_slice_ty = gen_ty(&actual_type, &mut ctx.into());
                let tmp = ctx.bb.build_alloca(llvm_slice_ty, "slice_tmp").unwrap();
                ctx.bb.build_store(tmp, slice_value).unwrap();
                tmp
            } else {
                gen_place(ctx, collection)
            };

            let llvm_slice_ty = gen_ty(&actual_type, &mut ctx.into());
            let data_ptr = gen_slice_data_ptr(ctx, slice_ptr, llvm_slice_ty);

            // GEP on the element pointer with the single index.
            let llvm_elem_ty = gen_ty(element_type, &mut ctx.into());
            build_gep(ctx, data_ptr, llvm_elem_ty, &[index_int], "slice_index_gep")
        }

        _ => {
            // For trait-based `Index` resolution we would call the `index`
            // method. That path is not yet fully supported; produce a
            // temporary place containing the result of the method call.
            let llvm_element_ty = gen_ty(&collection_type, &mut ctx.into());
            let alloca = ctx.bb.build_alloca(llvm_element_ty, "index_result_place").unwrap();
            let rv = gen_rval(ctx, collection);
            ctx.bb.build_store(alloca, rv).unwrap();
            alloca
        }
    }
}

/// Compute the *place* (memory address) denoted by a dereference.
///
/// For `*ptr` where `ptr: *const T` or `ptr: &T`, the pointee's address IS
/// the pointer value itself. Previously this function loaded the pointee into
/// a fresh alloca, producing a *copy* rather than a reference into the
/// original storage — breaking borrows like `&arr[i]`.
fn gen_place_deref<'ctx>(ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>, place: &hir::Value) -> PointerValue<'ctx> {
    let llvm_value = gen_rval(ctx, place);

    if !llvm_value.get_type().is_pointer_type() {
        panic!("Cannot dereference non-pointer type");
    }

    // The dereferenced place is exactly the pointer value.
    llvm_value.into_pointer_value()
}

pub(crate) fn gen_place<'ctx>(
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    hir_value: &hir::Value,
) -> PointerValue<'ctx> {
    match hir_value {
        hir::Value::Range { .. } => panic!("Value is not a place"),
        hir::Value::InferredInteger { .. }
        | hir::Value::InferredFloat { .. }
        | hir::Value::Assign { .. }
        | hir::Value::Borrow { .. }
        | hir::Value::If { .. }
        | hir::Value::While { .. }
        | hir::Value::Loop { .. }
        | hir::Value::Break { .. }
        | hir::Value::Continue { .. }
        | hir::Value::Return { .. } => panic!("Value is not a place"),

        // Non-place rvalues: materialize into a temporary alloca so they can
        // be addressed. This is Rust's "rvalue materialization" semantics.
        hir::Value::Unit { .. }
        | hir::Value::Bool { .. }
        | hir::Value::I8 { .. }
        | hir::Value::I16 { .. }
        | hir::Value::I32 { .. }
        | hir::Value::I64 { .. }
        | hir::Value::I128 { .. }
        | hir::Value::U8 { .. }
        | hir::Value::U16 { .. }
        | hir::Value::U32 { .. }
        | hir::Value::U64 { .. }
        | hir::Value::U128 { .. }
        | hir::Value::F32 { .. }
        | hir::Value::F64 { .. }
        | hir::Value::USize { .. }
        | hir::Value::StringLit { .. }
        | hir::Value::BStringLit { .. }
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
                &hir_value.determine_type(ctx.tab).expect("unable to get value type"),
                &mut ctx.into(),
            );

            let alloca = ctx.bb.build_alloca(tmp_ty, "tmp").unwrap();
            let llvm_value = gen_rval(ctx, hir_value);
            ctx.bb.build_store(alloca, llvm_value).unwrap();
            alloca
        }

        hir::Value::FieldAccess { expr, field_name, .. } => gen_place_field_access(ctx, &expr.borrow(), field_name),

        hir::Value::IndexAccess { collection, index, .. } => {
            gen_place_index_access(ctx, &collection.borrow(), &index.borrow())
        }

        hir::Value::Deref { place, .. } => gen_place_deref(ctx, &place.borrow()),

        hir::Value::FunctionSymbol { id, .. } => {
            match ctx.module.get_function(id.borrow().mangled_name.as_ref().unwrap()) {
                Some(func) => func.as_global_value().as_pointer_value(),
                None => panic!("Function symbol not found in module"),
            }
        }

        hir::Value::GlobalVariableSymbol { id, .. } => {
            match ctx.globals.get(id.borrow().mangled_name.as_ref().unwrap()) {
                Some(ptr) => ptr.0,
                None => panic!("Global variable symbol not found"),
            }
        }

        hir::Value::LocalVariableSymbol { id, .. } => match ctx.locals.get(&id.borrow().name) {
            Some(ptr) => ptr.0,
            None => panic!("Local variable symbol not found"),
        },

        hir::Value::ParameterSymbol { id, .. } => match ctx.parameters.get(&id.borrow().name) {
            Some(ptr) => ptr.0,
            None => panic!("Parameter symbol not found"),
        },
    }
}
