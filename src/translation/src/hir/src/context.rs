use crate::prelude::*;
use std::cell::RefCell;
use std::collections::{HashMap, HashSet};
use std::num::NonZeroU32;
use std::ops::Deref;

pub struct HirCtx {
    store: Store,
    symbol_map: HashMap<QualifiedName, SymbolId>,
    type_map: HashMap<QualifiedName, TypeId>,
    impl_map: HashMap<TypeId, HashSet<TraitId>>,
    type_infer_id_ctr: NonZeroU32,
    unique_name_ctr: u32,
    ptr_size: PtrSize,
}

impl HirCtx {
    pub fn new(ptr_size: PtrSize) -> Self {
        Self {
            store: Store::new(),
            symbol_map: HashMap::new(),
            type_map: HashMap::new(),
            impl_map: HashMap::new(),
            type_infer_id_ctr: NonZeroU32::new(1).unwrap(),
            unique_name_ctr: 0,
            ptr_size,
        }
    }

    pub fn store(&self) -> &Store {
        &self.store
    }

    pub fn store_mut(&mut self) -> &mut Store {
        &mut self.store
    }

    pub fn ptr_size(&self) -> PtrSize {
        self.ptr_size
    }

    pub fn resolve_symbol(&self, name: &QualifiedName) -> Option<&SymbolId> {
        self.symbol_map.get(name)
    }

    pub fn resolve_type(&self, name: &QualifiedName) -> Option<&TypeId> {
        self.type_map.get(name)
    }

    pub fn has_trait(&self, ty: &TypeId, trait_id: &TraitId) -> bool {
        if let Some(impls) = self.impl_map.get(ty) {
            impls.contains(trait_id)
        } else {
            false
        }
    }

    pub fn find_unambiguous_trait_method(
        &self,
        ty: &TypeId,
        method_name: &str,
    ) -> Option<FunctionId> {
        let trait_set = match self.impl_map.get(ty) {
            Some(trait_set) => trait_set,
            None => return None,
        };

        let mut found: Option<FunctionId> = None;

        for trait_id in trait_set {
            let trait_def = &self[trait_id].borrow();

            for method_id in &trait_def.methods {
                let method_def = &self[method_id].borrow();

                if method_def.name.deref() == method_name {
                    if found.is_some() {
                        // Ambiguous, multiple traits have the same method
                        return None;
                    } else {
                        found = Some(method_id.clone());
                    }
                }
            }
        }

        found
    }

    pub fn get_unique_name(&mut self) -> EntityName {
        const COMPILER_RESERVED_PREFIX: &str = "⚙️";

        let name = format!("{}{}", COMPILER_RESERVED_PREFIX, self.unique_name_ctr);
        self.unique_name_ctr += 1;
        name.into()
    }

    pub fn create_inference_placeholder(&mut self) -> Type {
        let id = self.type_infer_id_ctr;
        self.type_infer_id_ctr = id.checked_add(1).expect("Type infer ID overflow");
        Type::Inferred { id }
    }

    pub fn create_std_meta_type_instance(&mut self, of: Type) -> Value {
        // TODO: Finish this function
        let std_meta_type = self
            .resolve_type(&"::std::meta::Type".into())
            .expect("compiler prelude is missing the defintion of `::std::meta::Type`")
            .to_owned();

        if !self[&std_meta_type].is_enum() {
            panic!("compiler prelude has an invalid definition of `::std::meta::Type`");
        }

        let enum_type = match &self[&std_meta_type] {
            Type::Enum { enum_type } => enum_type.to_owned(),
            _ => unreachable!(),
        };

        match of {
            Type::Never => Value::EnumVariant {
                enum_type,
                variant: "Never".into(),
                value: Value::Unit.into_id(self.store()),
            },

            Type::Unit => Value::EnumVariant {
                enum_type,
                variant: "Unit".into(),
                value: Value::Unit.into_id(self.store()),
            },

            Type::Bool => Value::EnumVariant {
                enum_type,
                variant: "Bool".into(),
                value: Value::Unit.into_id(self.store()),
            },

            Type::U8 => Value::EnumVariant {
                enum_type,
                variant: "U8".into(),
                value: Value::Unit.into_id(self.store()),
            },

            Type::U16 => Value::EnumVariant {
                enum_type,
                variant: "U16".into(),
                value: Value::Unit.into_id(self.store()),
            },

            Type::U32 => Value::EnumVariant {
                enum_type,
                variant: "U32".into(),
                value: Value::Unit.into_id(self.store()),
            },

            Type::U64 => Value::EnumVariant {
                enum_type,
                variant: "U64".into(),
                value: Value::Unit.into_id(self.store()),
            },

            Type::U128 => Value::EnumVariant {
                enum_type,
                variant: "U128".into(),
                value: Value::Unit.into_id(self.store()),
            },

            Type::USize => Value::EnumVariant {
                enum_type,
                variant: "USize".into(),
                value: Value::Unit.into_id(self.store()),
            },

            Type::I8 => Value::EnumVariant {
                enum_type,
                variant: "I8".into(),
                value: Value::Unit.into_id(self.store()),
            },

            Type::I16 => Value::EnumVariant {
                enum_type,
                variant: "I16".into(),
                value: Value::Unit.into_id(self.store()),
            },

            Type::I32 => Value::EnumVariant {
                enum_type,
                variant: "I32".into(),
                value: Value::Unit.into_id(self.store()),
            },

            Type::I64 => Value::EnumVariant {
                enum_type,
                variant: "I64".into(),
                value: Value::Unit.into_id(self.store()),
            },

            Type::I128 => Value::EnumVariant {
                enum_type,
                variant: "I128".into(),
                value: Value::Unit.into_id(self.store()),
            },

            Type::F8 => Value::EnumVariant {
                enum_type,
                variant: "F8".into(),
                value: Value::Unit.into_id(self.store()),
            },

            Type::F16 => Value::EnumVariant {
                enum_type,
                variant: "F16".into(),
                value: Value::Unit.into_id(self.store()),
            },

            Type::F32 => Value::EnumVariant {
                enum_type,
                variant: "F32".into(),
                value: Value::Unit.into_id(self.store()),
            },

            Type::F64 => Value::EnumVariant {
                enum_type,
                variant: "F64".into(),
                value: Value::Unit.into_id(self.store()),
            },

            Type::F128 => Value::EnumVariant {
                enum_type,
                variant: "F128".into(),
                value: Value::Unit.into_id(self.store()),
            },

            Type::Opaque { name } => todo!(),

            Type::Array { element_type, len } => todo!(),

            Type::Tuple { element_types } => todo!(),

            Type::Slice { element_type } => todo!(),

            Type::Struct { struct_type } => todo!(),

            Type::Enum { enum_type } => todo!(),

            Type::Refine { base, min, max } => todo!(),

            Type::Bitfield { base, bits } => todo!(),

            Type::Function { function_type } => todo!(),

            Type::Reference {
                lifetime,
                exclusive,
                mutable,
                to,
            } => todo!(),

            Type::Pointer {
                exclusive,
                mutable,
                to,
            } => todo!(),

            Type::TypeAlias { name, aliased } => todo!(),

            Type::InferredFloat => todo!(),

            Type::InferredInteger => todo!(),

            Type::Inferred { id } => todo!(),
        }
    }
}

impl std::ops::Index<&TypeId> for HirCtx {
    type Output = Type;

    fn index(&self, index: &TypeId) -> &Self::Output {
        &self.store[index]
    }
}

impl std::ops::Index<&StructTypeId> for HirCtx {
    type Output = StructType;

    fn index(&self, index: &StructTypeId) -> &Self::Output {
        &self.store[index]
    }
}

impl std::ops::Index<&EnumTypeId> for HirCtx {
    type Output = EnumType;

    fn index(&self, index: &EnumTypeId) -> &Self::Output {
        &self.store[index]
    }
}

impl std::ops::Index<&FunctionTypeId> for HirCtx {
    type Output = FunctionType;

    fn index(&self, index: &FunctionTypeId) -> &Self::Output {
        &self.store[index]
    }
}

impl std::ops::Index<&SymbolId> for HirCtx {
    type Output = RefCell<Symbol>;

    fn index(&self, index: &SymbolId) -> &Self::Output {
        &self.store[index]
    }
}

impl std::ops::Index<&GlobalVariableId> for HirCtx {
    type Output = RefCell<GlobalVariable>;

    fn index(&self, index: &GlobalVariableId) -> &Self::Output {
        &self.store[index]
    }
}

impl std::ops::Index<&LocalVariableId> for HirCtx {
    type Output = RefCell<LocalVariable>;

    fn index(&self, index: &LocalVariableId) -> &Self::Output {
        &self.store[index]
    }
}

impl std::ops::Index<&ParameterId> for HirCtx {
    type Output = RefCell<Parameter>;

    fn index(&self, index: &ParameterId) -> &Self::Output {
        &self.store[index]
    }
}

impl std::ops::Index<&FunctionId> for HirCtx {
    type Output = RefCell<Function>;

    fn index(&self, index: &FunctionId) -> &Self::Output {
        &self.store[index]
    }
}

impl std::ops::Index<&TraitId> for HirCtx {
    type Output = RefCell<Trait>;

    fn index(&self, index: &TraitId) -> &Self::Output {
        &self.store[index]
    }
}

impl std::ops::Index<&ModuleId> for HirCtx {
    type Output = RefCell<Module>;

    fn index(&self, index: &ModuleId) -> &Self::Output {
        &self.store[index]
    }
}

impl std::ops::Index<&ValueId> for HirCtx {
    type Output = RefCell<Value>;

    fn index(&self, index: &ValueId) -> &Self::Output {
        &self.store[index]
    }
}

impl std::ops::Index<&LiteralId> for HirCtx {
    type Output = Lit;

    fn index(&self, index: &LiteralId) -> &Self::Output {
        &self.store[index]
    }
}

impl std::ops::Index<&BlockId> for HirCtx {
    type Output = RefCell<Block>;

    fn index(&self, index: &BlockId) -> &Self::Output {
        &self.store[index]
    }
}
