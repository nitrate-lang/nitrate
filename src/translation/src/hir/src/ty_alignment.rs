use crate::prelude::*;
use std::cmp::max;

pub fn get_align_of(ty: &Type, ctx: &LayoutCtx) -> Result<u64, LayoutError> {
    match ty {
        Type::Never => Ok(1),
        Type::Unit => Ok(1),
        Type::Bool => Ok(1),
        Type::U8 | Type::I8 => Ok(1),
        Type::U16 | Type::I16 => Ok(2),
        Type::U32 | Type::I32 | Type::F32 => Ok(4),
        Type::U64 | Type::I64 | Type::F64 => Ok(8),
        Type::U128 | Type::I128 => Ok(16),
        Type::USize => Ok(ctx.ptr_size as u64),

        Type::Array { element_type, len } => {
            if *len == 0 {
                Ok(1)
            } else {
                get_align_of(element_type, ctx)
            }
        }

        Type::Tuple {
            element_types: elements,
        } => {
            let mut max_align = 1;

            for element in elements {
                let element_align = get_align_of(element, ctx)?;
                max_align = max(max_align, element_align);
            }

            Ok(max_align)
        }

        Type::Struct { def } => {
            let StructDef { fields, attributes, .. } = &*def.borrow();

            if attributes.contains(&StructAttribute::Packed) {
                return Ok(1);
            }

            let mut max_align = 1;

            for field in fields.values() {
                let field_align = get_align_of(&field.ty, ctx)?;
                max_align = max(max_align, field_align);
            }

            Ok(max_align)
        }

        Type::Enum { def } => {
            let EnumDef { variants, .. } = &*def.borrow();

            let mut max_align = 1;

            for variant in variants {
                let variant_align = get_align_of(&variant.ty, ctx)?;
                max_align = max(max_align, variant_align);
            }

            let discrim_align = match variants.len() as u64 {
                0..=256 => 1,
                257..=65536 => 2,
                65537..=4294967296 => 4,
                4294967297.. => 8,
            };

            max_align = max(max_align, discrim_align);

            Ok(max_align)
        }

        Type::TypeAlias { def } => {
            let type_alias = &def.borrow().type_id;
            get_align_of(type_alias, ctx)
        }

        Type::Refine { base, .. } => Ok(get_align_of(base, ctx)?),

        Type::Function { .. } => Ok(ctx.ptr_size as u64),
        Type::Reference { .. } => Ok(ctx.ptr_size as u64),
        Type::SliceRef { .. } => Ok(ctx.ptr_size as u64),
        Type::Pointer { .. } => Ok(ctx.ptr_size as u64),

        Type::InferredInteger | Type::InferredFloat | Type::Inferred { .. } => Err(LayoutError::NotInferred),
    }
}
