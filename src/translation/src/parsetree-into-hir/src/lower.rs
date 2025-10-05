use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;

#[derive(Clone)]
pub struct SymbolTable {}

#[derive(Clone)]
pub struct TypeTable {}

#[derive(Clone)]
pub struct HirCtx {
    storage: Store,
    symbol_table: SymbolTable,
    type_table: TypeTable,
}

impl HirCtx {
    pub fn new() -> Self {
        Self {
            storage: Store::new(),
            symbol_table: SymbolTable {},
            type_table: TypeTable {},
        }
    }

    pub fn storage(&self) -> &Store {
        &self.storage
    }

    pub fn storage_mut(&mut self) -> &mut Store {
        &mut self.storage
    }

    pub fn symbol_table(&self) -> &SymbolTable {
        &self.symbol_table
    }

    pub fn symbol_table_mut(&mut self) -> &mut SymbolTable {
        &mut self.symbol_table
    }

    pub fn type_table(&self) -> &TypeTable {
        &self.type_table
    }

    pub fn type_table_mut(&mut self) -> &mut TypeTable {
        &mut self.type_table
    }
}

pub trait TryIntoHir {
    type Hir;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()>;
}
