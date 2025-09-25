use nitrate_diagnosis::{CompilerLog, intern_file_id};
use nitrate_parsetree::{
    Order, ParseTreeIterMut, RefNodeMut,
    kind::{Import, Module},
};
use nitrate_tokenize::Lexer;

use std::path::{MAIN_SEPARATOR, PathBuf};

use crate::ResolveIssue;

fn resolve_import(import: &mut Import, log: &CompilerLog) {
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

    let module_path = PathBuf::from(module_dir).join(module_filename);

    let source_code = match std::fs::read_to_string(&module_path) {
        Ok(code) => code,

        Err(err) => {
            log.report(&ResolveIssue::ModuleNotFound((module_path, err)));
            return;
        }
    };

    let lexer = Lexer::new(
        source_code.as_bytes(),
        intern_file_id(&module_path.to_string_lossy()),
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
