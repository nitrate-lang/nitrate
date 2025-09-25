use crate::ResolveIssue;

use nitrate_diagnosis::{CompilerLog, intern_file_id};
use nitrate_parse::Parser;
use nitrate_parsetree::{
    Order, ParseTreeIterMut, RefNodeMut,
    kind::{Import, Module, Visibility},
    tag::intern_module_name,
};
use nitrate_tokenize::{Lexer, LexerError};

use std::collections::HashSet;
use std::path::{MAIN_SEPARATOR, PathBuf};

fn import_module(
    module_path: PathBuf,
    name: String,
    visibility: Option<Visibility>,
    log: &CompilerLog,
    visited: &mut HashSet<PathBuf>,
    depth: &mut Vec<PathBuf>,
) -> Option<Module> {
    let source_code = match std::fs::read_to_string(&module_path) {
        Ok(code) => code,
        Err(err) => {
            log.report(&ResolveIssue::ModuleNotFound((module_path, err)));
            return None;
        }
    };

    let file_id = intern_file_id(&module_path.to_string_lossy());
    let lexer = match Lexer::new(source_code.as_bytes(), file_id) {
        Ok(l) => l,

        Err(LexerError::SourceTooBig) => {
            log.report(&ResolveIssue::ImportSourceCodeSizeLimitExceeded(
                module_path,
            ));
            return None;
        }
    };

    let mut module = Module {
        items: Parser::new(lexer, log).parse_source(),
        name: intern_module_name(name),

        visibility,
        attributes: None,
    };

    resolve_imports_inner(&mut module, log, visited, depth);

    Some(module)
}

fn resolve_import(
    import: &mut Import,
    log: &CompilerLog,
    visited: &mut HashSet<PathBuf>,
    depth: &mut Vec<PathBuf>,
) {
    if import.content.is_some() {
        return;
    }

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

    const MAX_IMPORT_DEPTH: usize = 256;
    if depth.len() >= MAX_IMPORT_DEPTH {
        // This prevents stack overflow and other bugs.
        // For example, prevents crashes when cyclic importing symlinked files.
        log.report(&ResolveIssue::ImportDepthLimitExceeded(module_path));
        return;
    }

    depth.push(module_path.clone());
    if visited.contains(&module_path) {
        log.report(&ResolveIssue::CircularImport {
            path: module_path,
            depth: depth.clone(),
        });
        depth.pop();
        return;
    } else {
        visited.insert(module_path.clone());
    }

    import.content = import_module(
        module_path,
        parts[parts.len() - 1].segment.to_owned(),
        import.visibility.to_owned(),
        log,
        visited,
        depth,
    );

    depth.pop();
}

fn resolve_imports_inner(
    module: &mut Module,
    log: &CompilerLog,
    visitied: &mut HashSet<PathBuf>,
    depth: &mut Vec<PathBuf>,
) {
    module.depth_first_iter_mut(&mut |order, node| {
        if order != Order::Enter {
            return;
        }

        if let RefNodeMut::ItemImport(import) = node {
            resolve_import(import, log, visitied, depth);
        }
    });
}

pub fn resolve_imports(module: &mut Module, log: &CompilerLog) {
    let mut visited = HashSet::new();
    let mut depth = Vec::new();

    resolve_imports_inner(module, log, &mut visited, &mut depth);
}
