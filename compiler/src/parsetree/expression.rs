use super::array_type::ArrayType;
use super::binary_op::BinaryExpr;
use super::block::Block;
use super::character::CharLit;
use super::function::Function;
use super::function_type::FunctionType;
use super::list::List;
use super::number::{FloatLit, IntegerLit};
use super::object::Object;
use super::returns::Return;
use super::statement::Statement;
use super::string::StringLit;
use super::struct_type::StructType;
use super::tuple_type::TupleType;
use super::types::Type;
use super::unary_op::UnaryExpr;
use super::variable::Variable;

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub enum Expr<'a> {
    Discard,

    /* Primitive Expressions */
    Integer(IntegerLit),
    Float(FloatLit),
    String(StringLit<'a>),
    Char(CharLit),
    List(List<'a>),
    Object(Object<'a>),

    /* Compound Expressions */
    UnaryOp(UnaryExpr<'a>),
    BinaryOp(BinaryExpr<'a>),
    Statement(Statement<'a>),
    Block(Block<'a>),

    /* Definition */
    Function(Function<'a>),
    Variable(Variable<'a>),

    /* Control Flow */
    Return(Return<'a>),

    /* Primitive Types */
    Bool,
    UInt8,
    UInt16,
    UInt32,
    UInt64,
    UInt128,
    Int8,
    Int16,
    Int32,
    Int64,
    Int128,
    Float8,
    Float16,
    Float32,
    Float64,
    Float128,

    /* Compound Types */
    InferType,
    TupleType(TupleType<'a>),
    ArrayType(ArrayType<'a>),
    StructType(StructType<'a>),
    FunctionType(FunctionType<'a>),
}

impl<'a> Expr<'a> {
    pub fn discard(&mut self) {
        *self = Expr::Discard;
    }

    pub fn is_discarded(&self) -> bool {
        matches!(self, Expr::Discard)
    }

    pub fn into_type(self) -> Option<Type<'a>> {
        match self {
            Expr::Discard => None,

            Expr::Integer(_) => None,
            Expr::Float(_) => None,
            Expr::String(_) => None,
            Expr::Char(_) => None,
            Expr::List(_) => None,
            Expr::Object(_) => None,

            Expr::UnaryOp(_) => None,
            Expr::BinaryOp(_) => None,
            Expr::Statement(_) => None,
            Expr::Block(_) => None,

            Expr::Function(_) => None,
            Expr::Variable(_) => None,

            Expr::Return(_) => None,

            Expr::Bool => Some(Type::Bool),
            Expr::UInt8 => Some(Type::UInt8),
            Expr::UInt16 => Some(Type::UInt16),
            Expr::UInt32 => Some(Type::UInt32),
            Expr::UInt64 => Some(Type::UInt64),
            Expr::UInt128 => Some(Type::UInt128),
            Expr::Int8 => Some(Type::Int8),
            Expr::Int16 => Some(Type::Int16),
            Expr::Int32 => Some(Type::Int32),
            Expr::Int64 => Some(Type::Int64),
            Expr::Int128 => Some(Type::Int128),
            Expr::Float8 => Some(Type::Float8),
            Expr::Float16 => Some(Type::Float16),
            Expr::Float32 => Some(Type::Float32),
            Expr::Float64 => Some(Type::Float64),
            Expr::Float128 => Some(Type::Float128),

            Expr::InferType => Some(Type::InferType),
            Expr::TupleType(tuple) => Some(Type::TupleType(tuple)),
            Expr::ArrayType(array) => Some(Type::ArrayType(array)),
            Expr::StructType(struct_type) => Some(Type::StructType(struct_type)),
            Expr::FunctionType(function) => Some(Type::FunctionType(function)),
        }
    }
}
