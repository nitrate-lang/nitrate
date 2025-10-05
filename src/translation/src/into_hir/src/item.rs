use crate::{HirCtx, TryIntoHir};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::{
    hir::{EntityName, GlobalVariable, Item, SaveToStorage, Type, Value},
    prelude::*,
};
use nitrate_parsetree::kind as ast;

impl TryIntoHir for ast::Module {
    type Error = Self;
    type Hir = hir::Module;

    fn try_into_hir(self, ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        // TODO: Implement conversion from AST to HIR

        let global_var_type = Type::USize.save(ctx.store_mut());
        let global_var_init = Value::USize(0).save(ctx.store_mut());

        let item = Item::GlobalVariable(GlobalVariable {
            visibility: hir::Visibility::Pub,
            name: EntityName("GLOBAL_CTR".into()),
            ty: global_var_type,
            initializer: global_var_init,
        })
        .save(ctx.store_mut());

        Ok(hir::Module {
            visibility: hir::Visibility::Sec,
            attributes: vec![],
            name: EntityName("".into()),
            items: vec![item],
        })
    }
}
