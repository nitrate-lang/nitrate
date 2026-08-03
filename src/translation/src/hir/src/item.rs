use crate::{helper::PowOf2, prelude::*};
use nitrate_nstring::NString;
use nitrate_tree::SrcPos;
use serde::{Deserialize, Serialize};
use std::{
    collections::{BTreeMap, BTreeSet},
    matches,
    num::NonZeroUsize,
};
use thin_vec::ThinVec;

/// Visibility modifier for items (fields, functions, types, etc.).
#[derive(Debug, Clone, Copy, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum Visibility {
    /// Private (module-scoped).
    Sec,
    /// Protected (accessible within the package).
    Pro,
    /// Public (accessible from any package).
    Pub,
}

/// Attributes that can be applied to global variables.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum GlobalVariableAttribute {
    /// Do not mangle the global's symbol name.
    NoMangle,
}

/// A global variable definition (module-level or static).
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct GlobalVariable {
    /// Source location.
    pub span: SrcPos,
    /// Visibility of this global variable.
    pub visibility: Visibility,
    /// Attributes applied to this global variable.
    pub attributes: BTreeSet<GlobalVariableAttribute>,
    /// Whether the global can be mutated after initialization.
    pub is_mutable: bool,
    /// The variable name.
    pub name: NString,
    /// The mangled symbol name (populated by the mangler).
    pub mangled_name: Option<NString>,
    /// The declared type of the global variable.
    pub ty: TypeId,
    /// The initializer expression.
    pub initializer: ValueId,
}

/// Attributes that can be applied to local variable declarations.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum LocalVariableAttribute {
    /// Alignment constraint for the local variable.
    Align {
        /// The required byte alignment (must be a power of 2).
        alignment: PowOf2<u32>,
    },
}

/// The kind of a local variable binding.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum LocalKind {
    /// A `let` binding (immutable or mutable with shadowing semantics).
    Let,
    /// A `var` binding (mutable variable, may be reassigned).
    Var,
    /// A `static` binding (local static with program lifetime).
    Static,
}

/// A local variable declared within a function body.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct LocalVariable {
    /// Source location.
    pub span: SrcPos,
    /// The kind of binding (`let`, `var`, `static`).
    pub kind: LocalKind,
    /// Attributes applied to this local.
    pub attributes: BTreeSet<LocalVariableAttribute>,
    /// Whether the variable can be mutated.
    pub is_mutable: bool,
    /// The variable name.
    pub name: NString,
    /// The declared or inferred type of the variable.
    pub ty: TypeId,
    /// The optional initializer expression (`None` for uninitialized `var`).
    pub initializer: Option<ValueId>,
}

/// Attributes that can be applied to function parameters.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum ParameterAttribute {
    /// Alignment constraint for the parameter.
    Align {
        /// The required byte alignment (must be a power of 2).
        alignment: PowOf2<u32>,
    },
}

/// A function parameter.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct Parameter {
    /// Source location.
    pub span: SrcPos,
    /// Attributes applied to this parameter.
    pub attributes: BTreeSet<ParameterAttribute>,
    /// Whether the parameter is mutable (can be reassigned in the function body).
    pub is_mutable: bool,
    /// The parameter name.
    pub name: NString,
    /// The declared type of the parameter.
    pub ty: TypeId,
    /// Optional default value for this parameter.
    pub default_value: Option<ValueId>,
}

/// A function definition.
///
/// This is the primary executable unit in Nitrate. Functions may have
/// generic type parameters, which are monomorphized during solving.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct Function {
    /// Source location of the function signature.
    pub span: SrcPos,
    /// Visibility of the function.
    pub visibility: Visibility,
    /// Attributes applied to this function (CVariadic, NoMangle, ExternAbi).
    pub attributes: BTreeSet<FunctionAttribute>,
    /// Whether the function body is unsafe.
    pub is_unsafe: bool,
    /// The function name.
    pub name: NString,
    /// The mangled symbol name (populated by the mangler).
    pub mangled_name: Option<NString>,
    /// Optional generic type parameters.
    /// Maps parameter names to optional default type bindings.
    pub generics: Option<BTreeMap<NString, Option<TypeId>>>,
    /// The list of parameters.
    pub params: Vec<ParameterId>,
    /// The return type of the function.
    pub return_type: TypeId,
    /// The function body (`None` for external/declaration-only functions).
    pub body: Option<Vec<BlockElement>>,
}

impl Function {
    /// Constructs a [`FunctionType`] from this function's signature.
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

/// A trait definition.
///
/// Traits define a set of methods, associated types, and associated
/// constants that implementing types must provide.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct Trait {
    /// Source location of the trait definition.
    pub span: SrcPos,
    /// Visibility of the trait.
    pub visibility: Visibility,
    /// The trait name.
    pub name: NString,
    /// Optional generic type parameters for the trait.
    pub generics: Option<BTreeMap<NString, Option<TypeId>>>,
    /// Supertraits that must be implemented for this trait to be satisfied.
    pub supertraits: Vec<TraitId>,
    /// Optional `where` clause constraints.
    pub where_clause: Option<Vec<WhereClause>>,
    /// Methods declared on this trait (may have default implementations).
    pub methods: Vec<FunctionId>,
    /// Associated type names declared in this trait.
    pub associated_types: Vec<NString>,
    /// Associated constant names declared in this trait.
    pub associated_constants: Vec<NString>,
}

/// Attributes that can be applied to modules.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum ModuleAttribute {
    /// Placeholder for invalid/unknown module attributes.
    Invalid,
}

/// A module definition (a named group of items).
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct Module {
    /// Source location.
    pub span: SrcPos,
    /// Visibility of the module.
    pub visibility: Visibility,
    /// The module name.
    pub name: NString,
    /// Attributes applied to this module.
    pub attributes: BTreeSet<ModuleAttribute>,
    /// The items contained in this module (functions, structs, enums, etc.).
    pub items: Vec<Item>,
}

/// A type alias definition (`type AliasName = UnderlyingType`).
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct TypeAliasDef {
    /// Source location.
    pub span: SrcPos,
    /// Visibility of the type alias.
    pub visibility: Visibility,
    /// The alias name.
    pub name: NString,
    /// Optional generic type parameters.
    pub generics: Option<BTreeMap<NString, Option<TypeId>>>,
    /// The underlying type being aliased.
    pub type_id: TypeId,
}

/// Attributes that can be applied to struct definitions.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum StructAttribute {
    /// The struct should have packed (1-byte) alignment without padding.
    Packed,
}

/// Attributes that can be applied to struct fields.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum StructFieldAttribute {
    /// Alignment override for this field.
    Align {
        /// The required byte alignment (must be a power of 2).
        alignment: PowOf2<u32>,
    },
}

/// A field within a struct definition.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct StructField {
    /// Source location.
    pub span: SrcPos,
    /// Visibility of this field.
    pub visibility: Visibility,
    /// Attributes applied to this field.
    pub attributes: BTreeSet<StructFieldAttribute>,
    /// The field name.
    pub name: NString,
    /// The declared type of the field.
    pub ty: TypeId,
    /// Optional default value for this field.
    pub default_value: Option<ValueId>,
}

/// A single cell in a struct's memory layout.
///
/// The layout is a linear sequence of cells describing how fields and
/// padding are arranged in memory.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum StructMemoryLayoutCell {
    /// A field at this position in the layout.
    Field {
        /// The name of the field.
        field_name: NString,
    },
    /// Padding bytes inserted for alignment.
    Padding(
        /// Number of padding bytes.
        NonZeroUsize,
    ),
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

/// The memory layout of a struct — a sequence of fields and padding cells.
pub type StructLayout = ThinVec<StructMemoryLayoutCell>;

/// A struct definition.
///
/// Structs are named composite types with named, typed fields. They may
/// have generic type parameters and a defined memory layout.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct StructDef {
    /// Source location.
    pub span: SrcPos,
    /// Visibility of the struct.
    pub visibility: Visibility,
    /// The struct name.
    pub name: NString,
    /// Attributes applied to this struct (e.g., `Packed`).
    pub attributes: BTreeSet<StructAttribute>,
    /// The fields of the struct, keyed by name.
    pub fields: BTreeMap<NString, StructField>,
    /// Optional generic type parameters.
    pub generics: Option<BTreeMap<NString, Option<TypeId>>>,
    /// The computed memory layout (field order + padding).
    pub layout: StructLayout,
}

/// Attributes that can be applied to enum definitions.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum EnumAttribute {
    /// Placeholder for invalid/unknown enum attributes.
    Invalid,
}

/// Attributes that can be applied to enum variant definitions.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum EnumVariantAttribute {
    /// Placeholder for invalid/unknown variant attributes.
    Invalid,
}

/// A single variant within an enum definition.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct EnumVariant {
    /// Source location.
    pub span: SrcPos,
    /// Attributes applied to this variant.
    pub attributes: BTreeSet<EnumVariantAttribute>,
    /// The variant name.
    pub name: NString,
    /// The type of the variant's payload (use `Unit` for no payload).
    pub ty: TypeId,
    /// Optional default value for this variant.
    pub default_value: Option<ValueId>,
}

/// An enum definition.
///
/// Enums are tagged unions — a type that can hold exactly one of
/// several named variants, each with an optional payload type.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct EnumDef {
    /// Source location.
    pub span: SrcPos,
    /// Visibility of the enum.
    pub visibility: Visibility,
    /// The enum name.
    pub name: NString,
    /// Attributes applied to this enum.
    pub attributes: BTreeSet<EnumAttribute>,
    /// Optional generic type parameters.
    pub generics: Option<BTreeMap<NString, Option<TypeId>>>,
    /// The variants of this enum.
    pub variants: ThinVec<EnumVariant>,
}

/// A top-level item in a module.
///
/// Items are the building blocks of Nitrate programs — modules, globals,
/// functions, type aliases, structs, enums, and traits.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum Item {
    /// A submodule.
    Module(ModuleId),
    /// A global variable.
    GlobalVariable(GlobalVariableId),
    /// A function definition.
    Function(FunctionId),
    /// A type alias definition.
    TypeAliasDef(TypeAliasDefId),
    /// A struct definition.
    StructDef(StructDefId),
    /// An enum definition.
    EnumDef(EnumDefId),
    /// A trait definition.
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
