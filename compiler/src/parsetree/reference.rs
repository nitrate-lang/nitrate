use super::storage::TypeKey;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct ManagedRefType<'a> {
    target: TypeKey<'a>,
    is_mutable: bool,
}

impl<'a> ManagedRefType<'a> {
    pub(crate) fn new(target: TypeKey<'a>, is_mutable: bool) -> Self {
        ManagedRefType { target, is_mutable }
    }

    pub fn target(&self) -> TypeKey<'a> {
        self.target
    }

    pub fn is_mutable(&self) -> bool {
        self.is_mutable
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct UnmanagedRefType<'a> {
    target: TypeKey<'a>,
    is_mutable: bool,
}

impl<'a> UnmanagedRefType<'a> {
    pub(crate) fn new(target: TypeKey<'a>, is_mutable: bool) -> Self {
        UnmanagedRefType { target, is_mutable }
    }

    pub fn target(&self) -> TypeKey<'a> {
        self.target
    }

    pub fn is_mutable(&self) -> bool {
        self.is_mutable
    }
}
