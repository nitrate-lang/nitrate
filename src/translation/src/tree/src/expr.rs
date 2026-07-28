use crate::prelude::*;
use nitrate_nstring::NString;
use nitrate_token::IntegerKind;
use ordered_float::NotNan;
use serde::{Deserialize, Serialize};
use serde_with::skip_serializing_none;

/// Trait for types that have a source span.
pub trait Spanned {
    fn span(&self) -> ByteSpan;
    fn set_span(&mut self, span: ByteSpan);
    fn reconstruct(&self, source: &[u8]) -> String {
        self.span().extract_str(source).to_string()
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ExprSyntaxError {
    pub span: ByteSpan,
}

impl Spanned for ExprSyntaxError {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ExprParentheses {
    pub span: ByteSpan,
    pub inner: Expr,
}

impl Spanned for ExprParentheses {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct BooleanLit {
    pub span: ByteSpan,
    pub value: bool,
}

impl Spanned for BooleanLit {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct IntegerLit {
    pub span: ByteSpan,
    pub value: u128,
    pub kind: IntegerKind,
}

impl Spanned for IntegerLit {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FloatLit {
    pub span: ByteSpan,
    pub value: NotNan<f64>,
}

impl Spanned for FloatLit {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct StringLit {
    pub span: ByteSpan,
    pub value: String,
}

impl Spanned for StringLit {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct BStringLit {
    pub span: ByteSpan,
    pub value: Vec<u8>,
}

impl Spanned for BStringLit {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TypeInfo {
    pub span: ByteSpan,
    pub the: Type,
}

impl Spanned for TypeInfo {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct List {
    pub span: ByteSpan,
    pub elements: Vec<Expr>,
}

impl Spanned for List {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Tuple {
    pub span: ByteSpan,
    pub elements: Vec<Expr>,
}

impl Spanned for Tuple {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct StructInit {
    pub span: ByteSpan,
    pub path: ExprPath,
    pub fields: Vec<(NString, Expr)>,
}

impl Spanned for StructInit {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum UnaryExprOp {
    Add,
    Sub,
    Deref,
    Borrow,
    Not,
    Typeof,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct UnaryExpr {
    pub span: ByteSpan,
    pub operator: UnaryExprOp,
    pub operand: Expr,
}

impl Spanned for UnaryExpr {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
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
    Range,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct BinExpr {
    pub span: ByteSpan,
    pub operator: BinExprOp,
    pub left: Expr,
    pub right: Expr,
}

impl Spanned for BinExpr {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Cast {
    pub span: ByteSpan,
    pub value: Expr,
    pub to: Type,
}

impl Spanned for Cast {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Copy, Serialize, Deserialize)]
pub enum LocalVariableKind {
    Let,
    Var,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct LocalVariable {
    pub span: ByteSpan,
    pub kind: LocalVariableKind,
    pub attributes: Option<AttributeList>,
    pub mutability: Option<Mutability>,
    pub name: NString,
    pub ty: Option<Type>,
    pub initializer: Option<Expr>,
}

impl Spanned for LocalVariable {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum BlockItem {
    Variable(LocalVariable),
    Expr(Expr),
    Stmt(Expr),
}

impl BlockItem {
    pub fn as_variable(self) -> Option<LocalVariable> {
        match self {
            BlockItem::Variable(v) => Some(v),
            _ => None,
        }
    }
    pub fn as_expr(self) -> Option<Expr> {
        match self {
            BlockItem::Expr(e) => Some(e),
            _ => None,
        }
    }
    pub fn as_stmt(self) -> Option<Expr> {
        match self {
            BlockItem::Stmt(s) => Some(s),
            _ => None,
        }
    }
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
    pub span: ByteSpan,
    pub safety: Option<Safety>,
    pub elements: Vec<BlockItem>,
}

impl Spanned for Block {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

pub type AttributeList = Vec<Expr>;

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Closure {
    pub span: ByteSpan,
    pub attributes: Option<AttributeList>,
    pub parameters: Option<Vec<FuncParam>>,
    pub return_type: Option<Type>,
    pub definition: Block,
}

impl Spanned for Closure {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TypeArgument {
    pub span: ByteSpan,
    pub name: Option<NString>,
    pub value: Type,
}

impl Spanned for TypeArgument {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ExprPathSegment {
    pub span: ByteSpan,
    pub name: String,
    pub type_arguments: Option<Vec<TypeArgument>>,
}

impl Spanned for ExprPathSegment {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ExprPath {
    pub span: ByteSpan,
    pub segments: Vec<ExprPathSegment>,
    pub resolved_path: Option<NString>,
}

impl Spanned for ExprPath {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct IndexAccess {
    pub span: ByteSpan,
    pub collection: Expr,
    pub index: Expr,
}

impl Spanned for IndexAccess {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FieldAccess {
    pub span: ByteSpan,
    pub object: Expr,
    pub field: String,
}

impl Spanned for FieldAccess {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
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
    pub span: ByteSpan,
    pub condition: Expr,
    pub true_branch: Block,
    pub false_branch: Option<ElseIf>,
}

impl Spanned for If {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct WhileLoop {
    pub span: ByteSpan,
    pub condition: Option<Expr>,
    pub body: Block,
}

impl Spanned for WhileLoop {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct MatchCase {
    pub span: ByteSpan,
    pub condition: Expr,
    pub body: Block,
}

impl Spanned for MatchCase {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Match {
    pub span: ByteSpan,
    pub condition: Expr,
    pub cases: Vec<MatchCase>,
    pub default_case: Option<Block>,
}

impl Spanned for Match {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Break {
    pub span: ByteSpan,
    pub label: Option<NString>,
}

impl Spanned for Break {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Continue {
    pub span: ByteSpan,
    pub label: Option<NString>,
}

impl Spanned for Continue {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Return {
    pub span: ByteSpan,
    pub value: Option<Expr>,
}

impl Spanned for Return {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ForEach {
    pub span: ByteSpan,
    pub attributes: Option<AttributeList>,
    pub bindings: Vec<NString>,
    pub iterable: Expr,
    pub body: Block,
}

impl Spanned for ForEach {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Await {
    pub span: ByteSpan,
    pub future: Expr,
}

impl Spanned for Await {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FunctionCall {
    pub span: ByteSpan,
    pub callee: Expr,
    pub positional: Vec<Expr>,
    pub named: Vec<(NString, Expr)>,
}

impl Spanned for FunctionCall {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct MethodCall {
    pub span: ByteSpan,
    pub object: Expr,
    pub method_name: String,
    pub positional: Vec<Expr>,
    pub named: Vec<(NString, Expr)>,
}

impl Spanned for MethodCall {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
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
    FieldAccess(Box<FieldAccess>),
    If(Box<If>),
    While(Box<WhileLoop>),
    Match(Box<Match>),
    Break(Box<Break>),
    Continue(Box<Continue>),
    Return(Box<Return>),
    For(Box<ForEach>),
    Await(Box<Await>),
    FunctionCall(Box<FunctionCall>),
    MethodCall(Box<MethodCall>),
}

impl Expr {
    pub fn span(&self) -> ByteSpan {
        match self {
            Expr::SyntaxError(e) => e.span,
            Expr::Parentheses(e) => e.span,
            Expr::Boolean(e) => e.span,
            Expr::Integer(e) => e.span,
            Expr::Float(e) => e.span,
            Expr::String(e) => e.span,
            Expr::BString(e) => e.span,
            Expr::TypeInfo(e) => e.span,
            Expr::List(e) => e.span,
            Expr::Tuple(e) => e.span,
            Expr::StructInit(e) => e.span,
            Expr::UnaryExpr(e) => e.span,
            Expr::BinExpr(e) => e.span,
            Expr::Cast(e) => e.span,
            Expr::Block(e) => e.span,
            Expr::Closure(e) => e.span,
            Expr::Path(e) => e.span,
            Expr::IndexAccess(e) => e.span,
            Expr::FieldAccess(e) => e.span,
            Expr::If(e) => e.span,
            Expr::While(e) => e.span,
            Expr::Match(e) => e.span,
            Expr::Break(e) => e.span,
            Expr::Continue(e) => e.span,
            Expr::Return(e) => e.span,
            Expr::For(e) => e.span,
            Expr::Await(e) => e.span,
            Expr::FunctionCall(e) => e.span,
            Expr::MethodCall(e) => e.span,
        }
    }

    pub fn set_span(&mut self, span: ByteSpan) {
        match self {
            Expr::SyntaxError(e) => e.span = span,
            Expr::Parentheses(e) => e.span = span,
            Expr::Boolean(e) => e.span = span,
            Expr::Integer(e) => e.span = span,
            Expr::Float(e) => e.span = span,
            Expr::String(e) => e.span = span,
            Expr::BString(e) => e.span = span,
            Expr::TypeInfo(e) => e.span = span,
            Expr::List(e) => e.span = span,
            Expr::Tuple(e) => e.span = span,
            Expr::StructInit(e) => e.span = span,
            Expr::UnaryExpr(e) => e.span = span,
            Expr::BinExpr(e) => e.span = span,
            Expr::Cast(e) => e.span = span,
            Expr::Block(e) => e.span = span,
            Expr::Closure(e) => e.span = span,
            Expr::Path(e) => e.span = span,
            Expr::IndexAccess(e) => e.span = span,
            Expr::FieldAccess(e) => e.span = span,
            Expr::If(e) => e.span = span,
            Expr::While(e) => e.span = span,
            Expr::Match(e) => e.span = span,
            Expr::Break(e) => e.span = span,
            Expr::Continue(e) => e.span = span,
            Expr::Return(e) => e.span = span,
            Expr::For(e) => e.span = span,
            Expr::Await(e) => e.span = span,
            Expr::FunctionCall(e) => e.span = span,
            Expr::MethodCall(e) => e.span = span,
        }
    }

    pub fn reconstruct(&self, source: &[u8]) -> String {
        self.span().extract_str(source).to_string()
    }

    pub fn as_parentheses(self) -> Option<ExprParentheses> {
        match self {
            Expr::Parentheses(e) => Some(*e),
            _ => None,
        }
    }
    pub fn as_boolean(self) -> Option<BooleanLit> {
        match self {
            Expr::Boolean(e) => Some(e),
            _ => None,
        }
    }
    pub fn as_integer(self) -> Option<IntegerLit> {
        match self {
            Expr::Integer(e) => Some(*e),
            _ => None,
        }
    }
    pub fn as_float(self) -> Option<FloatLit> {
        match self {
            Expr::Float(e) => Some(e),
            _ => None,
        }
    }
    pub fn as_string(self) -> Option<StringLit> {
        match self {
            Expr::String(e) => Some(e),
            _ => None,
        }
    }
    pub fn as_bstring(self) -> Option<BStringLit> {
        match self {
            Expr::BString(e) => Some(*e),
            _ => None,
        }
    }
    pub fn as_type_info(self) -> Option<TypeInfo> {
        match self {
            Expr::TypeInfo(e) => Some(*e),
            _ => None,
        }
    }
    pub fn as_list(self) -> Option<List> {
        match self {
            Expr::List(e) => Some(*e),
            _ => None,
        }
    }
    pub fn as_tuple(self) -> Option<Tuple> {
        match self {
            Expr::Tuple(e) => Some(*e),
            _ => None,
        }
    }
    pub fn as_struct_init(self) -> Option<StructInit> {
        match self {
            Expr::StructInit(e) => Some(*e),
            _ => None,
        }
    }
    pub fn as_unary_expr(self) -> Option<UnaryExpr> {
        match self {
            Expr::UnaryExpr(e) => Some(*e),
            _ => None,
        }
    }
    pub fn as_bin_expr(self) -> Option<BinExpr> {
        match self {
            Expr::BinExpr(e) => Some(*e),
            _ => None,
        }
    }
    pub fn as_cast(self) -> Option<Cast> {
        match self {
            Expr::Cast(e) => Some(*e),
            _ => None,
        }
    }
    pub fn as_block(self) -> Option<Block> {
        match self {
            Expr::Block(e) => Some(*e),
            _ => None,
        }
    }
    pub fn as_closure(self) -> Option<Closure> {
        match self {
            Expr::Closure(e) => Some(*e),
            _ => None,
        }
    }
    pub fn as_path(self) -> Option<ExprPath> {
        match self {
            Expr::Path(e) => Some(*e),
            _ => None,
        }
    }
    pub fn as_index_access(self) -> Option<IndexAccess> {
        match self {
            Expr::IndexAccess(e) => Some(*e),
            _ => None,
        }
    }
    pub fn as_if(self) -> Option<If> {
        match self {
            Expr::If(e) => Some(*e),
            _ => None,
        }
    }
    pub fn as_while(self) -> Option<WhileLoop> {
        match self {
            Expr::While(e) => Some(*e),
            _ => None,
        }
    }
    pub fn as_match(self) -> Option<Match> {
        match self {
            Expr::Match(e) => Some(*e),
            _ => None,
        }
    }
    pub fn as_break(self) -> Option<Break> {
        match self {
            Expr::Break(e) => Some(*e),
            _ => None,
        }
    }
    pub fn as_continue(self) -> Option<Continue> {
        match self {
            Expr::Continue(e) => Some(*e),
            _ => None,
        }
    }
    pub fn as_return(self) -> Option<Return> {
        match self {
            Expr::Return(e) => Some(*e),
            _ => None,
        }
    }
    pub fn as_for(self) -> Option<ForEach> {
        match self {
            Expr::For(e) => Some(*e),
            _ => None,
        }
    }
    pub fn as_await(self) -> Option<Await> {
        match self {
            Expr::Await(e) => Some(*e),
            _ => None,
        }
    }
    pub fn as_function_call(self) -> Option<FunctionCall> {
        match self {
            Expr::FunctionCall(e) => Some(*e),
            _ => None,
        }
    }
    pub fn as_method_call(self) -> Option<MethodCall> {
        match self {
            Expr::MethodCall(e) => Some(*e),
            _ => None,
        }
    }
}
