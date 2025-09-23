use nitrate_diagnosis::DiagnosticCollector;
use nitrate_parsetree::{
    Order, ParseTreeIterMut, RefNodeMut,
    kind::{Import, Module},
};

fn resolve_import(_import: &mut Import, _bugs: &DiagnosticCollector) {
    // TODO: Implement import resolution
}

pub fn resolve_imports(module: &mut Module, bugs: &DiagnosticCollector) {
    module.depth_first_iter_mut(&mut |order, node| {
        if order != Order::Enter {
            return;
        }

        if let RefNodeMut::ItemImport(import) = node {
            resolve_import(import, bugs);
        }
    });
}
