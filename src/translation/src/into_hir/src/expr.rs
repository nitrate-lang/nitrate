use crate::{HirCtx, TryIntoHir};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_parsetree::kind as ast;

impl TryIntoHir for ast::Expr {
    type Error = Self;
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        // TODO: Implement conversion from AST to HIR
        Err(self)
    }
}
