use apint::ApInt;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct NumberLit {
    value: ApInt,
}

impl NumberLit {
    pub fn new(value: ApInt) -> Self {
        NumberLit { value }
    }

    pub fn from_u128(value: u128) -> Self {
        NumberLit {
            value: ApInt::from_u128(value),
        }
    }

    pub fn into_inner(self) -> ApInt {
        self.value
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
