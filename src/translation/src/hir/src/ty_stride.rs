use crate::prelude::*;

pub fn get_stride_of(ty: &Type, ctx: &LayoutCtx) -> Result<u64, LayoutError> {
    let element_size = get_size_of(ty, ctx)?;
    if element_size == 0 {
        return Ok(0);
    }

    let element_align = get_align_of(ty, ctx)?;

    Ok(element_size.next_multiple_of(element_align))
}
