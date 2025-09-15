use crate::kind::{Block, Expr, FunctionParameter, Path, StructField};
use interned_string::IString;
use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct RefinementType {
    pub basis_type: Type,
    pub width: Option<Expr>,
    pub minimum: Option<Expr>,
    pub maximum: Option<Expr>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TupleType {
    pub element_types: Vec<Type>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ArrayType {
    pub element_type: Type,
    pub len: Expr,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct MapType {
    pub key_type: Type,
    pub value_type: Type,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct SliceType {
    pub element_type: Type,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FunctionType {
    pub attributes: Vec<Expr>,
    pub parameters: Vec<FunctionParameter>,
    pub return_type: Option<Type>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum Lifetime {
    ManuallyManaged,
    ProcessLocal,
    CollectorManaged,
    ThreadLocal,
    TaskLocal,
    StackLocal { name: IString },
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ReferenceType {
    pub lifetime: Option<Lifetime>,
    pub mutability: Option<bool>,
    pub exclusive: Option<bool>,
    pub to: Type,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct GenericArgument {
    pub name: Option<IString>,
    pub value: Option<Type>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct GenericType {
    pub basis_type: Type,
    pub arguments: Vec<GenericArgument>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct StructType {
    pub fields: Vec<StructField>,
}

#[derive(Clone, Serialize, Deserialize)]
pub enum Type {
    SyntaxError,

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
    UnitType,
    InferType,
    TypeName(Box<Path>),
    RefinementType(Box<RefinementType>),
    TupleType(Box<TupleType>),
    ArrayType(Box<ArrayType>),
    MapType(Box<MapType>),
    SliceType(Box<SliceType>),
    FunctionType(Box<FunctionType>),
    ReferenceType(Box<ReferenceType>),
    GenericType(Box<GenericType>),
    OpaqueType(IString),
    StructType(Box<StructType>),
    LatentType(Box<Block>),
    Parentheses(Box<Type>),
}

impl std::fmt::Debug for Type {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Type::SyntaxError => write!(f, "SyntaxError"),

            Type::Bool => write!(f, "bool"),
            Type::UInt8 => write!(f, "u8"),
            Type::UInt16 => write!(f, "u16"),
            Type::UInt32 => write!(f, "u32"),
            Type::UInt64 => write!(f, "u64"),
            Type::UInt128 => write!(f, "u128"),
            Type::Int8 => write!(f, "i8"),
            Type::Int16 => write!(f, "i16"),
            Type::Int32 => write!(f, "i32"),
            Type::Int64 => write!(f, "i64"),
            Type::Int128 => write!(f, "i128"),
            Type::Float8 => write!(f, "f8"),
            Type::Float16 => write!(f, "f16"),
            Type::Float32 => write!(f, "f32"),
            Type::Float64 => write!(f, "f64"),
            Type::Float128 => write!(f, "f128"),
            Type::UnitType => write!(f, "()"),
            Type::InferType => write!(f, "_"),
            Type::TypeName(e) => f.debug_struct("TypeName").field("name", &e).finish(),
            Type::RefinementType(e) => e.fmt(f),
            Type::TupleType(e) => e.fmt(f),
            Type::ArrayType(e) => e.fmt(f),
            Type::MapType(e) => e.fmt(f),
            Type::SliceType(e) => e.fmt(f),
            Type::FunctionType(e) => e.fmt(f),
            Type::ReferenceType(e) => e.fmt(f),
            Type::GenericType(e) => e.fmt(f),
            Type::OpaqueType(e) => f.debug_struct("OpaqueType").field("name", e).finish(),
            Type::StructType(e) => e.fmt(f),
            Type::LatentType(e) => f.debug_struct("LatentType").field("type", e).finish(),
            Type::Parentheses(e) => f.debug_struct("Parentheses").field("type", e).finish(),
        }
    }
}
