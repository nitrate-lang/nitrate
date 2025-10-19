use crate::lower::Ast2Hir;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::hir::{self, HirCtx};
use nitrate_source::ast;

pub fn ast_mod2hir(
    module: ast::Module,
    ctx: &mut HirCtx,
    log: &CompilerLog,
) -> Result<hir::Module, ()> {
    module.ast2hir(ctx, log)
    // TODO: Finalize by resolving symbol links
}

pub fn ast_expr2hir(
    expr: ast::Expr,
    ctx: &mut HirCtx,
    log: &CompilerLog,
) -> Result<hir::Value, ()> {
    expr.ast2hir(ctx, log)
    // TODO: Finalize by resolving symbol links
}

pub fn ast_type2hir(ty: ast::Type, ctx: &mut HirCtx, log: &CompilerLog) -> Result<hir::Type, ()> {
    ty.ast2hir(ctx, log)
    // TODO: Finalize by resolving symbol links
}
