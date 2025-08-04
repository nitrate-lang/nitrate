use super::array_type::ArrayType;
use super::binary_op::BinaryOp;
use super::block::Block;
use super::character::CharLit;
use super::function::Function;
use super::function_type::FunctionType;
use super::list::ListLit;
use super::number::{FloatLit, IntegerLit};
use super::object::ObjectLit;
use super::refinement_type::RefinementType;
use super::returns::Return;
use super::slice_type::SliceType;
use super::statement::Statement;
use super::string::StringLit;
use super::struct_type::StructType;
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
    SliceType,
    StructType,
    FunctionType,

    Discard,

    IntegerLit,
    FloatLit,
    StringLit,
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
    SliceType,
    StructType,
    FunctionType,
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

    /* Compound Types */
    InferType,
    TypeName(&'a str),
    RefinementType(RefinementType<'a>),
    TupleType(TupleType<'a>),
    ArrayType(ArrayType<'a>),
    SliceType(SliceType<'a>),
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
pub(crate) enum TypeOwned<'a> {
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
    TypeName(&'a str),
    RefinementType(RefinementType<'a>),
    TupleType(TupleType<'a>),
    ArrayType(ArrayType<'a>),
    SliceType(SliceType<'a>),
    StructType(StructType<'a>),
    FunctionType(FunctionType<'a>),
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

    /* Compound Types */
    InferType,
    TypeName(&'a str),
    RefinementType(&'storage RefinementType<'a>),
    TupleType(&'storage TupleType<'a>),
    ArrayType(&'storage ArrayType<'a>),
    SliceType(&'storage SliceType<'a>),
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

    /* Compound Types */
    InferType,
    TypeName(&'a str),
    RefinementType(&'storage mut RefinementType<'a>),
    TupleType(&'storage mut TupleType<'a>),
    ArrayType(&'storage mut ArrayType<'a>),
    SliceType(&'storage mut SliceType<'a>),
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
pub enum TypeRef<'storage, 'a> {
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
    TypeName(&'a str),
    RefinementType(&'storage RefinementType<'a>),
    TupleType(&'storage TupleType<'a>),
    ArrayType(&'storage ArrayType<'a>),
    SliceType(&'storage SliceType<'a>),
    StructType(&'storage StructType<'a>),
    FunctionType(&'storage FunctionType<'a>),
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
            ExprKind::SliceType => Ok(TypeKind::SliceType),
            ExprKind::StructType => Ok(TypeKind::StructType),
            ExprKind::FunctionType => Ok(TypeKind::FunctionType),

            ExprKind::Discard
            | ExprKind::IntegerLit
            | ExprKind::FloatLit
            | ExprKind::StringLit
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
            TypeKind::SliceType => ExprKind::SliceType,
            TypeKind::StructType => ExprKind::StructType,
            TypeKind::FunctionType => ExprKind::FunctionType,
        }
    }
}

impl std::fmt::Display for ExprKind {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            ExprKind::Bool => write!(f, "Bool"),
            ExprKind::UInt8 => write!(f, "UInt8"),
            ExprKind::UInt16 => write!(f, "UInt16"),
            ExprKind::UInt32 => write!(f, "UInt32"),
            ExprKind::UInt64 => write!(f, "UInt64"),
            ExprKind::UInt128 => write!(f, "UInt128"),
            ExprKind::Int8 => write!(f, "Int8"),
            ExprKind::Int16 => write!(f, "Int16"),
            ExprKind::Int32 => write!(f, "Int32"),
            ExprKind::Int64 => write!(f, "Int64"),
            ExprKind::Int128 => write!(f, "Int128"),
            ExprKind::Float8 => write!(f, "Float8"),
            ExprKind::Float16 => write!(f, "Float16"),
            ExprKind::Float32 => write!(f, "Float32"),
            ExprKind::Float64 => write!(f, "Float64"),
            ExprKind::Float128 => write!(f, "Float128"),

            ExprKind::InferType => write!(f, "InferType"),
            ExprKind::TypeName => write!(f, "TypeName"),
            ExprKind::RefinementType => write!(f, "RefinementType"),
            ExprKind::TupleType => write!(f, "TupleType"),
            ExprKind::ArrayType => write!(f, "ArrayType"),
            ExprKind::SliceType => write!(f, "SliceType"),
            ExprKind::StructType => write!(f, "StructType"),
            ExprKind::FunctionType => write!(f, "FunctionType"),

            ExprKind::Discard => write!(f, "Discard"),

            ExprKind::IntegerLit => write!(f, "IntegerLit"),
            ExprKind::FloatLit => write!(f, "FloatLit"),
            ExprKind::StringLit => write!(f, "StringLit"),
            ExprKind::CharLit => write!(f, "CharLit"),
            ExprKind::ListLit => write!(f, "ListLit"),
            ExprKind::ObjectLit => write!(f, "ObjectLit"),

            ExprKind::UnaryOp => write!(f, "UnaryOp"),
            ExprKind::BinaryOp => write!(f, "BinaryOp"),
            ExprKind::Statement => write!(f, "Statement"),
            ExprKind::Block => write!(f, "Block"),

            ExprKind::Function => write!(f, "Function"),
            ExprKind::Variable => write!(f, "Variable"),

            ExprKind::Return => write!(f, "Return"),
        }
    }
}

impl std::fmt::Display for TypeKind {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            TypeKind::Bool => write!(f, "Bool"),
            TypeKind::UInt8 => write!(f, "UInt8"),
            TypeKind::UInt16 => write!(f, "UInt16"),
            TypeKind::UInt32 => write!(f, "UInt32"),
            TypeKind::UInt64 => write!(f, "UInt64"),
            TypeKind::UInt128 => write!(f, "UInt128"),
            TypeKind::Int8 => write!(f, "Int8"),
            TypeKind::Int16 => write!(f, "Int16"),
            TypeKind::Int32 => write!(f, "Int32"),
            TypeKind::Int64 => write!(f, "Int64"),
            TypeKind::Int128 => write!(f, "Int128"),
            TypeKind::Float8 => write!(f, "Float8"),
            TypeKind::Float16 => write!(f, "Float16"),
            TypeKind::Float32 => write!(f, "Float32"),
            TypeKind::Float64 => write!(f, "Float64"),
            TypeKind::Float128 => write!(f, "Float128"),

            TypeKind::InferType => write!(f, "InferType"),
            TypeKind::TypeName => write!(f, "TypeName"),
            TypeKind::RefinementType => write!(f, "RefinementType"),
            TypeKind::TupleType => write!(f, "TupleType"),
            TypeKind::ArrayType => write!(f, "ArrayType"),
            TypeKind::SliceType => write!(f, "SliceType"),
            TypeKind::StructType => write!(f, "StructType"),
            TypeKind::FunctionType => write!(f, "FunctionType"),
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
            ExprOwned::SliceType(x) => Ok(TypeOwned::SliceType(x)),
            ExprOwned::StructType(x) => Ok(TypeOwned::StructType(x)),
            ExprOwned::FunctionType(x) => Ok(TypeOwned::FunctionType(x)),

            ExprOwned::Discard
            | ExprOwned::IntegerLit(_)
            | ExprOwned::FloatLit(_)
            | ExprOwned::StringLit(_)
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
            TypeOwned::SliceType(x) => ExprOwned::SliceType(x),
            TypeOwned::StructType(x) => ExprOwned::StructType(x),
            TypeOwned::FunctionType(x) => ExprOwned::FunctionType(x),
        }
    }
}

impl<'storage, 'a> TryInto<TypeRef<'storage, 'a>> for ExprRef<'storage, 'a> {
    type Error = Self;

    fn try_into(self) -> Result<TypeRef<'storage, 'a>, Self::Error> {
        match self {
            ExprRef::Bool => Ok(TypeRef::Bool),
            ExprRef::UInt8 => Ok(TypeRef::UInt8),
            ExprRef::UInt16 => Ok(TypeRef::UInt16),
            ExprRef::UInt32 => Ok(TypeRef::UInt32),
            ExprRef::UInt64 => Ok(TypeRef::UInt64),
            ExprRef::UInt128 => Ok(TypeRef::UInt128),
            ExprRef::Int8 => Ok(TypeRef::Int8),
            ExprRef::Int16 => Ok(TypeRef::Int16),
            ExprRef::Int32 => Ok(TypeRef::Int32),
            ExprRef::Int64 => Ok(TypeRef::Int64),
            ExprRef::Int128 => Ok(TypeRef::Int128),
            ExprRef::Float8 => Ok(TypeRef::Float8),
            ExprRef::Float16 => Ok(TypeRef::Float16),
            ExprRef::Float32 => Ok(TypeRef::Float32),
            ExprRef::Float64 => Ok(TypeRef::Float64),
            ExprRef::Float128 => Ok(TypeRef::Float128),

            ExprRef::InferType => Ok(TypeRef::InferType),
            ExprRef::TypeName(x) => Ok(TypeRef::TypeName(x)),
            ExprRef::RefinementType(x) => Ok(TypeRef::RefinementType(x)),
            ExprRef::TupleType(x) => Ok(TypeRef::TupleType(x)),
            ExprRef::ArrayType(x) => Ok(TypeRef::ArrayType(x)),
            ExprRef::SliceType(x) => Ok(TypeRef::SliceType(x)),
            ExprRef::StructType(x) => Ok(TypeRef::StructType(x)),
            ExprRef::FunctionType(x) => Ok(TypeRef::FunctionType(x)),

            ExprRef::Discard
            | ExprRef::IntegerLit(_)
            | ExprRef::FloatLit(_)
            | ExprRef::StringLit(_)
            | ExprRef::CharLit(_)
            | ExprRef::ListLit(_)
            | ExprRef::ObjectLit(_)
            | ExprRef::UnaryOp(_)
            | ExprRef::BinaryOp(_)
            | ExprRef::Statement(_)
            | ExprRef::Block(_)
            | ExprRef::Function(_)
            | ExprRef::Variable(_)
            | ExprRef::Return(_) => Err(self),
        }
    }
}

impl<'storage, 'a> TryInto<TypeRef<'storage, 'a>> for ExprRefMut<'storage, 'a> {
    type Error = Self;

    fn try_into(self) -> Result<TypeRef<'storage, 'a>, Self::Error> {
        match self {
            ExprRefMut::Bool => Ok(TypeRef::Bool),
            ExprRefMut::UInt8 => Ok(TypeRef::UInt8),
            ExprRefMut::UInt16 => Ok(TypeRef::UInt16),
            ExprRefMut::UInt32 => Ok(TypeRef::UInt32),
            ExprRefMut::UInt64 => Ok(TypeRef::UInt64),
            ExprRefMut::UInt128 => Ok(TypeRef::UInt128),
            ExprRefMut::Int8 => Ok(TypeRef::Int8),
            ExprRefMut::Int16 => Ok(TypeRef::Int16),
            ExprRefMut::Int32 => Ok(TypeRef::Int32),
            ExprRefMut::Int64 => Ok(TypeRef::Int64),
            ExprRefMut::Int128 => Ok(TypeRef::Int128),
            ExprRefMut::Float8 => Ok(TypeRef::Float8),
            ExprRefMut::Float16 => Ok(TypeRef::Float16),
            ExprRefMut::Float32 => Ok(TypeRef::Float32),
            ExprRefMut::Float64 => Ok(TypeRef::Float64),
            ExprRefMut::Float128 => Ok(TypeRef::Float128),

            ExprRefMut::InferType => Ok(TypeRef::InferType),
            ExprRefMut::TypeName(x) => Ok(TypeRef::TypeName(x)),
            ExprRefMut::RefinementType(x) => Ok(TypeRef::RefinementType(x)),
            ExprRefMut::TupleType(x) => Ok(TypeRef::TupleType(x)),
            ExprRefMut::ArrayType(x) => Ok(TypeRef::ArrayType(x)),
            ExprRefMut::SliceType(x) => Ok(TypeRef::SliceType(x)),
            ExprRefMut::StructType(x) => Ok(TypeRef::StructType(x)),
            ExprRefMut::FunctionType(x) => Ok(TypeRef::FunctionType(x)),

            ExprRefMut::Discard
            | ExprRefMut::IntegerLit(_)
            | ExprRefMut::FloatLit(_)
            | ExprRefMut::StringLit(_)
            | ExprRefMut::CharLit(_)
            | ExprRefMut::ListLit(_)
            | ExprRefMut::ObjectLit(_)
            | ExprRefMut::UnaryOp(_)
            | ExprRefMut::BinaryOp(_)
            | ExprRefMut::Statement(_)
            | ExprRefMut::Block(_)
            | ExprRefMut::Function(_)
            | ExprRefMut::Variable(_)
            | ExprRefMut::Return(_) => Err(self),
        }
    }
}

impl<'storage, 'a> Into<ExprRef<'storage, 'a>> for TypeRef<'storage, 'a> {
    fn into(self) -> ExprRef<'storage, 'a> {
        match self {
            TypeRef::Bool => ExprRef::Bool,
            TypeRef::UInt8 => ExprRef::UInt8,
            TypeRef::UInt16 => ExprRef::UInt16,
            TypeRef::UInt32 => ExprRef::UInt32,
            TypeRef::UInt64 => ExprRef::UInt64,
            TypeRef::UInt128 => ExprRef::UInt128,
            TypeRef::Int8 => ExprRef::Int8,
            TypeRef::Int16 => ExprRef::Int16,
            TypeRef::Int32 => ExprRef::Int32,
            TypeRef::Int64 => ExprRef::Int64,
            TypeRef::Int128 => ExprRef::Int128,
            TypeRef::Float8 => ExprRef::Float8,
            TypeRef::Float16 => ExprRef::Float16,
            TypeRef::Float32 => ExprRef::Float32,
            TypeRef::Float64 => ExprRef::Float64,
            TypeRef::Float128 => ExprRef::Float128,

            TypeRef::InferType => ExprRef::InferType,
            TypeRef::TypeName(x) => ExprRef::TypeName(x),
            TypeRef::RefinementType(x) => ExprRef::RefinementType(x),
            TypeRef::TupleType(x) => ExprRef::TupleType(x),
            TypeRef::ArrayType(x) => ExprRef::ArrayType(x),
            TypeRef::SliceType(x) => ExprRef::SliceType(x),
            TypeRef::StructType(x) => ExprRef::StructType(x),
            TypeRef::FunctionType(x) => ExprRef::FunctionType(x),
        }
    }
}
