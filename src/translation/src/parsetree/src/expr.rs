use crate::item::Variable;
use crate::kind::{FunctionParameter, Type};
use crate::tag::{ArgNameId, LabelNameId, StringLiteralId, StructFieldNameId, VariableNameId};
use crate::ty::GenericArgument;
use crate::{Order, ParseTreeIterMut, RefNodeMut};

use nitrate_tokenize::IntegerKind;
use ordered_float::NotNan;
use serde::{Deserialize, Serialize};
use std::collections::HashMap;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Parentheses {
    pub inner: Expr,
}

impl ParseTreeIterMut for Parentheses {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct BooleanLit {
    pub value: bool,
}

impl ParseTreeIterMut for BooleanLit {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct IntegerLit {
    pub value: u128,
    pub kind: IntegerKind,
}

impl ParseTreeIterMut for IntegerLit {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FloatLit {
    pub value: NotNan<f64>,
}

impl ParseTreeIterMut for FloatLit {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct StringLit {
    pub value: StringLiteralId,
}

impl ParseTreeIterMut for StringLit {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct BStringLit {
    pub value: Vec<u8>,
}

impl ParseTreeIterMut for BStringLit {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct UnitLit {}

impl ParseTreeIterMut for UnitLit {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TypeInfo {
    pub the: Type,
}

impl ParseTreeIterMut for TypeInfo {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct List {
    pub elements: Vec<Expr>,
}

impl ParseTreeIterMut for List {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Object {
    pub fields: HashMap<StructFieldNameId, Expr>,
}

impl ParseTreeIterMut for Object {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
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

impl ParseTreeIterMut for UnaryExpr {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
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

impl ParseTreeIterMut for BinExpr {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Cast {
    pub value: Expr,
    pub to: Type,
}

impl ParseTreeIterMut for Cast {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum BlockItem {
    Variable(Variable),
    Expr(Expr),
}

impl ParseTreeIterMut for BlockItem {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum Safety {
    Safe,
    Unsafe,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Block {
    pub safety: Option<Safety>,
    pub elements: Vec<BlockItem>,
    pub ends_with_semi: bool,
}

impl ParseTreeIterMut for Block {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Closure {
    pub attributes: Option<Vec<Expr>>,
    pub parameters: Vec<FunctionParameter>,
    pub return_type: Option<Type>,
    pub definition: Block,
}

impl ParseTreeIterMut for Closure {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Path {
    pub path: Vec<String>,
    pub type_arguments: Vec<GenericArgument>,
}

impl ParseTreeIterMut for Path {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct IndexAccess {
    pub collection: Expr,
    pub index: Expr,
}

impl ParseTreeIterMut for IndexAccess {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct If {
    pub condition: Expr,
    pub then_branch: Block,
    pub else_branch: Option<Block>,
}

impl ParseTreeIterMut for If {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct WhileLoop {
    pub condition: Expr,
    pub body: Block,
}

impl ParseTreeIterMut for WhileLoop {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct DoWhileLoop {
    pub condition: Expr,
    pub body: Block,
}

impl ParseTreeIterMut for DoWhileLoop {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Switch {
    pub condition: Expr,
    pub cases: Vec<(Expr, Block)>,
    pub default_case: Option<Block>,
}

impl ParseTreeIterMut for Switch {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Break {
    pub label: Option<LabelNameId>,
}

impl ParseTreeIterMut for Break {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Continue {
    pub label: Option<LabelNameId>,
}

impl ParseTreeIterMut for Continue {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Return {
    pub value: Option<Expr>,
}

impl ParseTreeIterMut for Return {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ForEach {
    pub attributes: Option<Vec<Expr>>,
    pub iterable: Expr,
    pub bindings: Vec<(VariableNameId, Option<Type>)>,
    pub body: Block,
}

impl ParseTreeIterMut for ForEach {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Await {
    pub future: Expr,
}

impl ParseTreeIterMut for Await {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct CallArgument {
    pub name: Option<ArgNameId>,
    pub value: Expr,
}

impl ParseTreeIterMut for CallArgument {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Call {
    pub callee: Expr,
    pub arguments: Vec<CallArgument>,
}

impl ParseTreeIterMut for Call {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Clone, Serialize, Deserialize)]
pub enum Expr {
    SyntaxError,

    Parentheses(Box<Parentheses>),

    Boolean(BooleanLit),
    Integer(Box<IntegerLit>),
    Float(FloatLit),
    String(StringLit),
    BString(BStringLit),
    Unit(UnitLit),

    TypeInfo(Box<TypeInfo>),
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

impl ParseTreeIterMut for Expr {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        match self {
            Expr::SyntaxError => {
                f(Order::Pre, RefNodeMut::ExprSyntaxError);
                f(Order::Post, RefNodeMut::ExprSyntaxError);
            }

            Expr::Parentheses(e) => e.depth_first_iter_mut(f),
            Expr::Boolean(e) => e.depth_first_iter_mut(f),
            Expr::Float(e) => e.depth_first_iter_mut(f),
            Expr::String(e) => e.depth_first_iter_mut(f),
            Expr::BString(e) => e.depth_first_iter_mut(f),
            Expr::Unit(e) => e.depth_first_iter_mut(f),

            Expr::Integer(e) => e.depth_first_iter_mut(f),
            Expr::TypeInfo(e) => e.depth_first_iter_mut(f),
            Expr::List(e) => e.depth_first_iter_mut(f),
            Expr::Object(e) => e.depth_first_iter_mut(f),
            Expr::UnaryExpr(e) => e.depth_first_iter_mut(f),
            Expr::BinExpr(e) => e.depth_first_iter_mut(f),
            Expr::Cast(e) => e.depth_first_iter_mut(f),
            Expr::Block(e) => e.depth_first_iter_mut(f),
            Expr::Closure(e) => e.depth_first_iter_mut(f),
            Expr::Variable(e) => e.depth_first_iter_mut(f),
            Expr::Path(e) => e.depth_first_iter_mut(f),
            Expr::IndexAccess(e) => e.depth_first_iter_mut(f),
            Expr::If(e) => e.depth_first_iter_mut(f),
            Expr::While(e) => e.depth_first_iter_mut(f),
            Expr::DoWhileLoop(e) => e.depth_first_iter_mut(f),
            Expr::Switch(e) => e.depth_first_iter_mut(f),
            Expr::Break(e) => e.depth_first_iter_mut(f),
            Expr::Continue(e) => e.depth_first_iter_mut(f),
            Expr::Return(e) => e.depth_first_iter_mut(f),
            Expr::For(e) => e.depth_first_iter_mut(f),
            Expr::Await(e) => e.depth_first_iter_mut(f),
            Expr::Call(e) => e.depth_first_iter_mut(f),
        }
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
            Expr::Unit(_) => write!(f, "()"),

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
