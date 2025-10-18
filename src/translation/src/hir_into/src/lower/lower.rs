use nitrate_diagnosis::CompilerLog;
use nitrate_hir::hir::HirCtx;

pub trait Ast2Hir {
    type Hir;

    fn ast2hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()>;
}
