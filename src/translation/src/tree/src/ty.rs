use crate::{
    ast::{Block, Expr},
    expr::{AttributeList, TypeArgument},
    item::Mutability,
};

use interned_string::IString;
use serde::{Deserialize, Serialize};
use serde_with::skip_serializing_none;

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
pub struct USize;

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
pub struct Float32;

#[skip_serializing_none]
#[derive(Clone, Serialize, Deserialize)]
pub struct Float64;

#[skip_serializing_none]
#[derive(Clone, Serialize, Deserialize)]
pub struct InferType;

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
    pub resolved_path: Option<IString>,
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
pub struct FuncTypeParam {
    pub attributes: Option<AttributeList>,
    pub name: IString,
    pub ty: Type,
}

pub type FuncTypeParams = Vec<FuncTypeParam>;

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FunctionType {
    pub attributes: Option<AttributeList>,
    pub parameters: FuncTypeParams,
    pub return_type: Option<Type>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Lifetime {
    pub name: IString,
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
    pub exclusivity: Option<Exclusivity>,
    pub mutability: Option<Mutability>,
    pub to: Type,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PointerType {
    pub exclusivity: Option<Exclusivity>,
    pub mutability: Option<Mutability>,
    pub to: Type,
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
    USize(USize),
    Int8(Int8),
    Int16(Int16),
    Int32(Int32),
    Int64(Int64),
    Int128(Int128),
    Float32(Float32),
    Float64(Float64),
    InferType(InferType),
    TypePath(Box<TypePath>),
    RefinementType(Box<RefinementType>),
    TupleType(Box<TupleType>),
    ArrayType(Box<ArrayType>),
    SliceType(Box<SliceType>),
    FunctionType(Box<FunctionType>),
    ReferenceType(Box<ReferenceType>),
    PointerType(Box<PointerType>),
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
            Type::USize(_) => write!(f, "usize"),
            Type::Int8(_) => write!(f, "i8"),
            Type::Int16(_) => write!(f, "i16"),
            Type::Int32(_) => write!(f, "i32"),
            Type::Int64(_) => write!(f, "i64"),
            Type::Int128(_) => write!(f, "i128"),
            Type::Float32(_) => write!(f, "f32"),
            Type::Float64(_) => write!(f, "f64"),
            Type::InferType(_) => write!(f, "_"),
            Type::TypePath(e) => e.fmt(f),
            Type::RefinementType(e) => e.fmt(f),
            Type::TupleType(e) => e.fmt(f),
            Type::ArrayType(e) => e.fmt(f),
            Type::SliceType(e) => e.fmt(f),
            Type::FunctionType(e) => e.fmt(f),
            Type::ReferenceType(e) => e.fmt(f),
            Type::PointerType(e) => e.fmt(f),
            Type::LatentType(e) => e.fmt(f),
            Type::Lifetime(e) => e.fmt(f),
            Type::Parentheses(e) => e.fmt(f),
        }
    }
}

impl Type {
    pub fn as_bool(self) -> Option<Bool> {
        match self {
            Type::Bool(b) => Some(b),
            _ => None,
        }
    }

    pub fn as_uint8(self) -> Option<UInt8> {
        match self {
            Type::UInt8(b) => Some(b),
            _ => None,
        }
    }

    pub fn as_uint16(self) -> Option<UInt16> {
        match self {
            Type::UInt16(b) => Some(b),
            _ => None,
        }
    }

    pub fn as_uint32(self) -> Option<UInt32> {
        match self {
            Type::UInt32(b) => Some(b),
            _ => None,
        }
    }

    pub fn as_uint64(self) -> Option<UInt64> {
        match self {
            Type::UInt64(b) => Some(b),
            _ => None,
        }
    }

    pub fn as_uint128(self) -> Option<UInt128> {
        match self {
            Type::UInt128(b) => Some(b),
            _ => None,
        }
    }

    pub fn as_int8(self) -> Option<Int8> {
        match self {
            Type::Int8(b) => Some(b),
            _ => None,
        }
    }

    pub fn as_int16(self) -> Option<Int16> {
        match self {
            Type::Int16(b) => Some(b),
            _ => None,
        }
    }

    pub fn as_int32(self) -> Option<Int32> {
        match self {
            Type::Int32(b) => Some(b),
            _ => None,
        }
    }

    pub fn as_int64(self) -> Option<Int64> {
        match self {
            Type::Int64(b) => Some(b),
            _ => None,
        }
    }

    pub fn as_int128(self) -> Option<Int128> {
        match self {
            Type::Int128(b) => Some(b),
            _ => None,
        }
    }

    pub fn as_float32(self) -> Option<Float32> {
        match self {
            Type::Float32(b) => Some(b),
            _ => None,
        }
    }

    pub fn as_float64(self) -> Option<Float64> {
        match self {
            Type::Float64(b) => Some(b),
            _ => None,
        }
    }

    pub fn as_infer_type(self) -> Option<InferType> {
        match self {
            Type::InferType(b) => Some(b),
            _ => None,
        }
    }

    pub fn as_type_path(self) -> Option<TypePath> {
        match self {
            Type::TypePath(b) => Some(*b),
            _ => None,
        }
    }

    pub fn as_refinement_type(self) -> Option<RefinementType> {
        match self {
            Type::RefinementType(b) => Some(*b),
            _ => None,
        }
    }

    pub fn as_tuple_type(self) -> Option<TupleType> {
        match self {
            Type::TupleType(b) => Some(*b),
            _ => None,
        }
    }

    pub fn as_array_type(self) -> Option<ArrayType> {
        match self {
            Type::ArrayType(b) => Some(*b),
            _ => None,
        }
    }

    pub fn as_slice_type(self) -> Option<SliceType> {
        match self {
            Type::SliceType(b) => Some(*b),
            _ => None,
        }
    }

    pub fn as_function_type(self) -> Option<FunctionType> {
        match self {
            Type::FunctionType(b) => Some(*b),
            _ => None,
        }
    }

    pub fn as_reference_type(self) -> Option<ReferenceType> {
        match self {
            Type::ReferenceType(b) => Some(*b),
            _ => None,
        }
    }

    pub fn as_latent_type(self) -> Option<LatentType> {
        match self {
            Type::LatentType(b) => Some(*b),
            _ => None,
        }
    }

    pub fn as_lifetime(self) -> Option<Lifetime> {
        match self {
            Type::Lifetime(b) => Some(*b),
            _ => None,
        }
    }

    pub fn as_parentheses(self) -> Option<TypeParentheses> {
        match self {
            Type::Parentheses(b) => Some(*b),
            _ => None,
        }
    }
}
