use crate::{
    TypeStore, get_align_of,
    ty::{PointerSize, StructAttribute, Type},
    type_alignment::AlignofError,
};

pub enum SizeofError {
    UnknownSize,
}

pub fn get_size_of(
    ty: &Type,
    store: &TypeStore,
    ptr_size: PointerSize,
) -> Result<u64, SizeofError> {
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
            // In nitrate, type sizes need not be a multiple of their alignment.
            // However, arrays do have padding added to their elements to ensure
            // that each element is properly aligned.

            // So the size of an array is the size of its element type
            // rounded up to the next multiple of its alignment, times the length.

            let element_type = &store[element_type];

            let element_align = match get_align_of(element_type, store, ptr_size) {
                Ok(align) => Ok(align),
                Err(AlignofError::UnknownAlignment) => Err(SizeofError::UnknownSize),
            }?;

            let element_size = get_size_of(element_type, store, ptr_size)?;
            let element_size_with_trailing_padding = element_size.next_multiple_of(element_align);

            Ok(element_size_with_trailing_padding * (*len as u64))
        }

        Type::Tuple { elements } => {
            // The size of a tuple is the sum of the sizes of its element types,
            // plus any padding needed to align each element type.

            let mut offset = 0_u64;

            for element in elements {
                let element = &store[element];

                let element_align = match get_align_of(element, store, ptr_size) {
                    Ok(align) => Ok(align),
                    Err(AlignofError::UnknownAlignment) => Err(SizeofError::UnknownSize),
                }?;

                let element_size = get_size_of(element, store, ptr_size)?;

                offset = offset.next_multiple_of(element_align);
                offset += element_size;
            }

            Ok(offset)
        }

        Type::Slice { element_type: _ } => {
            // A slice itself has no complete size, just like in Rust.
            Err(SizeofError::UnknownSize)
        }

        Type::Struct(struct_type) => {
            // The size of a struct is the sum of the sizes of its field types,
            // plus any padding needed to align each field type.

            if struct_type.attributes.contains(&StructAttribute::Packed) {
                // Packed structs have no padding between fields.
                let mut total_size = 0_u64;

                for (_, field_type) in &struct_type.fields {
                    let field_size = get_size_of(&store[field_type], store, ptr_size)?;
                    total_size += field_size;
                }

                return Ok(total_size);
            }

            let mut offset = 0_u64;

            for (_, field_type) in &struct_type.fields {
                let field_type = &store[field_type];

                let field_align = match get_align_of(field_type, store, ptr_size) {
                    Ok(align) => Ok(align),
                    Err(AlignofError::UnknownAlignment) => Err(SizeofError::UnknownSize),
                }?;

                let field_size = get_size_of(field_type, store, ptr_size)?;

                offset = offset.next_multiple_of(field_align);
                offset += field_size;
            }

            Ok(offset)
        }

        Type::Enum(enum_type) => {
            // The size of an enum is the size of its largest variant,
            // plus the size of a discriminant.

            let mut max_variant_size = 0_u64;

            for (_, variant_type) in &enum_type.variants {
                let variant_size = get_size_of(&store[variant_type], store, ptr_size)?;
                if variant_size > max_variant_size {
                    max_variant_size = variant_size;
                }
            }

            let discrim_size = match enum_type.variants.len() {
                0..=1 => 0,
                2..=256 => 1,
                257..=65536 => 2,
                65537..=4294967296 => 4,
                4294967297.. => 8,
            };

            Ok(max_variant_size + discrim_size)
        }

        Type::Function(_) => Err(SizeofError::UnknownSize),
        Type::Reference(_) => Ok(ptr_size as u64),
    }
}
