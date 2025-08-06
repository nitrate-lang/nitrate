#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct OpaqueType<'a> {
    identity: &'a str,
}

impl<'a> OpaqueType<'a> {
    pub fn new(identity: &'a str) -> Self {
        OpaqueType { identity }
    }

    pub fn identity(&self) -> &'a str {
        self.identity
    }
}
