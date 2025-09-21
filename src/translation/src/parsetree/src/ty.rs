use crate::{
    Order, ParseTreeIterMut, RefNodeMut,
    kind::{Block, Expr, Path},
    tag::{ArgNameId, LifetimeNameId, OpaqueTypeNameId, ParameterNameId},
};
use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct RefinementType {
    pub basis_type: Type,
    pub width: Option<Expr>,
    pub minimum: Option<Expr>,
    pub maximum: Option<Expr>,
}

impl ParseTreeIterMut for RefinementType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TupleType {
    pub element_types: Vec<Type>,
}

impl ParseTreeIterMut for TupleType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ArrayType {
    pub element_type: Type,
    pub len: Expr,
}

impl ParseTreeIterMut for ArrayType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct SliceType {
    pub element_type: Type,
}

impl ParseTreeIterMut for SliceType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FunctionTypeParameter {
    pub attributes: Option<Vec<Expr>>,
    pub name: Option<ParameterNameId>,
    pub param_type: Type,
    pub default: Option<Expr>,
}

impl ParseTreeIterMut for FunctionTypeParameter {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FunctionType {
    pub attributes: Option<Vec<Expr>>,
    pub parameters: Vec<FunctionTypeParameter>,
    pub return_type: Option<Type>,
}

impl ParseTreeIterMut for FunctionType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum Lifetime {
    SyntaxError,

    Manual,
    Static,
    GarbageCollected,
    Thread,
    Task,
    Other { name: LifetimeNameId },
}

impl ParseTreeIterMut for Lifetime {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ReferenceType {
    pub lifetime: Option<Lifetime>,
    pub mutability: Option<bool>,
    pub exclusive: Option<bool>,
    pub to: Type,
}

impl ParseTreeIterMut for ReferenceType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct GenericArgument {
    pub name: Option<ArgNameId>,
    pub value: Type,
}

impl ParseTreeIterMut for GenericArgument {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
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
    SliceType(Box<SliceType>),
    FunctionType(Box<FunctionType>),
    ReferenceType(Box<ReferenceType>),
    OpaqueType(OpaqueTypeNameId),
    LatentType(Box<Block>),
    Lifetime(Box<Lifetime>),
    Parentheses(Box<Type>),
}

impl ParseTreeIterMut for Type {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        match self {
            Type::SyntaxError => {
                f(Order::Pre, RefNodeMut::TypeSyntaxError);
                f(Order::Post, RefNodeMut::TypeSyntaxError);
            }

            Type::Bool => {
                f(Order::Pre, RefNodeMut::TypeBool);
                f(Order::Post, RefNodeMut::TypeBool);
            }

            Type::UInt8 => {
                f(Order::Pre, RefNodeMut::TypeUInt8);
                f(Order::Post, RefNodeMut::TypeUInt8);
            }

            Type::UInt16 => {
                f(Order::Pre, RefNodeMut::TypeUInt16);
                f(Order::Post, RefNodeMut::TypeUInt16);
            }

            Type::UInt32 => {
                f(Order::Pre, RefNodeMut::TypeUInt32);
                f(Order::Post, RefNodeMut::TypeUInt32);
            }

            Type::UInt64 => {
                f(Order::Pre, RefNodeMut::TypeUInt64);
                f(Order::Post, RefNodeMut::TypeUInt64);
            }

            Type::UInt128 => {
                f(Order::Pre, RefNodeMut::TypeUInt128);
                f(Order::Post, RefNodeMut::TypeUInt128);
            }

            Type::Int8 => {
                f(Order::Pre, RefNodeMut::TypeInt8);
                f(Order::Post, RefNodeMut::TypeInt8);
            }

            Type::Int16 => {
                f(Order::Pre, RefNodeMut::TypeInt16);
                f(Order::Post, RefNodeMut::TypeInt16);
            }

            Type::Int32 => {
                f(Order::Pre, RefNodeMut::TypeInt32);
                f(Order::Post, RefNodeMut::TypeInt32);
            }

            Type::Int64 => {
                f(Order::Pre, RefNodeMut::TypeInt64);
                f(Order::Post, RefNodeMut::TypeInt64);
            }

            Type::Int128 => {
                f(Order::Pre, RefNodeMut::TypeInt128);
                f(Order::Post, RefNodeMut::TypeInt128);
            }

            Type::Float8 => {
                f(Order::Pre, RefNodeMut::TypeFloat8);
                f(Order::Post, RefNodeMut::TypeFloat8);
            }

            Type::Float16 => {
                f(Order::Pre, RefNodeMut::TypeFloat16);
                f(Order::Post, RefNodeMut::TypeFloat16);
            }

            Type::Float32 => {
                f(Order::Pre, RefNodeMut::TypeFloat32);
                f(Order::Post, RefNodeMut::TypeFloat32);
            }

            Type::Float64 => {
                f(Order::Pre, RefNodeMut::TypeFloat64);
                f(Order::Post, RefNodeMut::TypeFloat64);
            }

            Type::Float128 => {
                f(Order::Pre, RefNodeMut::TypeFloat128);
                f(Order::Post, RefNodeMut::TypeFloat128);
            }

            Type::UnitType => {
                f(Order::Pre, RefNodeMut::TypeUnitType);
                f(Order::Post, RefNodeMut::TypeUnitType);
            }

            Type::InferType => {
                f(Order::Pre, RefNodeMut::TypeInferType);
                f(Order::Post, RefNodeMut::TypeInferType);
            }

            Type::TypeName(ty) => ty.depth_first_iter_mut(f),
            Type::RefinementType(ty) => ty.depth_first_iter_mut(f),
            Type::TupleType(ty) => ty.depth_first_iter_mut(f),
            Type::ArrayType(ty) => ty.depth_first_iter_mut(f),
            Type::SliceType(ty) => ty.depth_first_iter_mut(f),
            Type::FunctionType(ty) => ty.depth_first_iter_mut(f),
            Type::ReferenceType(ty) => ty.depth_first_iter_mut(f),

            Type::OpaqueType(ty) => {
                f(Order::Pre, RefNodeMut::TypeOpaqueType(ty));
                f(Order::Post, RefNodeMut::TypeOpaqueType(ty));
            }

            Type::LatentType(ty) => ty.depth_first_iter_mut(f),
            Type::Lifetime(ty) => ty.depth_first_iter_mut(f),
            Type::Parentheses(ty) => ty.depth_first_iter_mut(f),
        }
    }
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
            Type::SliceType(e) => e.fmt(f),
            Type::FunctionType(e) => e.fmt(f),
            Type::ReferenceType(e) => e.fmt(f),
            Type::OpaqueType(e) => f.debug_struct("OpaqueType").field("name", e).finish(),
            Type::LatentType(e) => f.debug_struct("LatentType").field("type", e).finish(),
            Type::Lifetime(e) => e.fmt(f),
            Type::Parentheses(e) => f.debug_struct("Parentheses").field("type", e).finish(),
        }
    }
}
