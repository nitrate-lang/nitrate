use crate::ast::{FuncParam, Mutability, Type};
use crate::tag::{ArgNameId, LabelNameId, StringLiteralId, StructFieldNameId, VariableNameId};
use crate::ty::TypePath;

use nitrate_token::IntegerKind;
use ordered_float::NotNan;
use serde::{Deserialize, Serialize};
use serde_with::skip_serializing_none;

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
    Borrow,
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
#[derive(Debug, Clone, Copy, Serialize, Deserialize)]
pub enum LocalVariableKind {
    Let,
    Var,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct LocalVariable {
    pub kind: LocalVariableKind,
    pub attributes: Option<AttributeList>,
    pub mutability: Option<Mutability>,
    pub name: VariableNameId,
    pub ty: Option<Type>,
    pub initializer: Option<Expr>,
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
    pub resolved_path: Option<String>,
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
pub struct FunctionCall {
    pub callee: Expr,
    pub positional: Vec<Expr>,
    pub named: Vec<(ArgNameId, Expr)>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct MethodCall {
    pub object: Expr,
    pub method_name: String,
    pub positional: Vec<Expr>,
    pub named: Vec<(ArgNameId, Expr)>,
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
    FunctionCall(Box<FunctionCall>),
    MethodCall(Box<MethodCall>),
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
            Expr::FunctionCall(e) => e.fmt(f),
            Expr::MethodCall(e) => e.fmt(f),
        }
    }
}

impl Expr {
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
