use crate::{get_align_of, get_stride_of};
use nitrate_hir::prelude::*;
use std::cmp::max;

#[derive(Debug, Clone, Copy)]
pub enum LayoutError {
    NotInferred,
    UnresolvedSymbol,
    UninstantiatedGeneric,
}

pub struct LayoutCtx<'a> {
    pub tab: &'a SymbolTab,
    pub ptr_size: PtrSize,
}

pub fn get_size_of(ty: &Type, ctx: &LayoutCtx) -> Result<u64, LayoutError> {
    match ty {
        Type::Never { .. } => Ok(0),
        Type::Unit { .. } => Ok(0),
        Type::Bool { .. } => Ok(1),
        Type::U8 { .. } | Type::I8 { .. } => Ok(1),
        Type::U16 { .. } | Type::I16 { .. } => Ok(2),
        Type::U32 { .. } | Type::I32 { .. } | Type::F32 { .. } => Ok(4),
        Type::U64 { .. } | Type::I64 { .. } | Type::F64 { .. } => Ok(8),
        Type::U128 { .. } | Type::I128 { .. } => Ok(16),
        Type::USize { .. } => Ok(ctx.ptr_size as u64),

        Type::Array { element_type, len, .. } => {
            let element_stride = get_stride_of(element_type, ctx)?;
            Ok(element_stride * u64::from(*len))
        }

        Type::Tuple {
            element_types: elements,
            ..
        } => {
            let mut size = 0_u64;
            for element in elements {
                let element_size = get_size_of(element, ctx)?;
                let element_align = get_align_of(element, ctx)?;
                size = size.next_multiple_of(element_align);
                size += element_size;
            }
            Ok(size)
        }

        Type::Struct { def, .. } => {
            let StructDef { fields, attributes, .. } = &*def.borrow();
            if attributes.contains(&StructAttribute::Packed) {
                let mut total_size = 0_u64;
                for field in fields.values() {
                    total_size += get_size_of(&field.ty, ctx)?;
                }
                return Ok(total_size);
            }
            let mut offset = 0_u64;
            for field in fields.values() {
                let field_size = get_size_of(&field.ty, ctx)?;
                let field_align = get_align_of(&field.ty, ctx)?;
                offset = offset.next_multiple_of(field_align);
                offset += field_size;
            }
            Ok(offset)
        }

        Type::Enum { def, .. } => {
            let EnumDef { variants, .. } = &*def.borrow();
            let mut size = 0_u64;
            for variant in variants {
                let variant_size = get_size_of(&variant.ty, ctx)?;
                size = max(size, variant_size);
            }
            let (discrim_size, discrim_align) = match variants.len() as u64 {
                0..=1 => (0, 1),
                2..=256 => (1, 1),
                257..=65536 => (2, 2),
                65537..=4294967296 => (4, 4),
                4294967297.. => (8, 8),
            };
            size = size.next_multiple_of(discrim_align);
            size += discrim_size;
            Ok(size)
        }

        Type::TypeAlias { def, .. } => {
            let type_alias = &def.borrow().type_id;
            get_size_of(type_alias, ctx)
        }

        Type::Refine { base, .. } => Ok(get_size_of(base, ctx)?),
        Type::UnresolvedArray { .. } => Err(LayoutError::NotInferred),
        Type::UnresolvedRefine { base, .. } => get_size_of(base, ctx),
        Type::Range { .. } => Ok(0),
        Type::Str { .. } => Ok(ctx.ptr_size as u64 * 2),

        Type::Function { .. } => Ok(ctx.ptr_size as u64),
        Type::Reference { .. } => Ok(ctx.ptr_size as u64),
        Type::SliceRef { .. } => Ok(ctx.ptr_size as u64 * 2),
        Type::Pointer { .. } => Ok(ctx.ptr_size as u64),
        Type::SlicePtr { .. } => Ok(ctx.ptr_size as u64 * 2),

        Type::TraitObject { .. } => Ok(ctx.ptr_size as u64),
        Type::Parameterized { .. } => Err(LayoutError::UninstantiatedGeneric),
        Type::GenericParam { .. } => Err(LayoutError::UninstantiatedGeneric),
        Type::InferredInteger { .. } | Type::InferredFloat { .. } | Type::Inferred { .. } => {
            Err(LayoutError::NotInferred)
        }
    }
}
