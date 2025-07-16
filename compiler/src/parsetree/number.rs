use super::origin::OriginTag;

use apint::ApInt;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct NumberLit {
    value: ApInt,
    origin: OriginTag,
}

impl NumberLit {
    pub fn new(value: ApInt, origin: OriginTag) -> Self {
        NumberLit { value, origin }
    }

    pub fn from_u128(value: u128, origin: OriginTag) -> Self {
        NumberLit {
            value: ApInt::from_u128(value),
            origin,
        }
    }

    pub fn into_inner(self) -> ApInt {
        self.value
    }

    pub fn origin(&self) -> OriginTag {
        self.origin
    }
}

impl std::ops::Deref for NumberLit {
    type Target = ApInt;

    fn deref(&self) -> &Self::Target {
        &self.value
    }
}

impl std::ops::DerefMut for NumberLit {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.value
    }
}
