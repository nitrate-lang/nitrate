use crate::{
    Store,
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
}
