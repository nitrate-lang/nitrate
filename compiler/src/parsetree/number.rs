use super::expression::{CodeFormat, ToCode};
use crate::lexer::{Float, Integer, IntegerKind, Token};
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

impl<'a> ToCode<'a> for IntegerLit {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
        let u128 = self
            .try_to_u128()
            .expect("IntegerLit apint::UInt value should fit in u128");

        let number = Integer::new(u128, self.kind());
        tokens.push(Token::Integer(number));
    }
}

impl std::ops::Deref for IntegerLit {
    type Target = UInt;

    fn deref(&self) -> &Self::Target {
        &self.value
    }
}

#[derive(Debug, Clone, Default, PartialEq, Eq, PartialOrd, Hash)]
pub struct IntegerLitBuilder {
    value: Option<UInt>,
    kind: Option<IntegerKind>,
}

impl IntegerLitBuilder {
    pub fn with_value(mut self, value: UInt) -> Self {
        self.value = Some(value);
        self
    }

    pub fn with_kind(mut self, kind: IntegerKind) -> Self {
        self.kind = Some(kind);
        self
    }

    pub fn build(self) -> IntegerLit {
        let value = self.value.expect("IntegerLitBuilder must have a value");
        let kind = self.kind.expect("IntegerLitBuilder must have a kind");

        IntegerLit::new(value, kind).expect("IntegerLitBuilder must create a valid IntegerLit")
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

impl<'a> ToCode<'a> for FloatLit {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
        let number = Float::new(self.get());
        tokens.push(Token::Float(number));
    }
}

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

#[derive(Debug, Clone, Default, PartialEq, PartialOrd)]
pub struct FloatLitBuilder {
    value: Option<f64>,
}

impl FloatLitBuilder {
    pub fn with_value(mut self, value: f64) -> Self {
        self.value = Some(value);
        self
    }

    pub fn build(self) -> FloatLit {
        let value = self.value.expect("FloatLitBuilder must have a value");
        FloatLit::new(value)
    }
}
