use nitrate_diagnosis::DiagnosticCollector;

use nitrate_parsetree::{
    Order, ParseTreeIterMut, RefNodeMut,
    kind::{ExprPath, ExprPathTarget, ItemPath, Module, TypePath},
};

use crate::{SymbolTable, build_symbol_table};

fn resolve_item_path(path: &mut ItemPath, scope: &Vec<String>, symbol_table: &SymbolTable) {
    // TODO: Resolve the path
}

fn resolve_expr_path(path: &mut ExprPath, scope: &Vec<String>, symbol_table: &SymbolTable) {
    if matches!(path.to, ExprPathTarget::Unresolved) {
        return;
    }

    // TODO: Resolve the expr path
}

fn resolve_type_path(path: &mut TypePath, scope: &Vec<String>, symbol_table: &SymbolTable) {
    // TODO: Resolve the type path
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
            resolve_item_path(path, &scope_vec, &symbol_table);
        } else if let RefNodeMut::TypePath(path) = node {
            resolve_type_path(path, &scope_vec, &symbol_table);
        } else if let RefNodeMut::ExprPath(path) = node {
            resolve_expr_path(path, &scope_vec, &symbol_table);
        }
    });
}
