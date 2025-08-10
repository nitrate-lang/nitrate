use crate::lexer::{BinaryData, StringData};

use super::array_type::ArrayType;
use super::binary_op::BinaryOp;
use super::block::Block;
use super::function::Function;
use super::function_type::FunctionType;
use super::generic_type::GenericType;
use super::list::ListLit;
use super::map_type::MapType;
use super::number::IntegerLit;
use super::object::ObjectLit;
use super::opaque_type::OpaqueType;
use super::reference::{ManagedRefType, UnmanagedRefType};
use super::refinement_type::RefinementType;
use super::returns::Return;
use super::slice_type::SliceType;
use super::statement::Statement;
use super::tuple_type::TupleType;
use super::unary_op::UnaryOp;
use super::variable::Variable;

#[derive(Debug, PartialEq, Eq, Clone, Copy)]
pub enum ExprKind {
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

    InferType,
    TypeName,
    RefinementType,
    TupleType,
    ArrayType,
    MapType,
    SliceType,
    FunctionType,
    ManagedRefType,
    UnmanagedRefType,
    GenericType,
    OpaqueType,

    Discard,

    IntegerLit,
    FloatLit,
    StringLit,
    BinaryLit,
    CharLit,
    ListLit,
    ObjectLit,

    UnaryOp,
    BinaryOp,
    Statement,
    Block,

    Function,
    Variable,

    Return,
}

#[derive(Debug, PartialEq, Eq, Clone, Copy)]
pub(crate) enum TypeKind {
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

    InferType,
    TypeName,
    RefinementType,
    TupleType,
    ArrayType,
    MapType,
    SliceType,
    FunctionType,
    ManagedRefType,
    UnmanagedRefType,
    GenericType,
    OpaqueType,
}

#[derive(Debug, Clone)]
pub(crate) enum ExprOwned<'a> {
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

    /* Non-primitive Types */
    InferType,
    TypeName(&'a str),
    RefinementType(RefinementType<'a>),
    TupleType(TupleType<'a>),
    ArrayType(ArrayType<'a>),
    MapType(MapType<'a>),
    SliceType(SliceType<'a>),
    FunctionType(FunctionType<'a>),
    ManagedRefType(ManagedRefType<'a>),
    UnmanagedRefType(UnmanagedRefType<'a>),
    GenericType(GenericType<'a>),
    OpaqueType(OpaqueType<'a>),

    Discard,

    /* Literal Expressions */
    IntegerLit(IntegerLit),
    FloatLit(f64),
    StringLit(StringData<'a>),
    BinaryLit(BinaryData<'a>),
    CharLit(char),
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

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum TypeOwned<'a> {
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

    /* Non-primitive  Types */
    InferType,
    TypeName(&'a str),
    RefinementType(RefinementType<'a>),
    TupleType(TupleType<'a>),
    ArrayType(ArrayType<'a>),
    MapType(MapType<'a>),
    SliceType(SliceType<'a>),
    FunctionType(FunctionType<'a>),
    ManagedRefType(ManagedRefType<'a>),
    UnmanagedRefType(UnmanagedRefType<'a>),
    GenericType(GenericType<'a>),
    OpaqueType(OpaqueType<'a>),
}

#[derive(Debug, Clone, Copy)]
pub enum ExprRef<'storage, 'a> {
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

    /* Non-primitive  Types */
    InferType,
    TypeName(&'a str),
    RefinementType(&'storage RefinementType<'a>),
    TupleType(&'storage TupleType<'a>),
    ArrayType(&'storage ArrayType<'a>),
    MapType(&'storage MapType<'a>),
    SliceType(&'storage SliceType<'a>),
    FunctionType(&'storage FunctionType<'a>),
    ManagedRefType(&'storage ManagedRefType<'a>),
    UnmanagedRefType(&'storage UnmanagedRefType<'a>),
    GenericType(&'storage GenericType<'a>),
    OpaqueType(&'storage OpaqueType<'a>),

    Discard,

    /* Literal Expressions */
    IntegerLit(&'storage IntegerLit),
    FloatLit(f64),
    StringLit(&'storage StringData<'a>),
    BinaryLit(&'storage BinaryData<'a>),
    CharLit(char),
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
pub enum ExprRefMut<'storage, 'a> {
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

    /* Non-primitive  Types */
    InferType,
    TypeName(&'a str),
    RefinementType(&'storage RefinementType<'a>),
    TupleType(&'storage TupleType<'a>),
    ArrayType(&'storage ArrayType<'a>),
    MapType(&'storage MapType<'a>),
    SliceType(&'storage SliceType<'a>),
    FunctionType(&'storage FunctionType<'a>),
    ManagedRefType(&'storage ManagedRefType<'a>),
    UnmanagedRefType(&'storage UnmanagedRefType<'a>),
    GenericType(&'storage GenericType<'a>),
    OpaqueType(&'storage OpaqueType<'a>),

    Discard,

    /* Literal Expressions */
    IntegerLit(&'storage IntegerLit),
    FloatLit(f64),
    StringLit(&'storage StringData<'a>),
    BinaryLit(&'storage BinaryData<'a>),
    CharLit(char),
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

impl TryInto<TypeKind> for ExprKind {
    type Error = ();

    fn try_into(self) -> Result<TypeKind, Self::Error> {
        match self {
            ExprKind::Bool => Ok(TypeKind::Bool),
            ExprKind::UInt8 => Ok(TypeKind::UInt8),
            ExprKind::UInt16 => Ok(TypeKind::UInt16),
            ExprKind::UInt32 => Ok(TypeKind::UInt32),
            ExprKind::UInt64 => Ok(TypeKind::UInt64),
            ExprKind::UInt128 => Ok(TypeKind::UInt128),
            ExprKind::Int8 => Ok(TypeKind::Int8),
            ExprKind::Int16 => Ok(TypeKind::Int16),
            ExprKind::Int32 => Ok(TypeKind::Int32),
            ExprKind::Int64 => Ok(TypeKind::Int64),
            ExprKind::Int128 => Ok(TypeKind::Int128),
            ExprKind::Float8 => Ok(TypeKind::Float8),
            ExprKind::Float16 => Ok(TypeKind::Float16),
            ExprKind::Float32 => Ok(TypeKind::Float32),
            ExprKind::Float64 => Ok(TypeKind::Float64),
            ExprKind::Float128 => Ok(TypeKind::Float128),

            ExprKind::InferType => Ok(TypeKind::InferType),
            ExprKind::TypeName => Ok(TypeKind::TypeName),
            ExprKind::RefinementType => Ok(TypeKind::RefinementType),
            ExprKind::TupleType => Ok(TypeKind::TupleType),
            ExprKind::ArrayType => Ok(TypeKind::ArrayType),
            ExprKind::MapType => Ok(TypeKind::MapType),
            ExprKind::SliceType => Ok(TypeKind::SliceType),
            ExprKind::FunctionType => Ok(TypeKind::FunctionType),
            ExprKind::ManagedRefType => Ok(TypeKind::ManagedRefType),
            ExprKind::UnmanagedRefType => Ok(TypeKind::UnmanagedRefType),
            ExprKind::GenericType => Ok(TypeKind::GenericType),
            ExprKind::OpaqueType => Ok(TypeKind::OpaqueType),

            ExprKind::Discard
            | ExprKind::IntegerLit
            | ExprKind::FloatLit
            | ExprKind::StringLit
            | ExprKind::BinaryLit
            | ExprKind::CharLit
            | ExprKind::ListLit
            | ExprKind::ObjectLit
            | ExprKind::UnaryOp
            | ExprKind::BinaryOp
            | ExprKind::Statement
            | ExprKind::Block
            | ExprKind::Function
            | ExprKind::Variable
            | ExprKind::Return => Err(()),
        }
    }
}

impl Into<ExprKind> for TypeKind {
    fn into(self) -> ExprKind {
        match self {
            TypeKind::Bool => ExprKind::Bool,
            TypeKind::UInt8 => ExprKind::UInt8,
            TypeKind::UInt16 => ExprKind::UInt16,
            TypeKind::UInt32 => ExprKind::UInt32,
            TypeKind::UInt64 => ExprKind::UInt64,
            TypeKind::UInt128 => ExprKind::UInt128,
            TypeKind::Int8 => ExprKind::Int8,
            TypeKind::Int16 => ExprKind::Int16,
            TypeKind::Int32 => ExprKind::Int32,
            TypeKind::Int64 => ExprKind::Int64,
            TypeKind::Int128 => ExprKind::Int128,
            TypeKind::Float8 => ExprKind::Float8,
            TypeKind::Float16 => ExprKind::Float16,
            TypeKind::Float32 => ExprKind::Float32,
            TypeKind::Float64 => ExprKind::Float64,
            TypeKind::Float128 => ExprKind::Float128,

            TypeKind::InferType => ExprKind::InferType,
            TypeKind::TypeName => ExprKind::TypeName,
            TypeKind::RefinementType => ExprKind::RefinementType,
            TypeKind::TupleType => ExprKind::TupleType,
            TypeKind::ArrayType => ExprKind::ArrayType,
            TypeKind::MapType => ExprKind::MapType,
            TypeKind::SliceType => ExprKind::SliceType,
            TypeKind::FunctionType => ExprKind::FunctionType,
            TypeKind::ManagedRefType => ExprKind::ManagedRefType,
            TypeKind::UnmanagedRefType => ExprKind::UnmanagedRefType,
            TypeKind::GenericType => ExprKind::GenericType,
            TypeKind::OpaqueType => ExprKind::OpaqueType,
        }
    }
}

impl<'a> TryInto<TypeOwned<'a>> for ExprOwned<'a> {
    type Error = Self;

    fn try_into(self) -> Result<TypeOwned<'a>, Self::Error> {
        match self {
            ExprOwned::Bool => Ok(TypeOwned::Bool),
            ExprOwned::UInt8 => Ok(TypeOwned::UInt8),
            ExprOwned::UInt16 => Ok(TypeOwned::UInt16),
            ExprOwned::UInt32 => Ok(TypeOwned::UInt32),
            ExprOwned::UInt64 => Ok(TypeOwned::UInt64),
            ExprOwned::UInt128 => Ok(TypeOwned::UInt128),
            ExprOwned::Int8 => Ok(TypeOwned::Int8),
            ExprOwned::Int16 => Ok(TypeOwned::Int16),
            ExprOwned::Int32 => Ok(TypeOwned::Int32),
            ExprOwned::Int64 => Ok(TypeOwned::Int64),
            ExprOwned::Int128 => Ok(TypeOwned::Int128),
            ExprOwned::Float8 => Ok(TypeOwned::Float8),
            ExprOwned::Float16 => Ok(TypeOwned::Float16),
            ExprOwned::Float32 => Ok(TypeOwned::Float32),
            ExprOwned::Float64 => Ok(TypeOwned::Float64),
            ExprOwned::Float128 => Ok(TypeOwned::Float128),

            ExprOwned::InferType => Ok(TypeOwned::InferType),
            ExprOwned::TypeName(x) => Ok(TypeOwned::TypeName(x)),
            ExprOwned::RefinementType(x) => Ok(TypeOwned::RefinementType(x)),
            ExprOwned::TupleType(x) => Ok(TypeOwned::TupleType(x)),
            ExprOwned::ArrayType(x) => Ok(TypeOwned::ArrayType(x)),
            ExprOwned::MapType(x) => Ok(TypeOwned::MapType(x)),
            ExprOwned::SliceType(x) => Ok(TypeOwned::SliceType(x)),
            ExprOwned::FunctionType(x) => Ok(TypeOwned::FunctionType(x)),
            ExprOwned::ManagedRefType(x) => Ok(TypeOwned::ManagedRefType(x)),
            ExprOwned::UnmanagedRefType(x) => Ok(TypeOwned::UnmanagedRefType(x)),
            ExprOwned::GenericType(x) => Ok(TypeOwned::GenericType(x)),
            ExprOwned::OpaqueType(x) => Ok(TypeOwned::OpaqueType(x)),

            ExprOwned::Discard
            | ExprOwned::IntegerLit(_)
            | ExprOwned::FloatLit(_)
            | ExprOwned::StringLit(_)
            | ExprOwned::BinaryLit(_)
            | ExprOwned::CharLit(_)
            | ExprOwned::ListLit(_)
            | ExprOwned::ObjectLit(_)
            | ExprOwned::UnaryOp(_)
            | ExprOwned::BinaryOp(_)
            | ExprOwned::Statement(_)
            | ExprOwned::Block(_)
            | ExprOwned::Function(_)
            | ExprOwned::Variable(_)
            | ExprOwned::Return(_) => Err(self),
        }
    }
}

impl<'a> Into<ExprOwned<'a>> for TypeOwned<'a> {
    fn into(self) -> ExprOwned<'a> {
        match self {
            TypeOwned::Bool => ExprOwned::Bool,
            TypeOwned::UInt8 => ExprOwned::UInt8,
            TypeOwned::UInt16 => ExprOwned::UInt16,
            TypeOwned::UInt32 => ExprOwned::UInt32,
            TypeOwned::UInt64 => ExprOwned::UInt64,
            TypeOwned::UInt128 => ExprOwned::UInt128,
            TypeOwned::Int8 => ExprOwned::Int8,
            TypeOwned::Int16 => ExprOwned::Int16,
            TypeOwned::Int32 => ExprOwned::Int32,
            TypeOwned::Int64 => ExprOwned::Int64,
            TypeOwned::Int128 => ExprOwned::Int128,
            TypeOwned::Float8 => ExprOwned::Float8,
            TypeOwned::Float16 => ExprOwned::Float16,
            TypeOwned::Float32 => ExprOwned::Float32,
            TypeOwned::Float64 => ExprOwned::Float64,
            TypeOwned::Float128 => ExprOwned::Float128,

            TypeOwned::InferType => ExprOwned::InferType,
            TypeOwned::TypeName(x) => ExprOwned::TypeName(x),
            TypeOwned::RefinementType(x) => ExprOwned::RefinementType(x),
            TypeOwned::TupleType(x) => ExprOwned::TupleType(x),
            TypeOwned::ArrayType(x) => ExprOwned::ArrayType(x),
            TypeOwned::MapType(x) => ExprOwned::MapType(x),
            TypeOwned::SliceType(x) => ExprOwned::SliceType(x),
            TypeOwned::FunctionType(x) => ExprOwned::FunctionType(x),
            TypeOwned::ManagedRefType(x) => ExprOwned::ManagedRefType(x),
            TypeOwned::UnmanagedRefType(x) => ExprOwned::UnmanagedRefType(x),
            TypeOwned::GenericType(x) => ExprOwned::GenericType(x),
            TypeOwned::OpaqueType(x) => ExprOwned::OpaqueType(x),
        }
    }
}

impl<'storage, 'a> Into<ExprRef<'storage, 'a>> for &'storage TypeOwned<'a> {
    fn into(self) -> ExprRef<'storage, 'a> {
        match self {
            TypeOwned::Bool => ExprRef::Bool,
            TypeOwned::UInt8 => ExprRef::UInt8,
            TypeOwned::UInt16 => ExprRef::UInt16,
            TypeOwned::UInt32 => ExprRef::UInt32,
            TypeOwned::UInt64 => ExprRef::UInt64,
            TypeOwned::UInt128 => ExprRef::UInt128,
            TypeOwned::Int8 => ExprRef::Int8,
            TypeOwned::Int16 => ExprRef::Int16,
            TypeOwned::Int32 => ExprRef::Int32,
            TypeOwned::Int64 => ExprRef::Int64,
            TypeOwned::Int128 => ExprRef::Int128,
            TypeOwned::Float8 => ExprRef::Float8,
            TypeOwned::Float16 => ExprRef::Float16,
            TypeOwned::Float32 => ExprRef::Float32,
            TypeOwned::Float64 => ExprRef::Float64,
            TypeOwned::Float128 => ExprRef::Float128,

            TypeOwned::InferType => ExprRef::InferType,
            TypeOwned::TypeName(x) => ExprRef::TypeName(x),
            TypeOwned::RefinementType(x) => ExprRef::RefinementType(x),
            TypeOwned::TupleType(x) => ExprRef::TupleType(x),
            TypeOwned::ArrayType(x) => ExprRef::ArrayType(x),
            TypeOwned::MapType(x) => ExprRef::MapType(x),
            TypeOwned::SliceType(x) => ExprRef::SliceType(x),
            TypeOwned::FunctionType(x) => ExprRef::FunctionType(x),
            TypeOwned::ManagedRefType(x) => ExprRef::ManagedRefType(x),
            TypeOwned::UnmanagedRefType(x) => ExprRef::UnmanagedRefType(x),
            TypeOwned::GenericType(x) => ExprRef::GenericType(x),
            TypeOwned::OpaqueType(x) => ExprRef::OpaqueType(x),
        }
    }
}

impl<'storage, 'a> Into<ExprRefMut<'storage, 'a>> for &'storage TypeOwned<'a> {
    fn into(self) -> ExprRefMut<'storage, 'a> {
        match self {
            TypeOwned::Bool => ExprRefMut::Bool,
            TypeOwned::UInt8 => ExprRefMut::UInt8,
            TypeOwned::UInt16 => ExprRefMut::UInt16,
            TypeOwned::UInt32 => ExprRefMut::UInt32,
            TypeOwned::UInt64 => ExprRefMut::UInt64,
            TypeOwned::UInt128 => ExprRefMut::UInt128,
            TypeOwned::Int8 => ExprRefMut::Int8,
            TypeOwned::Int16 => ExprRefMut::Int16,
            TypeOwned::Int32 => ExprRefMut::Int32,
            TypeOwned::Int64 => ExprRefMut::Int64,
            TypeOwned::Int128 => ExprRefMut::Int128,
            TypeOwned::Float8 => ExprRefMut::Float8,
            TypeOwned::Float16 => ExprRefMut::Float16,
            TypeOwned::Float32 => ExprRefMut::Float32,
            TypeOwned::Float64 => ExprRefMut::Float64,
            TypeOwned::Float128 => ExprRefMut::Float128,

            TypeOwned::InferType => ExprRefMut::InferType,
            TypeOwned::TypeName(x) => ExprRefMut::TypeName(x),
            TypeOwned::RefinementType(x) => ExprRefMut::RefinementType(x),
            TypeOwned::TupleType(x) => ExprRefMut::TupleType(x),
            TypeOwned::ArrayType(x) => ExprRefMut::ArrayType(x),
            TypeOwned::MapType(x) => ExprRefMut::MapType(x),
            TypeOwned::SliceType(x) => ExprRefMut::SliceType(x),
            TypeOwned::FunctionType(x) => ExprRefMut::FunctionType(x),
            TypeOwned::ManagedRefType(x) => ExprRefMut::ManagedRefType(x),
            TypeOwned::UnmanagedRefType(x) => ExprRefMut::UnmanagedRefType(x),
            TypeOwned::GenericType(x) => ExprRefMut::GenericType(x),
            TypeOwned::OpaqueType(x) => ExprRefMut::OpaqueType(x),
        }
    }
}
