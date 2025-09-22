use nitrate_diagnosis::DiagnosticCollector;

use nitrate_parsetree::{Order, ParseTreeIterMut, RefNodeMut, kind::Module};

use crate::build_symbol_table;

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

        if let RefNodeMut::ExprPath(path) = node {
            let _ = &symbol_table;
            let _ = &scope_vec;
            let _ = &path;

            // TODO: Resolve the path using the symbol table.
        }
    });
}
