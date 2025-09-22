use crate::{Symbol, SymbolName, SymbolTable, build_symbol_table};

use nitrate_diagnosis::DiagnosticCollector;
use nitrate_parsetree::{
    Order, ParseTreeIterMut, RefNodeMut,
    kind::{ExprPath, ExprPathTarget, ItemPath, Module, TypePath, TypePathTarget},
};

use std::sync::Arc;

fn resolve_item_path(_scope: &Vec<String>, _path: &mut ItemPath, _symbol_table: &SymbolTable) {
    // https://doc.rust-lang.org/reference/paths.html#r-paths.simple.intro

    // TODO: Resolve imports here?
}

fn resolve_expr_path(scope: &Vec<String>, path: &mut ExprPath, symbol_table: &SymbolTable) {
    let pathname = path
        .segments
        .iter()
        .map(|seg| seg.identifier.clone())
        .collect::<Vec<_>>()
        .join("::");

    let mut scope = scope.to_owned();

    while !scope.is_empty() {
        let candidate = format!("{}::{}", scope.join("::"), pathname);

        if let Some(ty) = symbol_table.get(&SymbolName(candidate)) {
            match ty.to_owned() {
                Symbol::TypeAlias(sym) => path.to = ExprPathTarget::TypeAlias(Arc::downgrade(sym)),
                Symbol::Struct(sym) => path.to = ExprPathTarget::Struct(Arc::downgrade(sym)),
                Symbol::Enum(sym) => path.to = ExprPathTarget::Enum(Arc::downgrade(sym)),
                Symbol::Function(sym) => path.to = ExprPathTarget::Function(Arc::downgrade(sym)),
                Symbol::Variable(sym) => path.to = ExprPathTarget::Variable(Arc::downgrade(sym)),
                _ => {}
            }

            return;
        }

        scope.pop();
    }
}

fn resolve_type_path(scope: &Vec<String>, path: &mut TypePath, symbol_table: &SymbolTable) {
    let pathname = path
        .segments
        .iter()
        .map(|seg| seg.identifier.clone())
        .collect::<Vec<_>>()
        .join("::");

    let mut scope = scope.to_owned();

    while !scope.is_empty() {
        let candidate = format!("{}::{}", scope.join("::"), pathname);

        if let Some(ty) = symbol_table.get(&SymbolName(candidate)) {
            match ty.to_owned() {
                Symbol::TypeAlias(sym) => path.to = TypePathTarget::TypeAlias(Arc::downgrade(sym)),
                Symbol::Struct(sym) => path.to = TypePathTarget::Struct(Arc::downgrade(sym)),
                Symbol::Enum(sym) => path.to = TypePathTarget::Enum(Arc::downgrade(sym)),
                _ => {}
            }

            return;
        }

        scope.pop();
    }
}

pub fn resolve(module: &mut Module, _bugs: &DiagnosticCollector) {
    let symbol_table = build_symbol_table(module);
    let mut scope_vec = Vec::new();

    module.depth_first_iter_mut(&mut |order, node| {
        if let RefNodeMut::ItemModule(module) = node {
            match order {
                Order::Enter => {
                    scope_vec.push(module.name.to_string());
                }

                Order::Leave => {
                    scope_vec.pop();
                }
            }

            return;
        }

        if order != Order::Enter {
            return;
        }

        if let RefNodeMut::ItemPath(path) = node {
            resolve_item_path(&scope_vec, path, &symbol_table);
        } else if let RefNodeMut::TypePath(path) = node {
            resolve_type_path(&scope_vec, path, &symbol_table);
        } else if let RefNodeMut::ExprPath(path) = node {
            resolve_expr_path(&scope_vec, path, &symbol_table);
        }
    });
}
