use crate::prelude::{hir::*, *};
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

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct EntityName(pub IString);

#[derive(Debug, Serialize, Deserialize)]
pub enum Function {
    External {
        visibility: Visibility,
        attributes: Vec<FunctionAttribute>,
        name: EntityName,
        parameters: Vec<TypeId>,
        return_type: TypeId,
    },

    Static {
        visibility: Visibility,
        attributes: Vec<FunctionAttribute>,
        name: EntityName,
        parameters: Vec<TypeId>,
        return_type: TypeId,
        body: BlockId,
    },

    Closure {
        closure_unique_id: u64,
        callee: Box<Function>,
        captures: Vec<ValueId>,
    },
}

#[derive(Debug, Serialize, Deserialize)]
pub enum Symbol {
    Unresolved {
        name: QualifiedName,
    },

    GlobalVariable {
        visibility: Visibility,
        name: EntityName,
        ty: TypeId,
        initializer: ValueId,
    },

    LocalVariable {
        name: EntityName,
        ty: TypeId,
        initializer: ValueId,
    },

    Parameter {
        name: EntityName,
        ty: TypeId,
        default_value: Option<ValueId>,
    },

    Function(Function),
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
pub struct TraitDef {
    // TODO:
}

#[derive(Debug, Serialize, Deserialize)]
pub struct ImplDef {
    // TODO:
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Foreign {
    // TODO:
}

#[derive(Debug, Serialize, Deserialize)]
pub enum Item {
    // TODO:
}
