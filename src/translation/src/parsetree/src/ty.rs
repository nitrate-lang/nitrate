use crate::{
    Order, ParseTreeIterMut, RefNodeMut,
    kind::{Block, Expr, Path},
    tag::{ArgNameId, LifetimeNameId, OpaqueTypeNameId, ParameterNameId},
};
use serde::{Deserialize, Serialize};

#[derive(Clone, Serialize, Deserialize)]
pub struct TypeSyntaxError;

impl ParseTreeIterMut for TypeSyntaxError {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Clone, Serialize, Deserialize)]
pub struct Bool;

impl ParseTreeIterMut for Bool {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Clone, Serialize, Deserialize)]
pub struct UInt8;

impl ParseTreeIterMut for UInt8 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Clone, Serialize, Deserialize)]
pub struct UInt16;

impl ParseTreeIterMut for UInt16 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Clone, Serialize, Deserialize)]
pub struct UInt32;

impl ParseTreeIterMut for UInt32 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Clone, Serialize, Deserialize)]
pub struct UInt64;

impl ParseTreeIterMut for UInt64 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Clone, Serialize, Deserialize)]
pub struct UInt128;

impl ParseTreeIterMut for UInt128 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Clone, Serialize, Deserialize)]
pub struct Int8;

impl ParseTreeIterMut for Int8 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Clone, Serialize, Deserialize)]
pub struct Int16;

impl ParseTreeIterMut for Int16 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Clone, Serialize, Deserialize)]
pub struct Int32;

impl ParseTreeIterMut for Int32 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Clone, Serialize, Deserialize)]
pub struct Int64;

impl ParseTreeIterMut for Int64 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Clone, Serialize, Deserialize)]
pub struct Int128;

impl ParseTreeIterMut for Int128 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Clone, Serialize, Deserialize)]
pub struct Float8;

impl ParseTreeIterMut for Float8 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Clone, Serialize, Deserialize)]
pub struct Float16;

impl ParseTreeIterMut for Float16 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Clone, Serialize, Deserialize)]
pub struct Float32;

impl ParseTreeIterMut for Float32 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Clone, Serialize, Deserialize)]
pub struct Float64;

impl ParseTreeIterMut for Float64 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Clone, Serialize, Deserialize)]
pub struct Float128;

impl ParseTreeIterMut for Float128 {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Clone, Serialize, Deserialize)]
pub struct UnitType;

impl ParseTreeIterMut for UnitType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Clone, Serialize, Deserialize)]
pub struct InferType;

impl ParseTreeIterMut for InferType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TypeName {
    pub name: Path,
}

impl ParseTreeIterMut for TypeName {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

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
pub struct OpaqueType {
    pub name: OpaqueTypeNameId,
}

impl ParseTreeIterMut for OpaqueType {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct LatentType {
    pub body: Block,
}

impl ParseTreeIterMut for LatentType {
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

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TypeParentheses {
    pub inner: Type,
}

impl ParseTreeIterMut for TypeParentheses {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

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
    UnitType(UnitType),
    InferType(InferType),
    TypeName(Box<TypeName>),
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

impl ParseTreeIterMut for Type {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        match self {
            Type::SyntaxError(ty) => ty.depth_first_iter_mut(f),
            Type::Bool(ty) => ty.depth_first_iter_mut(f),
            Type::UInt8(ty) => ty.depth_first_iter_mut(f),
            Type::UInt16(ty) => ty.depth_first_iter_mut(f),
            Type::UInt32(ty) => ty.depth_first_iter_mut(f),
            Type::UInt64(ty) => ty.depth_first_iter_mut(f),
            Type::UInt128(ty) => ty.depth_first_iter_mut(f),
            Type::Int8(ty) => ty.depth_first_iter_mut(f),
            Type::Int16(ty) => ty.depth_first_iter_mut(f),
            Type::Int32(ty) => ty.depth_first_iter_mut(f),
            Type::Int64(ty) => ty.depth_first_iter_mut(f),
            Type::Int128(ty) => ty.depth_first_iter_mut(f),
            Type::Float8(ty) => ty.depth_first_iter_mut(f),
            Type::Float16(ty) => ty.depth_first_iter_mut(f),
            Type::Float32(ty) => ty.depth_first_iter_mut(f),
            Type::Float64(ty) => ty.depth_first_iter_mut(f),
            Type::Float128(ty) => ty.depth_first_iter_mut(f),
            Type::UnitType(ty) => ty.depth_first_iter_mut(f),
            Type::InferType(ty) => ty.depth_first_iter_mut(f),
            Type::TypeName(ty) => ty.depth_first_iter_mut(f),
            Type::RefinementType(ty) => ty.depth_first_iter_mut(f),
            Type::TupleType(ty) => ty.depth_first_iter_mut(f),
            Type::ArrayType(ty) => ty.depth_first_iter_mut(f),
            Type::SliceType(ty) => ty.depth_first_iter_mut(f),
            Type::FunctionType(ty) => ty.depth_first_iter_mut(f),
            Type::ReferenceType(ty) => ty.depth_first_iter_mut(f),
            Type::OpaqueType(ty) => ty.depth_first_iter_mut(f),
            Type::LatentType(ty) => ty.depth_first_iter_mut(f),
            Type::Lifetime(ty) => ty.depth_first_iter_mut(f),
            Type::Parentheses(ty) => ty.depth_first_iter_mut(f),
        }
    }
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
            Type::UnitType(_) => write!(f, "()"),
            Type::InferType(_) => write!(f, "_"),
            Type::TypeName(e) => e.fmt(f),
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
