use crate::{
    rvalue::{CodegenCtx, gen_rval},
    ty::gen_ty,
};

use core::panic;
use inkwell::values::PointerValue;
use nitrate_hir::{StructMemoryLayoutCell, prelude as hir};
use nitrate_hir_get_type::HirGetType;
use nitrate_nstring::NString;
use nitrate_tree::ByteSpan;
use std::ops::Deref;

fn gen_place_field_access<'ctx>(
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    struct_value: &hir::Value,
    field_name: &NString,
) -> PointerValue<'ctx> {
    // If the value is behind a reference/pointer, first dereference it
    let resolved_value = if let hir::Type::Reference { to: _, .. } | hir::Type::Pointer { to: _, .. } =
        struct_value.determine_type(ctx.tab).expect("Failed to get type")
    {
        // Dereference the reference to get the underlying struct
        let deref_value = hir::Value::Deref {
            span: ByteSpan::default(),
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

    unsafe {
        // SAFETY: ** I don't know if this is safe or not
        ctx.bb.build_in_bounds_gep(
            llvm_struct_ty,
            llvm_struct_value,
            &[ctx.llvm.i32_type().const_int(0, false), index],
            "field_access_gep",
        )
    }
    .unwrap()
}

fn gen_place_index_access<'ctx>(
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    collection: &hir::Value,
    index: &hir::Value,
) -> PointerValue<'ctx> {
    // Get the collection type to determine if it's an array, slice, or trait-based
    let collection_type = collection
        .determine_type(ctx.tab)
        .expect("Failed to get collection type");

    // First, get a pointer to the collection (the place)
    let collection_ptr = if let hir::Type::Reference { .. } | hir::Type::Pointer { .. } = &collection_type {
        // Auto-deref to get the underlying collection
        let deref_value = hir::Value::Deref {
            span: ByteSpan::default(),
            place: collection.clone().into(),
        };
        gen_place(ctx, &deref_value)
    } else {
        gen_place(ctx, collection)
    };

    // Get the actual type after deref
    let actual_type = match &collection_type {
        hir::Type::Reference { to, .. } | hir::Type::Pointer { to, .. } => to.deref().clone(),
        _ => collection_type.clone(),
    };

    match &actual_type {
        hir::Type::Array {
            element_type: _element_type,
            ..
        }
        | hir::Type::SliceRef {
            element_type: _element_type,
            ..
        }
        | hir::Type::SlicePtr {
            element_type: _element_type,
            ..
        } => {
            let llvm_collection_ty = gen_ty(&actual_type, &mut ctx.into());

            let index_val = gen_rval(ctx, index);
            let index_int = if index_val.is_int_value() {
                index_val.into_int_value()
            } else {
                panic!("Index must be an integer");
            };

            unsafe {
                // SAFETY: Array/Slice indexing via GEP
                ctx.bb.build_in_bounds_gep(
                    llvm_collection_ty,
                    collection_ptr,
                    &[ctx.llvm.i32_type().const_int(0, false), index_int],
                    "index_access_gep",
                )
            }
            .unwrap()
        }
        _ => {
            // For trait-based Index resolution, we need to call the `index` method
            // and return a pointer to the result.
            // For now, we treat it as a rvalue (load the result via method call)
            // and store it in a temporary alloca to make it a place.
            let llvm_element_ty = gen_ty(&collection_type, &mut ctx.into());
            let alloca = ctx.bb.build_alloca(llvm_element_ty, "index_result_place").unwrap();
            let rv = gen_rval(ctx, collection); // For traits, we just get the rvalue
            ctx.bb.build_store(alloca, rv).unwrap();
            alloca
        }
    }
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
                &hir_value.determine_type(ctx.tab).expect("unable to get bool type"),
                &mut ctx.into(),
            );

            let alloca = ctx.bb.build_alloca(tmp_ty, "").unwrap();
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
