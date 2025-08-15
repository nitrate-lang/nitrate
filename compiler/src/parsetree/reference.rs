use super::expression::Type;

#[derive(Debug, Clone, PartialEq)]
pub struct ManagedRefType<'a> {
    target: Type<'a>,
    is_mutable: bool,
}

impl<'a> ManagedRefType<'a> {
    #[must_use]
    pub(crate) fn new(target: Type<'a>, is_mutable: bool) -> Self {
        ManagedRefType { target, is_mutable }
    }

    #[must_use]
    pub fn target(&self) -> Type<'a> {
        self.target.clone()
    }

    #[must_use]
    pub fn is_mutable(&self) -> bool {
        self.is_mutable
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct UnmanagedRefType<'a> {
    target: Type<'a>,
    is_mutable: bool,
}

impl<'a> UnmanagedRefType<'a> {
    #[must_use]
    pub(crate) fn new(target: Type<'a>, is_mutable: bool) -> Self {
        UnmanagedRefType { target, is_mutable }
    }

    #[must_use]
    pub fn target(&self) -> Type<'a> {
        self.target.clone()
    }

    #[must_use]
    pub fn is_mutable(&self) -> bool {
        self.is_mutable
    }
}
