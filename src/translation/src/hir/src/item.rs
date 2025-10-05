use interned_string::IString;
use serde::{Deserialize, Serialize};

use crate::ItemId;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum Visibility {
    Sec,
    Pro,
    Pub,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum ModuleAttribute {}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Module {
    pub visibility: Visibility,
    pub name: IString,
    pub attributes: Vec<ModuleAttribute>,
    pub items: Vec<ItemId>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct StructDef {
    // TODO:
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct EnumDef {
    // TODO:
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TraitDef {
    // TODO:
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ImplDef {
    // TODO:
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Foreign {
    // TODO:
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum Item {
    // TODO:
}
