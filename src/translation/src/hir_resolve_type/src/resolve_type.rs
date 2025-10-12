use nitrate_diagnosis::CompilerLog;
use nitrate_hir::Store;

pub struct TyCtx<'store> {
    pub(crate) store: &'store mut Store,
}

impl<'store> TyCtx<'store> {
    pub fn new(store: &'store mut Store) -> Self {
        Self { store }
    }
}

pub trait TypeResolver {
    fn resolve_type(&self, ctx: &mut TyCtx, log: &CompilerLog);
}
