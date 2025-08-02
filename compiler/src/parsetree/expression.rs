use super::array_type::ArrayType;
use super::binary_op::BinaryOp;
use super::block::Block;
use super::character::CharLit;
use super::function::Function;
use super::function_type::FunctionType;
use super::list::ListLit;
use super::number::{FloatLit, IntegerLit};
use super::object::ObjectLit;
use super::returns::Return;
use super::statement::Statement;
use super::string::StringLit;
use super::struct_type::StructType;
use super::tuple_type::TupleType;
use super::unary_op::UnaryOp;
use super::variable::Variable;

#[derive(Debug, Clone)]
pub enum Expr<'a> {
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

    Discard,

    /* Literal Expressions */
    IntegerLit(IntegerLit),
    FloatLit(FloatLit),
    StringLit(StringLit<'a>),
    CharLit(CharLit),
    ListLit(ListLit<'a>),
    ObjectLit(ObjectLit<'a>),

    /* Compound Expressions */
    UnaryOp(UnaryOp<'a>),
    BinaryOp(BinaryOp<'a>),
    Statement(Statement<'a>),
    Block(Block<'a>),

    /* Definition */
    Function(Function<'a>),
    Variable(Variable<'a>),

    /* Control Flow */
    Return(Return<'a>),
}

#[derive(Debug, Clone)]
pub enum Type<'a> {
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

#[derive(Debug, Clone)]
pub enum RefExpr<'storage, 'a> {
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
    TupleType(&'storage TupleType<'a>),
    ArrayType(&'storage ArrayType<'a>),
    StructType(&'storage StructType<'a>),
    FunctionType(&'storage FunctionType<'a>),

    Discard,

    /* Literal Expressions */
    IntegerLit(&'storage IntegerLit),
    FloatLit(&'storage FloatLit),
    StringLit(&'storage StringLit<'a>),
    CharLit(&'storage CharLit),
    ListLit(&'storage ListLit<'a>),
    ObjectLit(&'storage ObjectLit<'a>),

    /* Compound Expressions */
    UnaryOp(&'storage UnaryOp<'a>),
    BinaryOp(&'storage BinaryOp<'a>),
    Statement(&'storage Statement<'a>),
    Block(&'storage Block<'a>),

    /* Definition */
    Function(&'storage Function<'a>),
    Variable(&'storage Variable<'a>),

    /* Control Flow */
    Return(&'storage Return<'a>),
}

#[derive(Debug, Clone)]
pub enum RefType<'storage, 'a> {
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
    TupleType(&'storage TupleType<'a>),
    ArrayType(&'storage ArrayType<'a>),
    StructType(&'storage StructType<'a>),
    FunctionType(&'storage FunctionType<'a>),
}

impl<'a> TryInto<Type<'a>> for Expr<'a> {
    type Error = Self;

    fn try_into(self) -> Result<Type<'a>, Self::Error> {
        match self {
            Expr::Bool => Ok(Type::Bool),
            Expr::UInt8 => Ok(Type::UInt8),
            Expr::UInt16 => Ok(Type::UInt16),
            Expr::UInt32 => Ok(Type::UInt32),
            Expr::UInt64 => Ok(Type::UInt64),
            Expr::UInt128 => Ok(Type::UInt128),
            Expr::Int8 => Ok(Type::Int8),
            Expr::Int16 => Ok(Type::Int16),
            Expr::Int32 => Ok(Type::Int32),
            Expr::Int64 => Ok(Type::Int64),
            Expr::Int128 => Ok(Type::Int128),
            Expr::Float8 => Ok(Type::Float8),
            Expr::Float16 => Ok(Type::Float16),
            Expr::Float32 => Ok(Type::Float32),
            Expr::Float64 => Ok(Type::Float64),
            Expr::Float128 => Ok(Type::Float128),

            Expr::InferType => Ok(Type::InferType),
            Expr::TupleType(x) => Ok(Type::TupleType(x)),
            Expr::ArrayType(x) => Ok(Type::ArrayType(x)),
            Expr::StructType(x) => Ok(Type::StructType(x)),
            Expr::FunctionType(x) => Ok(Type::FunctionType(x)),

            Expr::Discard
            | Expr::IntegerLit(_)
            | Expr::FloatLit(_)
            | Expr::StringLit(_)
            | Expr::CharLit(_)
            | Expr::ListLit(_)
            | Expr::ObjectLit(_)
            | Expr::UnaryOp(_)
            | Expr::BinaryOp(_)
            | Expr::Statement(_)
            | Expr::Block(_)
            | Expr::Function(_)
            | Expr::Variable(_)
            | Expr::Return(_) => Err(self),
        }
    }
}

impl<'a> Into<Expr<'a>> for Type<'a> {
    fn into(self) -> Expr<'a> {
        match self {
            Type::Bool => Expr::Bool,
            Type::UInt8 => Expr::UInt8,
            Type::UInt16 => Expr::UInt16,
            Type::UInt32 => Expr::UInt32,
            Type::UInt64 => Expr::UInt64,
            Type::UInt128 => Expr::UInt128,
            Type::Int8 => Expr::Int8,
            Type::Int16 => Expr::Int16,
            Type::Int32 => Expr::Int32,
            Type::Int64 => Expr::Int64,
            Type::Int128 => Expr::Int128,
            Type::Float8 => Expr::Float8,
            Type::Float16 => Expr::Float16,
            Type::Float32 => Expr::Float32,
            Type::Float64 => Expr::Float64,
            Type::Float128 => Expr::Float128,

            Type::InferType => Expr::InferType,
            Type::TupleType(x) => Expr::TupleType(x),
            Type::ArrayType(x) => Expr::ArrayType(x),
            Type::StructType(x) => Expr::StructType(x),
            Type::FunctionType(x) => Expr::FunctionType(x),
        }
    }
}

impl<'storage, 'a> TryInto<RefType<'storage, 'a>> for RefExpr<'storage, 'a> {
    type Error = Self;

    fn try_into(self) -> Result<RefType<'storage, 'a>, Self::Error> {
        match self {
            RefExpr::Bool => Ok(RefType::Bool),
            RefExpr::UInt8 => Ok(RefType::UInt8),
            RefExpr::UInt16 => Ok(RefType::UInt16),
            RefExpr::UInt32 => Ok(RefType::UInt32),
            RefExpr::UInt64 => Ok(RefType::UInt64),
            RefExpr::UInt128 => Ok(RefType::UInt128),
            RefExpr::Int8 => Ok(RefType::Int8),
            RefExpr::Int16 => Ok(RefType::Int16),
            RefExpr::Int32 => Ok(RefType::Int32),
            RefExpr::Int64 => Ok(RefType::Int64),
            RefExpr::Int128 => Ok(RefType::Int128),
            RefExpr::Float8 => Ok(RefType::Float8),
            RefExpr::Float16 => Ok(RefType::Float16),
            RefExpr::Float32 => Ok(RefType::Float32),
            RefExpr::Float64 => Ok(RefType::Float64),
            RefExpr::Float128 => Ok(RefType::Float128),

            RefExpr::InferType => Ok(RefType::InferType),
            RefExpr::TupleType(x) => Ok(RefType::TupleType(x)),
            RefExpr::ArrayType(x) => Ok(RefType::ArrayType(x)),
            RefExpr::StructType(x) => Ok(RefType::StructType(x)),
            RefExpr::FunctionType(x) => Ok(RefType::FunctionType(x)),

            RefExpr::Discard
            | RefExpr::IntegerLit(_)
            | RefExpr::FloatLit(_)
            | RefExpr::StringLit(_)
            | RefExpr::CharLit(_)
            | RefExpr::ListLit(_)
            | RefExpr::ObjectLit(_)
            | RefExpr::UnaryOp(_)
            | RefExpr::BinaryOp(_)
            | RefExpr::Statement(_)
            | RefExpr::Block(_)
            | RefExpr::Function(_)
            | RefExpr::Variable(_)
            | RefExpr::Return(_) => Err(self),
        }
    }
}

impl<'storage, 'a> Into<RefExpr<'storage, 'a>> for RefType<'storage, 'a> {
    fn into(self) -> RefExpr<'storage, 'a> {
        match self {
            RefType::Bool => RefExpr::Bool,
            RefType::UInt8 => RefExpr::UInt8,
            RefType::UInt16 => RefExpr::UInt16,
            RefType::UInt32 => RefExpr::UInt32,
            RefType::UInt64 => RefExpr::UInt64,
            RefType::UInt128 => RefExpr::UInt128,
            RefType::Int8 => RefExpr::Int8,
            RefType::Int16 => RefExpr::Int16,
            RefType::Int32 => RefExpr::Int32,
            RefType::Int64 => RefExpr::Int64,
            RefType::Int128 => RefExpr::Int128,
            RefType::Float8 => RefExpr::Float8,
            RefType::Float16 => RefExpr::Float16,
            RefType::Float32 => RefExpr::Float32,
            RefType::Float64 => RefExpr::Float64,
            RefType::Float128 => RefExpr::Float128,

            RefType::InferType => RefExpr::InferType,
            RefType::TupleType(x) => RefExpr::TupleType(x),
            RefType::ArrayType(x) => RefExpr::ArrayType(x),
            RefType::StructType(x) => RefExpr::StructType(x),
            RefType::FunctionType(x) => RefExpr::FunctionType(x),
        }
    }
}
