use nitrate_diagnosis::CompilerLog;
use nitrate_hir::{hir::QualifiedName, prelude::*};
use std::collections::HashMap;

#[derive(Clone)]
pub struct HirCtx {
    storage: Store,
    symbol_table: HashMap<QualifiedName, SymbolId>,
    type_table: HashMap<QualifiedName, TypeId>,
}

impl HirCtx {
    pub fn new() -> Self {
        Self {
            storage: Store::new(),
            symbol_table: HashMap::new(),
            type_table: HashMap::new(),
        }
    }

    pub fn storage(&self) -> &Store {
        &self.storage
    }

    pub fn storage_mut(&mut self) -> &mut Store {
        &mut self.storage
    }

    pub fn resolve_symbol(&self, name: &QualifiedName) -> Option<&SymbolId> {
        self.symbol_table.get(name)
    }

    pub fn resolve_type(&self, name: &QualifiedName) -> Option<&TypeId> {
        self.type_table.get(name)
    }
}

pub trait TryIntoHir {
    type Error;
    type Hir;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, Self::Error>;
}
