use crate::ResolveIssue;

use nitrate_diagnosis::{CompilerLog, intern_file_id};
use nitrate_parse::Parser;
use nitrate_parsetree::{
    Order, ParseTreeIterMut, RefNodeMut,
    kind::{Import, Item, Module, Visibility},
    tag::intern_module_name,
};
use nitrate_tokenize::{Lexer, LexerError};

use std::collections::HashSet;
use std::path::{MAIN_SEPARATOR, PathBuf};

fn import_module_file(
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

    let all_items = Parser::new(lexer, log).parse_source();

    let visible_items = all_items
        .into_iter()
        .filter(|item| match item {
            Item::SyntaxError(_) | Item::ItemPath(_) => false,
            Item::Impl(_) => true,

            Item::Module(i) => i.visibility == Some(Visibility::Public),
            Item::Import(i) => i.visibility == Some(Visibility::Public),

            Item::TypeAlias(i) => i.read().unwrap().visibility == Some(Visibility::Public),
            Item::Struct(i) => i.read().unwrap().visibility == Some(Visibility::Public),
            Item::Enum(i) => i.read().unwrap().visibility == Some(Visibility::Public),
            Item::Trait(i) => i.read().unwrap().visibility == Some(Visibility::Public),
            Item::Function(i) => i.read().unwrap().visibility == Some(Visibility::Public),
            Item::Variable(i) => i.read().unwrap().visibility == Some(Visibility::Public),
        })
        .collect::<Vec<_>>();

    let mut module = Module {
        visibility,
        attributes: None,
        name: intern_module_name(name),
        items: visible_items,
    };

    resolve_imports_guarded(&mut module, log, visited, depth);

    Some(module)
}

fn resolve_import_item(
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

    import.content = import_module_file(
        module_path,
        parts[parts.len() - 1].segment.to_owned(),
        import.visibility.to_owned(),
        log,
        visited,
        depth,
    );

    depth.pop();
}

fn resolve_imports_guarded(
    module: &mut Module,
    log: &CompilerLog,
    visitied: &mut HashSet<PathBuf>,
    depth: &mut Vec<PathBuf>,
) {
    module.depth_first_iter_mut(&mut |order, node| {
        if order == Order::Leave {
            if let RefNodeMut::ItemImport(import) = node {
                resolve_import_item(import, log, visitied, depth);
            }
        }
    });
}

pub fn resolve_imports(module: &mut Module, log: &CompilerLog) {
    let mut visited = HashSet::new();
    let mut depth = Vec::new();

    resolve_imports_guarded(module, log, &mut visited, &mut depth);
}
