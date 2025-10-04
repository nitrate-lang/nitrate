use crate::ResolveIssue;

use nitrate_diagnosis::{CompilerLog, intern_file_id};
use nitrate_parse::Parser;
use nitrate_parsetree::{
    Order, ParseTreeIterMut, RefNodeMut,
    kind::{Import, Item, Module, Visibility},
    tag::{ImportNameId, PackageNameId, intern_module_name},
};
use nitrate_tokenize::{Lexer, LexerError};

use std::collections::HashSet;

pub struct ImportContext {
    pub package_name: Option<PackageNameId>,
    pub source_code_filepath: std::path::PathBuf,
}

fn is_visible(vis: Option<Visibility>, is_same_package: bool) -> bool {
    match vis.unwrap_or(Visibility::Private) {
        Visibility::Public => true,
        Visibility::Protected => is_same_package,
        Visibility::Private => false,
    }
}

fn visibility_filter(item: Item, is_same_package: bool) -> Option<Item> {
    match item {
        Item::SyntaxError(_) => None,

        Item::Impl(i) => Some(Item::Impl(i)),

        Item::Module(mut node) => {
            if !is_visible(node.visibility, is_same_package) {
                return None;
            }

            node.items = node
                .items
                .into_iter()
                .filter_map(|item| visibility_filter(item, is_same_package))
                .collect();

            Some(Item::Module(node))
        }

        Item::Import(node) => {
            if !is_visible(node.visibility, is_same_package) {
                return None;
            }

            Some(Item::Import(node))
        }

        Item::TypeAlias(node) => {
            if !is_visible(node.read().unwrap().visibility, is_same_package) {
                return None;
            }

            Some(Item::TypeAlias(node))
        }

        Item::Struct(node) => {
            let mut lock = node.write().unwrap();

            if !is_visible(lock.visibility, is_same_package) {
                return None;
            }

            lock.methods
                .retain(|method| is_visible(method.read().unwrap().visibility, is_same_package));

            drop(lock);
            Some(Item::Struct(node))
        }

        Item::Enum(node) => {
            if !is_visible(node.read().unwrap().visibility, is_same_package) {
                return None;
            }

            Some(Item::Enum(node))
        }

        Item::Trait(node) => {
            if !is_visible(node.read().unwrap().visibility, is_same_package) {
                return None;
            }

            Some(Item::Trait(node))
        }

        Item::Function(node) => {
            if !is_visible(node.read().unwrap().visibility, is_same_package) {
                return None;
            }

            Some(Item::Function(node))
        }

        Item::Variable(node) => {
            if !is_visible(node.read().unwrap().visibility, is_same_package) {
                return None;
            }

            Some(Item::Variable(node))
        }
    }
}

fn load_source_file(
    path: &std::path::Path,
    import_name: String,
    is_same_package: bool,
    log: &CompilerLog,
) -> Option<Module> {
    let source_code = match std::fs::read_to_string(&path) {
        Ok(code) => code,
        Err(err) => {
            log.report(&ResolveIssue::ImportNotFound((import_name, err)));
            return None;
        }
    };

    let lexer = match Lexer::new(
        source_code.as_bytes(),
        intern_file_id(&path.to_string_lossy()),
    ) {
        Ok(lex) => lex,
        Err(LexerError::SourceTooBig) => {
            log.report(&ResolveIssue::ImportSourceCodeSizeLimitExceeded(
                path.to_path_buf(),
            ));

            return None;
        }
    };

    let all_items = Parser::new(lexer, log).parse_source();
    drop(source_code);

    let items = all_items
        .into_iter()
        .filter_map(|item| visibility_filter(item, is_same_package))
        .collect();

    let module = Module {
        visibility: None,
        attributes: None,
        name: intern_module_name(import_name),
        items,
    };

    Some(module)
}

fn decide_what_to_import(
    ctx: &ImportContext,
    import_name: &str,
    log: &CompilerLog,
) -> Option<ImportContext> {
    let folder = ctx.source_code_filepath.parent()?;

    let sibling_path = folder.join(format!("{}.nit", import_name));
    if sibling_path.exists() {
        return Some(ImportContext {
            package_name: ctx.package_name.clone(),
            source_code_filepath: sibling_path,
        });
    }

    let subfolder_path = folder.join(import_name).join("mod.nit");
    if subfolder_path.exists() {
        return Some(ImportContext {
            package_name: ctx.package_name.clone(),
            source_code_filepath: subfolder_path,
        });
    }

    // TODO: Handle package lookup: search in packages PATH.
    // For now, just report not found.

    log.report(&ResolveIssue::ImportNotFound((
        import_name.to_string(),
        std::io::Error::from(std::io::ErrorKind::NotFound),
    )));

    None
}

fn resolve_import(
    ctx: &ImportContext,
    import: &mut Import,
    log: &CompilerLog,
    visited: &mut HashSet<ImportNameId>,
    depth: &mut Vec<ImportNameId>,
) {
    let import_name_id = import.import_name.clone();
    let import_name = import_name_id.to_string();

    const MAX_IMPORT_DEPTH: usize = 256;
    if depth.len() >= MAX_IMPORT_DEPTH {
        // This prevents stack overflow and other bugs.
        // For example, prevents crashes when cyclic importing symlinked files.
        log.report(&ResolveIssue::ImportDepthLimitExceeded(import_name));
        return;
    } else {
        depth.push(import_name_id.clone());
    }

    if visited.contains(&import_name_id) {
        log.report(&ResolveIssue::CircularImport {
            path: import_name_id,
            depth: depth.clone(),
        });
        depth.pop();
        return;
    } else {
        visited.insert(import_name_id.clone());
    }

    if let Some(what) = decide_what_to_import(ctx, &import_name, log) {
        let inside = match (&ctx.package_name, &what.package_name) {
            (Some(a), Some(b)) => a == b,
            _ => false,
        };

        let content = load_source_file(&what.source_code_filepath, import_name, inside, log);

        if let Some(mut module) = content {
            resolve_imports_guarded(&what, &mut module, log, visited, depth);
            module.visibility = import.visibility;

            import.module = Some(module);
        }
    }

    visited.remove(&import_name_id);
    depth.pop();
}

fn resolve_imports_guarded(
    ctx: &ImportContext,
    module: &mut Module,
    log: &CompilerLog,
    visited: &mut HashSet<ImportNameId>,
    depth: &mut Vec<ImportNameId>,
) {
    module.depth_first_iter_mut(&mut |order, node| {
        if order == Order::Leave {
            if let RefNodeMut::ItemImport(import) = node {
                resolve_import(ctx, import, log, visited, depth);
            }
        }
    });
}

pub fn resolve_imports(ctx: &ImportContext, module: &mut Module, log: &CompilerLog) {
    let mut visited = HashSet::new();
    let mut depth = Vec::new();

    resolve_imports_guarded(ctx, module, log, &mut visited, &mut depth);
}
