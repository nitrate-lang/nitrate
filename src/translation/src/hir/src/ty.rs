use crate::prelude::*;
use crate::store::LiteralId;
use nitrate_nstring::NString;
use serde::{Deserialize, Serialize};
use std::collections::BTreeSet;
use std::num::NonZeroU32;
use thin_vec::ThinVec;

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum Lifetime {
    Static,
    Gc,
    ThreadLocal,
    TaskLocal,
    Inferred,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum StructAttribute {
    Packed,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum StructFieldAttribute {
    Invalid,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct StructField {
    pub attributes: BTreeSet<StructFieldAttribute>,
    pub name: NString,
    pub ty: TypeId,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct StructType {
    pub attributes: BTreeSet<StructAttribute>,
    pub fields: ThinVec<StructField>,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum EnumAttribute {
    Invalid,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum EnumVariantAttribute {
    Invalid,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct EnumVariant {
    pub attributes: BTreeSet<EnumVariantAttribute>,
    pub name: NString,
    pub ty: TypeId,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct EnumType {
    pub attributes: BTreeSet<EnumAttribute>,
    pub variants: ThinVec<EnumVariant>,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum FunctionAttribute {
    CVariadic,
    NoMangle,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct FunctionType {
    pub attributes: BTreeSet<FunctionAttribute>,
    pub params: ThinVec<(NString, TypeId)>,
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
    F32,
    F64,

    Array {
        element_type: TypeId,
        len: u32,
    },

    Tuple {
        element_types: ThinVec<TypeId>,
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

    Function {
        function_type: FunctionTypeId,
    },

    Reference {
        lifetime: Lifetime,
        exclusive: bool,
        mutable: bool,
        to: TypeId,
    },

    SliceRef {
        lifetime: Lifetime,
        exclusive: bool,
        mutable: bool,
        element_type: TypeId,
    },

    Pointer {
        exclusive: bool,
        mutable: bool,
        to: TypeId,
    },

    InferredFloat,
    InferredInteger,
    Inferred {
        id: NonZeroU32,
    },
}

impl Type {
    pub fn is_diverging(&self) -> bool {
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
        matches!(self, Type::F32 | Type::F64)
    }

    pub fn is_array(&self) -> bool {
        matches!(self, Type::Array { .. })
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

    pub fn is_pointer(&self) -> bool {
        matches!(self, Type::Pointer { .. })
    }

    pub fn is_slice_ref(&self) -> bool {
        matches!(self, Type::SliceRef { .. })
    }

    pub fn as_struct(&self) -> Option<&StructTypeId> {
        if let Type::Struct { struct_type } = self {
            Some(struct_type)
        } else {
            None
        }
    }

    pub fn as_enum(&self) -> Option<&EnumTypeId> {
        if let Type::Enum { enum_type } = self {
            Some(enum_type)
        } else {
            None
        }
    }

    pub fn as_function(&self) -> Option<&FunctionTypeId> {
        if let Type::Function { function_type } = self {
            Some(function_type)
        } else {
            None
        }
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
