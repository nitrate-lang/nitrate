use crate::prelude::*;
use nitrate_nstring::NString;
use std::collections::HashMap;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
enum SymbolId {
    GlobalVariable(GlobalVariableId),
    LocalVariable(LocalVariableId),
    Parameter(ParameterId),
    Function(FunctionId),
    EnumVariant(EnumDefId),
}

#[derive(Debug, Default)]
pub struct SymbolTab {
    symbols: HashMap<NString, SymbolId>,
    types: HashMap<NString, TypeDefinition>,
    methods: HashMap<(TypeId, NString), FunctionId>,
}

impl SymbolTab {
    #[must_use]
    pub fn new() -> Self {
        Self {
            symbols: HashMap::new(),
            types: HashMap::new(),
            methods: HashMap::new(),
        }
    }

    pub fn reset(&mut self) {
        *self = Self::default();
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

    #[must_use]
    pub fn get_global_variable(&self, name: &NString) -> Option<&GlobalVariableId> {
        match self.symbols.get(name) {
            Some(SymbolId::GlobalVariable(global_var_id)) => Some(global_var_id),
            _ => None,
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

    #[must_use]
    pub fn get_local_variable(&self, name: &NString) -> Option<&LocalVariableId> {
        match self.symbols.get(name) {
            Some(SymbolId::LocalVariable(local_var_id)) => Some(local_var_id),
            _ => None,
        }
    }

    #[must_use]
    pub fn get_parameter(&self, name: &NString) -> Option<&ParameterId> {
        match self.symbols.get(name) {
            Some(SymbolId::Parameter(param_id)) => Some(param_id),
            _ => None,
        }
    }

    #[must_use]
    pub fn get_function(&self, name: &NString) -> Option<&FunctionId> {
        match self.symbols.get(name) {
            Some(SymbolId::Function(func_id)) => Some(func_id),
            _ => None,
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

    pub fn get_enum_variant(&self, name: &NString) -> Option<&EnumDefId> {
        match self.symbols.get(name) {
            Some(SymbolId::EnumVariant(enum_def_id)) => Some(enum_def_id),
            _ => None,
        }
    }

    #[must_use]
    pub fn get_method(&self, type_def: &TypeId, method_name: &NString) -> Option<&FunctionId> {
        self.methods.get(&(*type_def, method_name.clone()))
    }

    #[must_use]
    pub fn get_type_alias(&self, name: &NString) -> Option<&TypeAliasDefId> {
        match self.types.get(name) {
            Some(TypeDefinition::TypeAliasDef(type_alias_id)) => Some(type_alias_id),
            _ => None,
        }
    }

    #[must_use]
    pub fn get_struct(&self, name: &NString) -> Option<&StructDefId> {
        match self.types.get(name) {
            Some(TypeDefinition::StructDef(struct_def_id)) => Some(struct_def_id),
            _ => None,
        }
    }

    #[must_use]
    pub fn get_enum(&self, name: &NString) -> Option<&EnumDefId> {
        match self.types.get(name) {
            Some(TypeDefinition::EnumDef(enum_def_id)) => Some(enum_def_id),
            _ => None,
        }
    }
}
