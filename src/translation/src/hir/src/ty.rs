use crate::prelude::*;
use crate::store::LiteralId;
use interned_string::IString;
use serde::{Deserialize, Serialize};
use std::collections::{BTreeMap, BTreeSet};
use std::num::NonZeroU32;

pub type TypeList = Vec<TypeId>;

impl IntoStoreId for TypeList {
    type Id = TypeListId;

    fn into_id(self, ctx: &Store) -> Self::Id {
        ctx.store_type_list(self)
    }
}

pub type StructFields = Vec<(IString, TypeId)>;

impl IntoStoreId for StructFields {
    type Id = StructFieldsId;

    fn into_id(self, ctx: &Store) -> Self::Id {
        ctx.store_struct_fields(self)
    }
}

pub type EnumVariants = BTreeMap<IString, TypeId>;

impl IntoStoreId for EnumVariants {
    type Id = EnumVariantsId;

    fn into_id(self, ctx: &Store) -> Self::Id {
        ctx.store_enum_variants(self)
    }
}

pub type StructAttributes = BTreeSet<StructAttribute>;

impl IntoStoreId for StructAttributes {
    type Id = StructAttributesId;

    fn into_id(self, ctx: &Store) -> Self::Id {
        ctx.store_struct_attributes(self)
    }
}

pub type EnumAttributes = BTreeSet<EnumAttribute>;

impl IntoStoreId for EnumAttributes {
    type Id = EnumAttributesId;

    fn into_id(self, ctx: &Store) -> Self::Id {
        ctx.store_enum_attributes(self)
    }
}

pub type FunctionAttributes = BTreeSet<FunctionAttribute>;

impl IntoStoreId for FunctionAttributes {
    type Id = FuncAttributesId;

    fn into_id(self, ctx: &Store) -> Self::Id {
        ctx.store_function_attributes(self)
    }
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

#[derive(Debug, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum StructAttribute {
    Packed,
}

#[derive(Debug, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum EnumAttribute {}

#[derive(Debug, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum FunctionAttribute {
    Variadic,
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
        element_types: TypeListId,
    },

    Slice {
        element_type: TypeId,
    },

    Struct {
        attributes: StructAttributesId,
        fields: StructFieldsId,
    },

    Enum {
        attributes: EnumAttributesId,
        variants: EnumVariantsId,
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
        attributes: FuncAttributesId,
        parameters: TypeListId,
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

impl IntoStoreId for Type {
    type Id = TypeId;

    fn into_id(self, ctx: &Store) -> Self::Id {
        ctx.store_type(self)
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub enum PtrSize {
    U32 = 4,
    U64 = 8,
}
