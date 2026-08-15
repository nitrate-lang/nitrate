use crate::context::CodegenCtx;
use crate::ty::gen_ty;
use core::panic;
use inkwell::values::PointerValue;
use nitrate_mir::prelude as mir;

/// Compute the address (PointerValue) of a MIR Place.
///
/// This always returns a pointer to the place's storage and never loads the
/// place's value, preserving aliasing (see LLVM_CODEGEN.md "Place Semantics").
pub fn gen_place<'ctx>(ctx: &mut CodegenCtx<'ctx, '_>, place: &mir::Place) -> PointerValue<'ctx> {
    match place {
        mir::Place::Local(local_id) => {
            let idx = local_id.as_usize() as u32;
            ctx.locals
                .get(&idx)
                .unwrap_or_else(|| panic!("local {} not found in codegen context", idx))
                .0
        }
        mir::Place::Static(name) => {
            ctx.globals
                .get(name)
                .unwrap_or_else(|| panic!("global '{}' not found", name))
                .ptr
        }
        mir::Place::Deref(base) => {
            // The address of `*base` is the pointer value stored at `base`.
            // Load that pointer from base's storage — no copy of the pointee.
            let base_ptr = gen_place(ctx, base);
            let base_ty = get_place_type_for_load(ctx, base);
            let llvm_ty = gen_ty(&base_ty, &mut ctx.ty_ctx());
            let ptr_val = ctx
                .builder
                .build_load(llvm_ty, base_ptr, "deref_addr")
                .expect("failed to build deref load");
            if !ptr_val.is_pointer_value() {
                panic!("Cannot dereference non-pointer type {:?}", &*base_ty);
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
                    .unwrap_or_else(|| panic!("field '{}' not found in struct layout", field_name));
                let struct_ty = gen_ty(&base_ty, &mut ctx.ty_ctx());
                unsafe {
                    ctx.builder
                        .build_in_bounds_gep(
                            struct_ty,
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
                panic!("Field access on non-struct type {:?}", &*base_ty);
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
            match &*base_ty {
                mir::MirType::Array { element_type, .. } => {
                    let elem_ty = gen_ty(&*element_type, &mut ctx.ty_ctx());
                    unsafe {
                        ctx.builder
                            .build_in_bounds_gep(elem_ty, base_ptr, &[idx_int], "index_gep")
                            .unwrap()
                    }
                }
                mir::MirType::SliceRef { element_type, .. } | mir::MirType::SlicePtr { element_type, .. } => {
                    // Fat pointer `{ data_ptr, len }`. GEP field 0 to load the
                    // data pointer, then index the backing elements.
                    let slice_ty = gen_ty(&base_ty, &mut ctx.ty_ctx());
                    let zero = ctx.llvm.i32_type().const_zero();
                    let data_gep = unsafe {
                        ctx.builder
                            .build_in_bounds_gep(slice_ty, base_ptr, &[zero, zero], "slice_data_ptr")
                            .unwrap()
                    };
                    let data_ptr = ctx
                        .builder
                        .build_load(
                            ctx.llvm.ptr_type(inkwell::AddressSpace::default()),
                            data_gep,
                            "slice_data_load",
                        )
                        .unwrap()
                        .into_pointer_value();
                    let elem_ty = gen_ty(&*element_type, &mut ctx.ty_ctx());
                    unsafe {
                        ctx.builder
                            .build_in_bounds_gep(elem_ty, data_ptr, &[idx_int], "index_gep")
                            .unwrap()
                    }
                }
                _ => panic!("Index access requires an array or slice type, got {:?}", &*base_ty),
            }
        }
        // Downcast projects to the payload storage of the selected variant.
        // The enum is `{ [N x i8] payload, tag }`, and the payload lives at
        // field 0. We return a pointer to that payload storage (an opaque
        // pointer), letting the load type be the variant's payload type.
        mir::Place::Downcast { base, variant_name } => {
            let base_ty = get_place_type_for_load(ctx, base);
            if let mir::MirType::Enum { variants, .. } = &*base_ty {
                let variant = variants
                    .iter()
                    .find(|v| v.name == *variant_name)
                    .unwrap_or_else(|| panic!("variant '{}' not found in enum", variant_name));
                if variant.payload.is_none() {
                    panic!("variant '{}' has no payload to downcast to", variant_name);
                }
                let enum_llvm_ty = gen_ty(&base_ty, &mut ctx.ty_ctx());
                let base_ptr = gen_place(ctx, base);
                let zero = ctx.llvm.i32_type().const_zero();
                unsafe {
                    ctx.builder
                        .build_in_bounds_gep(enum_llvm_ty, base_ptr, &[zero, zero], "downcast_payload")
                        .unwrap()
                }
            } else {
                panic!("Downcast on non-enum type {:?}", &*base_ty);
            }
        }
    }
}

/// Determine the MIR type of a place so codegen knows what LLVM type to load.
pub fn get_place_type_for_load(ctx: &CodegenCtx<'_, '_>, place: &mir::Place) -> mir::MirTypeId {
    match place {
        mir::Place::Local(local_id) => local_id.borrow().ty.clone(),
        mir::Place::Static(name) => ctx
            .globals
            .get(name)
            .map(|info| info.mir_ty.clone())
            .unwrap_or_else(|| panic!("global '{}' not found", name)),
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
            if let mir::MirType::Struct { fields, .. } = &*base_ty {
                fields
                    .iter()
                    .find(|(name, _)| name == field_name)
                    .map(|(_, ty)| ty.clone())
                    .unwrap_or_else(|| panic!("field '{}' not found", field_name))
            } else {
                base_ty
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
        mir::Place::Downcast { base, variant_name } => {
            let base_ty = get_place_type_for_load(ctx, base);
            if let mir::MirType::Enum { variants, .. } = &*base_ty {
                variants
                    .iter()
                    .find(|v| v.name == *variant_name)
                    .and_then(|v| v.payload.clone())
                    .unwrap_or_else(|| panic!("variant '{}' has no payload", variant_name))
            } else {
                panic!("Downcast on non-enum type {:?}", &*base_ty);
            }
        }
    }
}
