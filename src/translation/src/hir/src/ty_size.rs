use crate::prelude::{hir::*, *};
use std::cmp::max;

pub enum SizeofError {
    UnknownSize,
}

pub fn get_size_of(ty: &Type, store: &Store, ptr_size: PointerSize) -> Result<u64, SizeofError> {
    match ty {
        Type::Never => Ok(0),
        Type::Bool => Ok(1),
        Type::U8 | Type::I8 | Type::F8 => Ok(1),
        Type::U16 | Type::I16 | Type::F16 => Ok(2),
        Type::U32 | Type::I32 | Type::F32 => Ok(4),
        Type::U64 | Type::I64 | Type::F64 => Ok(8),
        Type::U128 | Type::I128 | Type::F128 => Ok(16),
        Type::USize | Type::ISize => Ok(ptr_size as u64),

        Type::Array { element_type, len } => {
            let element_stride = match get_stride_of(&store[element_type], store, ptr_size) {
                Ok(stride) => Ok(stride),
                Err(StrideOfError::UnknownStride) => Err(SizeofError::UnknownSize),
            }?;

            Ok(element_stride * (*len as u64))
        }

        Type::Tuple { elements } => {
            let mut size = 0_u64;

            for element in elements {
                let element = &store[element];

                let element_size = get_size_of(element, store, ptr_size)?;
                let element_align = match get_align_of(element, store, ptr_size) {
                    Ok(align) => Ok(align),
                    Err(AlignofError::UnknownAlignment) => Err(SizeofError::UnknownSize),
                }?;

                size = size.next_multiple_of(element_align);
                size += element_size;
            }

            Ok(size)
        }

        Type::Slice { element_type: _ } => Err(SizeofError::UnknownSize),

        Type::Struct(struct_type) => {
            if struct_type.attributes.contains(&StructAttribute::Packed) {
                let mut total_size = 0_u64;

                for (_, field_type) in &struct_type.fields {
                    total_size += get_size_of(&store[field_type], store, ptr_size)?;
                }

                return Ok(total_size);
            }

            let mut offset = 0_u64;

            for (_, field_type) in &struct_type.fields {
                let field_type = &store[field_type];

                let field_size = get_size_of(field_type, store, ptr_size)?;
                let field_align = match get_align_of(field_type, store, ptr_size) {
                    Ok(align) => Ok(align),
                    Err(AlignofError::UnknownAlignment) => Err(SizeofError::UnknownSize),
                }?;

                offset = offset.next_multiple_of(field_align);
                offset += field_size;
            }

            Ok(offset)
        }

        Type::Enum(enum_type) => {
            let mut size = 0_u64;

            for (_, variant_type) in &enum_type.variants {
                let variant_size = get_size_of(&store[variant_type], store, ptr_size)?;
                size = max(size, variant_size);
            }

            let (discrim_size, discrim_align) = match enum_type.variants.len() {
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

        Type::Function(_) => Err(SizeofError::UnknownSize),

        Type::Reference(_) => Ok(ptr_size as u64),
    }
}
