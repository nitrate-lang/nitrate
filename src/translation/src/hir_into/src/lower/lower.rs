use crate::context::HirCtx;
use nitrate_diagnosis::CompilerLog;

pub trait Ast2Hir {
    type Hir;

    fn ast2hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()>;
}
