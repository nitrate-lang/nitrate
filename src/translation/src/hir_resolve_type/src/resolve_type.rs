use nitrate_diagnosis::CompilerLog;

pub struct TyCtx {}

impl TyCtx {
    pub fn new() -> Self {
        Self {}
    }
}

pub trait TypeResolver {
    fn resolve_type(&mut self, ctx: &TyCtx, log: &CompilerLog);
}
