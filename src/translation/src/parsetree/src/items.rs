use crate::kind::{Block, Expr, Path, Type};

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
    pub language: Option<IString>,
    pub attributes: Vec<Expr>,
    pub path: Path,
    pub alias: Option<IString>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct GenericParameter {
    pub name: IString,
    pub default: Option<Type>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TypeAlias {
    pub attributes: Vec<Expr>,
    pub name: IString,
    pub type_params: Vec<GenericParameter>,
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
    pub type_params: Vec<GenericParameter>,
    pub fields: Vec<StructField>,
    pub methods: Vec<NamedFunction>,
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
    pub type_params: Vec<GenericParameter>,
    pub variants: Vec<EnumVariant>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FunctionParameter {
    pub attributes: Vec<Expr>,
    pub name: IString,
    pub param_type: Type,
    pub default: Option<Expr>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct NamedFunction {
    pub attributes: Vec<Expr>,
    pub name: IString,
    pub type_params: Vec<GenericParameter>,
    pub parameters: Vec<FunctionParameter>,
    pub return_type: Type,
    pub definition: Option<Block>,
}

impl NamedFunction {
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
    pub is_mutable: bool,
    pub name: IString,
    pub var_type: Type,
    pub initializer: Option<Expr>,
}

#[derive(Clone, Serialize, Deserialize)]
pub enum Item {
    Module(Box<Module>),
    Import(Box<Import>),
    TypeAlias(Box<TypeAlias>),
    StructDefinition(Box<StructDefinition>),
    EnumDefinition(Box<EnumDefinition>),
    NamedFunction(Box<NamedFunction>),
    GlobalVariable(Box<GlobalVariable>),
}

impl std::fmt::Debug for Item {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Item::Module(e) => e.fmt(f),
            Item::Import(e) => e.fmt(f),
            Item::TypeAlias(e) => e.fmt(f),
            Item::StructDefinition(e) => e.fmt(f),
            Item::EnumDefinition(e) => e.fmt(f),
            Item::NamedFunction(e) => e.fmt(f),
            Item::GlobalVariable(e) => e.fmt(f),
        }
    }
}
