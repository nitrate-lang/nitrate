use super::array_type::ArrayType;
use super::bin_expr::BinExpr;
use super::block::Block;
use super::control_flow::{
    Assert, Await, Break, Continue, DoWhileLoop, ForEach, If, Return, Switch, WhileLoop,
};
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
use super::slice_type::SliceType;
use super::statement::Statement;
use super::tuple_type::TupleType;
use super::unary_expr::UnaryExpr;
use super::variable::Variable;
use crate::lexer::{BStringData, StringData};

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
    BStringLit,
    CharLit,
    ListLit,
    ObjectLit,

    UnaryExpr,
    BinExpr,
    Statement,
    Block,

    Function,
    Variable,
    Identifier,

    If,
    WhileLoop,
    DoWhileLoop,
    Switch,
    Break,
    Continue,
    Return,
    ForEach,
    Await,
    Assert,
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

    IntegerLit(IntegerLit),
    FloatLit(f64),
    StringLit(StringData<'a>),
    BStringLit(BStringData<'a>),
    CharLit(char),
    ListLit(ListLit<'a>),
    ObjectLit(ObjectLit<'a>),

    UnaryExpr(UnaryExpr<'a>),
    BinExpr(BinExpr<'a>),
    Statement(Statement<'a>),
    Block(Block<'a>),

    Function(Function<'a>),
    Variable(Variable<'a>),
    Identifier(&'a str),

    If(If<'a>),
    WhileLoop(WhileLoop<'a>),
    DoWhileLoop(DoWhileLoop<'a>),
    Switch(Switch<'a>),
    Break(Break<'a>),
    Continue(Continue<'a>),
    Return(Return<'a>),
    ForEach(ForEach<'a>),
    Await(Await<'a>),
    Assert(Assert<'a>),
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum TypeOwned<'a> {
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

    IntegerLit(&'storage IntegerLit),
    FloatLit(f64),
    StringLit(&'storage StringData<'a>),
    BStringLit(&'storage BStringData<'a>),
    CharLit(char),
    ListLit(&'storage ListLit<'a>),
    ObjectLit(&'storage ObjectLit<'a>),

    UnaryExpr(&'storage UnaryExpr<'a>),
    BinExpr(&'storage BinExpr<'a>),
    Statement(&'storage Statement<'a>),
    Block(&'storage Block<'a>),

    Function(&'storage Function<'a>),
    Variable(&'storage Variable<'a>),
    Identifier(&'a str),

    If(&'storage If<'a>),
    WhileLoop(&'storage WhileLoop<'a>),
    DoWhileLoop(&'storage DoWhileLoop<'a>),
    Switch(&'storage Switch<'a>),
    Break(&'storage Break<'a>),
    Continue(&'storage Continue<'a>),
    Return(&'storage Return<'a>),
    ForEach(&'storage ForEach<'a>),
    Await(&'storage Await<'a>),
    Assert(&'storage Assert<'a>),
}

#[derive(Debug)]
pub enum ExprRefMut<'storage, 'a> {
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

    IntegerLit(&'storage IntegerLit),
    FloatLit(f64),
    StringLit(&'storage StringData<'a>),
    BStringLit(&'storage BStringData<'a>),
    CharLit(char),
    ListLit(&'storage mut ListLit<'a>),
    ObjectLit(&'storage mut ObjectLit<'a>),

    UnaryExpr(&'storage mut UnaryExpr<'a>),
    BinExpr(&'storage mut BinExpr<'a>),
    Statement(&'storage mut Statement<'a>),
    Block(&'storage mut Block<'a>),

    Function(&'storage mut Function<'a>),
    Variable(&'storage mut Variable<'a>),
    Identifier(&'a str),

    If(&'storage mut If<'a>),
    WhileLoop(&'storage mut WhileLoop<'a>),
    DoWhileLoop(&'storage mut DoWhileLoop<'a>),
    Switch(&'storage mut Switch<'a>),
    Break(&'storage mut Break<'a>),
    Continue(&'storage mut Continue<'a>),
    Return(&'storage mut Return<'a>),
    ForEach(&'storage mut ForEach<'a>),
    Await(&'storage mut Await<'a>),
    Assert(&'storage mut Assert<'a>),
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
            | ExprKind::BStringLit
            | ExprKind::CharLit
            | ExprKind::ListLit
            | ExprKind::ObjectLit
            | ExprKind::UnaryExpr
            | ExprKind::BinExpr
            | ExprKind::Statement
            | ExprKind::Block
            | ExprKind::Function
            | ExprKind::Variable
            | ExprKind::Identifier
            | ExprKind::If
            | ExprKind::WhileLoop
            | ExprKind::DoWhileLoop
            | ExprKind::Switch
            | ExprKind::Break
            | ExprKind::Continue
            | ExprKind::Return
            | ExprKind::ForEach
            | ExprKind::Await
            | ExprKind::Assert => Err(()),
        }
    }
}

impl TryFrom<u8> for ExprKind {
    type Error = ();

    fn try_from(value: u8) -> Result<Self, Self::Error> {
        match value {
            x if x == ExprKind::Bool as u8 => Ok(ExprKind::Bool),
            x if x == ExprKind::UInt8 as u8 => Ok(ExprKind::UInt8),
            x if x == ExprKind::UInt16 as u8 => Ok(ExprKind::UInt16),
            x if x == ExprKind::UInt32 as u8 => Ok(ExprKind::UInt32),
            x if x == ExprKind::UInt64 as u8 => Ok(ExprKind::UInt64),
            x if x == ExprKind::UInt128 as u8 => Ok(ExprKind::UInt128),
            x if x == ExprKind::Int8 as u8 => Ok(ExprKind::Int8),
            x if x == ExprKind::Int16 as u8 => Ok(ExprKind::Int16),
            x if x == ExprKind::Int32 as u8 => Ok(ExprKind::Int32),
            x if x == ExprKind::Int64 as u8 => Ok(ExprKind::Int64),
            x if x == ExprKind::Int128 as u8 => Ok(ExprKind::Int128),
            x if x == ExprKind::Float8 as u8 => Ok(ExprKind::Float8),
            x if x == ExprKind::Float16 as u8 => Ok(ExprKind::Float16),
            x if x == ExprKind::Float32 as u8 => Ok(ExprKind::Float32),
            x if x == ExprKind::Float64 as u8 => Ok(ExprKind::Float64),
            x if x == ExprKind::Float128 as u8 => Ok(ExprKind::Float128),

            x if x == ExprKind::InferType as u8 => Ok(ExprKind::InferType),
            x if x == ExprKind::TypeName as u8 => Ok(ExprKind::TypeName),
            x if x == ExprKind::RefinementType as u8 => Ok(ExprKind::RefinementType),
            x if x == ExprKind::TupleType as u8 => Ok(ExprKind::TupleType),
            x if x == ExprKind::ArrayType as u8 => Ok(ExprKind::ArrayType),
            x if x == ExprKind::MapType as u8 => Ok(ExprKind::MapType),
            x if x == ExprKind::SliceType as u8 => Ok(ExprKind::SliceType),
            x if x == ExprKind::FunctionType as u8 => Ok(ExprKind::FunctionType),
            x if x == ExprKind::ManagedRefType as u8 => Ok(ExprKind::ManagedRefType),
            x if x == ExprKind::UnmanagedRefType as u8 => Ok(ExprKind::UnmanagedRefType),
            x if x == ExprKind::GenericType as u8 => Ok(ExprKind::GenericType),
            x if x == ExprKind::OpaqueType as u8 => Ok(ExprKind::OpaqueType),

            x if x == ExprKind::Discard as u8 => Ok(ExprKind::Discard),

            x if x == ExprKind::IntegerLit as u8 => Ok(ExprKind::IntegerLit),
            x if x == ExprKind::FloatLit as u8 => Ok(ExprKind::FloatLit),
            x if x == ExprKind::StringLit as u8 => Ok(ExprKind::StringLit),
            x if x == ExprKind::BStringLit as u8 => Ok(ExprKind::BStringLit),
            x if x == ExprKind::CharLit as u8 => Ok(ExprKind::CharLit),
            x if x == ExprKind::ListLit as u8 => Ok(ExprKind::ListLit),
            x if x == ExprKind::ObjectLit as u8 => Ok(ExprKind::ObjectLit),

            x if x == ExprKind::UnaryExpr as u8 => Ok(ExprKind::UnaryExpr),
            x if x == ExprKind::BinExpr as u8 => Ok(ExprKind::BinExpr),
            x if x == ExprKind::Statement as u8 => Ok(ExprKind::Statement),
            x if x == ExprKind::Block as u8 => Ok(ExprKind::Block),

            x if x == ExprKind::Function as u8 => Ok(ExprKind::Function),
            x if x == ExprKind::Variable as u8 => Ok(ExprKind::Variable),
            x if x == ExprKind::Identifier as u8 => Ok(ExprKind::Identifier),

            x if x == ExprKind::If as u8 => Ok(ExprKind::If),
            x if x == ExprKind::WhileLoop as u8 => Ok(ExprKind::WhileLoop),
            x if x == ExprKind::DoWhileLoop as u8 => Ok(ExprKind::DoWhileLoop),
            x if x == ExprKind::Switch as u8 => Ok(ExprKind::Switch),
            x if x == ExprKind::Break as u8 => Ok(ExprKind::Break),
            x if x == ExprKind::Continue as u8 => Ok(ExprKind::Continue),
            x if x == ExprKind::Return as u8 => Ok(ExprKind::Return),
            x if x == ExprKind::ForEach as u8 => Ok(ExprKind::ForEach),
            x if x == ExprKind::Await as u8 => Ok(ExprKind::Await),
            x if x == ExprKind::Assert as u8 => Ok(ExprKind::Assert),

            _ => Err(()),
        }
    }
}

impl From<TypeKind> for ExprKind {
    fn from(val: TypeKind) -> ExprKind {
        match val {
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
            | ExprOwned::BStringLit(_)
            | ExprOwned::CharLit(_)
            | ExprOwned::ListLit(_)
            | ExprOwned::ObjectLit(_)
            | ExprOwned::UnaryExpr(_)
            | ExprOwned::BinExpr(_)
            | ExprOwned::Statement(_)
            | ExprOwned::Block(_)
            | ExprOwned::Function(_)
            | ExprOwned::Variable(_)
            | ExprOwned::Identifier(_)
            | ExprOwned::If(_)
            | ExprOwned::WhileLoop(_)
            | ExprOwned::DoWhileLoop(_)
            | ExprOwned::Switch(_)
            | ExprOwned::Break(_)
            | ExprOwned::Continue(_)
            | ExprOwned::Return(_)
            | ExprOwned::ForEach(_)
            | ExprOwned::Await(_)
            | ExprOwned::Assert(_) => Err(self),
        }
    }
}

impl<'a> From<TypeOwned<'a>> for ExprOwned<'a> {
    fn from(val: TypeOwned<'a>) -> ExprOwned<'a> {
        match val {
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

impl<'storage, 'a> From<&'storage TypeOwned<'a>> for ExprRef<'storage, 'a> {
    fn from(val: &'storage TypeOwned<'a>) -> ExprRef<'storage, 'a> {
        match val {
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

impl<'storage, 'a> From<&'storage TypeOwned<'a>> for ExprRefMut<'storage, 'a> {
    fn from(val: &'storage TypeOwned<'a>) -> ExprRefMut<'storage, 'a> {
        match val {
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
