use crate::prelude::*;
use nitrate_nstring::NString;
use serde::{Deserialize, Serialize};
use std::collections::BTreeSet;

#[derive(Debug, Clone, Copy, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum Visibility {
    Sec,
    Pro,
    Pub,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum GlobalVariableAttribute {
    Invalid,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct GlobalVariable {
    pub visibility: Visibility,
    pub attributes: BTreeSet<GlobalVariableAttribute>,
    pub is_mutable: bool,
    pub name: NString,
    pub ty: TypeId,
    pub init: ValueId,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum LocalVariableAttribute {
    Invalid,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum LocalVariableKind {
    Stack,
    Dynamic,
    Static,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct LocalVariable {
    pub kind: LocalVariableKind,
    pub attributes: BTreeSet<LocalVariableAttribute>,
    pub is_mutable: bool,
    pub name: NString,
    pub ty: TypeId,
    pub init: Option<ValueId>,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum ParameterAttribute {
    Invalid,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct Parameter {
    pub attributes: BTreeSet<ParameterAttribute>,
    pub is_mutable: bool,
    pub name: NString,
    pub ty: TypeId,
    pub default_value: Option<ValueId>,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct Function {
    pub visibility: Visibility,
    pub attributes: BTreeSet<FunctionAttribute>,
    pub name: NString,
    pub params: Vec<ParameterId>,
    pub return_type: TypeId,
    pub body: Option<BlockId>,
}

impl Function {
    pub fn get_type(&self, store: &Store) -> FunctionType {
        let params: Vec<(NString, TypeId)> = self
            .params
            .iter()
            .map(|param_id| {
                let p = store[param_id].borrow();
                (p.name.clone(), p.ty)
            })
            .collect();

        FunctionType {
            attributes: self.attributes.clone(),
            params,
            return_type: self.return_type,
        }
    }
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct Trait {
    pub visibility: Visibility,
    pub name: NString,
    pub methods: Vec<FunctionId>,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum ModuleAttribute {
    Invalid,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct Module {
    pub visibility: Visibility,
    pub name: NString,
    pub attributes: BTreeSet<ModuleAttribute>,
    pub items: Vec<Item>,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct TypeAliasDef {
    pub visibility: Visibility,
    pub name: NString,
    pub type_id: TypeId,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct StructDef {
    pub visibility: Visibility,
    pub name: NString,
    pub field_extras: Vec<(Visibility, Option<ValueId>)>,
    pub struct_id: StructTypeId,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct EnumDef {
    pub visibility: Visibility,
    pub name: NString,
    pub variant_extras: Vec<Option<ValueId>>,
    pub enum_id: EnumTypeId,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum TypeDefinition {
    TypeAliasDef(TypeAliasDefId),
    StructDef(StructDefId),
    EnumDef(EnumDefId),
}

impl TypeDefinition {
    pub fn name(&self, store: &Store) -> NString {
        match self {
            TypeDefinition::TypeAliasDef(def) => store[def].borrow().name.clone(),
            TypeDefinition::StructDef(def) => store[def].borrow().name.clone(),
            TypeDefinition::EnumDef(def) => store[def].borrow().name.clone(),
        }
    }
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum SymbolId {
    GlobalVariable(GlobalVariableId),
    LocalVariable(LocalVariableId),
    Parameter(ParameterId),
    Function(FunctionId),
}

impl SymbolId {
    pub fn name(&self, store: &Store) -> NString {
        match self {
            SymbolId::Function(id) => store[id].borrow().name.clone(),
            SymbolId::GlobalVariable(id) => store[id].borrow().name.clone(),
            SymbolId::LocalVariable(id) => store[id].borrow().name.clone(),
            SymbolId::Parameter(id) => store[id].borrow().name.clone(),
        }
    }
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum Item {
    Module(ModuleId),
    GlobalVariable(GlobalVariableId),
    Function(FunctionId),
    TypeAliasDef(TypeAliasDefId),
    StructDef(StructDefId),
    EnumDef(EnumDefId),
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

impl IntoStoreId for TypeAliasDef {
    type Id = TypeAliasDefId;

    fn into_id(self, store: &Store) -> Self::Id {
        store.store_type_alias(self)
    }
}

impl IntoStoreId for StructDef {
    type Id = StructDefId;

    fn into_id(self, store: &Store) -> Self::Id {
        store.store_struct_def(self)
    }
}

impl IntoStoreId for EnumDef {
    type Id = EnumDefId;

    fn into_id(self, store: &Store) -> Self::Id {
        store.store_enum_def(self)
    }
}
