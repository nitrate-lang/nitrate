use std::collections::BTreeSet;

use crate::prelude::*;
use interned_string::IString;
use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum Visibility {
    Sec,
    Pro,
    Pub,
}

pub type QualifiedName = IString;

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct EntityName(pub IString);

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct ExternalFunction {
    pub visibility: Visibility,
    pub attributes: BTreeSet<FunctionAttribute>,
    pub name: EntityName,
    pub parameters: Vec<(IString, TypeId, Option<ValueId>)>,
    pub return_type: TypeId,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct Function {
    pub visibility: Visibility,
    pub attributes: BTreeSet<FunctionAttribute>,
    pub name: EntityName,
    pub parameters: Vec<(IString, TypeId, Option<ValueId>)>,
    pub return_type: TypeId,
    pub body: BlockId,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct ClosureFunction {
    pub closure_unique_id: u64,
    pub captures: Vec<SymbolId>,
    pub callee: Box<Function>,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum FunctionSymbol {
    External(ExternalFunction),
    Static(Function),
    Closure(ClosureFunction),
}

impl FunctionSymbol {
    pub fn attributes(&self) -> &BTreeSet<FunctionAttribute> {
        match self {
            FunctionSymbol::External(func) => &func.attributes,
            FunctionSymbol::Static(func) => &func.attributes,
            FunctionSymbol::Closure(func) => &func.callee.attributes,
        }
    }

    pub fn parameters(&self) -> &Vec<(IString, TypeId, Option<ValueId>)> {
        match self {
            FunctionSymbol::External(func) => &func.parameters,
            FunctionSymbol::Static(func) => &func.parameters,
            FunctionSymbol::Closure(func) => &func.callee.parameters,
        }
    }

    pub fn return_type(&self) -> &TypeId {
        match self {
            FunctionSymbol::External(func) => &func.return_type,
            FunctionSymbol::Static(func) => &func.return_type,
            FunctionSymbol::Closure(func) => &func.callee.return_type,
        }
    }
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
pub enum Symbol {
    Unresolved { name: QualifiedName },
    GlobalVariable(GlobalVariable),
    LocalVariable(LocalVariable),
    Parameter(Parameter),
    Function(FunctionSymbol),
}

impl IntoStoreId for Symbol {
    type Id = SymbolId;

    fn into_id(self, store: &Store) -> Self::Id {
        store.store_symbol(self)
    }
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum ModuleAttribute {}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct Module {
    pub visibility: Visibility,
    pub name: EntityName,
    pub attributes: Vec<ModuleAttribute>,
    pub items: Vec<ItemId>,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum Item {
    Module(Module),
    GlobalVariable(GlobalVariable),
    ExternalFunction(ExternalFunction),
    StaticFunction(Function),
}

impl IntoStoreId for Item {
    type Id = ItemId;

    fn into_id(self, store: &Store) -> Self::Id {
        store.store_item(self)
    }
}
