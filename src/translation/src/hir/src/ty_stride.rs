use crate::prelude::*;

pub enum StrideOfError {
    UnknownStride,
}

pub fn get_stride_of(ty: &Type, store: &Store, ptr_size: PtrSize) -> Result<u64, StrideOfError> {
    let element_size = match get_size_of(ty, store, ptr_size) {
        Ok(size) => Ok(size),
        Err(SizeofError::UnknownSize) => Err(StrideOfError::UnknownStride),
    }?;

    if element_size == 0 {
        return Ok(0);
    }

    let element_align = match get_align_of(ty, store, ptr_size) {
        Ok(align) => Ok(align),
        Err(AlignofError::UnknownAlignment) => Err(StrideOfError::UnknownStride),
    }?;

    Ok(element_size.next_multiple_of(element_align))
}
