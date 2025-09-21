use crate::{
    Order, ParseTreeIterMut, RefNodeMut,
    kind::{Block, Expr, Path, Type},
    tag::{
        EnumVariantNameId, FunctionNameId, ImportAliasNameId, ModuleNameId, PackageNameId,
        ParameterNameId, StructFieldNameId, TraitNameId, TypeNameId, VariableNameId,
    },
};

use serde::{Deserialize, Serialize};

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

impl ParseTreeIterMut for Package {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Module {
    pub visibility: Option<Visibility>,
    pub attributes: Option<Vec<Expr>>,
    pub name: ModuleNameId,
    pub items: Vec<Item>,
}

impl ParseTreeIterMut for Module {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Import {
    pub visibility: Option<Visibility>,
    pub attributes: Option<Vec<Expr>>,
    pub path: Path,
    pub alias: Option<ImportAliasNameId>,
}

impl ParseTreeIterMut for Import {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct GenericParameter {
    pub name: ParameterNameId,
    pub default: Option<Type>,
}

impl ParseTreeIterMut for GenericParameter {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TypeAlias {
    pub visibility: Option<Visibility>,
    pub attributes: Option<Vec<Expr>>,
    pub name: TypeNameId,
    pub type_params: Option<Vec<GenericParameter>>,
    pub aliased_type: Option<Type>,
}

impl ParseTreeIterMut for TypeAlias {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct StructField {
    pub visibility: Option<Visibility>,
    pub attributes: Option<Vec<Expr>>,
    pub name: StructFieldNameId,
    pub field_type: Type,
    pub default: Option<Expr>,
}

impl ParseTreeIterMut for StructField {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Struct {
    pub visibility: Option<Visibility>,
    pub attributes: Option<Vec<Expr>>,
    pub name: TypeNameId,
    pub type_params: Option<Vec<GenericParameter>>,
    pub fields: Vec<StructField>,
    pub methods: Vec<NamedFunction>,
}

impl ParseTreeIterMut for Struct {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct EnumVariant {
    pub visibility: Option<Visibility>,
    pub attributes: Option<Vec<Expr>>,
    pub name: EnumVariantNameId,
    pub variant_type: Option<Type>,
    pub value: Option<Expr>,
}

impl ParseTreeIterMut for EnumVariant {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Enum {
    pub visibility: Option<Visibility>,
    pub attributes: Option<Vec<Expr>>,
    pub name: TypeNameId,
    pub type_params: Option<Vec<GenericParameter>>,
    pub variants: Vec<EnumVariant>,
}

impl ParseTreeIterMut for Enum {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum AssociatedItem {
    SyntaxError,
    TypeAlias(TypeAlias),
    ConstantItem(Variable),
    Method(NamedFunction),
}

impl ParseTreeIterMut for AssociatedItem {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Trait {
    pub visibility: Option<Visibility>,
    pub attributes: Option<Vec<Expr>>,
    pub name: TraitNameId,
    pub type_params: Option<Vec<GenericParameter>>,
    pub items: Vec<AssociatedItem>,
}

impl ParseTreeIterMut for Trait {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Impl {
    pub visibility: Option<Visibility>,
    pub attributes: Option<Vec<Expr>>,
    pub type_params: Option<Vec<GenericParameter>>,
    pub trait_path: Option<Path>,
    pub for_type: Type,
    pub items: Vec<AssociatedItem>,
}

impl ParseTreeIterMut for Impl {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
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

impl ParseTreeIterMut for FunctionParameter {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
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

impl ParseTreeIterMut for NamedFunction {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
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

impl ParseTreeIterMut for Variable {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
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

impl ParseTreeIterMut for Item {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        match self {
            Item::SyntaxError => {
                f(Order::Pre, RefNodeMut::ItemSyntaxError);
                f(Order::Post, RefNodeMut::ItemSyntaxError);
            }

            Item::Module(module) => module.depth_first_iter_mut(f),
            Item::Import(import) => import.depth_first_iter_mut(f),
            Item::TypeAlias(type_alias) => type_alias.depth_first_iter_mut(f),
            Item::Struct(struct_) => struct_.depth_first_iter_mut(f),
            Item::Enum(enum_) => enum_.depth_first_iter_mut(f),
            Item::Trait(trait_) => trait_.depth_first_iter_mut(f),
            Item::Impl(impl_) => impl_.depth_first_iter_mut(f),
            Item::NamedFunction(func) => func.depth_first_iter_mut(f),
            Item::Variable(var) => var.depth_first_iter_mut(f),
        }
    }
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
