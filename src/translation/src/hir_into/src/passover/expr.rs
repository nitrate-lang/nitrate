use std::sync::{Arc, RwLock};

use nitrate_diagnosis::CompilerLog;
use nitrate_hir::hir::HirCtx;
use nitrate_parsetree::{
    ast::{self, Block, BlockItem, Function, Item, Module},
    tag::intern_function_name,
};

use crate::passover::passover_module;

pub fn passover_expr(expr: ast::Expr, ctx: &mut HirCtx, log: &CompilerLog) -> ast::Expr {
    let function_cradle = Function {
        visibility: None,
        attributes: None,
        name: intern_function_name(ctx.get_unique_name().to_string()),
        generics: None,
        parameters: Vec::new(),
        return_type: None,
        definition: Some(Block {
            safety: None,
            elements: vec![BlockItem::Expr(expr.clone())],
        }),
    };

    let mut ast_mock_module = Module {
        visibility: None,
        name: None,
        attributes: None,
        items: vec![Item::Function(Arc::new(RwLock::new(function_cradle)))],
    };

    passover_module(&mut ast_mock_module, ctx, log);

    expr
}
