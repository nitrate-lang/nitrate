use crate::kind::{Block, Expr, Type};

use interned_string::IString;
use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Module {
    pub attributes: Vec<Expr>,
    pub name: IString,
    pub items: Vec<Item>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Import {
    pub path: Vec<IString>,
    pub alias: Option<IString>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TypeAlias {
    pub name: IString,
    pub aliased_type: Type,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct StructField {
    pub attributes: Vec<Expr>,
    pub name: IString,
    pub field_type: Type,
    pub default: Option<Expr>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct StructDefinition {
    pub attributes: Vec<Expr>,
    pub name: IString,
    pub fields: Vec<StructField>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct EnumVariant {
    pub attributes: Vec<Expr>,
    pub name: IString,
    pub variant_type: Option<Type>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct EnumDefinition {
    pub attributes: Vec<Expr>,
    pub name: IString,
    pub variants: Vec<EnumVariant>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FunctionParameter {
    pub name: IString,
    pub param_type: Type,
    pub default: Option<Expr>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct GlobalFunction {
    pub attributes: Vec<Expr>,
    pub parameters: Vec<FunctionParameter>,
    pub return_type: Type,
    pub definition: Option<Block>,
}

impl GlobalFunction {
    #[must_use]
    pub fn is_definition(&self) -> bool {
        self.definition.is_some()
    }

    #[must_use]
    pub fn is_declaration(&self) -> bool {
        self.definition.is_none()
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct GlobalVariable {
    pub attributes: Vec<Expr>,
    pub name: IString,
    pub var_type: Type,
    pub initializer: Option<Expr>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Scope {
    pub name: IString,
    pub attributes: Vec<Expr>,
    pub items: Vec<Item>,
}

#[derive(Clone, Serialize, Deserialize)]
pub enum Item {
    Module(Box<Module>),
    Import(Box<Import>),
    TypeAlias(Box<TypeAlias>),
    StructDefinition(Box<StructDefinition>),
    EnumDefinition(Box<EnumDefinition>),
    GlobalFunction(Box<GlobalFunction>),
    GlobalVariable(Box<GlobalVariable>),
    Scope(Box<Scope>),
}

impl std::fmt::Debug for Item {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Item::Module(e) => e.fmt(f),
            Item::Import(e) => e.fmt(f),
            Item::TypeAlias(e) => e.fmt(f),
            Item::StructDefinition(e) => e.fmt(f),
            Item::EnumDefinition(e) => e.fmt(f),
            Item::GlobalFunction(e) => e.fmt(f),
            Item::GlobalVariable(e) => e.fmt(f),
            Item::Scope(e) => e.fmt(f),
        }
    }
}
