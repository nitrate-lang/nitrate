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
pub(crate) enum OwnedExpr<'a> {
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
pub(crate) enum OwnedType<'a> {
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

#[derive(Debug, Clone, Copy)]
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

#[derive(Debug)]
pub enum MutRefExpr<'storage, 'a> {
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
    TupleType(&'storage mut TupleType<'a>),
    ArrayType(&'storage mut ArrayType<'a>),
    StructType(&'storage mut StructType<'a>),
    FunctionType(&'storage mut FunctionType<'a>),

    Discard,

    /* Literal Expressions */
    IntegerLit(&'storage mut IntegerLit),
    FloatLit(&'storage mut FloatLit),
    StringLit(&'storage mut StringLit<'a>),
    CharLit(&'storage mut CharLit),
    ListLit(&'storage mut ListLit<'a>),
    ObjectLit(&'storage mut ObjectLit<'a>),

    /* Compound Expressions */
    UnaryOp(&'storage mut UnaryOp<'a>),
    BinaryOp(&'storage mut BinaryOp<'a>),
    Statement(&'storage mut Statement<'a>),
    Block(&'storage mut Block<'a>),

    /* Definition */
    Function(&'storage mut Function<'a>),
    Variable(&'storage mut Variable<'a>),

    /* Control Flow */
    Return(&'storage mut Return<'a>),
}

#[derive(Debug, Clone, Copy)]
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

#[derive(Debug)]
pub enum MutRefType<'storage, 'a> {
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
    TupleType(&'storage mut TupleType<'a>),
    ArrayType(&'storage mut ArrayType<'a>),
    StructType(&'storage mut StructType<'a>),
    FunctionType(&'storage mut FunctionType<'a>),
}

impl<'a> TryInto<OwnedType<'a>> for OwnedExpr<'a> {
    type Error = Self;

    fn try_into(self) -> Result<OwnedType<'a>, Self::Error> {
        match self {
            OwnedExpr::Bool => Ok(OwnedType::Bool),
            OwnedExpr::UInt8 => Ok(OwnedType::UInt8),
            OwnedExpr::UInt16 => Ok(OwnedType::UInt16),
            OwnedExpr::UInt32 => Ok(OwnedType::UInt32),
            OwnedExpr::UInt64 => Ok(OwnedType::UInt64),
            OwnedExpr::UInt128 => Ok(OwnedType::UInt128),
            OwnedExpr::Int8 => Ok(OwnedType::Int8),
            OwnedExpr::Int16 => Ok(OwnedType::Int16),
            OwnedExpr::Int32 => Ok(OwnedType::Int32),
            OwnedExpr::Int64 => Ok(OwnedType::Int64),
            OwnedExpr::Int128 => Ok(OwnedType::Int128),
            OwnedExpr::Float8 => Ok(OwnedType::Float8),
            OwnedExpr::Float16 => Ok(OwnedType::Float16),
            OwnedExpr::Float32 => Ok(OwnedType::Float32),
            OwnedExpr::Float64 => Ok(OwnedType::Float64),
            OwnedExpr::Float128 => Ok(OwnedType::Float128),

            OwnedExpr::InferType => Ok(OwnedType::InferType),
            OwnedExpr::TupleType(x) => Ok(OwnedType::TupleType(x)),
            OwnedExpr::ArrayType(x) => Ok(OwnedType::ArrayType(x)),
            OwnedExpr::StructType(x) => Ok(OwnedType::StructType(x)),
            OwnedExpr::FunctionType(x) => Ok(OwnedType::FunctionType(x)),

            OwnedExpr::Discard
            | OwnedExpr::IntegerLit(_)
            | OwnedExpr::FloatLit(_)
            | OwnedExpr::StringLit(_)
            | OwnedExpr::CharLit(_)
            | OwnedExpr::ListLit(_)
            | OwnedExpr::ObjectLit(_)
            | OwnedExpr::UnaryOp(_)
            | OwnedExpr::BinaryOp(_)
            | OwnedExpr::Statement(_)
            | OwnedExpr::Block(_)
            | OwnedExpr::Function(_)
            | OwnedExpr::Variable(_)
            | OwnedExpr::Return(_) => Err(self),
        }
    }
}

impl<'a> Into<OwnedExpr<'a>> for OwnedType<'a> {
    fn into(self) -> OwnedExpr<'a> {
        match self {
            OwnedType::Bool => OwnedExpr::Bool,
            OwnedType::UInt8 => OwnedExpr::UInt8,
            OwnedType::UInt16 => OwnedExpr::UInt16,
            OwnedType::UInt32 => OwnedExpr::UInt32,
            OwnedType::UInt64 => OwnedExpr::UInt64,
            OwnedType::UInt128 => OwnedExpr::UInt128,
            OwnedType::Int8 => OwnedExpr::Int8,
            OwnedType::Int16 => OwnedExpr::Int16,
            OwnedType::Int32 => OwnedExpr::Int32,
            OwnedType::Int64 => OwnedExpr::Int64,
            OwnedType::Int128 => OwnedExpr::Int128,
            OwnedType::Float8 => OwnedExpr::Float8,
            OwnedType::Float16 => OwnedExpr::Float16,
            OwnedType::Float32 => OwnedExpr::Float32,
            OwnedType::Float64 => OwnedExpr::Float64,
            OwnedType::Float128 => OwnedExpr::Float128,

            OwnedType::InferType => OwnedExpr::InferType,
            OwnedType::TupleType(x) => OwnedExpr::TupleType(x),
            OwnedType::ArrayType(x) => OwnedExpr::ArrayType(x),
            OwnedType::StructType(x) => OwnedExpr::StructType(x),
            OwnedType::FunctionType(x) => OwnedExpr::FunctionType(x),
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

impl<'storage, 'a> TryInto<MutRefType<'storage, 'a>> for MutRefExpr<'storage, 'a> {
    type Error = Self;

    fn try_into(self) -> Result<MutRefType<'storage, 'a>, Self::Error> {
        match self {
            MutRefExpr::Bool => Ok(MutRefType::Bool),
            MutRefExpr::UInt8 => Ok(MutRefType::UInt8),
            MutRefExpr::UInt16 => Ok(MutRefType::UInt16),
            MutRefExpr::UInt32 => Ok(MutRefType::UInt32),
            MutRefExpr::UInt64 => Ok(MutRefType::UInt64),
            MutRefExpr::UInt128 => Ok(MutRefType::UInt128),
            MutRefExpr::Int8 => Ok(MutRefType::Int8),
            MutRefExpr::Int16 => Ok(MutRefType::Int16),
            MutRefExpr::Int32 => Ok(MutRefType::Int32),
            MutRefExpr::Int64 => Ok(MutRefType::Int64),
            MutRefExpr::Int128 => Ok(MutRefType::Int128),
            MutRefExpr::Float8 => Ok(MutRefType::Float8),
            MutRefExpr::Float16 => Ok(MutRefType::Float16),
            MutRefExpr::Float32 => Ok(MutRefType::Float32),
            MutRefExpr::Float64 => Ok(MutRefType::Float64),
            MutRefExpr::Float128 => Ok(MutRefType::Float128),

            MutRefExpr::InferType => Ok(MutRefType::InferType),
            MutRefExpr::TupleType(x) => Ok(MutRefType::TupleType(x)),
            MutRefExpr::ArrayType(x) => Ok(MutRefType::ArrayType(x)),
            MutRefExpr::StructType(x) => Ok(MutRefType::StructType(x)),
            MutRefExpr::FunctionType(x) => Ok(MutRefType::FunctionType(x)),

            MutRefExpr::Discard
            | MutRefExpr::IntegerLit(_)
            | MutRefExpr::FloatLit(_)
            | MutRefExpr::StringLit(_)
            | MutRefExpr::CharLit(_)
            | MutRefExpr::ListLit(_)
            | MutRefExpr::ObjectLit(_)
            | MutRefExpr::UnaryOp(_)
            | MutRefExpr::BinaryOp(_)
            | MutRefExpr::Statement(_)
            | MutRefExpr::Block(_)
            | MutRefExpr::Function(_)
            | MutRefExpr::Variable(_)
            | MutRefExpr::Return(_) => Err(self),
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

impl<'storage, 'a> Into<MutRefExpr<'storage, 'a>> for MutRefType<'storage, 'a> {
    fn into(self) -> MutRefExpr<'storage, 'a> {
        match self {
            MutRefType::Bool => MutRefExpr::Bool,
            MutRefType::UInt8 => MutRefExpr::UInt8,
            MutRefType::UInt16 => MutRefExpr::UInt16,
            MutRefType::UInt32 => MutRefExpr::UInt32,
            MutRefType::UInt64 => MutRefExpr::UInt64,
            MutRefType::UInt128 => MutRefExpr::UInt128,
            MutRefType::Int8 => MutRefExpr::Int8,
            MutRefType::Int16 => MutRefExpr::Int16,
            MutRefType::Int32 => MutRefExpr::Int32,
            MutRefType::Int64 => MutRefExpr::Int64,
            MutRefType::Int128 => MutRefExpr::Int128,
            MutRefType::Float8 => MutRefExpr::Float8,
            MutRefType::Float16 => MutRefExpr::Float16,
            MutRefType::Float32 => MutRefExpr::Float32,
            MutRefType::Float64 => MutRefExpr::Float64,
            MutRefType::Float128 => MutRefExpr::Float128,

            MutRefType::InferType => MutRefExpr::InferType,
            MutRefType::TupleType(x) => MutRefExpr::TupleType(x),
            MutRefType::ArrayType(x) => MutRefExpr::ArrayType(x),
            MutRefType::StructType(x) => MutRefExpr::StructType(x),
            MutRefType::FunctionType(x) => MutRefExpr::FunctionType(x),
        }
    }
}
