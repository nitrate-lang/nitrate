use crate::hir::{SymbolId, TypeDefinition};
use interned_string::IString;
use std::collections::HashMap;

#[derive(Debug, Default)]
pub struct SymbolTab {
    pub symbols: HashMap<IString, SymbolId>,
    pub types: HashMap<IString, TypeDefinition>,
}
