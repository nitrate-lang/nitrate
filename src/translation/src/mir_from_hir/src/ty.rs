use core::panic;
use nitrate_hir::prelude as hir;
use nitrate_mir::prelude as mir;
use nitrate_nstring::NString;
use std::ops::Deref;

/// Lower an HIR `Type` (fully resolved, no inference/generics) into a MIR `MirTypeId`.
pub fn lower_type(hir_ty: &hir::Type, store: &mir::MirStore) -> mir::MirTypeId {
    let mir_ty = match hir_ty {
        hir::Type::Never { .. } => mir::MirType::Never,
        hir::Type::Unit { .. } => mir::MirType::Unit,
        hir::Type::Bool { .. } => mir::MirType::Bool,
        hir::Type::U8 { .. } => mir::MirType::U8,
        hir::Type::U16 { .. } => mir::MirType::U16,
        hir::Type::U32 { .. } => mir::MirType::U32,
        hir::Type::U64 { .. } => mir::MirType::U64,
        hir::Type::U128 { .. } => mir::MirType::U128,
        hir::Type::USize { .. } => mir::MirType::USize,
        hir::Type::I8 { .. } => mir::MirType::I8,
        hir::Type::I16 { .. } => mir::MirType::I16,
        hir::Type::I32 { .. } => mir::MirType::I32,
        hir::Type::I64 { .. } => mir::MirType::I64,
        hir::Type::I128 { .. } => mir::MirType::I128,
        hir::Type::F32 { .. } => mir::MirType::F32,
        hir::Type::F64 { .. } => mir::MirType::F64,
        hir::Type::Range { .. } => mir::MirType::Range,

        hir::Type::Array { element_type, len, .. } => {
            let elem = lower_type(element_type, store);
            mir::MirType::Array {
                element_type: elem,
                len: *len,
            }
        }

        hir::Type::Tuple { element_types, .. } => {
            let lowered: thin_vec::ThinVec<mir::MirTypeId> =
                element_types.iter().map(|t| lower_type(t, store)).collect();
            mir::MirType::Tuple { element_types: lowered }
        }

        hir::Type::Struct { def, .. } => {
            let struct_def = def.borrow();
            let fields: thin_vec::ThinVec<(NString, mir::MirTypeId)> = struct_def
                .fields
                .iter()
                .map(|(name, field)| {
                    let mir_field_ty = lower_type(&field.ty, store);
                    (name.clone(), mir_field_ty)
                })
                .collect();

            let layout: mir::MirStructLayout = struct_def
                .layout
                .iter()
                .map(|cell| match cell {
                    hir::StructMemoryLayoutCell::Field { field_name } => mir::MirStructLayoutCell::Field {
                        field_name: field_name.clone(),
                    },
                    hir::StructMemoryLayoutCell::Padding(size) => mir::MirStructLayoutCell::Padding(
                        std::num::NonZeroU32::new(size.get() as u32).expect("padding size must be nonzero"),
                    ),
                })
                .collect();

            mir::MirType::Struct {
                name: struct_def.name.clone(),
                fields,
                layout,
            }
        }

        hir::Type::Enum { def, .. } => {
            let enum_def = def.borrow();
            let variants: thin_vec::ThinVec<mir::MirEnumVariant> = enum_def
                .variants
                .iter()
                .map(|v| {
                    let payload = if matches!(v.ty.deref(), hir::Type::Unit { .. }) {
                        None
                    } else {
                        Some(lower_type(&v.ty, store))
                    };
                    mir::MirEnumVariant {
                        name: v.name.clone(),
                        payload,
                    }
                })
                .collect();
            mir::MirType::Enum {
                name: enum_def.name.clone(),
                variants,
            }
        }

        hir::Type::Reference {
            lifetime: _,
            exclusive,
            mutable,
            to,
            ..
        } => {
            let to_id = lower_type(to, store);
            mir::MirType::Reference {
                exclusive: *exclusive,
                mutable: *mutable,
                to: to_id,
            }
        }

        hir::Type::SliceRef {
            lifetime: _,
            exclusive,
            mutable,
            element_type,
            ..
        } => {
            let elem_id = lower_type(element_type, store);
            mir::MirType::SliceRef {
                exclusive: *exclusive,
                mutable: *mutable,
                element_type: elem_id,
            }
        }

        hir::Type::Pointer {
            lifetime: _,
            exclusive,
            mutable,
            to,
            ..
        } => {
            let to_id = lower_type(to, store);
            mir::MirType::Pointer {
                exclusive: *exclusive,
                mutable: *mutable,
                to: to_id,
            }
        }

        hir::Type::SlicePtr {
            lifetime: _,
            exclusive,
            mutable,
            element_type,
            ..
        } => {
            let elem_id = lower_type(element_type, store);
            mir::MirType::SlicePtr {
                exclusive: *exclusive,
                mutable: *mutable,
                element_type: elem_id,
            }
        }

        hir::Type::Function { function_type, .. } => {
            let params: thin_vec::ThinVec<(NString, mir::MirTypeId)> = function_type
                .params
                .iter()
                .map(|(name, ty)| (name.clone(), lower_type(ty, store)))
                .collect();
            let ret = lower_type(&function_type.return_type, store);
            let is_c_variadic = function_type.attributes.contains(&hir::FunctionAttribute::CVariadic);
            mir::MirType::Function {
                params,
                return_type: ret,
                is_c_variadic,
            }
        }

        hir::Type::TypeAlias { def, .. } => {
            let resolved = def.borrow().type_id.deref().clone();
            return lower_type(&resolved, store);
        }

        hir::Type::Parameterized { base, .. } => {
            return lower_type(base, store);
        }

        hir::Type::Refine { base, .. } => {
            return lower_type(base, store);
        }

        hir::Type::Inferred { .. }
        | hir::Type::InferredFloat { .. }
        | hir::Type::InferredInteger { .. }
        | hir::Type::GenericParam { .. } => {
            panic!(
                "Unresolved HIR type encountered in MIR lowering: {:?}. \
                 The solver should have eliminated all inferred and generic types.",
                hir_ty
            );
        }

        hir::Type::TraitObject { .. } => mir::MirType::USize,
    };

    store.store_type(mir_ty)
}
