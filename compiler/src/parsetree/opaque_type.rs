use crate::lexer::StringData;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct OpaqueType<'a> {
    identity: StringData<'a>,
}

impl<'a> OpaqueType<'a> {
    pub(crate) fn new(identity: StringData<'a>) -> Self {
        OpaqueType { identity }
    }

    #[must_use]
    pub fn into_inner(self) -> StringData<'a> {
        self.identity
    }

    #[must_use]
    pub fn identity(&self) -> &str {
        self.identity.get()
    }
}
