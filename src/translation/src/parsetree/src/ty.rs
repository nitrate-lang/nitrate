use crate::{
    expr::{AttributeList, TypeArgument},
    item::{Enum, FuncParams, Mutability, Struct, TypeAlias},
    kind::{Block, Expr},
    tag::{LifetimeNameId, OpaqueTypeNameId},
};

use serde::{Deserialize, Serialize};
use serde_with::skip_serializing_none;
use std::sync::{RwLock, Weak};

#[skip_serializing_none]
#[derive(Clone, Serialize, Deserialize)]
pub struct TypeSyntaxError;

#[skip_serializing_none]
#[derive(Clone, Serialize, Deserialize)]
pub struct Bool;

#[skip_serializing_none]
#[derive(Clone, Serialize, Deserialize)]
pub struct UInt8;

#[skip_serializing_none]
#[derive(Clone, Serialize, Deserialize)]
pub struct UInt16;

#[skip_serializing_none]
#[derive(Clone, Serialize, Deserialize)]
pub struct UInt32;

#[skip_serializing_none]
#[derive(Clone, Serialize, Deserialize)]
pub struct UInt64;

#[skip_serializing_none]
#[derive(Clone, Serialize, Deserialize)]
pub struct UInt128;

#[skip_serializing_none]
#[derive(Clone, Serialize, Deserialize)]
pub struct Int8;

#[skip_serializing_none]
#[derive(Clone, Serialize, Deserialize)]
pub struct Int16;

#[skip_serializing_none]
#[derive(Clone, Serialize, Deserialize)]
pub struct Int32;

#[skip_serializing_none]
#[derive(Clone, Serialize, Deserialize)]
pub struct Int64;

#[skip_serializing_none]
#[derive(Clone, Serialize, Deserialize)]
pub struct Int128;

#[skip_serializing_none]
#[derive(Clone, Serialize, Deserialize)]
pub struct Float8;

#[skip_serializing_none]
#[derive(Clone, Serialize, Deserialize)]
pub struct Float16;

#[skip_serializing_none]
#[derive(Clone, Serialize, Deserialize)]
pub struct Float32;

#[skip_serializing_none]
#[derive(Clone, Serialize, Deserialize)]
pub struct Float64;

#[skip_serializing_none]
#[derive(Clone, Serialize, Deserialize)]
pub struct Float128;

#[skip_serializing_none]
#[derive(Clone, Serialize, Deserialize)]
pub struct InferType;

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum TypePathTarget {
    TypeAlias(Weak<RwLock<TypeAlias>>),
    Struct(Weak<RwLock<Struct>>),
    Enum(Weak<RwLock<Enum>>),
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TypePathSegment {
    pub name: String,
    pub type_arguments: Option<Vec<TypeArgument>>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TypePath {
    pub segments: Vec<TypePathSegment>,

    #[serde(skip)]
    pub resolved: Option<TypePathTarget>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct RefinementType {
    pub basis_type: Type,
    pub width: Option<Expr>,
    pub minimum: Option<Expr>,
    pub maximum: Option<Expr>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TupleType {
    pub element_types: Vec<Type>,
    pub ends_with_comma: bool,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ArrayType {
    pub element_type: Type,
    pub len: Expr,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct SliceType {
    pub element_type: Type,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FunctionType {
    pub attributes: Option<AttributeList>,
    pub parameters: FuncParams,
    pub return_type: Option<Type>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum Lifetime {
    SyntaxError,

    Static,
    GarbageCollected,
    Thread,
    Task,
    Other { name: LifetimeNameId },
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum Exclusivity {
    Iso,
    Poly,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ReferenceType {
    pub lifetime: Option<Lifetime>,
    pub mutability: Option<Mutability>,
    pub exclusivity: Option<Exclusivity>,
    pub to: Type,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct OpaqueType {
    pub name: OpaqueTypeNameId,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct LatentType {
    pub body: Block,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TypeParentheses {
    pub inner: Type,
}

#[skip_serializing_none]
#[derive(Clone, Serialize, Deserialize)]
pub enum Type {
    SyntaxError(TypeSyntaxError),
    Bool(Bool),
    UInt8(UInt8),
    UInt16(UInt16),
    UInt32(UInt32),
    UInt64(UInt64),
    UInt128(UInt128),
    Int8(Int8),
    Int16(Int16),
    Int32(Int32),
    Int64(Int64),
    Int128(Int128),
    Float8(Float8),
    Float16(Float16),
    Float32(Float32),
    Float64(Float64),
    Float128(Float128),
    InferType(InferType),
    TypePath(Box<TypePath>),
    RefinementType(Box<RefinementType>),
    TupleType(Box<TupleType>),
    ArrayType(Box<ArrayType>),
    SliceType(Box<SliceType>),
    FunctionType(Box<FunctionType>),
    ReferenceType(Box<ReferenceType>),
    OpaqueType(OpaqueType),
    LatentType(Box<LatentType>),
    Lifetime(Box<Lifetime>),
    Parentheses(Box<TypeParentheses>),
}

impl std::fmt::Debug for Type {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Type::SyntaxError(_) => write!(f, "SyntaxError"),
            Type::Bool(_) => write!(f, "bool"),
            Type::UInt8(_) => write!(f, "u8"),
            Type::UInt16(_) => write!(f, "u16"),
            Type::UInt32(_) => write!(f, "u32"),
            Type::UInt64(_) => write!(f, "u64"),
            Type::UInt128(_) => write!(f, "u128"),
            Type::Int8(_) => write!(f, "i8"),
            Type::Int16(_) => write!(f, "i16"),
            Type::Int32(_) => write!(f, "i32"),
            Type::Int64(_) => write!(f, "i64"),
            Type::Int128(_) => write!(f, "i128"),
            Type::Float8(_) => write!(f, "f8"),
            Type::Float16(_) => write!(f, "f16"),
            Type::Float32(_) => write!(f, "f32"),
            Type::Float64(_) => write!(f, "f64"),
            Type::Float128(_) => write!(f, "f128"),
            Type::InferType(_) => write!(f, "_"),
            Type::TypePath(e) => e.fmt(f),
            Type::RefinementType(e) => e.fmt(f),
            Type::TupleType(e) => e.fmt(f),
            Type::ArrayType(e) => e.fmt(f),
            Type::SliceType(e) => e.fmt(f),
            Type::FunctionType(e) => e.fmt(f),
            Type::ReferenceType(e) => e.fmt(f),
            Type::OpaqueType(e) => e.fmt(f),
            Type::LatentType(e) => e.fmt(f),
            Type::Lifetime(e) => e.fmt(f),
            Type::Parentheses(e) => e.fmt(f),
        }
    }
}
