use crate::kind::{FunctionParameter, Type};
use crate::ty::GenericArgument;

use interned_string::IString;
use nitrate_tokenize::{IntegerKind, Token};
use ordered_float::NotNan;
use serde::{Deserialize, Serialize};
use smallvec::SmallVec;
use std::collections::HashMap;
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
    pub fields: HashMap<IString, Expr>,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum UnaryExprOp {
    Add,
    Sub,
    Deref,
    AddressOf,
    BitNot,
    LogicNot,
    Typeof,
}

impl TryFrom<Token> for UnaryExprOp {
    type Error = ();

    fn try_from(op: Token) -> Result<Self, Self::Error> {
        match op {
            Token::Add => Ok(UnaryExprOp::Add),
            Token::Sub => Ok(UnaryExprOp::Sub),
            Token::Mul => Ok(UnaryExprOp::Deref),
            Token::BitAnd => Ok(UnaryExprOp::AddressOf),
            Token::BitNot => Ok(UnaryExprOp::BitNot),
            Token::LogicNot => Ok(UnaryExprOp::LogicNot),
            Token::Typeof => Ok(UnaryExprOp::Typeof),

            Token::Div
            | Token::Mod
            | Token::BitOr
            | Token::BitXor
            | Token::BitShl
            | Token::BitShr
            | Token::BitRol
            | Token::BitRor
            | Token::LogicAnd
            | Token::LogicOr
            | Token::LogicXor
            | Token::LogicLt
            | Token::LogicGt
            | Token::LogicLe
            | Token::LogicGe
            | Token::LogicEq
            | Token::LogicNe
            | Token::Set
            | Token::SetPlus
            | Token::SetMinus
            | Token::SetTimes
            | Token::SetSlash
            | Token::SetPercent
            | Token::SetBitAnd
            | Token::SetBitOr
            | Token::SetBitXor
            | Token::SetBitShl
            | Token::SetBitShr
            | Token::SetBitRotl
            | Token::SetBitRotr
            | Token::SetLogicAnd
            | Token::SetLogicOr
            | Token::SetLogicXor
            | Token::As
            | Token::Dot
            | Token::Ellipsis
            | Token::Scope
            | Token::Arrow
            | Token::BlockArrow
            | Token::Range => Err(()),

            _ => Err(()),
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
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    BitAnd,
    BitOr,
    BitXor,
    BitShl,
    BitShr,
    BitRol,
    BitRor,
    LogicAnd,
    LogicOr,
    LogicXor,
    LogicLt,
    LogicGt,
    LogicLe,
    LogicGe,
    LogicEq,
    LogicNe,
    Set,
    SetPlus,
    SetMinus,
    SetTimes,
    SetSlash,
    SetPercent,
    SetBitAnd,
    SetBitOr,
    SetBitXor,
    SetBitShl,
    SetBitShr,
    SetBitRotl,
    SetBitRotr,
    SetLogicAnd,
    SetLogicOr,
    SetLogicXor,
    As,
    Dot,
    Ellipsis,
    Scope,
    Arrow,
    BlockArrow,
    Range,
}

impl TryFrom<Token> for BinExprOp {
    type Error = ();

    fn try_from(op: Token) -> Result<Self, Self::Error> {
        match op {
            Token::Add => Ok(BinExprOp::Add),
            Token::Sub => Ok(BinExprOp::Sub),
            Token::Mul => Ok(BinExprOp::Mul),
            Token::Div => Ok(BinExprOp::Div),
            Token::Mod => Ok(BinExprOp::Mod),
            Token::BitAnd => Ok(BinExprOp::BitAnd),
            Token::BitOr => Ok(BinExprOp::BitOr),
            Token::BitXor => Ok(BinExprOp::BitXor),
            Token::BitShl => Ok(BinExprOp::BitShl),
            Token::BitShr => Ok(BinExprOp::BitShr),
            Token::BitRol => Ok(BinExprOp::BitRol),
            Token::BitRor => Ok(BinExprOp::BitRor),
            Token::LogicAnd => Ok(BinExprOp::LogicAnd),
            Token::LogicOr => Ok(BinExprOp::LogicOr),
            Token::LogicXor => Ok(BinExprOp::LogicXor),
            Token::LogicLt => Ok(BinExprOp::LogicLt),
            Token::LogicGt => Ok(BinExprOp::LogicGt),
            Token::LogicLe => Ok(BinExprOp::LogicLe),
            Token::LogicGe => Ok(BinExprOp::LogicGe),
            Token::LogicEq => Ok(BinExprOp::LogicEq),
            Token::LogicNe => Ok(BinExprOp::LogicNe),
            Token::Set => Ok(BinExprOp::Set),
            Token::SetPlus => Ok(BinExprOp::SetPlus),
            Token::SetMinus => Ok(BinExprOp::SetMinus),
            Token::SetTimes => Ok(BinExprOp::SetTimes),
            Token::SetSlash => Ok(BinExprOp::SetSlash),
            Token::SetPercent => Ok(BinExprOp::SetPercent),
            Token::SetBitAnd => Ok(BinExprOp::SetBitAnd),
            Token::SetBitOr => Ok(BinExprOp::SetBitOr),
            Token::SetBitXor => Ok(BinExprOp::SetBitXor),
            Token::SetBitShl => Ok(BinExprOp::SetBitShl),
            Token::SetBitShr => Ok(BinExprOp::SetBitShr),
            Token::SetBitRotl => Ok(BinExprOp::SetBitRotl),
            Token::SetBitRotr => Ok(BinExprOp::SetBitRotr),
            Token::SetLogicAnd => Ok(BinExprOp::SetLogicAnd),
            Token::SetLogicOr => Ok(BinExprOp::SetLogicOr),
            Token::SetLogicXor => Ok(BinExprOp::SetLogicXor),
            Token::As => Ok(BinExprOp::As),
            Token::Dot => Ok(BinExprOp::Dot),
            Token::Ellipsis => Ok(BinExprOp::Ellipsis),
            Token::Scope => Ok(BinExprOp::Scope),
            Token::Arrow => Ok(BinExprOp::Arrow),
            Token::BlockArrow => Ok(BinExprOp::BlockArrow),
            Token::Range => Ok(BinExprOp::Range),

            Token::BitNot | Token::LogicNot | Token::Typeof => Err(()),
            _ => Err(()),
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
pub struct Cast {
    pub value: Expr,
    pub to: Type,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Block {
    pub elements: Vec<Expr>,
    pub ends_with_semi: bool,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct AnonymousFunction {
    pub attributes: Vec<Expr>,
    pub parameters: Vec<FunctionParameter>,
    pub return_type: Option<Type>,
    pub definition: Block,
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
    pub var_type: Option<Type>,
    pub initializer: Option<Expr>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Path {
    pub path: SmallVec<[IString; 3]>,
    pub type_arguments: Vec<GenericArgument>,
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
    pub then_branch: Block,
    pub else_branch: Option<Block>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct WhileLoop {
    pub condition: Expr,
    pub body: Block,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct DoWhileLoop {
    pub condition: Expr,
    pub body: Block,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Switch {
    pub condition: Expr,
    pub cases: Vec<(Expr, Block)>,
    pub default_case: Option<Block>,
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
    pub attributes: Vec<Expr>,
    pub iterable: Expr,
    pub bindings: Vec<(IString, Option<Type>)>,
    pub body: Block,
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
    SyntaxError,

    Parentheses(Box<Expr>),

    Boolean(bool),
    Integer(Box<Integer>),
    Float(NotNan<f64>),
    String(IString),
    BString(Box<Vec<u8>>),
    Unit,

    TypeInfo(Box<Type>),
    List(Box<List>),
    Object(Box<Object>),
    UnaryExpr(Box<UnaryExpr>),
    BinExpr(Box<BinExpr>),
    Cast(Box<Cast>),
    Block(Box<Block>),

    Function(Box<AnonymousFunction>),
    Variable(Box<Variable>),
    Path(Box<Path>),
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
            Expr::SyntaxError => write!(f, "SyntaxError"),

            Expr::Parentheses(e) => f.debug_struct("Parentheses").field("expr", e).finish(),

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
            Expr::Cast(e) => e.fmt(f),
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
