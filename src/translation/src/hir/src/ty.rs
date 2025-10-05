use hashbrown::{HashMap, HashSet};
use serde::{Deserialize, Serialize};
use std::num::NonZeroU32;

use crate::{DumpContext, TypeId, dump::Dump};

#[derive(Debug, Serialize, Deserialize)]
pub enum Lifetime {
    Static,
    Gc,
    ThreadLocal,
    TaskLocal,
    Stack { id: NonZeroU32 },
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Reference {
    pub lifetime: Lifetime,
    pub exclusive: bool,
    pub mutable: bool,
    pub to: TypeId,
}

#[derive(Debug, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum StructAttribute {
    Packed,
}

impl Dump for StructAttribute {
    fn dump(
        &self,
        _ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match self {
            StructAttribute::Packed => write!(o, "packed"),
        }
    }
}

#[derive(Debug, Serialize, Deserialize)]
pub struct StructType {
    pub attributes: HashSet<StructAttribute>,
    pub fields: Vec<(String, TypeId)>,
}

#[derive(Debug, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum EnumAttribute {}

impl Dump for EnumAttribute {
    fn dump(
        &self,
        _ctx: &mut DumpContext,
        _o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match self {
            _ => Ok(()),
        }
    }
}

#[derive(Debug, Serialize, Deserialize)]
pub struct EnumType {
    pub attributes: HashSet<EnumAttribute>,
    pub variants: HashMap<String, TypeId>,
}

#[derive(Debug, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum FunctionAttribute {
    Variadic,
}

impl Dump for FunctionAttribute {
    fn dump(
        &self,
        _ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match self {
            FunctionAttribute::Variadic => write!(o, "variadic"),
        }
    }
}

#[derive(Debug, Serialize, Deserialize)]
pub struct FunctionType {
    pub attributes: HashSet<FunctionAttribute>,
    pub parameters: Vec<TypeId>,
    pub return_type: TypeId,
}

#[derive(Debug, Serialize, Deserialize)]
pub enum Type {
    /* ----------------------------------------------------- */
    /* Primitive Types                                       */
    Never,
    Bool,
    U8,
    U16,
    U32,
    U64,
    U128,
    USize,
    I8,
    I16,
    I32,
    I64,
    I128,
    ISize,
    F8,
    F16,
    F32,
    F64,
    F128,

    /* ----------------------------------------------------- */
    /* Compound Types                                        */
    Array { element_type: TypeId, len: u64 },
    Tuple { elements: Vec<TypeId> },
    Slice { element_type: TypeId },
    Struct(Box<StructType>),
    Enum(Box<EnumType>),

    /* ----------------------------------------------------- */
    /* Function Type                                         */
    Function(Box<FunctionType>),

    /* ----------------------------------------------------- */
    /* Reference Type                                        */
    Reference(Box<Reference>),
}

impl Type {
    #[allow(non_upper_case_globals)]
    pub const Unit: Type = Type::Tuple { elements: vec![] };
}

impl Type {
    pub fn is_never(&self) -> bool {
        matches!(self, Type::Never)
    }

    pub fn is_unit(&self) -> bool {
        matches!(self, Type::Tuple { elements } if elements.is_empty())
    }

    pub fn is_bool(&self) -> bool {
        matches!(self, Type::Bool)
    }

    pub fn is_unsigned_primitive(&self) -> bool {
        matches!(
            self,
            Type::U8 | Type::U16 | Type::U32 | Type::U64 | Type::U128 | Type::USize
        )
    }

    pub fn is_signed_primitive(&self) -> bool {
        matches!(
            self,
            Type::I8 | Type::I16 | Type::I32 | Type::I64 | Type::I128 | Type::ISize
        )
    }

    pub fn is_integer_primitive(&self) -> bool {
        self.is_unsigned_primitive() || self.is_signed_primitive()
    }

    pub fn is_float_primitive(&self) -> bool {
        matches!(
            self,
            Type::F8 | Type::F16 | Type::F32 | Type::F64 | Type::F128
        )
    }

    pub fn is_array(&self) -> bool {
        matches!(self, Type::Array { .. })
    }

    pub fn is_slice(&self) -> bool {
        matches!(self, Type::Slice { .. })
    }

    pub fn is_tuple(&self) -> bool {
        matches!(self, Type::Tuple { .. })
    }

    pub fn is_struct(&self) -> bool {
        matches!(self, Type::Struct { .. })
    }

    pub fn is_enum(&self) -> bool {
        matches!(self, Type::Enum { .. })
    }

    pub fn is_function(&self) -> bool {
        matches!(self, Type::Function(_))
    }

    pub fn is_reference(&self) -> bool {
        matches!(self, Type::Reference(_))
    }
}

#[derive(Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub enum PointerSize {
    U8 = 1,
    U16 = 2,
    U32 = 4,
    U64 = 8,
    U128 = 16,
}
