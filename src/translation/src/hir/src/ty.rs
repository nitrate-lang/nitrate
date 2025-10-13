use crate::prelude::*;
use crate::store::LiteralId;
use interned_string::IString;
use serde::{Deserialize, Serialize};
use std::collections::{BTreeMap, BTreeSet};
use std::num::NonZeroU32;
use thin_vec::ThinVec;

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum StructAttribute {
    Packed,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum EnumAttribute {}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum FunctionAttribute {
    Variadic,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum Lifetime {
    Static,
    Gc,
    ThreadLocal,
    TaskLocal,
    Stack { id: EntityName },
    Inferred,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct StructType {
    pub attributes: BTreeSet<StructAttribute>,
    pub fields: BTreeMap<IString, TypeId>,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct EnumType {
    pub attributes: BTreeSet<EnumAttribute>,
    pub variants: BTreeMap<IString, TypeId>,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct FunctionType {
    pub attributes: BTreeSet<FunctionAttribute>,
    pub params: Vec<ParameterId>,
    pub return_type: TypeId,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
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
    F8,
    F16,
    F32,
    F64,
    F128,

    Opaque {
        name: IString,
    },

    Array {
        element_type: TypeId,
        len: u64,
    },

    Tuple {
        element_types: ThinVec<Type>,
    },

    Slice {
        element_type: TypeId,
    },

    Struct {
        struct_type: StructTypeId,
    },

    Enum {
        enum_type: EnumTypeId,
    },

    Refine {
        base: TypeId,
        min: LiteralId,
        max: LiteralId,
    },

    Bitfield {
        base: TypeId,
        bits: u8,
    },

    Function {
        function_type: FunctionTypeId,
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

    TypeAlias {
        name: IString,
        aliased: TypeId,
    },

    InferredFloat,
    InferredInteger,
    Inferred {
        id: NonZeroU32,
    },
}

impl Type {
    pub fn is_never(&self) -> bool {
        matches!(self, Type::Never)
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
            Type::I8 | Type::I16 | Type::I32 | Type::I64 | Type::I128
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

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub enum PtrSize {
    U32 = 4,
    U64 = 8,
}

impl IntoStoreId for StructType {
    type Id = StructTypeId;

    fn into_id(self, store: &Store) -> Self::Id {
        store.store_struct_type(self)
    }
}

impl IntoStoreId for EnumType {
    type Id = EnumTypeId;

    fn into_id(self, store: &Store) -> Self::Id {
        store.store_enum_type(self)
    }
}

impl IntoStoreId for FunctionType {
    type Id = FunctionTypeId;

    fn into_id(self, store: &Store) -> Self::Id {
        store.store_function_type(self)
    }
}

impl IntoStoreId for Type {
    type Id = TypeId;

    fn into_id(self, store: &Store) -> Self::Id {
        store.store_type(self)
    }
}
