use crate::{
    Ast2HirCtx,
    lower::Ast2Hir,
    put_defaults::{module_put_defaults, type_put_defaults, value_put_defaults},
};

use nitrate_diagnosis::CompilerLog;
use nitrate_hir::hir::{Module, Type, Value};
use nitrate_source::ast;

pub fn ast_mod2hir(
    module: ast::Module,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Module, ()> {
    let mut module = module.ast2hir(ctx, log)?;
    module_put_defaults(&mut module, ctx, log);
    Ok(module)
}

pub fn ast_expr2hir(expr: ast::Expr, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    let mut value = expr.ast2hir(ctx, log)?;
    value_put_defaults(&mut value, ctx, log);
    Ok(value)
}

pub fn ast_type2hir(ty: ast::Type, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Type, ()> {
    let mut ty = ty.ast2hir(ctx, log)?;
    type_put_defaults(&mut ty, ctx, log);
    Ok(ty)
}
