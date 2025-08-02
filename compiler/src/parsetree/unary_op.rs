use super::expression::Expr;

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

#[derive(Debug, Clone)]
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
