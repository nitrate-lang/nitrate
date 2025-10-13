use nitrate_diagnosis::CompilerLog;
use nitrate_hir::hir::HirCtx;

pub trait TryIntoHir {
    type Hir;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()>;
}
