use crate::kind::{FunctionParameter, Type};
use crate::ty::GenericArgument;

use interned_string::IString;
use nitrate_tokenize::IntegerKind;
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
    /// `+`
    Add,
    /// `-`
    Sub,
    /// `*`
    Deref,
    /// `&`
    AddressOf,
    /// `~`
    BitNot,
    /// `!`
    LogicNot,
    /// `typeof`
    Typeof,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct UnaryExpr {
    pub operator: UnaryExprOp,
    pub operand: Expr,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum BinExprOp {
    /// `+`
    Add,
    /// `-`
    Sub,
    /// `*`
    Mul,
    /// `/`
    Div,
    /// `%`
    Mod,
    /// `&`
    BitAnd,
    /// `|`
    BitOr,
    /// `^`
    BitXor,
    /// `<<`
    BitShl,
    /// `>>`
    BitShr,
    /// `<<<`
    BitRol,
    /// `>>>`
    BitRor,
    /// `&&`
    LogicAnd,
    /// `||`
    LogicOr,
    /// `^^`
    LogicXor,
    /// `<`
    LogicLt,
    /// `>`
    LogicGt,
    /// `<=`
    LogicLe,
    /// `>=`
    LogicGe,
    /// `==`
    LogicEq,
    /// `!=`
    LogicNe,
    /// `=`
    Set,
    /// `+=`
    SetPlus,
    /// `-=`
    SetMinus,
    /// `*=`
    SetTimes,
    /// `/=`
    SetSlash,
    /// `%=`
    SetPercent,
    /// `&=`
    SetBitAnd,
    /// `|=`
    SetBitOr,
    /// `^=`
    SetBitXor,
    /// `<<=`
    SetBitShl,
    /// `>>=`
    SetBitShr,
    /// `<<<=`
    SetBitRotl,
    /// `>>>=`
    SetBitRotr,
    /// `&&=`
    SetLogicAnd,
    /// `||=`
    SetLogicOr,
    /// `^^=`
    SetLogicXor,
    /// `.`
    Dot,
    /// `->`
    Arrow,
    /// `..`
    Range,
    /// `as`
    As,
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
pub struct Closure {
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

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct CallArgument {
    pub name: Option<IString>,
    pub value: Expr,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Call {
    pub callee: Expr,
    pub arguments: Vec<CallArgument>,
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

    Closure(Box<Closure>),
    Variable(Box<Variable>),
    Path(Box<Path>),
    IndexAccess(Box<IndexAccess>),

    If(Box<If>),
    While(Box<WhileLoop>),
    DoWhileLoop(Box<DoWhileLoop>),
    Switch(Box<Switch>),
    Break(Box<Break>),
    Continue(Box<Continue>),
    Return(Box<Return>),
    For(Box<ForEach>),
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

            Expr::Closure(e) => e.fmt(f),
            Expr::Variable(e) => e.fmt(f),
            Expr::Path(e) => e.fmt(f),
            Expr::IndexAccess(e) => e.fmt(f),

            Expr::If(e) => e.fmt(f),
            Expr::While(e) => e.fmt(f),
            Expr::DoWhileLoop(e) => e.fmt(f),
            Expr::Switch(e) => e.fmt(f),
            Expr::Break(e) => e.fmt(f),
            Expr::Continue(e) => e.fmt(f),
            Expr::Return(e) => e.fmt(f),
            Expr::For(e) => e.fmt(f),
            Expr::Await(e) => e.fmt(f),
            Expr::Call(e) => e.fmt(f),
        }
    }
}
