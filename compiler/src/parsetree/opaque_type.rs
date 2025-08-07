use crate::lexer::StringData;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct OpaqueType<'a> {
    identity: StringData<'a>,
}

impl<'a> OpaqueType<'a> {
    pub(crate) fn new(identity: StringData<'a>) -> Self {
        OpaqueType { identity }
    }

    pub fn into_inner(self) -> StringData<'a> {
        self.identity
    }

    pub fn identity(&self) -> &str {
        self.identity.get()
    }
}
