use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_parsetree::kind as ast;

pub trait ConvertIntoToHir {
    type Hir;

    fn try_into_hir(self, log: &CompilerLog) -> Result<Self::Hir, ()>;
}

impl ConvertIntoToHir for ast::Expr {
    type Hir = hir::Expr;

    fn try_into_hir(self, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Err(())
    }
}
