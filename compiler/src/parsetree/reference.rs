use super::storage::TypeKey;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct ManagedType<'a> {
    target: TypeKey<'a>,
    is_mutable: bool,
}

impl<'a> ManagedType<'a> {
    pub(crate) fn new(target: TypeKey<'a>, is_mutable: bool) -> Self {
        ManagedType { target, is_mutable }
    }

    pub fn target(&self) -> TypeKey<'a> {
        self.target
    }

    pub fn is_mutable(&self) -> bool {
        self.is_mutable
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct UnmanagedType<'a> {
    target: TypeKey<'a>,
    is_mutable: bool,
}

impl<'a> UnmanagedType<'a> {
    pub(crate) fn new(target: TypeKey<'a>, is_mutable: bool) -> Self {
        UnmanagedType { target, is_mutable }
    }

    pub fn target(&self) -> TypeKey<'a> {
        self.target
    }

    pub fn is_mutable(&self) -> bool {
        self.is_mutable
    }
}
