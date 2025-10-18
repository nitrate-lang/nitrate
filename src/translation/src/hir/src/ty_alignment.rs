use crate::prelude::*;
use std::cmp::max;

pub fn get_align_of(ty: &Type, store: &Store, ptr_size: PtrSize) -> Result<u64, LayoutError> {
    match ty {
        Type::Never => Ok(1),
        Type::Unit => Ok(1),
        Type::Bool => Ok(1),
        Type::U8 | Type::I8 => Ok(1),
        Type::U16 | Type::I16 => Ok(2),
        Type::U32 | Type::I32 | Type::F32 => Ok(4),
        Type::U64 | Type::I64 | Type::F64 => Ok(8),
        Type::U128 | Type::I128 => Ok(16),
        Type::USize => Ok(ptr_size as u64),
        Type::Opaque { .. } => Ok(1),

        Type::Array { element_type, len } => {
            if *len == 0 {
                return Ok(1);
            } else {
                get_align_of(&store[element_type], store, ptr_size)
            }
        }

        Type::Tuple {
            element_types: elements,
        } => {
            let mut max_align = 1;

            for element in &*elements {
                let element_align = get_align_of(element, store, ptr_size)?;
                max_align = max(max_align, element_align);
            }

            Ok(max_align)
        }

        Type::Slice { element_type } => get_align_of(&store[element_type], store, ptr_size),

        Type::Struct { struct_type } => {
            let StructType {
                fields, attributes, ..
            } = &store[struct_type];

            if attributes.contains(&StructAttribute::Packed) {
                return Ok(1);
            }

            let mut max_align = 1;

            for (_, (field_type, _)) in fields {
                let field_align = get_align_of(&store[field_type], store, ptr_size)?;
                max_align = max(max_align, field_align);
            }

            Ok(max_align)
        }

        Type::Enum { enum_type } => {
            let EnumType { variants, .. } = &store[enum_type];

            let mut max_align = 1;

            for (_, variant_type) in variants {
                let variant_align = get_align_of(&store[variant_type], store, ptr_size)?;
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

        Type::Refine { base, .. } => Ok(get_align_of(&store[base], store, ptr_size)?),
        Type::Bitfield { base, .. } => Ok(get_align_of(&store[base], store, ptr_size)?),

        Type::Function { .. } => Err(LayoutError::Undefined),
        Type::Reference { .. } => Ok(ptr_size as u64),
        Type::Pointer { .. } => Ok(ptr_size as u64),

        Type::Symbol { link, .. } => match link {
            Some(type_id) => get_align_of(&store[type_id], store, ptr_size),
            None => Err(LayoutError::UnresolvedSymbol),
        },

        Type::InferredInteger { .. } | Type::InferredFloat | Type::Inferred { .. } => {
            Err(LayoutError::NotInferred)
        }
    }
}
