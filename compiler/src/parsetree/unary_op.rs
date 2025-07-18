use super::expression::Expr;
use super::expression::{CodeFormat, ToCode};
use crate::lexer::{Operator, Token};

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Hash)]
pub enum UnaryOperator {
    /*----------------------------------------------------------------*
     * Arithmetic Operators                                           *
     *----------------------------------------------------------------*/
    Add, /* '+': "Addition Operator" */
    Sub, /* '-': "Subtraction Operator" */
    Mul, /* '*': "Multiplication Operator" */

    /*----------------------------------------------------------------*
     * Bitwise Operators                                              *
     *----------------------------------------------------------------*/
    BitAnd, /* '&':   "Bitwise AND Operator" */
    BitNot, /* '~':   "Bitwise NOT Operator" */

    /*----------------------------------------------------------------*
     * Logical Operators                                              *
     *----------------------------------------------------------------*/
    LogicNot, /* '!':  "Logical NOT Operator" */

    /*----------------------------------------------------------------*
     * Assignment Operators                                           *
     *----------------------------------------------------------------*/
    Inc, /* '++':   "Increment Operator" */
    Dec, /* '--':   "Decrement Operator" */

    /*----------------------------------------------------------------*
     * Type System Operators                                          *
     *----------------------------------------------------------------*/
    Sizeof,  /* 'sizeof':     "Size Of Operator" */
    Alignof, /* 'alignof':    "Alignment Of Operator" */
    Typeof,  /* 'typeof':     "Type Of Operator" */

    /*----------------------------------------------------------------*
     * Special Operators                                              *
     *----------------------------------------------------------------*/
    Question, /* '?':          "Ternary Operator" */
}

impl<'a> ToCode<'a> for UnaryOperator {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
        let operator = Token::Operator(match self {
            UnaryOperator::Add => Operator::Add,
            UnaryOperator::Sub => Operator::Sub,
            UnaryOperator::Mul => Operator::Mul,
            UnaryOperator::BitAnd => Operator::BitAnd,
            UnaryOperator::BitNot => Operator::BitNot,
            UnaryOperator::LogicNot => Operator::LogicNot,
            UnaryOperator::Inc => Operator::Inc,
            UnaryOperator::Dec => Operator::Dec,
            UnaryOperator::Sizeof => Operator::Sizeof,
            UnaryOperator::Alignof => Operator::Alignof,
            UnaryOperator::Typeof => Operator::Typeof,
            UnaryOperator::Question => Operator::Question,
        });

        tokens.push(operator);
    }
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct UnaryExpr<'a> {
    operand: Box<Expr<'a>>,
    operator: UnaryOperator,
    is_postfix: bool,
}

impl<'a> UnaryExpr<'a> {
    pub fn new(operand: Box<Expr<'a>>, operator: UnaryOperator, is_postfix: bool) -> Self {
        UnaryExpr {
            operand,
            operator,
            is_postfix,
        }
    }

    pub fn operand(&self) -> &Expr<'a> {
        &self.operand
    }

    pub fn operator(&self) -> UnaryOperator {
        self.operator
    }

    pub fn is_postfix(&self) -> bool {
        self.is_postfix
    }
}

impl<'a> ToCode<'a> for UnaryExpr<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        if self.is_postfix() {
            self.operand.to_code(tokens, options);
            self.operator.to_code(tokens, options);
        } else {
            self.operator.to_code(tokens, options);
            self.operand.to_code(tokens, options);
        }
    }
}
