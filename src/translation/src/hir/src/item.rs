use crate::prelude::*;
use interned_string::IString;
use serde::{Deserialize, Serialize};
use std::collections::BTreeSet;

pub type QualifiedName = IString;

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct EntityName(pub IString);

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum Visibility {
    Sec,
    Pro,
    Pub,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct GlobalVariable {
    pub visibility: Visibility,
    pub is_mutable: bool,
    pub name: EntityName,
    pub ty: TypeId,
    pub initializer: ValueId,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct LocalVariable {
    pub is_mutable: bool,
    pub name: EntityName,
    pub ty: TypeId,
    pub initializer: ValueId,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct Parameter {
    pub name: EntityName,
    pub ty: TypeId,
    pub default_value: Option<ValueId>,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct Function {
    pub visibility: Visibility,
    pub attributes: BTreeSet<FunctionAttribute>,
    pub name: EntityName,
    pub parameters: Vec<ParameterId>,
    pub return_type: TypeId,
    pub body: BlockId,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct Closure {
    pub closure_unique_id: u64,
    pub captures: Vec<SymbolId>,
    pub callee: FunctionId,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct Trait {
    pub visibility: Visibility,
    pub name: EntityName,
    pub methods: Vec<FunctionId>,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum ModuleAttribute {}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct Module {
    pub visibility: Visibility,
    pub name: EntityName,
    pub attributes: Vec<ModuleAttribute>,
    pub items: Vec<Item>,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum Symbol {
    Unresolved { name: QualifiedName },
    GlobalVariable(GlobalVariableId),
    LocalVariable(LocalVariableId),
    Trait(TraitId),
    Parameter(ParameterId),
    Function(FunctionId),
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum Item {
    Module(ModuleId),
    GlobalVariable(GlobalVariableId),
    Function(FunctionId),
    Trait(TraitId),
}

impl IntoStoreId for GlobalVariable {
    type Id = GlobalVariableId;

    fn into_id(self, store: &Store) -> Self::Id {
        store.store_global_variable(self)
    }
}

impl IntoStoreId for LocalVariable {
    type Id = LocalVariableId;

    fn into_id(self, store: &Store) -> Self::Id {
        store.store_local_variable(self)
    }
}

impl IntoStoreId for Parameter {
    type Id = ParameterId;

    fn into_id(self, store: &Store) -> Self::Id {
        store.store_parameter(self)
    }
}

impl IntoStoreId for Function {
    type Id = FunctionId;

    fn into_id(self, store: &Store) -> Self::Id {
        store.store_function(self)
    }
}

impl IntoStoreId for Trait {
    type Id = TraitId;

    fn into_id(self, store: &Store) -> Self::Id {
        store.store_trait(self)
    }
}

impl IntoStoreId for Module {
    type Id = ModuleId;

    fn into_id(self, store: &Store) -> Self::Id {
        store.store_module(self)
    }
}
