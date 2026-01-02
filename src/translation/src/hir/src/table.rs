use crate::prelude::*;
use nitrate_nstring::NString;
use std::collections::{BTreeMap, BTreeSet, HashMap};

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
enum SymbolId {
    GlobalVariable(GlobalVariableId),
    LocalVariable(LocalVariableId),
    Parameter(ParameterId),
    Function(FunctionId),
    EnumVariant(EnumDefId),
}

#[derive(Debug)]
pub struct SymbolTab {
    symbols: HashMap<NString, SymbolId>,
    types: HashMap<NString, TypeDefinition>,
    methods: HashMap<(TypeId, NString), FunctionId>,
    traits: HashMap<NString, TraitId>,
    impls: HashMap<TypeId, HashMap<TraitId, HashMap<NString, FunctionId>>>,
    arch_ptr_size: PtrSize,
}

impl SymbolTab {
    #[must_use]
    pub fn new(arch_ptr_size: PtrSize) -> Self {
        Self {
            symbols: HashMap::new(),
            types: HashMap::new(),
            methods: HashMap::new(),
            traits: HashMap::new(),
            impls: HashMap::new(),
            arch_ptr_size,
        }
    }

    pub fn reset(&mut self) {
        *self = Self::new(self.arch_ptr_size);
    }

    pub fn add_global_variable(&mut self, global_var: GlobalVariableId) {
        let name = global_var.borrow().name.clone();
        let symbol = SymbolId::GlobalVariable(global_var);
        self.symbols.insert(name, symbol);
    }

    pub fn add_local_variable(&mut self, local_var: LocalVariableId) {
        let name = local_var.borrow().name.clone();
        let symbol = SymbolId::LocalVariable(local_var);
        self.symbols.insert(name, symbol);
    }

    pub fn add_parameter(&mut self, param: ParameterId) {
        let name = param.borrow().name.clone();
        let symbol = SymbolId::Parameter(param);
        self.symbols.insert(name, symbol);
    }

    pub fn add_function(&mut self, function: FunctionId) {
        let name = function.borrow().name.clone();
        let symbol = SymbolId::Function(function);
        self.symbols.insert(name, symbol);
    }

    pub fn add_enum_variant(&mut self, name: NString, enum_def_id: EnumDefId) {
        let symbol = SymbolId::EnumVariant(enum_def_id);
        self.symbols.insert(name, symbol);
    }

    pub fn add_method(&mut self, type_id: TypeId, method_name: NString, function_id: FunctionId) {
        self.methods.insert((type_id, method_name), function_id);
    }

    pub fn add_trait(&mut self, trait_id: TraitId) {
        let name = trait_id.borrow().name.clone();
        self.traits.insert(name, trait_id);
    }

    pub fn add_impl_trait(&mut self, type_id: TypeId, trait_id: TraitId) {
        self.impls
            .entry(type_id)
            .or_insert_with(HashMap::new)
            .entry(trait_id)
            .or_insert_with(HashMap::new);
    }

    pub fn add_trait_method(
        &mut self,
        type_id: TypeId,
        trait_id: TraitId,
        method_name: NString,
        function_id: FunctionId,
    ) {
        self.impls
            .entry(type_id)
            .or_insert_with(HashMap::new)
            .entry(trait_id)
            .or_insert_with(HashMap::new)
            .insert(method_name, function_id);
    }

    pub fn add_type_alias(&mut self, type_alias_id: TypeAliasDefId) {
        let name = type_alias_id.borrow().name.clone();
        let typedef = TypeDefinition::TypeAliasDef(type_alias_id);
        self.types.insert(name, typedef);
    }

    pub fn add_struct(&mut self, struct_def_id: StructDefId) {
        let name = struct_def_id.borrow().name.clone();
        let typedef = TypeDefinition::StructDef(struct_def_id);
        self.types.insert(name, typedef);
    }

    pub fn add_enum(&mut self, enum_def_id: EnumDefId) {
        let name = enum_def_id.borrow().name.clone();
        let typedef = TypeDefinition::EnumDef(enum_def_id);
        self.types.insert(name, typedef);
    }

    pub fn get_global_variable_or_insert_placeholder(&mut self, name: &NString) -> GlobalVariableId {
        if let Some(SymbolId::GlobalVariable(global_var_id)) = self.symbols.get(name).cloned() {
            return global_var_id;
        };

        let placeholder = GlobalVariable {
            visibility: Visibility::Sec,
            attributes: BTreeSet::new(),
            is_mutable: false,
            name: name.clone(),
            mangled_name: NString::default(),
            ty: Type::Unit.into(),
            initializer: Value::Unit.into(),
        };

        let global_var_id: GlobalVariableId = placeholder.into();
        self.add_global_variable(global_var_id.clone());
        self.get_global_variable_or_insert_placeholder(name)
    }

    pub fn get_global_variable(&self, name: &NString) -> Option<&GlobalVariableId> {
        if let Some(SymbolId::GlobalVariable(global_var_id)) = self.symbols.get(name) {
            Some(global_var_id)
        } else {
            None
        }
    }

    pub fn globals(&self) -> impl Iterator<Item = &GlobalVariableId> {
        self.symbols.values().filter_map(|symbol_id| {
            if let SymbolId::GlobalVariable(global_var_id) = symbol_id {
                Some(global_var_id)
            } else {
                None
            }
        })
    }

    pub fn get_local_variable_or_insert_placeholder(&mut self, name: &NString) -> LocalVariableId {
        if let Some(SymbolId::LocalVariable(local_var_id)) = self.symbols.get(name).cloned() {
            return local_var_id;
        };

        let placeholder = LocalVariable {
            kind: LocalKind::Let,
            attributes: BTreeSet::new(),
            is_mutable: false,
            name: name.clone(),
            ty: Type::Unit.into(),
            initializer: Value::Unit.into(),
        };

        let local_var_id: LocalVariableId = placeholder.into();
        self.add_local_variable(local_var_id.clone());
        self.get_local_variable_or_insert_placeholder(name)
    }

    pub fn get_local_variable(&self, name: &NString) -> Option<&LocalVariableId> {
        if let Some(SymbolId::LocalVariable(local_var_id)) = self.symbols.get(name) {
            Some(local_var_id)
        } else {
            None
        }
    }

    pub fn get_parameter_or_insert_placeholder(&mut self, name: &NString) -> ParameterId {
        if let Some(SymbolId::Parameter(param_id)) = self.symbols.get(name).cloned() {
            return param_id;
        };

        let placeholder = Parameter {
            attributes: BTreeSet::new(),
            is_mutable: false,
            name: name.clone(),
            ty: Type::Unit.into(),
            default_value: None,
        };

        let param_id: ParameterId = placeholder.into();
        self.add_parameter(param_id.clone());
        self.get_parameter_or_insert_placeholder(name)
    }

    pub fn get_parameter(&self, name: &NString) -> Option<&ParameterId> {
        if let Some(SymbolId::Parameter(param_id)) = self.symbols.get(name) {
            Some(param_id)
        } else {
            None
        }
    }

    pub fn get_function_or_insert_placeholder(&mut self, name: &NString) -> FunctionId {
        if let Some(SymbolId::Function(func_id)) = self.symbols.get(name).cloned() {
            return func_id;
        };

        let placeholder = Function {
            visibility: Visibility::Sec,
            attributes: BTreeSet::new(),
            name: name.clone(),
            mangled_name: NString::default(),
            params: Vec::new(),
            return_type: Type::Unit.into(),
            body: None,
        };

        let func_id: FunctionId = placeholder.into();
        self.add_function(func_id.clone());
        self.get_function_or_insert_placeholder(name)
    }

    pub fn get_function(&self, name: &NString) -> Option<&FunctionId> {
        if let Some(SymbolId::Function(func_id)) = self.symbols.get(name) {
            Some(func_id)
        } else {
            None
        }
    }

    pub fn functions(&self) -> impl Iterator<Item = &FunctionId> {
        self.symbols.values().filter_map(|symbol_id| {
            if let SymbolId::Function(func_id) = symbol_id {
                Some(func_id)
            } else {
                None
            }
        })
    }

    pub fn get_enum_variant_or_insert_placeholder(&mut self, name: &NString) -> EnumDefId {
        if let Some(SymbolId::EnumVariant(enum_def_id)) = self.symbols.get(name).cloned() {
            return enum_def_id;
        };

        let parts = name.split("::");
        let enum_name: NString = parts
            .clone()
            .take(parts.clone().count() - 1)
            .collect::<Vec<_>>()
            .join("::")
            .into();

        let enum_def = self.get_enum_or_insert_placeholder(&enum_name);
        self.add_enum_variant(name.clone(), enum_def.clone());
        enum_def
    }

    pub fn get_enum_variant(&self, name: &NString) -> Option<&EnumDefId> {
        if let Some(SymbolId::EnumVariant(enum_def_id)) = self.symbols.get(name) {
            Some(enum_def_id)
        } else {
            None
        }
    }

    pub fn get_method(&self, type_def: &TypeId, method_name: &NString) -> Option<&FunctionId> {
        if let Some(method) = self.methods.get(&(*type_def, method_name.clone())) {
            return Some(method);
        }

        if let Some(method) = self
            .impls
            .get(type_def)?
            .values()
            .find_map(|methods_map| methods_map.get(method_name))
        {
            return Some(method);
        }

        None
    }

    pub fn get_trait_or_insert_placeholder(&mut self, name: &NString) -> TraitId {
        if let Some(trait_id) = self.traits.get(name).cloned() {
            return trait_id;
        };

        let placeholder = Trait {
            visibility: Visibility::Sec,
            name: name.clone(),
            methods: Vec::new().into(),
        };

        let trait_id: TraitId = placeholder.into();
        self.add_trait(trait_id.clone());
        self.get_trait_or_insert_placeholder(name)
    }

    pub fn get_trait(&self, name: &NString) -> Option<&TraitId> {
        self.traits.get(name)
    }

    pub fn get_type_alias_or_insert_placeholder(&mut self, name: &NString) -> TypeAliasDefId {
        if let Some(TypeDefinition::TypeAliasDef(type_alias_id)) = self.types.get(name).cloned() {
            return type_alias_id;
        };

        let placeholder = TypeAliasDef {
            visibility: Visibility::Sec,
            name: name.clone(),
            type_id: Type::Unit.into(),
        };

        let type_alias_def: TypeAliasDefId = placeholder.into();
        self.add_type_alias(type_alias_def.clone());
        self.get_type_alias_or_insert_placeholder(name)
    }

    pub fn get_type_alias(&self, name: &NString) -> Option<&TypeAliasDefId> {
        if let Some(TypeDefinition::TypeAliasDef(type_alias_id)) = self.types.get(name) {
            Some(type_alias_id)
        } else {
            None
        }
    }

    pub fn get_struct_or_insert_placeholder(&mut self, name: &NString) -> StructDefId {
        if let Some(TypeDefinition::StructDef(struct_def_id)) = self.types.get(name).cloned() {
            return struct_def_id;
        };

        let placeholder = StructDef {
            visibility: Visibility::Sec,
            attributes: BTreeSet::new(),
            name: name.clone(),
            fields: BTreeMap::new(),
            layout: StructLayout::new(),
        };

        let struct_def: StructDefId = placeholder.into();
        self.add_struct(struct_def.clone());
        self.get_struct_or_insert_placeholder(name)
    }

    pub fn get_struct(&self, name: &NString) -> Option<&StructDefId> {
        if let Some(TypeDefinition::StructDef(struct_def_id)) = self.types.get(name) {
            Some(struct_def_id)
        } else {
            None
        }
    }

    pub fn get_enum_or_insert_placeholder(&mut self, name: &NString) -> EnumDefId {
        if let Some(TypeDefinition::EnumDef(enum_def_id)) = self.types.get(name).cloned() {
            return enum_def_id;
        };

        let placeholder = EnumDef {
            visibility: Visibility::Sec,
            attributes: BTreeSet::new(),
            name: name.clone(),
            variants: Vec::new().into(),
        };

        let enum_def: EnumDefId = placeholder.into();
        self.add_enum(enum_def.clone());
        self.get_enum_or_insert_placeholder(name)
    }

    pub fn get_enum(&self, name: &NString) -> Option<&EnumDefId> {
        if let Some(TypeDefinition::EnumDef(enum_def_id)) = self.types.get(name) {
            Some(enum_def_id)
        } else {
            None
        }
    }

    pub fn arch_ptr_size(&self) -> PtrSize {
        self.arch_ptr_size
    }
}
