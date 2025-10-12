use crate::{HirCtx, TryIntoHir};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_parsetree::kind as ast;

impl TryIntoHir for ast::Block {
    type Error = ();
    type Hir = Block;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        // TODO: Implement conversion from AST to HIR
        Err(())
    }
}

impl TryIntoHir for ast::Expr {
    type Error = ();
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        // TODO: Implement conversion from AST to HIR
        Err(())
    }
}
