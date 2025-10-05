use crate::prelude::{hir::*, *};
use std::{cmp::max, ops::Deref};

pub enum AlignofError {
    UnknownAlignment,
}

pub fn get_align_of(ty: &Type, store: &Store, ptr_size: PointerSize) -> Result<u64, AlignofError> {
    match ty {
        Type::Never => Ok(1),
        Type::Unit => Ok(1),
        Type::Bool => Ok(1),
        Type::U8 | Type::I8 | Type::F8 => Ok(1),
        Type::U16 | Type::I16 | Type::F16 => Ok(2),
        Type::U32 | Type::I32 | Type::F32 => Ok(4),
        Type::U64 | Type::I64 | Type::F64 => Ok(8),
        Type::U128 | Type::I128 | Type::F128 => Ok(16),
        Type::USize | Type::ISize => Ok(ptr_size as u64),

        Type::Array { element_type, len } => {
            if *len == 0 {
                return Ok(1);
            } else {
                get_align_of(&store[element_type], store, ptr_size)
            }
        }

        Type::Tuple { elements } => {
            let mut max_align = 1;

            for element in elements.deref() {
                let element_align = get_align_of(&store[element], store, ptr_size)?;
                max_align = max(max_align, element_align);
            }

            Ok(max_align)
        }

        Type::Slice { element_type } => get_align_of(&store[element_type], store, ptr_size),

        Type::Struct(struct_type) => {
            if struct_type.attributes.contains(&StructAttribute::Packed) {
                return Ok(1);
            }

            let mut max_align = 1;

            for (_, field_type) in &struct_type.fields {
                let field_align = get_align_of(&store[field_type], store, ptr_size)?;
                max_align = max(max_align, field_align);
            }

            Ok(max_align)
        }

        Type::Enum(enum_type) => {
            let mut max_align = 1;

            for (_, variant_type) in &enum_type.variants {
                let variant_align = get_align_of(&store[variant_type], store, ptr_size)?;
                max_align = max(max_align, variant_align);
            }

            let discrim_align = match enum_type.variants.len() {
                0..=256 => 1,
                257..=65536 => 2,
                65537..=4294967296 => 4,
                4294967297.. => 8,
            };

            max_align = max(max_align, discrim_align);

            Ok(max_align)
        }

        Type::Function(_) => Err(AlignofError::UnknownAlignment),

        Type::Reference(_) => Ok(ptr_size as u64),
        Type::Pointer(_) => Ok(ptr_size as u64),

        Type::InferredInteger { .. } | Type::InferredFloat | Type::Inferred { .. } => {
            Err(AlignofError::UnknownAlignment)
        }

        Type::TypeAlias { aliased, .. } => get_align_of(&store[aliased], store, ptr_size),
    }
}
