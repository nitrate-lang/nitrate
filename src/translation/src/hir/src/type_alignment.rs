use crate::{
    TypeStore,
    ty::{PointerSize, StructAttribute, Type},
};

pub enum AlignofError {
    UnknownAlignment,
}

pub fn get_align_of(
    ty: &Type,
    storage: &TypeStore,
    ptr_size: PointerSize,
) -> Result<u64, AlignofError> {
    match ty {
        Type::Never => Ok(1),
        Type::Bool => Ok(1),
        Type::U8 | Type::I8 | Type::F8 => Ok(1),
        Type::U16 | Type::I16 | Type::F16 => Ok(2),
        Type::U32 | Type::I32 | Type::F32 => Ok(4),
        Type::U64 | Type::I64 | Type::F64 => Ok(8),
        Type::U128 | Type::I128 | Type::F128 => Ok(16),
        Type::USize | Type::ISize => Ok(ptr_size as u64),

        Type::Array { element_type, .. } => {
            // Array alignment is the same as its element type alignment
            get_align_of(&storage[element_type], storage, ptr_size)
        }

        Type::Tuple { elements } => {
            // Tuple alignment is the max alignment among its element types

            let mut max_align = 1;

            for element in elements {
                let element_align = get_align_of(&storage[element], storage, ptr_size)?;

                if element_align > max_align {
                    max_align = element_align;
                }
            }

            Ok(max_align)
        }

        Type::Slice { element_type } => {
            // Slice alignment is the same as its element type alignment
            get_align_of(&storage[element_type], storage, ptr_size)
        }

        Type::Struct(struct_type) => {
            // Struct alignment is the same as its largest field alignment
            // unless it is packed, in which case it is 1

            if struct_type.attributes.contains(&StructAttribute::Packed) {
                return Ok(1);
            }

            let mut max_align = 1;

            for (_, field_type) in &struct_type.fields {
                let field_align = get_align_of(&storage[field_type], storage, ptr_size)?;

                if field_align > max_align {
                    max_align = field_align;
                }
            }

            Ok(max_align)
        }

        Type::Enum(enum_type) => {
            // Enum alignment is the same as its largest variant alignment.

            let mut max_align = 1;

            for (_, variant_type) in &enum_type.variants {
                let variant_align = get_align_of(&storage[variant_type], storage, ptr_size)?;

                if variant_align > max_align {
                    max_align = variant_align;
                }
            }

            let discrim_align = match enum_type.variants.len() {
                0..=1 => 1,
                2..=256 => 1,
                257..=65536 => 2,
                65537..=4294967296 => 4,
                4294967297.. => 8,
            };

            if discrim_align > max_align {
                max_align = discrim_align;
            }

            Ok(max_align)
        }

        Type::Function(_) => {
            // A Type::Function represents the literal machine code or whatever,
            // like just like a slice of bytes represents the literal bytes in memory.
            // This is not the same as a reference to a function which has an alignment.
            // We don't know the alignment of machine code, it could vary by platform.
            // This is why the function type itself has no alignment.

            Err(AlignofError::UnknownAlignment)
        }

        Type::Reference(_) => Ok(ptr_size as u64),
    }
}
