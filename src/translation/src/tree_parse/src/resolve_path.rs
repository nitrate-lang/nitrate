use crate::{diagnosis::ResolveIssue, symbol_table::discover_symbols};
use nitrate_diagnosis::CompilerLog;
use nitrate_nstring::NString;
use nitrate_tree::{
    Order, ParseTreeIterMut, RefNodeMut,
    ast::{ExprPath, Module, SymbolKind, TypePath},
};
use std::collections::HashMap;

fn resolve_expr_path(
    scope: &[String],
    path: &mut ExprPath,
    symbol_map: &HashMap<NString, SymbolKind>,
    log: &CompilerLog,
) -> bool {
    let pathname = NString::from(
        path.segments
            .iter()
            .map(|seg| seg.name.clone())
            .collect::<Vec<_>>()
            .join("::"),
    );

    let is_root_path = path
        .segments
        .first()
        .map(|seg| seg.name.is_empty())
        .unwrap_or(false);

    if is_root_path {
        if symbol_map.contains_key(&pathname) {
            path.resolved_path = Some(pathname);
            return true;
        }

        return false;
    }

    for i in (0..scope.len()).rev() {
        let current_scope = &scope[0..=i];
        let name = format!("{}::{}", current_scope.join("::"), pathname).into();

        if symbol_map.contains_key(&name) {
            path.resolved_path = Some(name);
            return true;
        }
    }

    let bug = ResolveIssue::ExprPathUnresolved(path.clone());
    log.report(&bug);

    false
}

fn resolve_type_path(
    scope: &[String],
    path: &mut TypePath,
    symbol_map: &HashMap<NString, SymbolKind>,
    log: &CompilerLog,
) -> bool {
    let pathname = NString::from(
        path.segments
            .iter()
            .map(|seg| seg.name.clone())
            .collect::<Vec<_>>()
            .join("::"),
    );

    let is_root_path = path
        .segments
        .first()
        .map(|seg| seg.name.is_empty())
        .unwrap_or(false);

    if is_root_path {
        if symbol_map.contains_key(&pathname) {
            path.resolved_path = Some(pathname);
            return true;
        }

        return false;
    }

    for i in (0..scope.len()).rev() {
        let current_scope = &scope[0..=i];
        let name = format!("{}::{}", current_scope.join("::"), pathname).into();

        if symbol_map.contains_key(&name) {
            path.resolved_path = Some(name);
            return true;
        }
    }

    let bug = ResolveIssue::TypePathUnresolved(path.clone());
    log.report(&bug);

    false
}

pub fn resolve_paths(module: &mut Module, log: &CompilerLog) {
    let symbol_map = discover_symbols(module);
    let mut scope_vec = Vec::new();

    module.depth_first_iter_mut(&mut |order, node| {
        if order == Order::Enter {
            if let RefNodeMut::TypePath(path) = node {
                resolve_type_path(&scope_vec, path, &symbol_map, log);
                return;
            } else if let RefNodeMut::ExprPath(path) = node {
                resolve_expr_path(&scope_vec, path, &symbol_map, log);
                return;
            }
        }

        if let RefNodeMut::ItemModule(module) = node {
            match order {
                Order::Enter => {
                    scope_vec.push(module.name.to_string());
                }

                Order::Leave => {
                    scope_vec.pop();
                }
            }
        } else if let RefNodeMut::ItemFunction(function) = node {
            match order {
                Order::Enter => {
                    scope_vec.push(function.name.to_string().into());
                }

                Order::Leave => {
                    scope_vec.pop();
                }
            }
        }
    });
}
