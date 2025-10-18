use crate::prelude::*;

pub fn get_stride_of(ty: &Type, store: &Store, ptr_size: PtrSize) -> Result<u64, LayoutError> {
    let element_size = get_size_of(ty, store, ptr_size)?;
    if element_size == 0 {
        return Ok(0);
    }

    let element_align = get_align_of(ty, store, ptr_size)?;

    Ok(element_size.next_multiple_of(element_align))
}
