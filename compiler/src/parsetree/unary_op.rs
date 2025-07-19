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
    fn new(operand: Box<Expr<'a>>, operator: UnaryOperator, is_postfix: bool) -> Self {
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

#[derive(Debug, Clone, Default, PartialEq, Eq, PartialOrd, Hash)]
pub struct UnaryExprBuilder<'a> {
    operand: Option<Box<Expr<'a>>>,
    operator: Option<UnaryOperator>,
    is_postfix: Option<bool>,
}

impl<'a> UnaryExprBuilder<'a> {
    pub fn with_operand(mut self, operand: Box<Expr<'a>>) -> Self {
        self.operand = Some(operand);
        self
    }

    pub fn with_operator(mut self, operator: UnaryOperator) -> Self {
        self.operator = Some(operator);
        self
    }

    pub fn set_postfix(mut self) -> Self {
        self.is_postfix = Some(true);
        self
    }

    pub fn set_prefix(mut self) -> Self {
        self.is_postfix = Some(false);
        self
    }

    pub fn build(self) -> UnaryExpr<'a> {
        let operand = self.operand.expect("UnaryExprBuilder operand must be set");
        let operator = self
            .operator
            .expect("UnaryExprBuilder operator must be set");

        let is_postfix = self
            .is_postfix
            .expect("UnaryExprBuilder is_postfix must be set");

        UnaryExpr::new(operand, operator, is_postfix)
    }
}
