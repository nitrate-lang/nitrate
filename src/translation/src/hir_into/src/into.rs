use crate::{
    lower::Ast2Hir,
    passover::{passover_expr, passover_item, passover_type},
};

use nitrate_diagnosis::CompilerLog;
use nitrate_hir::hir::{self, HirCtx};
use nitrate_parsetree::ast;

pub struct ModulePrep<'log, 'ctx> {
    log: &'log CompilerLog,
    ctx: &'ctx mut HirCtx,
    module: ast::Module,
}

impl<'log, 'ctx> ModulePrep<'log, 'ctx> {
    pub fn new(module: ast::Module, log: &'log CompilerLog, ctx: &'ctx mut HirCtx) -> Self {
        Self { log, ctx, module }
    }
}

pub struct ExprPrep<'log, 'ctx> {
    log: &'log CompilerLog,
    ctx: &'ctx mut HirCtx,
    expr: ast::Expr,
}

impl<'log, 'ctx> ExprPrep<'log, 'ctx> {
    pub fn new(expr: ast::Expr, log: &'log CompilerLog, ctx: &'ctx mut HirCtx) -> Self {
        Self { log, ctx, expr }
    }
}

pub struct TypePrep<'log, 'ctx> {
    log: &'log CompilerLog,
    ctx: &'ctx mut HirCtx,
    ty: ast::Type,
}

impl<'log, 'ctx> TypePrep<'log, 'ctx> {
    pub fn new(ty: ast::Type, log: &'log CompilerLog, ctx: &'ctx mut HirCtx) -> Self {
        Self { log, ctx, ty }
    }
}

impl TryFrom<ModulePrep<'_, '_>> for hir::Module {
    type Error = ();

    fn try_from(value: ModulePrep<'_, '_>) -> Result<Self, Self::Error> {
        for item in &value.module.items {
            passover_item(item, value.ctx, value.log);
        }
        value.module.ast2hir(value.ctx, value.log)
    }
}

impl TryFrom<ExprPrep<'_, '_>> for hir::Value {
    type Error = ();

    fn try_from(value: ExprPrep<'_, '_>) -> Result<Self, Self::Error> {
        passover_expr(&value.expr, value.ctx, value.log);
        value.expr.ast2hir(value.ctx, value.log)
    }
}

impl TryFrom<TypePrep<'_, '_>> for hir::Type {
    type Error = ();

    fn try_from(value: TypePrep<'_, '_>) -> Result<Self, Self::Error> {
        passover_type(&value.ty, value.ctx, value.log);
        value.ty.ast2hir(value.ctx, value.log)
    }
}
