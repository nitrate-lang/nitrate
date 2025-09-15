use crate::kind::{Block, Expr, Path, Type};

use interned_string::IString;
use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Package {
    pub name: IString,
    pub root: Module,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Module {
    pub attributes: Vec<Expr>,
    pub name: IString,
    pub items: Vec<Item>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Import {
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
    pub aliased_type: Option<Type>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct StructField {
    pub attributes: Vec<Expr>,
    pub name: IString,
    pub field_type: Type,
    pub default: Option<Expr>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Struct {
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
    pub value: Option<Expr>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Enum {
    pub attributes: Vec<Expr>,
    pub name: IString,
    pub type_params: Vec<GenericParameter>,
    pub variants: Vec<EnumVariant>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum AssociatedItem {
    SyntaxError,
    TypeAlias(TypeAlias),
    ConstantItem(ConstVariable),
    Method(NamedFunction),
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Trait {
    pub attributes: Vec<Expr>,
    pub name: IString,
    pub type_params: Vec<GenericParameter>,
    pub items: Vec<AssociatedItem>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Impl {}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FunctionParameter {
    pub attributes: Vec<Expr>,
    pub name: IString,
    pub param_type: Option<Type>,
    pub default: Option<Expr>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct NamedFunction {
    pub attributes: Vec<Expr>,
    pub name: IString,
    pub type_params: Vec<GenericParameter>,
    pub parameters: Vec<FunctionParameter>,
    pub return_type: Option<Type>,
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
pub struct StaticVariable {
    pub attributes: Vec<Expr>,
    pub is_mutable: bool,
    pub name: IString,
    pub var_type: Option<Type>,
    pub initializer: Option<Expr>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ConstVariable {
    pub attributes: Vec<Expr>,
    pub name: IString,
    pub var_type: Option<Type>,
    pub initializer: Option<Expr>,
}

#[derive(Clone, Serialize, Deserialize)]
pub enum Item {
    SyntaxError,

    Package(Box<Package>),
    Module(Box<Module>),
    Import(Box<Import>),
    TypeAlias(Box<TypeAlias>),
    Struct(Box<Struct>),
    Enum(Box<Enum>),
    Trait(Box<Trait>),
    Impl(Box<Impl>),
    NamedFunction(Box<NamedFunction>),
    StaticVariable(Box<StaticVariable>),
    ConstVariable(Box<ConstVariable>),
}

impl std::fmt::Debug for Item {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Item::SyntaxError => write!(f, "SyntaxError"),

            Item::Package(e) => e.fmt(f),
            Item::Module(e) => e.fmt(f),
            Item::Import(e) => e.fmt(f),
            Item::TypeAlias(e) => e.fmt(f),
            Item::Struct(e) => e.fmt(f),
            Item::Enum(e) => e.fmt(f),
            Item::Trait(e) => e.fmt(f),
            Item::Impl(e) => e.fmt(f),
            Item::NamedFunction(e) => e.fmt(f),
            Item::StaticVariable(e) => e.fmt(f),
            Item::ConstVariable(e) => e.fmt(f),
        }
    }
}
