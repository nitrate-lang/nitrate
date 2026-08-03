use crate::prelude::*;
use nitrate_nstring::NString;
use serde::{Deserialize, Serialize};
use serde_with::skip_serializing_none;

#[skip_serializing_none]
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default, Serialize, Deserialize)]
pub struct TypeSyntaxError {
    pub span: SrcSpan,
}

impl Spanned for TypeSyntaxError {
    fn span(&self) -> SrcSpan {
        self.span
    }
    fn set_span(&mut self, span: SrcSpan) {
        self.span = span;
    }
}

macro_rules! define_primitive_type {
    ($name:ident) => {
        #[skip_serializing_none]
        #[derive(Debug, Clone, Copy, PartialEq, Eq, Default, Serialize, Deserialize)]
        pub struct $name {
            pub span: SrcSpan,
        }
        impl Spanned for $name {
            fn span(&self) -> SrcSpan {
                self.span
            }
            fn set_span(&mut self, span: SrcSpan) {
                self.span = span;
            }
        }
    };
}

define_primitive_type!(Bool);
define_primitive_type!(UInt8);
define_primitive_type!(UInt16);
define_primitive_type!(UInt32);
define_primitive_type!(UInt64);
define_primitive_type!(UInt128);
define_primitive_type!(USize);
define_primitive_type!(Int8);
define_primitive_type!(Int16);
define_primitive_type!(Int32);
define_primitive_type!(Int64);
define_primitive_type!(Int128);
define_primitive_type!(Float32);
define_primitive_type!(Float64);
define_primitive_type!(InferType);

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TypePathSegment {
    pub span: SrcSpan,
    pub name: String,
    pub type_arguments: Option<Vec<TypeArgument>>,
}

impl Spanned for TypePathSegment {
    fn span(&self) -> SrcSpan {
        self.span
    }
    fn set_span(&mut self, span: SrcSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TypePath {
    pub span: SrcSpan,
    pub segments: Vec<TypePathSegment>,
    pub resolved_path: Option<NString>,
}

impl Spanned for TypePath {
    fn span(&self) -> SrcSpan {
        self.span
    }
    fn set_span(&mut self, span: SrcSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct RefinementType {
    pub span: SrcSpan,
    pub basis_type: Type,
    pub width: Option<Expr>,
    pub minimum: Option<Expr>,
    pub maximum: Option<Expr>,
}

impl Spanned for RefinementType {
    fn span(&self) -> SrcSpan {
        self.span
    }
    fn set_span(&mut self, span: SrcSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TupleType {
    pub span: SrcSpan,
    pub element_types: Vec<Type>,
}

impl Spanned for TupleType {
    fn span(&self) -> SrcSpan {
        self.span
    }
    fn set_span(&mut self, span: SrcSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ArrayType {
    pub span: SrcSpan,
    pub element_type: Type,
    pub len: Expr,
}

impl Spanned for ArrayType {
    fn span(&self) -> SrcSpan {
        self.span
    }
    fn set_span(&mut self, span: SrcSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct SliceType {
    pub span: SrcSpan,
    pub element_type: Type,
}

impl Spanned for SliceType {
    fn span(&self) -> SrcSpan {
        self.span
    }
    fn set_span(&mut self, span: SrcSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FuncTypeParam {
    pub span: SrcSpan,
    pub attributes: Option<AttributeList>,
    pub name: NString,
    pub ty: Type,
}

impl Spanned for FuncTypeParam {
    fn span(&self) -> SrcSpan {
        self.span
    }
    fn set_span(&mut self, span: SrcSpan) {
        self.span = span;
    }
}

pub type FuncTypeParams = Vec<FuncTypeParam>;

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FunctionType {
    pub span: SrcSpan,
    pub attributes: Option<AttributeList>,
    pub parameters: FuncTypeParams,
    pub return_type: Option<Type>,
}

impl Spanned for FunctionType {
    fn span(&self) -> SrcSpan {
        self.span
    }
    fn set_span(&mut self, span: SrcSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Lifetime {
    pub span: SrcSpan,
    pub name: NString,
}

impl Spanned for Lifetime {
    fn span(&self) -> SrcSpan {
        self.span
    }
    fn set_span(&mut self, span: SrcSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum Exclusivity {
    Iso,
    Poly,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ReferenceType {
    pub span: SrcSpan,
    pub lifetime: Option<Lifetime>,
    pub exclusivity: Option<Exclusivity>,
    pub mutability: Option<Mutability>,
    pub to: Type,
}

impl Spanned for ReferenceType {
    fn span(&self) -> SrcSpan {
        self.span
    }
    fn set_span(&mut self, span: SrcSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PointerType {
    pub span: SrcSpan,
    pub lifetime: Option<Lifetime>,
    pub exclusivity: Option<Exclusivity>,
    pub mutability: Option<Mutability>,
    pub to: Type,
}

impl Spanned for PointerType {
    fn span(&self) -> SrcSpan {
        self.span
    }
    fn set_span(&mut self, span: SrcSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TypePotential {
    pub span: SrcSpan,
    pub body: Block,
}

impl Spanned for TypePotential {
    fn span(&self) -> SrcSpan {
        self.span
    }
    fn set_span(&mut self, span: SrcSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TypeParentheses {
    pub span: SrcSpan,
    pub inner: Type,
}

impl Spanned for TypeParentheses {
    fn span(&self) -> SrcSpan {
        self.span
    }
    fn set_span(&mut self, span: SrcSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
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
    TypePotential(Box<TypePotential>),
    Lifetime(Box<Lifetime>),
    Parentheses(Box<TypeParentheses>),
}

impl Type {
    pub fn span(&self) -> SrcSpan {
        match self {
            Type::SyntaxError(e) => e.span,
            Type::Bool(e) => e.span,
            Type::UInt8(e) => e.span,
            Type::UInt16(e) => e.span,
            Type::UInt32(e) => e.span,
            Type::UInt64(e) => e.span,
            Type::UInt128(e) => e.span,
            Type::USize(e) => e.span,
            Type::Int8(e) => e.span,
            Type::Int16(e) => e.span,
            Type::Int32(e) => e.span,
            Type::Int64(e) => e.span,
            Type::Int128(e) => e.span,
            Type::Float32(e) => e.span,
            Type::Float64(e) => e.span,
            Type::InferType(e) => e.span,
            Type::TypePath(e) => e.span,
            Type::RefinementType(e) => e.span,
            Type::TupleType(e) => e.span,
            Type::ArrayType(e) => e.span,
            Type::SliceType(e) => e.span,
            Type::FunctionType(e) => e.span,
            Type::ReferenceType(e) => e.span,
            Type::PointerType(e) => e.span,
            Type::TypePotential(e) => e.span,
            Type::Lifetime(e) => e.span,
            Type::Parentheses(e) => e.span,
        }
    }
    pub fn set_span(&mut self, span: SrcSpan) {
        match self {
            Type::SyntaxError(e) => e.span = span,
            Type::Bool(e) => e.span = span,
            Type::UInt8(e) => e.span = span,
            Type::UInt16(e) => e.span = span,
            Type::UInt32(e) => e.span = span,
            Type::UInt64(e) => e.span = span,
            Type::UInt128(e) => e.span = span,
            Type::USize(e) => e.span = span,
            Type::Int8(e) => e.span = span,
            Type::Int16(e) => e.span = span,
            Type::Int32(e) => e.span = span,
            Type::Int64(e) => e.span = span,
            Type::Int128(e) => e.span = span,
            Type::Float32(e) => e.span = span,
            Type::Float64(e) => e.span = span,
            Type::InferType(e) => e.span = span,
            Type::TypePath(e) => e.span = span,
            Type::RefinementType(e) => e.span = span,
            Type::TupleType(e) => e.span = span,
            Type::ArrayType(e) => e.span = span,
            Type::SliceType(e) => e.span = span,
            Type::FunctionType(e) => e.span = span,
            Type::ReferenceType(e) => e.span = span,
            Type::PointerType(e) => e.span = span,
            Type::TypePotential(e) => e.span = span,
            Type::Lifetime(e) => e.span = span,
            Type::Parentheses(e) => e.span = span,
        }
    }
    pub fn reconstruct(&self, source: &[u8]) -> String {
        self.span().extract_str(source).to_string()
    }
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
    pub fn as_type_potential(self) -> Option<TypePotential> {
        match self {
            Type::TypePotential(b) => Some(*b),
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
