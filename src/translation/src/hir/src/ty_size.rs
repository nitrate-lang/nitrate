use crate::{SymbolTab, prelude::*};
use std::cmp::max;

#[derive(Debug, Clone, Copy)]
pub enum LayoutError {
    NotInferred,
    UnresolvedSymbol,
}

pub struct LayoutCtx<'a> {
    pub store: &'a Store,
    pub tab: &'a SymbolTab,
    pub ptr_size: PtrSize,
}

pub fn get_size_of(ty: &Type, ctx: &LayoutCtx) -> Result<u64, LayoutError> {
    match ty {
        Type::Never => Ok(0),
        Type::Unit => Ok(0),
        Type::Bool => Ok(1),
        Type::U8 | Type::I8 => Ok(1),
        Type::U16 | Type::I16 => Ok(2),
        Type::U32 | Type::I32 | Type::F32 => Ok(4),
        Type::U64 | Type::I64 | Type::F64 => Ok(8),
        Type::U128 | Type::I128 => Ok(16),
        Type::USize => Ok(ctx.ptr_size as u64),

        Type::Array { element_type, len } => {
            let element_stride = get_stride_of(&ctx.store[element_type], ctx)?;
            Ok(element_stride * (*len as u64))
        }

        Type::Tuple {
            element_types: elements,
        } => {
            let mut size = 0_u64;

            for element in &*elements {
                let element = &ctx.store[element];
                let element_size = get_size_of(element, ctx)?;
                let element_align = get_align_of(element, ctx)?;

                size = size.next_multiple_of(element_align);
                size += element_size;
            }

            Ok(size)
        }

        Type::Struct { struct_type } => {
            let StructType {
                fields, attributes, ..
            } = &ctx.store[struct_type];

            if attributes.contains(&StructAttribute::Packed) {
                let mut total_size = 0_u64;

                for field in fields {
                    total_size += get_size_of(&ctx.store[&field.ty], ctx)?;
                }

                return Ok(total_size);
            }

            let mut offset = 0_u64;

            for field in fields {
                let field_type = &ctx.store[&field.ty];

                let field_size = get_size_of(field_type, ctx)?;
                let field_align = get_align_of(field_type, ctx)?;

                offset = offset.next_multiple_of(field_align);
                offset += field_size;
            }

            Ok(offset)
        }

        Type::Enum { enum_type } => {
            let EnumType { variants, .. } = &ctx.store[enum_type];

            let mut size = 0_u64;

            for variant in variants {
                let variant_size = get_size_of(&ctx.store[&variant.ty], ctx)?;
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

        Type::Refine { base, .. } => Ok(get_size_of(&ctx.store[base], ctx)?),

        Type::Function { .. } => Ok(ctx.ptr_size as u64),
        Type::Reference { .. } => Ok(ctx.ptr_size as u64),
        Type::SliceRef { .. } => Ok(ctx.ptr_size as u64 * 2),
        Type::Pointer { .. } => Ok(ctx.ptr_size as u64),

        Type::Symbol { path } => match ctx.tab.get_type(&path) {
            Some(TypeDefinition::TypeAliasDef(type_alias_id)) => {
                let type_id = ctx.store[type_alias_id].borrow().type_id;
                get_size_of(&ctx.store[&type_id], ctx)
            }

            Some(TypeDefinition::EnumDef(enum_id)) => {
                let enum_type = Type::Enum {
                    enum_type: ctx.store[enum_id].borrow().enum_id,
                };
                get_size_of(&enum_type, ctx)
            }

            Some(TypeDefinition::StructDef(struct_id)) => {
                let struct_type = Type::Struct {
                    struct_type: ctx.store[struct_id].borrow().struct_id,
                };
                get_size_of(&struct_type, ctx)
            }

            None => Err(LayoutError::UnresolvedSymbol),
        },

        Type::InferredInteger { .. } | Type::InferredFloat | Type::Inferred { .. } => {
            Err(LayoutError::NotInferred)
        }
    }
}
