use crate::prelude::*;
use std::cell::RefCell;
use std::collections::{HashMap, HashSet};
use std::num::NonZeroU32;
use std::ops::Deref;

#[derive(Debug)]
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
