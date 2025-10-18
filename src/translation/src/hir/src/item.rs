use crate::prelude::*;
use interned_string::IString;
use serde::{Deserialize, Serialize};
use std::collections::BTreeSet;

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum Visibility {
    Sec,
    Pro,
    Pub,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum GlobalVariableAttribute {}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct GlobalVariable {
    pub visibility: Visibility,
    pub attributes: BTreeSet<GlobalVariableAttribute>,
    pub is_mutable: bool,
    pub name: IString,
    pub ty: TypeId,
    pub initializer: ValueId,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum LocalVariableAttribute {}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct LocalVariable {
    pub attributes: BTreeSet<LocalVariableAttribute>,
    pub is_mutable: bool,
    pub name: IString,
    pub ty: TypeId,
    pub initializer: ValueId,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum ParameterAttribute {}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct Parameter {
    pub attributes: BTreeSet<ParameterAttribute>,
    pub is_mutable: bool,
    pub name: IString,
    pub ty: TypeId,
    pub default_value: Option<ValueId>,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct Function {
    pub visibility: Visibility,
    pub attributes: BTreeSet<FunctionAttribute>,
    pub name: IString,
    pub parameters: Vec<ParameterId>,
    pub return_type: TypeId,
    pub body: Option<BlockId>,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct Trait {
    pub visibility: Visibility,
    pub name: IString,
    pub methods: Vec<FunctionId>,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum ModuleAttribute {}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct Module {
    pub visibility: Visibility,
    pub name: Option<IString>,
    pub attributes: BTreeSet<ModuleAttribute>,
    pub items: Vec<Item>,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum Symbol {
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
