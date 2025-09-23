use crate::{ResolveIssue, Symbol, SymbolTable, build_symbol_table};

use nitrate_diagnosis::DiagnosticCollector;
use nitrate_parsetree::{
    Order, ParseTreeIterMut, RefNodeMut,
    kind::{ExprPath, ExprPathTarget, ItemPath, Module, TypePath, TypePathTarget},
};

use std::sync::Arc;

fn resolve_item_path(
    _scope: &Vec<String>,
    _path: &mut ItemPath,
    _symbol_table: &SymbolTable,
    _bugs: &DiagnosticCollector,
) {
    // https://doc.rust-lang.org/reference/paths.html#r-paths.simple.intro

    // TODO: Resolve imports here?
}

fn resolve_expr_path_lookup(
    path: &mut ExprPath,
    candidate: String,
    symbol_table: &SymbolTable,
    bugs: &DiagnosticCollector,
) -> bool {
    let Some(symbols) = symbol_table.get(&candidate) else {
        return false;
    };

    if symbols.len() > 1 {
        let bug = ResolveIssue::ExprPathAmbiguous(path.clone(), candidate, symbols.clone());
        bugs.push(&bug);
        return false;
    }

    match symbols.first().expect("symbol table entry is empty") {
        Symbol::TypeAlias(sym) => path.to = ExprPathTarget::TypeAlias(Arc::downgrade(&sym)),
        Symbol::Struct(sym) => path.to = ExprPathTarget::Struct(Arc::downgrade(&sym)),
        Symbol::Enum(sym) => path.to = ExprPathTarget::Enum(Arc::downgrade(&sym)),
        Symbol::Trait(sym) => path.to = ExprPathTarget::Trait(Arc::downgrade(&sym)),
        Symbol::Function(sym) => path.to = ExprPathTarget::Function(Arc::downgrade(&sym)),
        Symbol::Variable(sym) => path.to = ExprPathTarget::Variable(Arc::downgrade(&sym)),
    }

    true
}

fn resolve_expr_path(
    scope: &[String],
    path: &mut ExprPath,
    symbol_table: &SymbolTable,
    bugs: &DiagnosticCollector,
) -> bool {
    let pathname = path
        .segments
        .iter()
        .map(|seg| seg.name.clone())
        .collect::<Vec<_>>()
        .join("::");

    let is_root_path = path
        .segments
        .first()
        .map(|seg| seg.name.is_empty())
        .unwrap_or(false);

    if is_root_path {
        return resolve_expr_path_lookup(path, pathname, symbol_table, bugs);
    }

    for i in (0..scope.len()).rev() {
        let current_scope = &scope[0..=i];
        let candidate = format!("{}::{}", current_scope.join("::"), pathname);

        if resolve_expr_path_lookup(path, candidate, symbol_table, bugs) {
            return true;
        }
    }

    let bug = ResolveIssue::ExprPathUnresolved(path.clone(), pathname);
    bugs.push(&bug);

    false
}

fn resolve_type_path_lookup(
    path: &mut TypePath,
    candidate: String,
    symbol_table: &SymbolTable,
    bugs: &DiagnosticCollector,
) -> bool {
    let Some(symbols) = symbol_table.get(&candidate) else {
        return false;
    };

    if symbols.len() > 1 {
        let bug = ResolveIssue::TypePathAmbiguous(path.clone(), candidate, symbols.clone());
        bugs.push(&bug);
        return false;
    }

    match symbols.first().expect("symbol table entry is empty") {
        Symbol::TypeAlias(sym) => {
            path.to = TypePathTarget::TypeAlias(Arc::downgrade(&sym));
            return true;
        }

        Symbol::Struct(sym) => {
            path.to = TypePathTarget::Struct(Arc::downgrade(&sym));
            return true;
        }

        Symbol::Enum(sym) => {
            path.to = TypePathTarget::Enum(Arc::downgrade(&sym));
            return true;
        }

        _ => {}
    }

    let bug = ResolveIssue::TypePathUnresolved(path.clone(), candidate);
    bugs.push(&bug);

    false
}

fn resolve_type_path(
    scope: &[String],
    path: &mut TypePath,
    symbol_table: &SymbolTable,
    bugs: &DiagnosticCollector,
) -> bool {
    let pathname = path
        .segments
        .iter()
        .map(|seg| seg.name.clone())
        .collect::<Vec<_>>()
        .join("::");

    let is_root_path = path
        .segments
        .first()
        .map(|seg| seg.name.is_empty())
        .unwrap_or(false);

    if is_root_path {
        return resolve_type_path_lookup(path, pathname, symbol_table, bugs);
    }

    for i in (0..scope.len()).rev() {
        let current_scope = &scope[0..=i];
        let candidate = format!("{}::{}", current_scope.join("::"), pathname);

        if resolve_type_path_lookup(path, candidate, symbol_table, bugs) {
            return true;
        }
    }

    let bug = ResolveIssue::TypePathUnresolved(path.clone(), pathname);
    bugs.push(&bug);

    false
}

pub fn resolve(module: &mut Module, bugs: &DiagnosticCollector) {
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
            resolve_item_path(&scope_vec, path, &symbol_table, bugs);
        } else if let RefNodeMut::TypePath(path) = node {
            resolve_type_path(&scope_vec, path, &symbol_table, bugs);
        } else if let RefNodeMut::ExprPath(path) = node {
            resolve_expr_path(&scope_vec, path, &symbol_table, bugs);
        }
    });
}
