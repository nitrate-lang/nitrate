use crate::kind::{FunctionParameter, Type};

use interned_string::IString;
use nitrate_tokenize::{IntegerKind, Op};
use ordered_float::NotNan;
use serde::{Deserialize, Serialize};
use smallvec::SmallVec;
use std::collections::BTreeMap;
use std::ops::Deref;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Integer {
    pub value: u128,
    pub kind: IntegerKind,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct List {
    pub elements: Vec<Expr>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Object {
    pub fields: BTreeMap<IString, Expr>,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum UnaryExprOp {
    /*----------------------------------------------------------------*
     * Arithmetic Operators                                           *
     *----------------------------------------------------------------*/
    Add,   /* '+': "Addition Operator" */
    Sub,   /* '-': "Subtraction Operator" */
    Deref, /* '*': "Multiplication Operator" */

    /*----------------------------------------------------------------*
     * Bitwise Operators                                              *
     *----------------------------------------------------------------*/
    AddressOf, /* '&':   "Bitwise AND Operator" */
    BitNot,    /* '~':   "Bitwise NOT Operator" */

    /*----------------------------------------------------------------*
     * Logical Operators                                              *
     *----------------------------------------------------------------*/
    LogicNot, /* '!':  "Logical NOT Operator" */

    /*----------------------------------------------------------------*
     * Type System Operators                                          *
     *----------------------------------------------------------------*/
    Typeof, /* 'typeof':     "Type Of Operator" */
}

impl TryFrom<Op> for UnaryExprOp {
    type Error = ();

    fn try_from(op: Op) -> Result<Self, Self::Error> {
        match op {
            Op::Add => Ok(UnaryExprOp::Add),
            Op::Sub => Ok(UnaryExprOp::Sub),
            Op::Mul => Ok(UnaryExprOp::Deref),
            Op::BitAnd => Ok(UnaryExprOp::AddressOf),
            Op::BitNot => Ok(UnaryExprOp::BitNot),
            Op::LogicNot => Ok(UnaryExprOp::LogicNot),
            Op::Typeof => Ok(UnaryExprOp::Typeof),

            Op::Div
            | Op::Mod
            | Op::BitOr
            | Op::BitXor
            | Op::BitShl
            | Op::BitShr
            | Op::BitRol
            | Op::BitRor
            | Op::LogicAnd
            | Op::LogicOr
            | Op::LogicXor
            | Op::LogicLt
            | Op::LogicGt
            | Op::LogicLe
            | Op::LogicGe
            | Op::LogicEq
            | Op::LogicNe
            | Op::Set
            | Op::SetPlus
            | Op::SetMinus
            | Op::SetTimes
            | Op::SetSlash
            | Op::SetPercent
            | Op::SetBitAnd
            | Op::SetBitOr
            | Op::SetBitXor
            | Op::SetBitShl
            | Op::SetBitShr
            | Op::SetBitRotl
            | Op::SetBitRotr
            | Op::SetLogicAnd
            | Op::SetLogicOr
            | Op::SetLogicXor
            | Op::As
            | Op::BitcastAs
            | Op::Dot
            | Op::Ellipsis
            | Op::Scope
            | Op::Arrow
            | Op::BlockArrow
            | Op::Range => Err(()),
        }
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct UnaryExpr {
    pub operator: UnaryExprOp,
    pub is_postfix: bool,
    pub operand: Expr,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum BinExprOp {
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
    BitAnd, /* '&':   "Bitwise AND Operator" */
    BitOr,  /* '|':   "Bitwise OR Operator" */
    BitXor, /* '^':   "Bitwise XOR Operator" */
    BitShl, /* '<<':  "Bitwise Left-Shift Operator" */
    BitShr, /* '>>':  "Bitwise Right-Shift Operator" */
    BitRol, /* '<<<': "Bitwise Left-Rotate Operator" */
    BitRor, /* '>>>': "Bitwise Right-Rotate Operator" */

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
    As,        /* 'as':         "Type Cast Operator" */
    BitcastAs, /* 'bitcast_as': "Bitwise Type Cast Operator" */

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
    Range, /* '..':         "Range Operator" */
}

impl TryFrom<Op> for BinExprOp {
    type Error = ();

    fn try_from(op: Op) -> Result<Self, Self::Error> {
        match op {
            Op::Add => Ok(BinExprOp::Add),
            Op::Sub => Ok(BinExprOp::Sub),
            Op::Mul => Ok(BinExprOp::Mul),
            Op::Div => Ok(BinExprOp::Div),
            Op::Mod => Ok(BinExprOp::Mod),
            Op::BitAnd => Ok(BinExprOp::BitAnd),
            Op::BitOr => Ok(BinExprOp::BitOr),
            Op::BitXor => Ok(BinExprOp::BitXor),
            Op::BitShl => Ok(BinExprOp::BitShl),
            Op::BitShr => Ok(BinExprOp::BitShr),
            Op::BitRol => Ok(BinExprOp::BitRol),
            Op::BitRor => Ok(BinExprOp::BitRor),
            Op::LogicAnd => Ok(BinExprOp::LogicAnd),
            Op::LogicOr => Ok(BinExprOp::LogicOr),
            Op::LogicXor => Ok(BinExprOp::LogicXor),
            Op::LogicLt => Ok(BinExprOp::LogicLt),
            Op::LogicGt => Ok(BinExprOp::LogicGt),
            Op::LogicLe => Ok(BinExprOp::LogicLe),
            Op::LogicGe => Ok(BinExprOp::LogicGe),
            Op::LogicEq => Ok(BinExprOp::LogicEq),
            Op::LogicNe => Ok(BinExprOp::LogicNe),
            Op::Set => Ok(BinExprOp::Set),
            Op::SetPlus => Ok(BinExprOp::SetPlus),
            Op::SetMinus => Ok(BinExprOp::SetMinus),
            Op::SetTimes => Ok(BinExprOp::SetTimes),
            Op::SetSlash => Ok(BinExprOp::SetSlash),
            Op::SetPercent => Ok(BinExprOp::SetPercent),
            Op::SetBitAnd => Ok(BinExprOp::SetBitAnd),
            Op::SetBitOr => Ok(BinExprOp::SetBitOr),
            Op::SetBitXor => Ok(BinExprOp::SetBitXor),
            Op::SetBitShl => Ok(BinExprOp::SetBitShl),
            Op::SetBitShr => Ok(BinExprOp::SetBitShr),
            Op::SetBitRotl => Ok(BinExprOp::SetBitRotl),
            Op::SetBitRotr => Ok(BinExprOp::SetBitRotr),
            Op::SetLogicAnd => Ok(BinExprOp::SetLogicAnd),
            Op::SetLogicOr => Ok(BinExprOp::SetLogicOr),
            Op::SetLogicXor => Ok(BinExprOp::SetLogicXor),
            Op::As => Ok(BinExprOp::As),
            Op::BitcastAs => Ok(BinExprOp::BitcastAs),
            Op::Dot => Ok(BinExprOp::Dot),
            Op::Ellipsis => Ok(BinExprOp::Ellipsis),
            Op::Scope => Ok(BinExprOp::Scope),
            Op::Arrow => Ok(BinExprOp::Arrow),
            Op::BlockArrow => Ok(BinExprOp::BlockArrow),
            Op::Range => Ok(BinExprOp::Range),

            Op::BitNot | Op::LogicNot | Op::Typeof => Err(()),
        }
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct BinExpr {
    pub operator: BinExprOp,
    pub left: Expr,
    pub right: Expr,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Block {
    pub elements: Vec<Expr>,
    pub ends_with_semi: bool,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Function {
    pub parameters: Vec<FunctionParameter>,
    pub return_type: Type,
    pub attributes: Vec<Expr>,
    pub definition: Option<Expr>,
}

impl Function {
    #[must_use]
    pub fn is_definition(&self) -> bool {
        self.definition.is_some()
    }

    #[must_use]
    pub fn is_declaration(&self) -> bool {
        self.definition.is_none()
    }
}

#[derive(Debug, Clone, Copy, Serialize, Deserialize)]
pub enum VariableKind {
    Let,
    Var,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Variable {
    pub kind: VariableKind,
    pub attributes: Vec<Expr>,
    pub is_mutable: bool,
    pub name: IString,
    pub var_type: Type,
    pub initializer: Option<Expr>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Path {
    pub path: SmallVec<[IString; 3]>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct IndexAccess {
    pub collection: Expr,
    pub index: Expr,
}

#[derive(Debug, Clone, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub struct QualifiedScope {
    scopes: SmallVec<[IString; 3]>,
}

impl QualifiedScope {
    #[must_use]
    pub fn new(scopes: SmallVec<[IString; 3]>) -> Self {
        Self { scopes }
    }

    #[must_use]
    pub fn parse(qualified_scope: IString) -> Self {
        let parts = qualified_scope
            .split("::")
            .filter(|s| !s.is_empty())
            .map(IString::from)
            .collect::<SmallVec<[IString; 3]>>();

        Self { scopes: parts }
    }

    #[must_use]
    pub fn is_root(&self) -> bool {
        self.scopes.is_empty()
    }

    pub fn pop(&mut self) {
        if !self.scopes.is_empty() {
            self.scopes.pop();
        }
    }

    pub fn push(&mut self, scope: IString) {
        self.scopes.push(scope);
    }

    #[must_use]
    pub fn scopes(&self) -> &[IString] {
        &self.scopes
    }
}

impl std::fmt::Display for QualifiedScope {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "{}",
            self.scopes()
                .iter()
                .map(Deref::deref)
                .collect::<Vec<&str>>()
                .join("::")
        )
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct If {
    pub condition: Expr,
    pub then_branch: Expr,
    pub else_branch: Option<Expr>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct WhileLoop {
    pub condition: Expr,
    pub body: Expr,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct DoWhileLoop {
    pub condition: Expr,
    pub body: Expr,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Switch {
    pub condition: Expr,
    pub cases: Vec<(Expr, Expr)>,
    pub default_case: Option<Expr>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Break {
    pub label: Option<IString>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Continue {
    pub label: Option<IString>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Return {
    pub value: Option<Expr>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ForEach {
    pub iterable: Expr,
    pub bindings: Vec<(IString, Option<Type>)>,
    pub body: Expr,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Await {
    pub future: Expr,
}

pub type CallArguments = Vec<(Option<IString>, Expr)>;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Call {
    pub callee: Expr,
    pub arguments: CallArguments,
}

#[derive(Clone, Serialize, Deserialize)]
pub enum Expr {
    Discard,
    HasParentheses(Box<Expr>),

    Boolean(bool),
    Integer(Box<Integer>),
    Float(NotNan<f64>),
    String(IString),
    BString(Box<Vec<u8>>),
    Unit,

    TypeInfo(Type),
    List(Box<List>),
    Object(Box<Object>),
    UnaryExpr(Box<UnaryExpr>),
    BinExpr(Box<BinExpr>),
    Block(Box<Block>),

    Function(Box<Function>),
    Variable(Box<Variable>),
    Path(Path),
    IndexAccess(Box<IndexAccess>),

    If(Box<If>),
    WhileLoop(Box<WhileLoop>),
    DoWhileLoop(Box<DoWhileLoop>),
    Switch(Box<Switch>),
    Break(Box<Break>),
    Continue(Box<Continue>),
    Return(Box<Return>),
    ForEach(Box<ForEach>),
    Await(Box<Await>),
    Call(Box<Call>),
}

impl Expr {
    #[must_use]
    pub fn is_discard(&self) -> bool {
        matches!(self, Expr::Discard)
    }

    pub fn digest_128(&self) -> [u8; 16] {
        let mut hasher = blake3::Hasher::new();
        hasher.update(&serde_json::to_vec(self).unwrap_or_default());
        let hash = hasher.finalize();
        let bytes = hash.as_bytes();

        let mut result = [0u8; 16];
        result.copy_from_slice(&bytes[..16]);
        result
    }
}

impl std::fmt::Debug for Expr {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Expr::Discard => write!(f, ""),
            Expr::HasParentheses(e) => f.debug_struct("Parentheses").field("expr", e).finish(),

            Expr::Boolean(e) => e.fmt(f),
            Expr::Integer(e) => e.fmt(f),
            Expr::Float(e) => e.fmt(f),
            Expr::String(e) => e.fmt(f),
            Expr::BString(e) => e.fmt(f),
            Expr::Unit => write!(f, "()"),

            Expr::TypeInfo(e) => f.debug_struct("TypeInfo").field("type", e).finish(),
            Expr::List(e) => e.fmt(f),
            Expr::Object(e) => e.fmt(f),
            Expr::UnaryExpr(e) => e.fmt(f),
            Expr::BinExpr(e) => e.fmt(f),
            Expr::Block(e) => e.fmt(f),

            Expr::Function(e) => e.fmt(f),
            Expr::Variable(e) => e.fmt(f),
            Expr::Path(e) => e.fmt(f),
            Expr::IndexAccess(e) => e.fmt(f),

            Expr::If(e) => e.fmt(f),
            Expr::WhileLoop(e) => e.fmt(f),
            Expr::DoWhileLoop(e) => e.fmt(f),
            Expr::Switch(e) => e.fmt(f),
            Expr::Break(e) => e.fmt(f),
            Expr::Continue(e) => e.fmt(f),
            Expr::Return(e) => e.fmt(f),
            Expr::ForEach(e) => e.fmt(f),
            Expr::Await(e) => e.fmt(f),
            Expr::Call(e) => e.fmt(f),
        }
    }
}
