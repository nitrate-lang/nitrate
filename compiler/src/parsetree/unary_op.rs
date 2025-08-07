use super::storage::ExprKey;

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
pub struct UnaryOp<'a> {
    operand: ExprKey<'a>,
    operator: UnaryOperator,
    is_postfix: bool,
}

impl<'a> UnaryOp<'a> {
    pub(crate) fn new(operand: ExprKey<'a>, operator: UnaryOperator, is_postfix: bool) -> Self {
        UnaryOp {
            operand,
            operator,
            is_postfix,
        }
    }

    pub fn operand(&self) -> ExprKey<'a> {
        self.operand
    }

    pub fn operator(&self) -> UnaryOperator {
        self.operator
    }

    pub fn is_postfix(&self) -> bool {
        self.is_postfix
    }
}
