use crate::prelude::*;
use nitrate_nstring::NString;
use serde::{Deserialize, Serialize};
use serde_with::skip_serializing_none;

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ItemSyntaxError {
    pub span: ByteSpan,
}

impl Spanned for ItemSyntaxError {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

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
    pub span: ByteSpan,
    pub visibility: Option<Visibility>,
    pub attributes: Option<AttributeList>,
    pub name: NString,
    pub items: Vec<Item>,
}

impl Spanned for Module {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ItemPathSegment {
    pub span: ByteSpan,
    pub segment: String,
}

impl Spanned for ItemPathSegment {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ItemPath {
    pub span: ByteSpan,
    pub segments: Vec<ItemPathSegment>,
}

impl Spanned for ItemPath {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum UseTree {
    Single {
        span: ByteSpan,
        path: ItemPath,
    },
    Alias {
        span: ByteSpan,
        path: ItemPath,
        alias: NString,
    },
    UseAll {
        span: ByteSpan,
        path: ItemPath,
    },
    Group {
        span: ByteSpan,
        path: ItemPath,
        group: Vec<UseTree>,
    },
}

impl UseTree {
    pub fn span(&self) -> ByteSpan {
        match self {
            UseTree::Single { span, .. }
            | UseTree::Alias { span, .. }
            | UseTree::UseAll { span, .. }
            | UseTree::Group { span, .. } => *span,
        }
    }
    pub fn set_span(&mut self, new_span: ByteSpan) {
        match self {
            UseTree::Single { span, .. }
            | UseTree::Alias { span, .. }
            | UseTree::UseAll { span, .. }
            | UseTree::Group { span, .. } => *span = new_span,
        }
    }
    pub fn path(&self) -> &ItemPath {
        match self {
            UseTree::Single { path, .. } => path,
            UseTree::Alias { path, .. } => path,
            UseTree::UseAll { path, .. } => path,
            UseTree::Group { path, .. } => path,
        }
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Import {
    pub span: ByteSpan,
    pub visibility: Option<Visibility>,
    pub attributes: Option<AttributeList>,
    pub use_tree: UseTree,
    pub resolved: Option<Vec<Item>>,
}

impl Spanned for Import {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TypeParam {
    pub span: ByteSpan,
    pub name: NString,
    pub default_value: Option<Type>,
}

impl Spanned for TypeParam {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Generics {
    pub span: ByteSpan,
    pub params: Vec<TypeParam>,
}

impl Spanned for Generics {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TypeAlias {
    pub span: ByteSpan,
    pub visibility: Option<Visibility>,
    pub attributes: Option<AttributeList>,
    pub name: NString,
    pub generics: Option<Generics>,
    pub alias_type: Option<Type>,
}

impl Spanned for TypeAlias {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct StructField {
    pub span: ByteSpan,
    pub visibility: Option<Visibility>,
    pub attributes: Option<AttributeList>,
    pub name: NString,
    pub ty: Type,
    pub default_value: Option<Expr>,
}

impl Spanned for StructField {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Struct {
    pub span: ByteSpan,
    pub visibility: Option<Visibility>,
    pub attributes: Option<AttributeList>,
    pub name: NString,
    pub generics: Option<Generics>,
    pub fields: Vec<StructField>,
}

impl Spanned for Struct {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct EnumVariant {
    pub span: ByteSpan,
    pub attributes: Option<AttributeList>,
    pub name: NString,
    pub ty: Option<Type>,
    pub default_value: Option<Expr>,
}

impl Spanned for EnumVariant {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Enum {
    pub span: ByteSpan,
    pub visibility: Option<Visibility>,
    pub attributes: Option<AttributeList>,
    pub name: NString,
    pub generics: Option<Generics>,
    pub variants: Vec<EnumVariant>,
}

impl Spanned for Enum {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
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
    pub span: ByteSpan,
    pub visibility: Option<Visibility>,
    pub attributes: Option<AttributeList>,
    pub name: NString,
    pub generics: Option<Generics>,
    pub items: Vec<AssociatedItem>,
}

impl Spanned for Trait {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Impl {
    pub span: ByteSpan,
    pub generics: Option<Generics>,
    pub trait_path: Option<TypePath>,
    pub for_type: Type,
    pub items: Vec<AssociatedItem>,
}

impl Spanned for Impl {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Copy, Serialize, Deserialize)]
pub enum Mutability {
    Mut,
    Const,
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FuncParam {
    pub span: ByteSpan,
    pub attributes: Option<AttributeList>,
    pub mutability: Option<Mutability>,
    pub name: NString,
    pub ty: Type,
    pub default_value: Option<Expr>,
}

impl Spanned for FuncParam {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FuncParams {
    pub span: ByteSpan,
    pub params: Vec<FuncParam>,
    pub variadic: bool,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ExternAbi {
    pub span: ByteSpan,
    pub name: NString,
}

impl Spanned for ExternAbi {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Function {
    pub span: ByteSpan,
    pub visibility: Option<Visibility>,
    pub attributes: Option<AttributeList>,
    pub name: NString,
    pub generics: Option<Generics>,
    pub parameters: FuncParams,
    pub return_type: Option<Type>,
    pub definition: Option<Block>,
    pub abi: Option<ExternAbi>,
}

impl Spanned for Function {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
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
    pub span: ByteSpan,
    pub visibility: Option<Visibility>,
    pub kind: GlobalVariableKind,
    pub attributes: Option<AttributeList>,
    pub mutability: Option<Mutability>,
    pub name: NString,
    pub ty: Option<Type>,
    pub initializer: Option<Expr>,
}

impl Spanned for GlobalVariable {
    fn span(&self) -> ByteSpan {
        self.span
    }
    fn set_span(&mut self, span: ByteSpan) {
        self.span = span;
    }
}

#[skip_serializing_none]
#[derive(Debug, Clone, Serialize, Deserialize)]
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

impl Item {
    pub fn span(&self) -> ByteSpan {
        match self {
            Item::SyntaxError(e) => e.span,
            Item::Module(e) => e.span,
            Item::Import(e) => e.span,
            Item::TypeAlias(e) => e.span,
            Item::Struct(e) => e.span,
            Item::Enum(e) => e.span,
            Item::Trait(e) => e.span,
            Item::Impl(e) => e.span,
            Item::Function(e) => e.span,
            Item::Variable(e) => e.span,
        }
    }
    pub fn set_span(&mut self, span: ByteSpan) {
        match self {
            Item::SyntaxError(e) => e.span = span,
            Item::Module(e) => e.span = span,
            Item::Import(e) => e.span = span,
            Item::TypeAlias(e) => e.span = span,
            Item::Struct(e) => e.span = span,
            Item::Enum(e) => e.span = span,
            Item::Trait(e) => e.span = span,
            Item::Impl(e) => e.span = span,
            Item::Function(e) => e.span = span,
            Item::Variable(e) => e.span = span,
        }
    }
    pub fn reconstruct(&self, source: &[u8]) -> String {
        self.span().extract_str(source).to_string()
    }
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

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum SymbolKind {
    TypeAlias,
    Struct,
    Enum,
    EnumVariant,
    Trait,
    Function,
    GlobalVariable,
    LocalVariable,
    Parameter,
    GenericParameter,
}
