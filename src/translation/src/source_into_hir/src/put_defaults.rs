use crate::Ast2HirCtx;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;

pub(crate) fn module_put_defaults(module: &mut Module, ctx: &mut Ast2HirCtx, log: &CompilerLog) {
    // module.iter().for_each(
    //     &ctx.store,
    //     &mut |value| {
    //         // TODO:
    //     },
    //     &mut |ty| {
    //         // TODO:
    //     },
    // );
}

pub(crate) fn value_put_defaults(value: &mut Value, ctx: &mut Ast2HirCtx, log: &CompilerLog) {
    value.iter().for_each_value(&ctx.store, &mut |value| {
        // TODO:
    });
}

pub(crate) fn type_put_defaults(ty: &mut Type, ctx: &mut Ast2HirCtx, log: &CompilerLog) {
    ty.iter().for_each_value(&ctx.store, &mut |value| {
        // TODO:
    });
}
