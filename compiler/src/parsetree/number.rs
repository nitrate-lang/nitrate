use super::expression::{CodeFormat, ToCode};
use crate::lexer::{Integer, IntegerKind, Operator, Token};

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
#[repr(packed)]
pub struct NumberLit {
    value: u128,
    is_negative: bool,
}

impl NumberLit {
    pub fn new(value: u128, is_negative: bool) -> Self {
        NumberLit { value, is_negative }
    }

    pub fn from_u128(value: u128) -> Self {
        NumberLit {
            value,
            is_negative: false,
        }
    }

    pub fn into_inner(self) -> u128 {
        self.value
    }

    pub fn value(&self) -> u128 {
        self.value
    }

    pub fn is_negative(&self) -> bool {
        self.is_negative
    }
}

impl<'a> ToCode<'a> for NumberLit {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
        if self.is_negative() {
            tokens.push(Token::Operator(Operator::Sub));
        }

        let number = Integer::new(self.value(), IntegerKind::Decimal);
        tokens.push(Token::Integer(number));
    }
}
