use nitrate_diagnosis::CompilerLog;
use nitrate_hir::Store;

pub struct TyCtx<'store> {
    pub(crate) store: &'store Store,
}

impl<'store> TyCtx<'store> {
    pub fn new(store: &'store Store) -> Self {
        Self { store }
    }
}

pub trait TypeResolver {
    fn resolve_type(&mut self, ctx: &mut TyCtx, log: &CompilerLog);
}
