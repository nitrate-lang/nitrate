use nitrate_diagnosis::CompilerLog;
use nitrate_hir::{hir::QualifiedName, prelude::*};
use std::{collections::HashMap, num::NonZeroU32};

pub struct HirCtx {
    storage: Store,
    symbol_table: HashMap<QualifiedName, SymbolId>,
    type_table: HashMap<QualifiedName, TypeId>,
    type_infer_id_ctr: NonZeroU32,
    // impl_table: HashMap<TypeId, Vec<ImplId>>,
}

impl HirCtx {
    pub fn new() -> Self {
        Self {
            storage: Store::new(),
            symbol_table: HashMap::new(),
            type_table: HashMap::new(),
            type_infer_id_ctr: NonZeroU32::new(1).unwrap(),
        }
    }

    pub(crate) fn next_type_infer_id(&mut self) -> NonZeroU32 {
        let id = self.type_infer_id_ctr;
        self.type_infer_id_ctr = id.checked_add(1).expect("Type infer ID overflow");
        id
    }

    pub fn store(&self) -> &Store {
        &self.storage
    }

    pub fn store_mut(&mut self) -> &mut Store {
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
