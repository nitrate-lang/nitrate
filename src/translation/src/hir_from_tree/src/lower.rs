use crate::context::Ast2HirCtx;
use nitrate_diagnosis::CompilerLog;

pub(crate) trait Ast2Hir {
    type Hir;

    fn ast2hir(self, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()>;
}
