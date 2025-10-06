use crate::{HirCtx, TryIntoHir};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_parsetree::kind as ast;

impl TryIntoHir for ast::Module {
    type Error = Self;
    type Hir = Module;

    fn try_into_hir(self, ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        // TODO: Implement conversion from AST to HIR

        let global_var_type = Type::USize.into_id(ctx.store());
        let global_var_init = Value::USize(0).into_id(ctx.store());

        let item = Item::GlobalVariable(GlobalVariable {
            visibility: Visibility::Pub,
            name: EntityName("GLOBAL_CTR".into()),
            ty: global_var_type,
            initializer: global_var_init,
        })
        .into_id(ctx.store());

        Ok(Module {
            visibility: Visibility::Sec,
            attributes: vec![],
            name: EntityName("".into()),
            items: vec![item],
        })
    }
}
