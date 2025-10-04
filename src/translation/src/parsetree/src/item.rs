use crate::{
    expr::AttributeList,
    kind::{Block, Expr, Type},
    tag::{
        EnumVariantNameId, FunctionNameId, ImportNameId, ModuleNameId, ParameterNameId,
        StructFieldNameId, TraitNameId, TypeNameId, VariableNameId,
    },
    ty::TypePath,
};

use serde::{Deserialize, Serialize};
use serde_with::skip_serializing_none;
use std::sync::{Arc, RwLock};

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ItemSyntaxError;

#[skip_serializing_none]
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum Visibility {
    Public,
    Private,
    Protected,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Module {
    pub visibility: Option<Visibility>,
    pub attributes: Option<AttributeList>,
    pub name: ModuleNameId,
    pub items: Vec<Item>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ItemPathSegment {
    pub segment: String,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ItemPath {
    pub segments: Vec<ItemPathSegment>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Import {
    pub visibility: Option<Visibility>,
    pub attributes: Option<AttributeList>,
    pub import_name: ImportNameId,
    pub items: Option<Vec<ItemPath>>,

    // Not set until import resolution
    pub resolved: Option<Item>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TypeParam {
    pub name: ParameterNameId,
    pub default_value: Option<Type>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Generics {
    pub params: Vec<TypeParam>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TypeAlias {
    pub visibility: Option<Visibility>,
    pub attributes: Option<AttributeList>,
    pub name: TypeNameId,
    pub generics: Option<Generics>,
    pub alias_type: Option<Type>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct StructField {
    pub visibility: Option<Visibility>,
    pub attributes: Option<AttributeList>,
    pub name: StructFieldNameId,
    pub field_type: Type,
    pub default_value: Option<Expr>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Struct {
    pub visibility: Option<Visibility>,
    pub attributes: Option<AttributeList>,
    pub name: TypeNameId,
    pub generics: Option<Generics>,
    pub fields: Vec<StructField>,
    pub methods: Vec<Arc<RwLock<Function>>>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct EnumVariant {
    pub visibility: Option<Visibility>,
    pub attributes: Option<AttributeList>,
    pub name: EnumVariantNameId,
    pub variant_type: Option<Type>,
    pub value: Option<Expr>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Enum {
    pub visibility: Option<Visibility>,
    pub attributes: Option<AttributeList>,
    pub name: TypeNameId,
    pub generics: Option<Generics>,
    pub variants: Vec<EnumVariant>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum AssociatedItem {
    SyntaxError(ItemSyntaxError),
    TypeAlias(Arc<RwLock<TypeAlias>>),
    ConstantItem(Arc<RwLock<Variable>>),
    Method(Arc<RwLock<Function>>),
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Trait {
    pub visibility: Option<Visibility>,
    pub attributes: Option<AttributeList>,
    pub name: TraitNameId,
    pub generics: Option<Generics>,
    pub items: Vec<AssociatedItem>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Impl {
    pub attributes: Option<AttributeList>,
    pub generics: Option<Generics>,
    pub trait_path: Option<TypePath>,
    pub for_type: Type,
    pub items: Vec<AssociatedItem>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum Mutability {
    Mutable,
    Const,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FuncParam {
    pub attributes: Option<AttributeList>,
    pub mutability: Option<Mutability>,
    pub name: ParameterNameId,
    pub param_type: Option<Type>,
    pub default_value: Option<Expr>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FuncParams {
    pub params: Vec<FuncParam>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Function {
    pub visibility: Option<Visibility>,
    pub attributes: Option<AttributeList>,
    pub name: FunctionNameId,
    pub generics: Option<Generics>,
    pub parameters: FuncParams,
    pub return_type: Option<Type>,
    pub definition: Option<Block>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Copy, Serialize, Deserialize)]
pub enum VariableKind {
    Static,
    Const,
    Let,
    Var,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Variable {
    pub visibility: Option<Visibility>,
    pub kind: VariableKind,
    pub attributes: Option<AttributeList>,
    pub mutability: Option<Mutability>,
    pub name: VariableNameId,
    pub ty: Option<Type>,
    pub initializer: Option<Expr>,
}

#[skip_serializing_none]
#[derive(Clone, Serialize, Deserialize)]
pub enum Item {
    SyntaxError(ItemSyntaxError),
    Module(Box<Module>),
    Import(Box<Import>),
    TypeAlias(Arc<RwLock<TypeAlias>>),
    Struct(Arc<RwLock<Struct>>),
    Enum(Arc<RwLock<Enum>>),
    Trait(Arc<RwLock<Trait>>),
    Impl(Box<Impl>),
    Function(Arc<RwLock<Function>>),
    Variable(Arc<RwLock<Variable>>),
}

impl std::fmt::Debug for Item {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Item::SyntaxError(e) => e.fmt(f),
            Item::Module(e) => e.fmt(f),
            Item::Import(e) => e.fmt(f),
            Item::TypeAlias(e) => e.fmt(f),
            Item::Struct(e) => e.fmt(f),
            Item::Enum(e) => e.fmt(f),
            Item::Trait(e) => e.fmt(f),
            Item::Impl(e) => e.fmt(f),
            Item::Function(e) => e.fmt(f),
            Item::Variable(e) => e.fmt(f),
        }
    }
}
