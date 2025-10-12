use crate::{HirCtx, TryIntoHir, diagnosis::HirErr};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_parsetree::kind as ast;

impl TryIntoHir for ast::Module {
    type Hir = Module;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: Implement conversion from AST to HIR
        log.report(&HirErr::UnspecifiedError);

        let global_var_type = Type::USize.into_id(ctx.store());
        let global_var_init = Value::U32(0).into_id(ctx.store());

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
