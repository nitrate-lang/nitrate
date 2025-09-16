use crate::kind::{Block, Expr, Path, Type};

use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum Visibility {
    Public,
    Private,
    Protected,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Package {
    pub name: String,
    pub root: Module,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Module {
    pub visibility: Option<Visibility>,
    pub attributes: Vec<Expr>,
    pub name: String,
    pub items: Vec<Item>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Import {
    pub visibility: Option<Visibility>,
    pub attributes: Vec<Expr>,
    pub path: Path,
    pub alias: Option<String>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct GenericParameter {
    pub name: String,
    pub default: Option<Type>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TypeAlias {
    pub visibility: Option<Visibility>,
    pub attributes: Vec<Expr>,
    pub name: String,
    pub type_params: Vec<GenericParameter>,
    pub aliased_type: Option<Type>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct StructField {
    pub visibility: Option<Visibility>,
    pub attributes: Vec<Expr>,
    pub name: String,
    pub field_type: Type,
    pub default: Option<Expr>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Struct {
    pub visibility: Option<Visibility>,
    pub attributes: Vec<Expr>,
    pub name: String,
    pub type_params: Vec<GenericParameter>,
    pub fields: Vec<StructField>,
    pub methods: Vec<NamedFunction>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct EnumVariant {
    pub visibility: Option<Visibility>,
    pub attributes: Vec<Expr>,
    pub name: String,
    pub variant_type: Option<Type>,
    pub value: Option<Expr>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Enum {
    pub visibility: Option<Visibility>,
    pub attributes: Vec<Expr>,
    pub name: String,
    pub type_params: Vec<GenericParameter>,
    pub variants: Vec<EnumVariant>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum AssociatedItem {
    SyntaxError,
    TypeAlias(TypeAlias),
    ConstantItem(Variable),
    Method(NamedFunction),
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Trait {
    pub visibility: Option<Visibility>,
    pub attributes: Vec<Expr>,
    pub name: String,
    pub type_params: Vec<GenericParameter>,
    pub items: Vec<AssociatedItem>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Impl {
    pub visibility: Option<Visibility>,
    pub attributes: Vec<Expr>,
    pub type_params: Vec<GenericParameter>,
    pub trait_path: Option<Path>,
    pub for_type: Type,
    pub items: Vec<AssociatedItem>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FunctionParameter {
    pub attributes: Vec<Expr>,
    pub name: String,
    pub param_type: Option<Type>,
    pub default: Option<Expr>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct NamedFunction {
    pub visibility: Option<Visibility>,
    pub attributes: Vec<Expr>,
    pub name: String,
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

#[derive(Debug, Clone, Copy, Serialize, Deserialize)]
pub enum VariableKind {
    Static,
    Const,
    Let,
    Var,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Variable {
    pub visibility: Option<Visibility>,
    pub kind: VariableKind,
    pub attributes: Vec<Expr>,
    pub is_mutable: bool,
    pub name: String,
    pub var_type: Option<Type>,
    pub initializer: Option<Expr>,
}

#[derive(Clone, Serialize, Deserialize)]
pub enum Item {
    SyntaxError,

    Module(Box<Module>),
    Import(Box<Import>),
    TypeAlias(Box<TypeAlias>),
    Struct(Box<Struct>),
    Enum(Box<Enum>),
    Trait(Box<Trait>),
    Impl(Box<Impl>),
    NamedFunction(Box<NamedFunction>),
    Variable(Box<Variable>),
}

impl std::fmt::Debug for Item {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Item::SyntaxError => write!(f, "SyntaxError"),

            Item::Module(e) => e.fmt(f),
            Item::Import(e) => e.fmt(f),
            Item::TypeAlias(e) => e.fmt(f),
            Item::Struct(e) => e.fmt(f),
            Item::Enum(e) => e.fmt(f),
            Item::Trait(e) => e.fmt(f),
            Item::Impl(e) => e.fmt(f),
            Item::NamedFunction(e) => e.fmt(f),
            Item::Variable(e) => e.fmt(f),
        }
    }
}
