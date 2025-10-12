use crate::prelude::*;
use std::cmp::max;

pub enum AlignofError {
    UnknownAlignment,
}

pub fn get_align_of(ty: &Type, store: &Store, ptr_size: PtrSize) -> Result<u64, AlignofError> {
    match ty {
        Type::Never => Ok(1),
        Type::Unit => Ok(1),
        Type::Bool => Ok(1),
        Type::U8 | Type::I8 | Type::F8 => Ok(1),
        Type::U16 | Type::I16 | Type::F16 => Ok(2),
        Type::U32 | Type::I32 | Type::F32 => Ok(4),
        Type::U64 | Type::I64 | Type::F64 => Ok(8),
        Type::U128 | Type::I128 | Type::F128 => Ok(16),
        Type::USize => Ok(ptr_size as u64),
        Type::Opaque { .. } => Ok(1),

        Type::Array { element_type, len } => {
            if *len == 0 {
                return Ok(1);
            } else {
                get_align_of(&store[element_type], store, ptr_size)
            }
        }

        Type::Tuple { elements } => {
            let mut max_align = 1;

            for element in &store[elements] {
                let element_align = get_align_of(&store[element], store, ptr_size)?;
                max_align = max(max_align, element_align);
            }

            Ok(max_align)
        }

        Type::Slice { element_type } => get_align_of(&store[element_type], store, ptr_size),

        Type::Struct { attributes, fields } => {
            if store[attributes].contains(&StructAttribute::Packed) {
                return Ok(1);
            }

            let mut max_align = 1;

            for (_, field_type) in &store[fields] {
                let field_align = get_align_of(&store[field_type], store, ptr_size)?;
                max_align = max(max_align, field_align);
            }

            Ok(max_align)
        }

        Type::Enum { variants, .. } => {
            let variants = &store[variants];
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

        Type::Function { .. } => Err(AlignofError::UnknownAlignment),
        Type::Reference { .. } => Ok(ptr_size as u64),
        Type::Pointer { .. } => Ok(ptr_size as u64),
        Type::TypeAlias { aliased, .. } => get_align_of(&store[aliased], store, ptr_size),

        Type::InferredInteger { .. } | Type::InferredFloat | Type::Inferred { .. } => {
            Err(AlignofError::UnknownAlignment)
        }
    }
}
