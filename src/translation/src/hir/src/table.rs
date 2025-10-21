use crate::{
    EnumDefId, FunctionId, GlobalVariableId, LocalVariableId, ParameterId, Store, StructDefId,
    TypeAliasDefId,
    hir::{SymbolId, TypeDefinition},
};
use interned_string::IString;
use std::collections::HashMap;

#[derive(Debug, Default)]
pub struct SymbolTab {
    symbols: HashMap<IString, SymbolId>,
    types: HashMap<IString, TypeDefinition>,
}

impl SymbolTab {
    pub fn new() -> Self {
        Self {
            symbols: HashMap::new(),
            types: HashMap::new(),
        }
    }

    pub fn clear_symbols(&mut self) {
        self.symbols.clear();
    }

    pub fn clear_types(&mut self) {
        self.types.clear();
    }

    pub fn add_symbol(&mut self, symbol: SymbolId, store: &Store) {
        self.symbols.insert(symbol.name(store), symbol);
    }

    pub fn add_type(&mut self, type_def: TypeDefinition, store: &Store) {
        self.types.insert(type_def.name(store), type_def);
    }

    pub fn get_symbol(&self, name: &IString) -> Option<&SymbolId> {
        self.symbols.get(name)
    }

    pub fn get_type(&self, name: &IString) -> Option<&TypeDefinition> {
        self.types.get(name)
    }

    pub fn get_type_alias(&self, name: &IString) -> Option<&TypeAliasDefId> {
        match self.types.get(name) {
            Some(TypeDefinition::TypeAliasDef(type_alias_id)) => Some(type_alias_id),
            _ => None,
        }
    }

    pub fn get_struct(&self, name: &IString) -> Option<&StructDefId> {
        match self.types.get(name) {
            Some(TypeDefinition::StructDef(struct_def_id)) => Some(struct_def_id),
            _ => None,
        }
    }

    pub fn get_enum(&self, name: &IString) -> Option<&EnumDefId> {
        match self.types.get(name) {
            Some(TypeDefinition::EnumDef(enum_def_id)) => Some(enum_def_id),
            _ => None,
        }
    }

    pub fn get_global_variable(&self, name: &IString) -> Option<&GlobalVariableId> {
        match self.symbols.get(name) {
            Some(SymbolId::GlobalVariable(global_var_id)) => Some(global_var_id),
            _ => None,
        }
    }

    pub fn get_local_variable(&self, name: &IString) -> Option<&LocalVariableId> {
        match self.symbols.get(name) {
            Some(SymbolId::LocalVariable(local_var_id)) => Some(local_var_id),
            _ => None,
        }
    }

    pub fn get_parameter(&self, name: &IString) -> Option<&ParameterId> {
        match self.symbols.get(name) {
            Some(SymbolId::Parameter(param_id)) => Some(param_id),
            _ => None,
        }
    }

    pub fn get_function(&self, name: &IString) -> Option<&FunctionId> {
        match self.symbols.get(name) {
            Some(SymbolId::Function(func_id)) => Some(func_id),
            _ => None,
        }
    }
}
