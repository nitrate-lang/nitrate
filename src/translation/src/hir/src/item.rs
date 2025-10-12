use crate::prelude::*;
use interned_string::IString;
use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum Visibility {
    Sec,
    Pro,
    Pub,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct QualifiedName(pub IString);

impl<T> From<T> for QualifiedName
where
    T: Into<IString>,
{
    fn from(value: T) -> Self {
        QualifiedName(value.into())
    }
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct EntityName(pub IString);

#[derive(Debug, Serialize, Deserialize)]
pub struct ExternalFunction {
    pub visibility: Visibility,
    pub attributes: Vec<FunctionAttribute>,
    pub name: EntityName,
    pub parameters: Vec<TypeId>,
    pub return_type: TypeId,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct StaticFunction {
    pub visibility: Visibility,
    pub attributes: Vec<FunctionAttribute>,
    pub name: EntityName,
    pub parameters: Vec<TypeId>,
    pub return_type: TypeId,
    pub body: BlockId,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct ClosureFunction {
    pub closure_unique_id: u64,
    pub captures: Vec<SymbolId>,
    pub callee: Box<StaticFunction>,
}

#[derive(Debug, Serialize, Deserialize)]
pub enum Function {
    External(ExternalFunction),
    Static(StaticFunction),
    Closure(ClosureFunction),
}

#[derive(Debug, Serialize, Deserialize)]
pub struct GlobalVariable {
    pub visibility: Visibility,
    pub is_mutable: bool,
    pub name: EntityName,
    pub ty: TypeId,
    pub initializer: ValueId,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct LocalVariable {
    pub is_mutable: bool,
    pub name: EntityName,
    pub ty: TypeId,
    pub initializer: ValueId,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Parameter {
    pub name: EntityName,
    pub ty: TypeId,
    pub default_value: Option<ValueId>,
}

#[derive(Debug, Serialize, Deserialize)]
pub enum Symbol {
    Unresolved { name: QualifiedName },
    GlobalVariable(GlobalVariable),
    LocalVariable(LocalVariable),
    Parameter(Parameter),
    Function(Function),
}

impl IntoStoreId for Symbol {
    type Id = SymbolId;

    fn into_id(self, ctx: &Store) -> Self::Id {
        ctx.store_symbol(self)
    }
}

#[derive(Debug, Serialize, Deserialize)]
pub enum ModuleAttribute {}

#[derive(Debug, Serialize, Deserialize)]
pub struct Module {
    pub visibility: Visibility,
    pub name: EntityName,
    pub attributes: Vec<ModuleAttribute>,
    pub items: Vec<ItemId>,
}

#[derive(Debug, Serialize, Deserialize)]
pub enum Item {
    Module(Module),
    GlobalVariable(GlobalVariable),
    ExternalFunction(ExternalFunction),
    StaticFunction(StaticFunction),
}

impl IntoStoreId for Item {
    type Id = ItemId;

    fn into_id(self, ctx: &Store) -> Self::Id {
        ctx.store_item(self)
    }
}
