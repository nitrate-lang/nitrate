#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct OriginTag {
    offset: u32,
}

impl OriginTag {
    fn new(offset: u32) -> Self {
        OriginTag { offset }
    }
}
