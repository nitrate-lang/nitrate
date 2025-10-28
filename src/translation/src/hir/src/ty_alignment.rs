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
                return Ok(1);
            } else {
                get_align_of(&ctx.store[element_type], ctx)
            }
        }

        Type::Tuple {
            element_types: elements,
        } => {
            let mut max_align = 1;

            for element in &*elements {
                let element_align = get_align_of(&ctx.store[element], ctx)?;
                max_align = max(max_align, element_align);
            }

            Ok(max_align)
        }

        Type::Struct { struct_type } => {
            let StructType {
                fields, attributes, ..
            } = &ctx.store[struct_type];

            if attributes.contains(&StructAttribute::Packed) {
                return Ok(1);
            }

            let mut max_align = 1;

            for field in fields {
                let field_align = get_align_of(&ctx.store[&field.ty], ctx)?;
                max_align = max(max_align, field_align);
            }

            Ok(max_align)
        }

        Type::Enum { enum_type } => {
            let EnumType { variants, .. } = &ctx.store[enum_type];

            let mut max_align = 1;

            for variant in variants {
                let variant_align = get_align_of(&ctx.store[&variant.ty], ctx)?;
                max_align = max(max_align, variant_align);
            }

            let discrim_align = match variants.len() {
                0..=256 => 1,
                257..=65536 => 2,
                65537..=4294967296 => 4,
                4294967297.. => 8,
            };

            max_align = max(max_align, discrim_align);

            Ok(max_align)
        }

        Type::Refine { base, .. } => Ok(get_align_of(&ctx.store[base], ctx)?),

        Type::Function { .. } => Ok(ctx.ptr_size as u64),
        Type::Reference { .. } => Ok(ctx.ptr_size as u64),
        Type::SliceRef { .. } => Ok(ctx.ptr_size as u64),
        Type::Pointer { .. } => Ok(ctx.ptr_size as u64),

        Type::Symbol { path } => match ctx.tab.get_type(path) {
            Some(TypeDefinition::TypeAliasDef(type_alias_id)) => {
                let type_id = ctx.store[type_alias_id].borrow().type_id;
                get_align_of(&ctx.store[&type_id], ctx)
            }

            Some(TypeDefinition::EnumDef(enum_id)) => {
                let enum_type = Type::Enum {
                    enum_type: ctx.store[enum_id].borrow().enum_id,
                };
                get_align_of(&enum_type, ctx)
            }

            Some(TypeDefinition::StructDef(struct_id)) => {
                let struct_type = Type::Struct {
                    struct_type: ctx.store[struct_id].borrow().struct_id,
                };
                get_align_of(&struct_type, ctx)
            }

            None => Err(LayoutError::UnresolvedSymbol),
        },

        Type::InferredInteger { .. } | Type::InferredFloat | Type::Inferred { .. } => {
            Err(LayoutError::NotInferred)
        }
    }
}
