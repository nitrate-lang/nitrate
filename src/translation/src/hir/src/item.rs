use crate::{helper::PowOf2, prelude::*};
use nitrate_nstring::NString;
use nitrate_tree::ByteSpan;
use serde::{Deserialize, Serialize};
use std::{
    collections::{BTreeMap, BTreeSet},
    matches,
    num::NonZeroUsize,
};
use thin_vec::ThinVec;

#[derive(Debug, Clone, Copy, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum Visibility {
    Sec,
    Pro,
    Pub,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum GlobalVariableAttribute {
    NoMangle,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct GlobalVariable {
    pub span: ByteSpan,
    pub visibility: Visibility,
    pub attributes: BTreeSet<GlobalVariableAttribute>,
    pub is_mutable: bool,
    pub name: NString,
    pub mangled_name: Option<NString>,
    pub ty: TypeId,
    pub initializer: ValueId,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum LocalVariableAttribute {
    Align { alignment: PowOf2<u32> },
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum LocalKind {
    Let,
    Var,
    Static,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct LocalVariable {
    pub span: ByteSpan,
    pub kind: LocalKind,
    pub attributes: BTreeSet<LocalVariableAttribute>,
    pub is_mutable: bool,
    pub name: NString,
    pub ty: TypeId,
    pub initializer: ValueId,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum ParameterAttribute {
    Align { alignment: PowOf2<u32> },
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct Parameter {
    pub span: ByteSpan,
    pub attributes: BTreeSet<ParameterAttribute>,
    pub is_mutable: bool,
    pub name: NString,
    pub ty: TypeId,
    pub default_value: Option<ValueId>,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct Function {
    pub span: ByteSpan,
    pub visibility: Visibility,
    pub attributes: BTreeSet<FunctionAttribute>,
    pub is_unsafe: bool,
    pub name: NString,
    pub mangled_name: Option<NString>,
    pub generics: Option<BTreeMap<NString, Option<TypeId>>>,
    pub params: Vec<ParameterId>,
    pub return_type: TypeId,
    pub body: Option<Vec<BlockElement>>,
}

impl Function {
    #[must_use]
    pub fn get_type(&self) -> FunctionType {
        let params: Vec<(NString, TypeId)> = self
            .params
            .iter()
            .map(|param_id| {
                let p = param_id.borrow();
                (p.name.clone(), p.ty)
            })
            .collect::<Vec<_>>();

        FunctionType {
            attributes: self.attributes.clone(),
            params: params.into(),
            return_type: self.return_type,
        }
    }
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct Trait {
    pub span: ByteSpan,
    pub visibility: Visibility,
    pub name: NString,
    pub generics: Option<BTreeMap<NString, Option<TypeId>>>,
    pub supertraits: Vec<TraitId>,
    pub where_clause: Option<Vec<WhereClause>>,
    pub methods: Vec<FunctionId>,
    pub associated_types: Vec<NString>,
    pub associated_constants: Vec<NString>,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum ModuleAttribute {
    Invalid,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct Module {
    pub span: ByteSpan,
    pub visibility: Visibility,
    pub name: NString,
    pub attributes: BTreeSet<ModuleAttribute>,
    pub items: Vec<Item>,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct TypeAliasDef {
    pub span: ByteSpan,
    pub visibility: Visibility,
    pub name: NString,
    pub generics: Option<BTreeMap<NString, Option<TypeId>>>,
    pub type_id: TypeId,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum StructAttribute {
    Packed,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum StructFieldAttribute {
    Align { alignment: PowOf2<u32> },
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct StructField {
    pub span: ByteSpan,
    pub visibility: Visibility,
    pub attributes: BTreeSet<StructFieldAttribute>,
    pub name: NString,
    pub ty: TypeId,
    pub default_value: Option<ValueId>,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum StructMemoryLayoutCell {
    Field { field_name: NString },
    Padding(NonZeroUsize),
}

impl StructMemoryLayoutCell {
    #[must_use]
    pub fn is_field(&self) -> bool {
        matches!(self, StructMemoryLayoutCell::Field { .. })
    }

    #[must_use]
    pub fn is_padding(&self) -> bool {
        matches!(self, StructMemoryLayoutCell::Padding(_))
    }

    #[must_use]
    pub fn as_field(&self) -> Option<&NString> {
        match self {
            StructMemoryLayoutCell::Field { field_name } => Some(field_name),
            _ => None,
        }
    }

    #[must_use]
    pub fn as_padding(&self) -> Option<NonZeroUsize> {
        match self {
            StructMemoryLayoutCell::Padding(size) => Some(*size),
            _ => None,
        }
    }
}

pub type StructLayout = ThinVec<StructMemoryLayoutCell>;

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct StructDef {
    pub span: ByteSpan,
    pub visibility: Visibility,
    pub name: NString,
    pub attributes: BTreeSet<StructAttribute>,
    pub fields: BTreeMap<NString, StructField>,
    pub generics: Option<BTreeMap<NString, Option<TypeId>>>,
    pub layout: StructLayout,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum EnumAttribute {
    Invalid,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum EnumVariantAttribute {
    Invalid,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct EnumVariant {
    pub span: ByteSpan,
    pub attributes: BTreeSet<EnumVariantAttribute>,
    pub name: NString,
    pub ty: TypeId,
    pub default_value: Option<ValueId>,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct EnumDef {
    pub span: ByteSpan,
    pub visibility: Visibility,
    pub name: NString,
    pub attributes: BTreeSet<EnumAttribute>,
    pub generics: Option<BTreeMap<NString, Option<TypeId>>>,
    pub variants: ThinVec<EnumVariant>,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum Item {
    Module(ModuleId),
    GlobalVariable(GlobalVariableId),
    Function(FunctionId),
    TypeAliasDef(TypeAliasDefId),
    StructDef(StructDefId),
    EnumDef(EnumDefId),
    Trait(TraitId),
}

impl From<GlobalVariable> for GlobalVariableId {
    fn from(gv: GlobalVariable) -> Self {
        get_storage(|store| store.store_global_variable(gv))
    }
}

impl From<LocalVariable> for LocalVariableId {
    fn from(lv: LocalVariable) -> Self {
        get_storage(|store| store.store_local_variable(lv))
    }
}

impl From<Parameter> for ParameterId {
    fn from(param: Parameter) -> Self {
        get_storage(|store| store.store_parameter(param))
    }
}

impl From<Function> for FunctionId {
    fn from(func: Function) -> Self {
        get_storage(|store| store.store_function(func))
    }
}

impl From<Trait> for TraitId {
    fn from(trait_: Trait) -> Self {
        get_storage(|store| store.store_trait(trait_))
    }
}

impl From<Module> for ModuleId {
    fn from(module: Module) -> Self {
        get_storage(|store| store.store_module(module))
    }
}

impl From<TypeAliasDef> for TypeAliasDefId {
    fn from(type_alias: TypeAliasDef) -> Self {
        get_storage(|store| store.store_type_alias(type_alias))
    }
}

impl From<StructDef> for StructDefId {
    fn from(struct_def: StructDef) -> Self {
        get_storage(|store| store.store_struct_def(struct_def))
    }
}

impl From<EnumDef> for EnumDefId {
    fn from(enum_def: EnumDef) -> Self {
        get_storage(|store| store.store_enum_def(enum_def))
    }
}
