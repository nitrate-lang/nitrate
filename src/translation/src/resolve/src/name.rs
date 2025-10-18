use crate::{ResolveIssue, Symbol, SymbolTable, build_symbol_table};

use nitrate_diagnosis::CompilerLog;
use nitrate_parsetree::{
    Order, ParseTreeIterMut, RefNodeMut,
    ast::{ExprPath, ExprPathTarget, Module, TypePath, TypePathTarget},
};

use std::sync::Arc;

fn resolve_expr_path_lookup(
    path: &mut ExprPath,
    name: String,
    symbol_table: &SymbolTable,
    log: &CompilerLog,
) -> bool {
    let Some(symbols) = symbol_table.get(&name) else {
        return false;
    };

    if symbols.len() > 1 {
        let bug = ResolveIssue::ExprPathAmbiguous(path.clone(), symbols.clone());
        log.report(&bug);
        return false;
    }

    match symbols.first().expect("symbol table entry is empty") {
        Symbol::TypeAlias(sym) => {
            path.resolved = Some(ExprPathTarget::TypeAlias(Arc::downgrade(&sym)))
        }
        Symbol::Struct(sym) => path.resolved = Some(ExprPathTarget::Struct(Arc::downgrade(&sym))),
        Symbol::Enum(sym) => path.resolved = Some(ExprPathTarget::Enum(Arc::downgrade(&sym))),
        Symbol::Trait(sym) => path.resolved = Some(ExprPathTarget::Trait(Arc::downgrade(&sym))),
        Symbol::Function(sym) => {
            path.resolved = Some(ExprPathTarget::Function(Arc::downgrade(&sym)))
        }
        Symbol::Variable(sym) => {
            path.resolved = Some(ExprPathTarget::Variable(Arc::downgrade(&sym)))
        }
    }

    true
}

fn resolve_expr_path(
    scope: &[String],
    path: &mut ExprPath,
    symbol_table: &SymbolTable,
    log: &CompilerLog,
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
        return resolve_expr_path_lookup(path, pathname, symbol_table, log);
    }

    for i in (0..scope.len()).rev() {
        let current_scope = &scope[0..=i];
        let name = format!("{}::{}", current_scope.join("::"), pathname);

        if resolve_expr_path_lookup(path, name, symbol_table, log) {
            return true;
        }
    }

    let bug = ResolveIssue::ExprPathUnresolved(path.clone());
    log.report(&bug);

    false
}

fn resolve_type_path_lookup(
    path: &mut TypePath,
    name: String,
    symbol_table: &SymbolTable,
    log: &CompilerLog,
) -> bool {
    let Some(symbols) = symbol_table.get(&name) else {
        return false;
    };

    if symbols.len() > 1 {
        let bug = ResolveIssue::TypePathAmbiguous(path.clone(), symbols.clone());
        log.report(&bug);
        return false;
    }

    match symbols.first().expect("symbol table entry is empty") {
        Symbol::TypeAlias(sym) => {
            path.resolved = Some(TypePathTarget::TypeAlias(Arc::downgrade(&sym)));
            return true;
        }

        Symbol::Struct(sym) => {
            path.resolved = Some(TypePathTarget::Struct(Arc::downgrade(&sym)));
            return true;
        }

        Symbol::Enum(sym) => {
            path.resolved = Some(TypePathTarget::Enum(Arc::downgrade(&sym)));
            return true;
        }

        _ => {}
    }

    let bug = ResolveIssue::TypePathUnresolved(path.clone());
    log.report(&bug);

    false
}

fn resolve_type_path(
    scope: &[String],
    path: &mut TypePath,
    symbol_table: &SymbolTable,
    log: &CompilerLog,
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
        return resolve_type_path_lookup(path, pathname, symbol_table, log);
    }

    for i in (0..scope.len()).rev() {
        let current_scope = &scope[0..=i];
        let name = format!("{}::{}", current_scope.join("::"), pathname);

        if resolve_type_path_lookup(path, name, symbol_table, log) {
            return true;
        }
    }

    let bug = ResolveIssue::TypePathUnresolved(path.clone());
    log.report(&bug);

    false
}

pub fn resolve_names(module: &mut Module, log: &CompilerLog) {
    let symbol_table = build_symbol_table(module);
    let mut scope_vec = Vec::new();

    module.depth_first_iter_mut(&mut |order, node| {
        if let RefNodeMut::ItemModule(module) = node {
            match order {
                Order::Enter => {
                    if let Some(name) = &module.name {
                        scope_vec.push(name.to_string());
                    } else {
                        scope_vec.push("".to_string());
                    }
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

        if let RefNodeMut::TypePath(path) = node {
            resolve_type_path(&scope_vec, path, &symbol_table, log);
        } else if let RefNodeMut::ExprPath(path) = node {
            resolve_expr_path(&scope_vec, path, &symbol_table, log);
        }
    });
}
