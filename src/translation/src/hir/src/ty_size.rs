use crate::prelude::*;
use std::cmp::max;

pub enum LayoutError {
    Undefined,
    NotInferred,
    UnresolvedSymbol,
}

pub fn get_size_of(ty: &Type, store: &Store, ptr_size: PtrSize) -> Result<u64, LayoutError> {
    match ty {
        Type::Never => Ok(0),
        Type::Unit => Ok(0),
        Type::Bool => Ok(1),
        Type::U8 | Type::I8 => Ok(1),
        Type::U16 | Type::I16 => Ok(2),
        Type::U32 | Type::I32 | Type::F32 => Ok(4),
        Type::U64 | Type::I64 | Type::F64 => Ok(8),
        Type::U128 | Type::I128 => Ok(16),
        Type::USize => Ok(ptr_size as u64),
        Type::Opaque { .. } => Ok(0),

        Type::Array { element_type, len } => {
            let element_stride = get_stride_of(&store[element_type], store, ptr_size)?;
            Ok(element_stride * (*len as u64))
        }

        Type::Tuple {
            element_types: elements,
        } => {
            let mut size = 0_u64;

            for element in &*elements {
                let element = &store[element];
                let element_size = get_size_of(element, store, ptr_size)?;
                let element_align = get_align_of(element, store, ptr_size)?;

                size = size.next_multiple_of(element_align);
                size += element_size;
            }

            Ok(size)
        }

        Type::Slice { element_type: _ } => Err(LayoutError::Undefined),

        Type::Struct { struct_type } => {
            let StructType {
                fields, attributes, ..
            } = &store[struct_type];

            if attributes.contains(&StructAttribute::Packed) {
                let mut total_size = 0_u64;

                for field in fields {
                    total_size += get_size_of(&store[&field.ty], store, ptr_size)?;
                }

                return Ok(total_size);
            }

            let mut offset = 0_u64;

            for field in fields {
                let field_type = &store[&field.ty];

                let field_size = get_size_of(field_type, store, ptr_size)?;
                let field_align = get_align_of(field_type, store, ptr_size)?;

                offset = offset.next_multiple_of(field_align);
                offset += field_size;
            }

            Ok(offset)
        }

        Type::Enum { enum_type } => {
            let EnumType { variants, .. } = &store[enum_type];

            let mut size = 0_u64;

            for variant in variants {
                let variant_size = get_size_of(&store[&variant.ty], store, ptr_size)?;
                size = max(size, variant_size);
            }

            let (discrim_size, discrim_align) = match variants.len() {
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

        Type::Refine { base, .. } => Ok(get_size_of(&store[base], store, ptr_size)?),
        Type::Bitfield { bits, .. } => Ok(bits.div_ceil(8) as u64),

        Type::Function { .. } => Err(LayoutError::Undefined),
        Type::Reference { .. } => Ok(ptr_size as u64),
        Type::Pointer { .. } => Ok(ptr_size as u64),

        Type::Symbol { path: _, link } => match link {
            TypeDefinition::TypeAliasDef(type_alias_id) => {
                let type_id = store[type_alias_id].borrow().type_id;
                get_size_of(&store[&type_id], store, ptr_size)
            }

            TypeDefinition::EnumDef(enum_id) => {
                let enum_type = Type::Enum {
                    enum_type: store[enum_id].borrow().enum_id,
                };
                get_size_of(&enum_type, store, ptr_size)
            }

            TypeDefinition::StructDef(struct_id) => {
                let struct_type = Type::Struct {
                    struct_type: store[struct_id].borrow().struct_id,
                };
                get_size_of(&struct_type, store, ptr_size)
            }
        },

        Type::InferredInteger { .. } | Type::InferredFloat | Type::Inferred { .. } => {
            Err(LayoutError::NotInferred)
        }
    }
}
