use crate::{
    ast::{Block, Expr, Type},
    expr::AttributeList,
    ty::TypePath,
};
use nitrate_nstring::NString;
use serde::{Deserialize, Serialize};
use serde_with::skip_serializing_none;

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
    pub name: NString,
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
pub enum UseTree {
    Single { path: ItemPath },
    Alias { path: ItemPath, alias: NString },
    UseAll { path: ItemPath },
    Group { path: ItemPath, group: Vec<UseTree> },
}

impl UseTree {
    pub fn path(&self) -> &ItemPath {
        match self {
            UseTree::Single { path } => path,
            UseTree::Alias { path, .. } => path,
            UseTree::UseAll { path } => path,
            UseTree::Group { path, .. } => path,
        }
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Import {
    pub visibility: Option<Visibility>,
    pub attributes: Option<AttributeList>,
    pub use_tree: UseTree,

    // Not set until import resolution
    pub resolved: Option<Vec<Item>>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TypeParam {
    pub name: NString,
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
    pub name: NString,
    pub generics: Option<Generics>,
    pub alias_type: Option<Type>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct StructField {
    pub visibility: Option<Visibility>,
    pub attributes: Option<AttributeList>,
    pub name: NString,
    pub ty: Type,
    pub default_value: Option<Expr>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Struct {
    pub visibility: Option<Visibility>,
    pub attributes: Option<AttributeList>,
    pub name: NString,
    pub generics: Option<Generics>,
    pub fields: Vec<StructField>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct EnumVariant {
    pub attributes: Option<AttributeList>,
    pub name: NString,
    pub ty: Option<Type>,
    pub default_value: Option<Expr>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Enum {
    pub visibility: Option<Visibility>,
    pub attributes: Option<AttributeList>,
    pub name: NString,
    pub generics: Option<Generics>,
    pub variants: Vec<EnumVariant>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum AssociatedItem {
    SyntaxError(ItemSyntaxError),
    TypeAlias(TypeAlias),
    ConstantItem(GlobalVariable),
    Method(Function),
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Trait {
    pub visibility: Option<Visibility>,
    pub attributes: Option<AttributeList>,
    pub name: NString,
    pub generics: Option<Generics>,
    pub items: Vec<AssociatedItem>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Impl {
    pub generics: Option<Generics>,
    pub attributes: Option<AttributeList>,
    pub trait_path: Option<TypePath>,
    pub for_type: Type,
    pub items: Vec<AssociatedItem>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum Mutability {
    Mut,
    Const,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FuncParam {
    pub attributes: Option<AttributeList>,
    pub mutability: Option<Mutability>,
    pub name: NString,
    pub ty: Type,
    pub default_value: Option<Expr>,
}

pub type FuncParams = Vec<FuncParam>;

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Function {
    pub visibility: Option<Visibility>,
    pub attributes: Option<AttributeList>,
    pub name: NString,
    pub generics: Option<Generics>,
    pub parameters: FuncParams,
    pub return_type: Option<Type>,
    pub definition: Option<Block>,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Copy, Serialize, Deserialize)]
pub enum GlobalVariableKind {
    Static,
    Const,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct GlobalVariable {
    pub visibility: Option<Visibility>,
    pub kind: GlobalVariableKind,
    pub attributes: Option<AttributeList>,
    pub mutability: Option<Mutability>,
    pub name: NString,
    pub ty: Option<Type>,
    pub initializer: Option<Expr>,
}

#[skip_serializing_none]
#[derive(Clone, Serialize, Deserialize)]
pub enum Item {
    SyntaxError(ItemSyntaxError),
    Module(Box<Module>),
    Import(Box<Import>),
    TypeAlias(TypeAlias),
    Struct(Struct),
    Enum(Enum),
    Trait(Trait),
    Impl(Box<Impl>),
    Function(Function),
    Variable(GlobalVariable),
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

impl Item {
    pub fn as_module(self) -> Option<Module> {
        match self {
            Item::Module(m) => Some(*m),
            _ => None,
        }
    }

    pub fn as_import(self) -> Option<Import> {
        match self {
            Item::Import(i) => Some(*i),
            _ => None,
        }
    }

    pub fn as_type_alias(self) -> Option<TypeAlias> {
        match self {
            Item::TypeAlias(t) => Some(t),
            _ => None,
        }
    }

    pub fn as_struct(self) -> Option<Struct> {
        match self {
            Item::Struct(s) => Some(s),
            _ => None,
        }
    }

    pub fn as_enum(self) -> Option<Enum> {
        match self {
            Item::Enum(e) => Some(e),
            _ => None,
        }
    }

    pub fn as_trait(self) -> Option<Trait> {
        match self {
            Item::Trait(t) => Some(t),
            _ => None,
        }
    }

    pub fn as_impl(self) -> Option<Impl> {
        match self {
            Item::Impl(i) => Some(*i),
            _ => None,
        }
    }

    pub fn as_function(self) -> Option<Function> {
        match self {
            Item::Function(f) => Some(f),
            _ => None,
        }
    }

    pub fn as_variable(self) -> Option<GlobalVariable> {
        match self {
            Item::Variable(v) => Some(v),
            _ => None,
        }
    }
}
