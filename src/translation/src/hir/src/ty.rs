use crate::prelude::*;
use interned_string::IString;
use serde::{Deserialize, Serialize};
use std::collections::HashSet;
use std::num::NonZeroU32;

#[derive(Debug, Serialize, Deserialize)]
pub enum Lifetime {
    Static,
    Gc,
    ThreadLocal,
    TaskLocal,
    Stack { id: EntityName },
    Inferred,
}

#[derive(Debug, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum StructAttribute {
    Packed,
}

#[derive(Debug, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum EnumAttribute {}

#[derive(Debug, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum FunctionAttribute {
    Variadic,
}

#[derive(Debug, Serialize, Deserialize)]
pub enum Type {
    Never,
    Unit,
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

    Array {
        element_type: TypeId,
        len: u64,
    },

    Tuple {
        elements: Vec<TypeId>,
    },

    Slice {
        element_type: TypeId,
    },

    Struct {
        attributes: Box<HashSet<StructAttribute>>,
        fields: Vec<(String, TypeId)>,
    },

    Enum {
        attributes: Box<HashSet<EnumAttribute>>,
        variants: Vec<(String, TypeId)>,
    },

    Function {
        attributes: Box<HashSet<FunctionAttribute>>,
        parameters: Vec<TypeId>,
        return_type: TypeId,
    },

    Reference {
        lifetime: Lifetime,
        exclusive: bool,
        mutable: bool,
        to: TypeId,
    },

    Pointer {
        exclusive: bool,
        mutable: bool,
        to: TypeId,
    },

    InferredFloat,

    InferredInteger {
        signed: bool,
    },

    Inferred {
        id: NonZeroU32,
    },

    TypeAlias {
        name: IString,
        aliased: TypeId,
    },
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
        matches!(self, Type::Function { .. })
    }

    pub fn is_reference(&self) -> bool {
        matches!(self, Type::Reference { .. })
    }
}

impl IntoStoreId for Type {
    type Id = TypeId;

    fn into_id(self, ctx: &mut Store) -> Self::Id {
        ctx.store_type(self)
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub enum PointerSize {
    U8 = 1,
    U16 = 2,
    U32 = 4,
    U64 = 8,
    U128 = 16,
}
