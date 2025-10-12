use crate::{HirCtx, TryIntoHir, diagnosis::HirErr};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_parsetree::kind as ast;

impl TryIntoHir for ast::Block {
    type Hir = Block;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let mut elements = Vec::with_capacity(self.elements.len());
        let mut ends_with_unit = false;

        for element in self.elements {
            match element {
                ast::BlockItem::Expr(e) => {
                    let hir_element = e.try_into_hir(ctx, log)?;
                    elements.push(hir_element);
                    ends_with_unit = false;
                }

                ast::BlockItem::Stmt(s) => {
                    let hir_element = s.try_into_hir(ctx, log)?;
                    elements.push(hir_element);
                    ends_with_unit = true;
                }

                ast::BlockItem::Variable(v) => {
                    // TODO: Handle variable declarations properly
                    log.report(&HirErr::UnimplementedFeature(
                        "variable declaration in block".into(),
                    ));

                    ends_with_unit = true;
                }
            }
        }

        if ends_with_unit {
            elements.push(Value::Unit);
        }

        let safety = match self.safety {
            Some(ast::Safety::Unsafe(None)) => BlockSafety::Unsafe,
            Some(ast::Safety::Safe) | None => BlockSafety::Safe,

            Some(ast::Safety::Unsafe(Some(_))) => {
                log.report(&HirErr::UnimplementedFeature(
                    "block safety unsafe expression".into(),
                ));
                return Err(());
            }
        };

        Ok(Block { safety, elements })
    }
}

impl TryIntoHir for ast::Expr {
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: Implement conversion from AST to HIR
        Err(())
    }
}
