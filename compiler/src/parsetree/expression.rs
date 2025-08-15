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
use super::reference::{ManagedRefType, UnmanagedRefType};
use super::refinement_type::RefinementType;
use super::scope::Scope;
use super::slice_type::SliceType;
use super::statement::Statement;
use super::tuple_type::TupleType;
use super::unary_expr::UnaryExpr;
use super::variable::Variable;
use crate::lexical::{BStringData, StringData};

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

    BooleanLit,
    IntegerLit,
    FloatLit,
    StringLit,
    BStringLit,
    ListLit,
    ObjectLit,

    UnaryExpr,
    BinExpr,
    Statement,
    Block,

    Function,
    Variable,
    Identifier,
    Scope,

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
pub enum TypeKind {
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

#[derive(Debug, Clone, PartialEq)]
pub enum Expr<'a> {
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
    OpaqueType(StringData<'a>),

    Discard,

    BooleanLit(bool),
    IntegerLit(IntegerLit),
    FloatLit(f64),
    StringLit(StringData<'a>),
    BStringLit(BStringData<'a>),
    ListLit(ListLit<'a>),
    ObjectLit(ObjectLit<'a>),

    UnaryExpr(UnaryExpr<'a>),
    BinExpr(BinExpr<'a>),
    Statement(Statement<'a>),
    Block(Block<'a>),

    Function(Function<'a>),
    Variable(Variable<'a>),
    Identifier(&'a str),
    Scope(Scope<'a>),

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

#[derive(Debug, Clone, PartialEq)]
pub enum Type<'a> {
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
    OpaqueType(StringData<'a>),
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
            | ExprKind::BooleanLit
            | ExprKind::IntegerLit
            | ExprKind::FloatLit
            | ExprKind::StringLit
            | ExprKind::BStringLit
            | ExprKind::ListLit
            | ExprKind::ObjectLit
            | ExprKind::UnaryExpr
            | ExprKind::BinExpr
            | ExprKind::Statement
            | ExprKind::Block
            | ExprKind::Function
            | ExprKind::Variable
            | ExprKind::Identifier
            | ExprKind::Scope
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

            x if x == ExprKind::BooleanLit as u8 => Ok(ExprKind::BooleanLit),
            x if x == ExprKind::IntegerLit as u8 => Ok(ExprKind::IntegerLit),
            x if x == ExprKind::FloatLit as u8 => Ok(ExprKind::FloatLit),
            x if x == ExprKind::StringLit as u8 => Ok(ExprKind::StringLit),
            x if x == ExprKind::BStringLit as u8 => Ok(ExprKind::BStringLit),
            x if x == ExprKind::ListLit as u8 => Ok(ExprKind::ListLit),
            x if x == ExprKind::ObjectLit as u8 => Ok(ExprKind::ObjectLit),

            x if x == ExprKind::UnaryExpr as u8 => Ok(ExprKind::UnaryExpr),
            x if x == ExprKind::BinExpr as u8 => Ok(ExprKind::BinExpr),
            x if x == ExprKind::Statement as u8 => Ok(ExprKind::Statement),
            x if x == ExprKind::Block as u8 => Ok(ExprKind::Block),

            x if x == ExprKind::Function as u8 => Ok(ExprKind::Function),
            x if x == ExprKind::Variable as u8 => Ok(ExprKind::Variable),
            x if x == ExprKind::Identifier as u8 => Ok(ExprKind::Identifier),
            x if x == ExprKind::Scope as u8 => Ok(ExprKind::Scope),

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
            Expr::TypeName(x) => Ok(Type::TypeName(x)),
            Expr::RefinementType(x) => Ok(Type::RefinementType(x)),
            Expr::TupleType(x) => Ok(Type::TupleType(x)),
            Expr::ArrayType(x) => Ok(Type::ArrayType(x)),
            Expr::MapType(x) => Ok(Type::MapType(x)),
            Expr::SliceType(x) => Ok(Type::SliceType(x)),
            Expr::FunctionType(x) => Ok(Type::FunctionType(x)),
            Expr::ManagedRefType(x) => Ok(Type::ManagedRefType(x)),
            Expr::UnmanagedRefType(x) => Ok(Type::UnmanagedRefType(x)),
            Expr::GenericType(x) => Ok(Type::GenericType(x)),
            Expr::OpaqueType(x) => Ok(Type::OpaqueType(x)),

            Expr::Discard
            | Expr::BooleanLit(_)
            | Expr::IntegerLit(_)
            | Expr::FloatLit(_)
            | Expr::StringLit(_)
            | Expr::BStringLit(_)
            | Expr::ListLit(_)
            | Expr::ObjectLit(_)
            | Expr::UnaryExpr(_)
            | Expr::BinExpr(_)
            | Expr::Statement(_)
            | Expr::Block(_)
            | Expr::Function(_)
            | Expr::Variable(_)
            | Expr::Identifier(_)
            | Expr::Scope(_)
            | Expr::If(_)
            | Expr::WhileLoop(_)
            | Expr::DoWhileLoop(_)
            | Expr::Switch(_)
            | Expr::Break(_)
            | Expr::Continue(_)
            | Expr::Return(_)
            | Expr::ForEach(_)
            | Expr::Await(_)
            | Expr::Assert(_) => Err(self),
        }
    }
}

impl<'a> From<Type<'a>> for Expr<'a> {
    fn from(val: Type<'a>) -> Expr<'a> {
        match val {
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
            Type::TypeName(x) => Expr::TypeName(x),
            Type::RefinementType(x) => Expr::RefinementType(x),
            Type::TupleType(x) => Expr::TupleType(x),
            Type::ArrayType(x) => Expr::ArrayType(x),
            Type::MapType(x) => Expr::MapType(x),
            Type::SliceType(x) => Expr::SliceType(x),
            Type::FunctionType(x) => Expr::FunctionType(x),
            Type::ManagedRefType(x) => Expr::ManagedRefType(x),
            Type::UnmanagedRefType(x) => Expr::UnmanagedRefType(x),
            Type::GenericType(x) => Expr::GenericType(x),
            Type::OpaqueType(x) => Expr::OpaqueType(x),
        }
    }
}
