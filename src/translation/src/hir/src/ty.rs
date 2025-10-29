use crate::store::LiteralId;
use crate::{SymbolTab, prelude::*};
use interned_string::IString;
use serde::{Deserialize, Serialize};
use std::collections::{BTreeSet, HashSet};
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
    pub name: IString,
    pub ty: TypeId,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct StructType {
    pub attributes: BTreeSet<StructAttribute>,
    pub fields: Vec<StructField>,
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
    pub name: IString,
    pub ty: TypeId,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct EnumType {
    pub attributes: BTreeSet<EnumAttribute>,
    pub variants: Vec<EnumVariant>,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum FunctionAttribute {
    Variadic,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct FunctionType {
    pub attributes: BTreeSet<FunctionAttribute>,
    pub params: Vec<(IString, TypeId)>,
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

    Symbol {
        path: IString,
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

    pub fn is_slice_ref(&self) -> bool {
        matches!(self, Type::SliceRef { .. })
    }

    fn is_known_inner(
        &self,
        store: &Store,
        symtab: &SymbolTab,
        visited: &mut HashSet<IString>,
    ) -> bool {
        self.iter().all(store, &mut |ty| {
            if let Type::Inferred { .. } | Type::InferredFloat | Type::InferredInteger = ty {
                return false;
            }

            if let Type::Reference { lifetime, .. } = ty {
                let ok = match lifetime {
                    Lifetime::Static
                    | Lifetime::Gc
                    | Lifetime::ThreadLocal
                    | Lifetime::TaskLocal => true,

                    Lifetime::Inferred => false,
                };

                return ok;
            }

            if let Type::Symbol { path } = ty {
                if visited.contains(path) {
                    return true;
                }

                visited.insert(path.clone());

                match symtab.get_type(path) {
                    Some(TypeDefinition::EnumDef(id)) => {
                        let enum_def = store[id].borrow();
                        let enum_type = Type::Enum {
                            enum_type: enum_def.enum_id,
                        };

                        return enum_type.is_known_inner(store, symtab, visited);
                    }

                    Some(TypeDefinition::StructDef(id)) => {
                        let struct_def = store[id].borrow();
                        let struct_type = Type::Struct {
                            struct_type: struct_def.struct_id,
                        };

                        return struct_type.is_known_inner(store, symtab, visited);
                    }

                    Some(TypeDefinition::TypeAliasDef(id)) => {
                        let type_alias_def = store[id].borrow();
                        let aliased_type = &store[&type_alias_def.type_id];
                        return aliased_type.is_known_inner(store, symtab, visited);
                    }

                    None => return false,
                };
            }

            true
        })
    }

    pub fn is_known(&self, store: &Store, symtab: &SymbolTab) -> bool {
        let mut visited = HashSet::new();
        self.is_known_inner(store, symtab, &mut visited)
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
