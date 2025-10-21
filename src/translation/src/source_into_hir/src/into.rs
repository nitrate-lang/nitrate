use nitrate_diagnosis::CompilerLog;
use nitrate_hir::hir::{Module, Type, Value};
use nitrate_source::ast;

use crate::{Ast2HirCtx, forward_decl::generate_forward_declarations, lower::Ast2Hir};

pub fn ast_mod2hir(
    mut module: ast::Module,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Module, ()> {
    generate_forward_declarations(&mut module, ctx, log);
    module.ast2hir(ctx, log)
}

pub fn ast_expr2hir(
    mut expr: ast::Expr,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Value, ()> {
    generate_forward_declarations(&mut expr, ctx, log);
    expr.ast2hir(ctx, log)
}

pub fn ast_type2hir(
    mut ty: ast::Type,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Type, ()> {
    generate_forward_declarations(&mut ty, ctx, log);
    ty.ast2hir(ctx, log)
}
