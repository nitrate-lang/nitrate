use crate::lexer::IntegerKind;
use apint::UInt;

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct IntegerLit {
    value: UInt,
    kind: IntegerKind,
}

impl IntegerLit {
    pub fn new(value: UInt, kind: IntegerKind) -> Option<Self> {
        if value.try_to_u128().is_ok() {
            Some(IntegerLit { value, kind })
        } else {
            None
        }
    }

    pub fn into_inner(self) -> UInt {
        self.value
    }

    pub fn get(&self) -> &UInt {
        &self.value
    }

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

#[derive(Debug, Clone, PartialOrd, PartialEq)]
pub struct FloatLit {
    value: f64,
}

impl FloatLit {
    pub fn new(value: f64) -> Self {
        FloatLit { value }
    }

    pub fn into_inner(self) -> f64 {
        self.value
    }

    pub fn get(&self) -> f64 {
        self.value
    }
}

impl std::cmp::Eq for FloatLit {}

impl std::ops::Deref for FloatLit {
    type Target = f64;

    fn deref(&self) -> &Self::Target {
        &self.value
    }
}

impl std::hash::Hash for FloatLit {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        self.value.to_bits().hash(state);
    }
}
