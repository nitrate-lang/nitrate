use crate::{HirCtx, TryIntoHir, diagnosis::HirErr};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_parsetree::kind as ast;

impl TryIntoHir for ast::Module {
    type Hir = Module;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: Implement conversion from AST to HIR

        let name = EntityName(self.name.map(|n| n.to_string()).unwrap_or_default().into());
        let ast_attributes = self.attributes.unwrap_or_default();

        let mut items = Vec::with_capacity(self.items.len());
        for item in self.items {
            let hir_item = item.try_into_hir(ctx, log)?.into_id(ctx.store());
            items.push(hir_item);
        }

        let attributes = Vec::with_capacity(ast_attributes.len());
        for _attr in ast_attributes {
            // TODO: Lower AST module-level attributes
            log.report(&HirErr::UnspecifiedError);
        }

        let visibility = match self.visibility {
            Some(ast::Visibility::Public) => Visibility::Pub,
            Some(ast::Visibility::Protected) => Visibility::Pro,
            Some(ast::Visibility::Private) | None => Visibility::Sec,
        };

        let module = Module {
            name,
            visibility,
            attributes,
            items,
        };

        Ok(module)
    }
}

impl TryIntoHir for ast::Item {
    type Hir = Item;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: Implement conversion from AST to HIR
        Err(())
    }
}
