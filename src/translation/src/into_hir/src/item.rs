use crate::{HirCtx, TryIntoHir};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::{
    hir::{EntityName, GlobalVariable, Item, Type, Value},
    prelude::*,
};
use nitrate_parsetree::kind as ast;

impl TryIntoHir for ast::Module {
    type Error = Self;
    type Hir = hir::Module;

    fn try_into_hir(self, ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, Self::Error> {
        // TODO: Implement conversion from AST to HIR

        let global_var_type = Type::USize;
        let global_var_init = Value::USize(0);

        let item = Item::GlobalVariable(GlobalVariable {
            visibility: hir::Visibility::Pro,
            name: EntityName("GLOBAL_CTR".into()),
            ty: ctx.storage_mut().store_type(global_var_type),
            initializer: ctx.storage_mut().store_value(global_var_init),
        });

        Ok(hir::Module {
            visibility: hir::Visibility::Sec,
            attributes: vec![],
            name: EntityName("".into()),
            items: vec![ctx.storage_mut().store_item(item)],
        })
    }
}
