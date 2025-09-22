use crate::{
    kind::{Block, Expr, Type},
    tag::{
        EnumVariantNameId, FunctionNameId, ImportAliasNameId, ModuleNameId, PackageNameId,
        ParameterNameId, StructFieldNameId, TraitNameId, TypeNameId, VariableNameId,
    },
    ty::TypePath,
};

use serde::{Deserialize, Serialize};
use std::sync::{Arc, RwLock};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ItemSyntaxError;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum Visibility {
    Public,
    Private,
    Protected,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Package {
    pub name: PackageNameId,
    pub root: Module,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Module {
    pub visibility: Option<Visibility>,
    pub attributes: Option<Vec<Expr>>,
    pub name: ModuleNameId,
    pub items: Vec<Item>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum ItemPathTarget {
    Unresolved,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ItemPathSegment {
    pub segment: String,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ItemPath {
    pub segments: Vec<ItemPathSegment>,
    pub to: ItemPathTarget,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Import {
    pub visibility: Option<Visibility>,
    pub attributes: Option<Vec<Expr>>,
    pub path: ItemPath,
    pub alias: Option<ImportAliasNameId>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct GenericParameter {
    pub name: ParameterNameId,
    pub default: Option<Type>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TypeAlias {
    pub visibility: Option<Visibility>,
    pub attributes: Option<Vec<Expr>>,
    pub name: TypeNameId,
    pub type_params: Option<Vec<GenericParameter>>,
    pub alias_type: Option<Type>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct StructField {
    pub visibility: Option<Visibility>,
    pub attributes: Option<Vec<Expr>>,
    pub name: StructFieldNameId,
    pub field_type: Type,
    pub default: Option<Expr>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Struct {
    pub visibility: Option<Visibility>,
    pub attributes: Option<Vec<Expr>>,
    pub name: TypeNameId,
    pub type_params: Option<Vec<GenericParameter>>,
    pub fields: Vec<StructField>,
    pub methods: Vec<Arc<RwLock<NamedFunction>>>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct EnumVariant {
    pub visibility: Option<Visibility>,
    pub attributes: Option<Vec<Expr>>,
    pub name: EnumVariantNameId,
    pub variant_type: Option<Type>,
    pub value: Option<Expr>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Enum {
    pub visibility: Option<Visibility>,
    pub attributes: Option<Vec<Expr>>,
    pub name: TypeNameId,
    pub type_params: Option<Vec<GenericParameter>>,
    pub variants: Vec<EnumVariant>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum AssociatedItem {
    SyntaxError(ItemSyntaxError),
    TypeAlias(Arc<RwLock<TypeAlias>>),
    ConstantItem(Arc<RwLock<Variable>>),
    Method(Arc<RwLock<NamedFunction>>),
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Trait {
    pub visibility: Option<Visibility>,
    pub attributes: Option<Vec<Expr>>,
    pub name: TraitNameId,
    pub type_params: Option<Vec<GenericParameter>>,
    pub items: Vec<AssociatedItem>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Impl {
    pub visibility: Option<Visibility>,
    pub attributes: Option<Vec<Expr>>,
    pub type_params: Option<Vec<GenericParameter>>,
    pub trait_path: Option<TypePath>,
    pub for_type: Type,
    pub items: Vec<AssociatedItem>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum Mutability {
    Mutable,
    Const,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FunctionParameter {
    pub attributes: Option<Vec<Expr>>,
    pub mutability: Option<Mutability>,
    pub name: ParameterNameId,
    pub param_type: Option<Type>,
    pub default: Option<Expr>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct NamedFunction {
    pub visibility: Option<Visibility>,
    pub attributes: Option<Vec<Expr>>,
    pub name: FunctionNameId,
    pub type_params: Option<Vec<GenericParameter>>,
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
    pub attributes: Option<Vec<Expr>>,
    pub mutability: Option<Mutability>,
    pub name: VariableNameId,
    pub var_type: Option<Type>,
    pub initializer: Option<Expr>,
}

#[derive(Clone, Serialize, Deserialize)]
pub enum Item {
    SyntaxError(ItemSyntaxError),
    Module(Box<Module>),
    ItemPath(Box<ItemPath>),
    Import(Box<Import>),
    TypeAlias(Arc<RwLock<TypeAlias>>),
    Struct(Arc<RwLock<Struct>>),
    Enum(Arc<RwLock<Enum>>),
    Trait(Arc<RwLock<Trait>>),
    Impl(Box<Impl>),
    NamedFunction(Arc<RwLock<NamedFunction>>),
    Variable(Arc<RwLock<Variable>>),
}

impl std::fmt::Debug for Item {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Item::SyntaxError(e) => e.fmt(f),
            Item::Module(e) => e.fmt(f),
            Item::ItemPath(e) => e.fmt(f),
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
