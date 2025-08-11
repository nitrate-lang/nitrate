use crate::lexer::IntegerKind;
use apint::UInt;

#[derive(Debug, Clone)]
pub struct IntegerLit {
    value: UInt,
    kind: IntegerKind,
}

impl IntegerLit {
    pub(crate) fn new(value: UInt, kind: IntegerKind) -> Option<Self> {
        if value.try_to_u128().is_ok() {
            Some(IntegerLit { value, kind })
        } else {
            None
        }
    }

    #[must_use]
    pub fn into_inner(self) -> UInt {
        self.value
    }

    #[must_use]
    pub fn get(&self) -> &UInt {
        &self.value
    }

    #[must_use]
    pub fn get_u128(&self) -> u128 {
        self.value
            .try_to_u128()
            .expect("IntegerLit value should fit in u128")
    }

    #[must_use]
    pub fn kind(&self) -> IntegerKind {
        self.kind
    }
}

impl std::ops::Deref for IntegerLit {
    type Target = UInt;

    fn deref(&self) -> &Self::Target {
        &self.value
    }
}
