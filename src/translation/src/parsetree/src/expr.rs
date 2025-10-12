use crate::item::{Enum, Function, Struct, Trait, TypeAlias, Variable};
use crate::kind::{FuncParam, Type};
use crate::tag::{ArgNameId, LabelNameId, StringLiteralId, StructFieldNameId, VariableNameId};
use crate::ty::TypePath;

use nitrate_tokenize::IntegerKind;
use ordered_float::NotNan;
use serde::{Deserialize, Serialize};
use serde_with::skip_serializing_none;
use std::sync::{Arc, RwLock, Weak};

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ExprSyntaxError;

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ExprParentheses {
    pub inner: Expr,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct BooleanLit {
    pub value: bool,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct IntegerLit {
    pub value: u128,
    pub kind: IntegerKind,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FloatLit {
    pub value: NotNan<f64>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct StringLit {
    pub value: StringLiteralId,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct BStringLit {
    pub value: Vec<u8>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TypeInfo {
    pub the: Type,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct List {
    pub elements: Vec<Expr>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Tuple {
    pub elements: Vec<Expr>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct StructInit {
    pub type_name: TypePath,
    pub fields: Vec<(StructFieldNameId, Expr)>,
}

#[skip_serializing_none]
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

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct UnaryExpr {
    pub operator: UnaryExprOp,
    pub operand: Expr,
}

#[skip_serializing_none]
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
    /// `.`
    Dot,
    /// `->`
    Arrow,
    /// `..`
    Range,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct BinExpr {
    pub operator: BinExprOp,
    pub left: Expr,
    pub right: Expr,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Cast {
    pub value: Expr,
    pub to: Type,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum BlockItem {
    Variable(Arc<RwLock<Variable>>),
    Expr(Expr),
    Stmt(Expr),
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum Safety {
    Safe,
    Unsafe(Option<Expr>),
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Block {
    pub safety: Option<Safety>,
    pub elements: Vec<BlockItem>,
}

pub type AttributeList = Vec<Expr>;

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Closure {
    pub attributes: Option<AttributeList>,
    pub unique_id: u64,
    pub parameters: Option<Vec<FuncParam>>,
    pub return_type: Option<Type>,
    pub definition: Block,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum ExprPathTarget {
    TypeAlias(Weak<RwLock<TypeAlias>>),
    Struct(Weak<RwLock<Struct>>),
    Enum(Weak<RwLock<Enum>>),
    Function(Weak<RwLock<Function>>),
    Variable(Weak<RwLock<Variable>>),
    Trait(Weak<RwLock<Trait>>),
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TypeArgument {
    pub name: Option<ArgNameId>,
    pub value: Type,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ExprPathSegment {
    pub name: String,
    pub type_arguments: Option<Vec<TypeArgument>>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ExprPath {
    pub segments: Vec<ExprPathSegment>,

    #[serde(skip)]
    pub resolved: Option<ExprPathTarget>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct IndexAccess {
    pub collection: Expr,
    pub index: Expr,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum ElseIf {
    If(Box<If>),
    Block(Block),
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct If {
    pub condition: Expr,
    pub true_branch: Block,
    pub false_branch: Option<ElseIf>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct WhileLoop {
    pub condition: Option<Expr>,
    pub body: Block,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct MatchCase {
    pub condition: Expr,
    pub body: Block,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Match {
    pub condition: Expr,
    pub cases: Vec<MatchCase>,
    pub default_case: Option<Block>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Break {
    pub label: Option<LabelNameId>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Continue {
    pub label: Option<LabelNameId>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Return {
    pub value: Option<Expr>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ForEach {
    pub attributes: Option<AttributeList>,
    pub bindings: Vec<VariableNameId>,
    pub iterable: Expr,
    pub body: Block,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Await {
    pub future: Expr,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct CallArgument {
    pub name: Option<ArgNameId>,
    pub value: Expr,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Call {
    pub callee: Expr,
    pub arguments: Vec<CallArgument>,
}

#[skip_serializing_none]
#[derive(Clone, Serialize, Deserialize)]
pub enum Expr {
    SyntaxError(ExprSyntaxError),
    Parentheses(Box<ExprParentheses>),
    Boolean(BooleanLit),
    Integer(Box<IntegerLit>),
    Float(FloatLit),
    String(StringLit),
    BString(Box<BStringLit>),
    TypeInfo(Box<TypeInfo>),
    List(Box<List>),
    Tuple(Box<Tuple>),
    StructInit(Box<StructInit>),
    UnaryExpr(Box<UnaryExpr>),
    BinExpr(Box<BinExpr>),
    Cast(Box<Cast>),
    Block(Box<Block>),
    Closure(Box<Closure>),
    Path(Box<ExprPath>),
    IndexAccess(Box<IndexAccess>),
    If(Box<If>),
    While(Box<WhileLoop>),
    Match(Box<Match>),
    Break(Box<Break>),
    Continue(Box<Continue>),
    Return(Box<Return>),
    For(Box<ForEach>),
    Await(Box<Await>),
    Call(Box<Call>),
}

impl std::fmt::Debug for Expr {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Expr::SyntaxError(_) => write!(f, "SyntaxError"),
            Expr::Parentheses(e) => f.debug_struct("Parentheses").field("expr", e).finish(),
            Expr::Boolean(e) => e.fmt(f),
            Expr::Integer(e) => e.fmt(f),
            Expr::Float(e) => e.fmt(f),
            Expr::String(e) => e.fmt(f),
            Expr::BString(e) => e.fmt(f),
            Expr::TypeInfo(e) => f.debug_struct("TypeInfo").field("type", e).finish(),
            Expr::List(e) => e.fmt(f),
            Expr::Tuple(e) => e.fmt(f),
            Expr::StructInit(e) => e.fmt(f),
            Expr::UnaryExpr(e) => e.fmt(f),
            Expr::BinExpr(e) => e.fmt(f),
            Expr::Cast(e) => e.fmt(f),
            Expr::Block(e) => e.fmt(f),
            Expr::Closure(e) => e.fmt(f),
            Expr::Path(e) => e.fmt(f),
            Expr::IndexAccess(e) => e.fmt(f),
            Expr::If(e) => e.fmt(f),
            Expr::While(e) => e.fmt(f),
            Expr::Match(e) => e.fmt(f),
            Expr::Break(e) => e.fmt(f),
            Expr::Continue(e) => e.fmt(f),
            Expr::Return(e) => e.fmt(f),
            Expr::For(e) => e.fmt(f),
            Expr::Await(e) => e.fmt(f),
            Expr::Call(e) => e.fmt(f),
        }
    }
}
