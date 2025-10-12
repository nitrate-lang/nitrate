use crate::{HirCtx, TryIntoHir};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_parsetree::kind as ast;

impl TryIntoHir for ast::Block {
    type Hir = Block;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: Implement conversion from AST to HIR
        Err(())
    }
}

impl TryIntoHir for ast::Expr {
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: Implement conversion from AST to HIR
        Err(())
    }
}
