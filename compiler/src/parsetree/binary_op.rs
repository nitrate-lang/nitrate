use super::expression::Expr;
use super::expression::{CodeFormat, ToCode};
use crate::lexer::{Operator, Token};

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Hash)]
pub enum BinaryOperator {
    /*----------------------------------------------------------------*
     * Arithmetic Operators                                           *
     *----------------------------------------------------------------*/
    Add, /* '+': "Addition Operator" */
    Sub, /* '-': "Subtraction Operator" */
    Mul, /* '*': "Multiplication Operator" */
    Div, /* '/': "Division Operator" */
    Mod, /* '%': "Modulus Operator" */

    /*----------------------------------------------------------------*
     * Bitwise Operators                                              *
     *----------------------------------------------------------------*/
    BitAnd,  /* '&':   "Bitwise AND Operator" */
    BitOr,   /* '|':   "Bitwise OR Operator" */
    BitXor,  /* '^':   "Bitwise XOR Operator" */
    BitShl,  /* '<<':  "Bitwise Left-Shift Operator" */
    BitShr,  /* '>>':  "Bitwise Right-Shift Operator" */
    BitRotl, /* '<<<': "Bitwise Left-Rotate Operator" */
    BitRotr, /* '>>>': "Bitwise Right-Rotate Operator" */

    /*----------------------------------------------------------------*
     * Logical Operators                                              *
     *----------------------------------------------------------------*/
    LogicAnd, /* '&&': "Logical AND Operator" */
    LogicOr,  /* '||': "Logical OR Operator" */
    LogicXor, /* '^^': "Logical XOR Operator" */
    LogicLt,  /* '<':  "Logical Less-Than Operator" */
    LogicGt,  /* '>':  "Logical Greater-Than Operator" */
    LogicLe,  /* '<=': "Logical Less-Than or Equal-To Operator" */
    LogicGe,  /* '>=': "Logical Greater-Than or Equal-To Operator" */
    LogicEq,  /* '==': "Logical Equal-To Operator" */
    LogicNe,  /* '!=': "Logical Not Equal-To Operator" */

    /*----------------------------------------------------------------*
     * Assignment Operators                                           *
     *----------------------------------------------------------------*/
    Set,         /* '=':    "Assignment Operator" */
    SetPlus,     /* '+=':   "Addition Assignment Operator" */
    SetMinus,    /* '-=':   "Subtraction Assignment Operator" */
    SetTimes,    /* '*=':   "Multiplication Assignment Operator" */
    SetSlash,    /* '/=':   "Division Assignment Operator" */
    SetPercent,  /* '%=':   "Modulus Assignment Operator" */
    SetBitAnd,   /* '&=':   "Bitwise AND Assignment Operator" */
    SetBitOr,    /* '|=':   "Bitwise OR Assignment Operator" */
    SetBitXor,   /* '^=':   "Bitwise XOR Assignment Operator" */
    SetBitShl,   /* '<<=':  "Bitwise Left-Shift Assignment Operator" */
    SetBitShr,   /* '>>=':  "Bitwise Right-Shift Assignment Operator" */
    SetBitRotl,  /* '<<<=': "Bitwise Rotate-Left Assignment Operator" */
    SetBitRotr,  /* '>>>=': "Bitwise Rotate-Right Assignment Operator" */
    SetLogicAnd, /* '&&=':  "Logical AND Assignment Operator" */
    SetLogicOr,  /* '||=':  "Logical OR Assignment Operator" */
    SetLogicXor, /* '^^=':  "Logical XOR Assignment Operator" */

    /*----------------------------------------------------------------*
     * Type System Operators                                          *
     *----------------------------------------------------------------*/
    As, /* 'as':         "Type Cast Operator" */

    /*----------------------------------------------------------------*
     * Syntactic Operators                                            *
     *----------------------------------------------------------------*/
    Dot,        /* '.':          "Dot Operator" */
    Ellipsis,   /* '...':        "Ellipsis Operator" */
    Scope,      /* '::':         "Scope Resolution Operator" */
    Arrow,      /* '->':         "Arrow Operator" */
    BlockArrow, /* '=>':         "Block Arrow Operator" */

    /*----------------------------------------------------------------*
     * Special Operators                                              *
     *----------------------------------------------------------------*/
    Range,     /* '..':         "Range Operator" */
    Question,  /* '?':          "Ternary Operator" */
    Spaceship, /* '<=>':        "Spaceship Operator" */
}

impl<'a> ToCode<'a> for BinaryOperator {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
        let operator = Token::Operator(match self {
            BinaryOperator::Add => Operator::Add,
            BinaryOperator::Sub => Operator::Sub,
            BinaryOperator::Mul => Operator::Mul,
            BinaryOperator::Div => Operator::Div,
            BinaryOperator::Mod => Operator::Mod,
            BinaryOperator::BitAnd => Operator::BitAnd,
            BinaryOperator::BitOr => Operator::BitOr,
            BinaryOperator::BitXor => Operator::BitXor,
            BinaryOperator::BitShl => Operator::BitShl,
            BinaryOperator::BitShr => Operator::BitShr,
            BinaryOperator::BitRotl => Operator::BitRotl,
            BinaryOperator::BitRotr => Operator::BitRotr,
            BinaryOperator::LogicAnd => Operator::LogicAnd,
            BinaryOperator::LogicOr => Operator::LogicOr,
            BinaryOperator::LogicXor => Operator::LogicXor,
            BinaryOperator::LogicLt => Operator::LogicLt,
            BinaryOperator::LogicGt => Operator::LogicGt,
            BinaryOperator::LogicLe => Operator::LogicLe,
            BinaryOperator::LogicGe => Operator::LogicGe,
            BinaryOperator::LogicEq => Operator::LogicEq,
            BinaryOperator::LogicNe => Operator::LogicNe,
            BinaryOperator::Set => Operator::Set,
            BinaryOperator::SetPlus => Operator::SetPlus,
            BinaryOperator::SetMinus => Operator::SetMinus,
            BinaryOperator::SetTimes => Operator::SetTimes,
            BinaryOperator::SetSlash => Operator::SetSlash,
            BinaryOperator::SetPercent => Operator::SetPercent,
            BinaryOperator::SetBitAnd => Operator::SetBitAnd,
            BinaryOperator::SetBitOr => Operator::SetBitOr,
            BinaryOperator::SetBitXor => Operator::SetBitXor,
            BinaryOperator::SetBitShl => Operator::SetBitShl,
            BinaryOperator::SetBitShr => Operator::SetBitShr,
            BinaryOperator::SetBitRotl => Operator::SetBitRotl,
            BinaryOperator::SetBitRotr => Operator::SetBitRotr,
            BinaryOperator::SetLogicAnd => Operator::SetLogicAnd,
            BinaryOperator::SetLogicOr => Operator::SetLogicOr,
            BinaryOperator::SetLogicXor => Operator::SetLogicXor,
            BinaryOperator::As => Operator::As,
            BinaryOperator::Dot => Operator::Dot,
            BinaryOperator::Ellipsis => Operator::Ellipsis,
            BinaryOperator::Scope => Operator::Scope,
            BinaryOperator::Arrow => Operator::Arrow,
            BinaryOperator::BlockArrow => Operator::BlockArrow,
            BinaryOperator::Range => Operator::Range,
            BinaryOperator::Question => Operator::Question,
            BinaryOperator::Spaceship => Operator::Spaceship,
        });

        tokens.push(operator);
    }
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct BinaryExpr<'a> {
    left: Expr<'a>,
    right: Expr<'a>,
    operator: BinaryOperator,
}

impl<'a> BinaryExpr<'a> {
    pub fn new(left: Expr<'a>, operator: BinaryOperator, right: Expr<'a>) -> Self {
        BinaryExpr {
            left,
            right,
            operator,
        }
    }

    pub fn left(&self) -> &Expr<'a> {
        &self.left
    }

    pub fn op(&self) -> BinaryOperator {
        self.operator
    }

    pub fn right(&self) -> &Expr<'a> {
        &self.right
    }
}

impl<'a> ToCode<'a> for BinaryExpr<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        self.left.to_code(tokens, options);
        self.operator.to_code(tokens, options);
        self.right.to_code(tokens, options);
    }
}
