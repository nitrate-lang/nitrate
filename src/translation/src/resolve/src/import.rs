use nitrate_diagnosis::CompilerLog;
use nitrate_parsetree::{
    Order, ParseTreeIterMut, RefNodeMut,
    kind::{Import, Module},
};

use std::{
    fmt::format,
    path::{self, MAIN_SEPARATOR},
};

fn resolve_import(import: &mut Import, _bugs: &CompilerLog) {
    // take the last segment of the import path as the imported module name
    // take the first n-1 segments as the parent module path

    let parts = &import.path.segments;

    if parts.is_empty() {
        return;
    }

    let module_dir = &parts[..parts.len() - 1]
        .iter()
        .map(|s| s.segment.to_owned())
        .collect::<Vec<_>>()
        .join(MAIN_SEPARATOR.to_string().as_str());

    let module_filename = format!("{}.nit", parts[parts.len() - 1].segment);

    let module_path = path::PathBuf::from(module_dir).join(module_filename);

    println!(
        "Resolving import: {} -> {}",
        import
            .path
            .segments
            .iter()
            .map(|s| s.segment.as_str())
            .collect::<Vec<_>>()
            .join("::"),
        module_path.display()
    );

    // TODO: Implement import resolution
}

pub fn resolve_imports(module: &mut Module, bugs: &CompilerLog) {
    module.depth_first_iter_mut(&mut |order, node| {
        if order != Order::Enter {
            return;
        }

        if let RefNodeMut::ItemImport(import) = node {
            resolve_import(import, bugs);
        }
    });
}
