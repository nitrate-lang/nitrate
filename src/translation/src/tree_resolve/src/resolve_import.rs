use crate::diagnosis::ResolveIssue;
use nitrate_diagnosis::{CompilerLog, intern_file_id};
use nitrate_nstring::NString;
use nitrate_token_lexer::{Lexer, LexerError};
use nitrate_tree::{
    Order, ParseTreeIterMut, RefNodeMut,
    ast::{Import, Item, Module, PathPrefix, Visibility},
};
use nitrate_tree_parse::Parser;
use std::{collections::HashSet, sync::Arc};
pub type SourceFilePath = std::path::PathBuf;
pub type FolderPath = std::path::PathBuf;

#[derive(Debug)]
pub struct ImportContext {
    pub package_name: NString,
    pub source_filepath: SourceFilePath,
    pub package_search_paths: Arc<Vec<FolderPath>>,
    /// The root directory of the current package (for `crate::` resolution).
    /// This is typically the directory containing `no3.toml`.
    pub package_root: Option<SourceFilePath>,
}

impl ImportContext {
    pub fn new(package_name: NString, source_filepath: SourceFilePath) -> Self {
        Self {
            package_name,
            source_filepath,
            package_search_paths: Arc::new(Vec::new()),
            package_root: None,
        }
    }

    pub fn with_package_search_paths(mut self, paths: Vec<FolderPath>) -> Self {
        self.package_search_paths = Arc::new(paths);
        self
    }

    pub fn with_package_root(mut self, root: SourceFilePath) -> Self {
        self.package_root = Some(root);
        self
    }

    fn find_package(&self, package_name: &str) -> Option<SourceFilePath> {
        for folder in &*self.package_search_paths {
            let candidate = folder.join(package_name).join("src").join("entry.nit");
            if candidate.exists() {
                return Some(candidate);
            }
        }

        None
    }

    /// Resolve a `crate::` path to the given relative module path within this package.
    fn resolve_crate_path(&self, relative_path: &str) -> Option<SourceFilePath> {
        let root = self.package_root.as_ref()?;
        let mut candidate = root.clone();
        candidate.pop(); // pop entry.nit
        candidate.pop(); // pop src
        candidate.push(format!("src/{}.nit", relative_path));
        if candidate.exists() {
            return Some(candidate);
        }
        // Try mod.nit variant
        candidate.pop();
        candidate.push(format!("{}/mod.nit", relative_path));
        if candidate.exists() {
            return Some(candidate);
        }
        None
    }

    /// Resolve a `self::` path (relative to current module's directory).
    fn resolve_self_path(&self, relative_path: &str) -> Option<SourceFilePath> {
        let parent = self.source_filepath.parent()?;
        let candidate = parent.join(format!("{}.nit", relative_path));
        if candidate.exists() {
            return Some(candidate);
        }
        let candidate = parent.join(relative_path).join("mod.nit");
        if candidate.exists() {
            return Some(candidate);
        }
        None
    }

    /// Resolve a `super::` path by going up N levels from the current file.
    fn resolve_super_path(&self, super_count: usize, relative_path: &str) -> Option<SourceFilePath> {
        let mut current = self.source_filepath.parent()?.to_path_buf();
        for _ in 0..super_count {
            current = current.parent()?.to_path_buf();
        }
        let candidate = current.join(format!("{}.nit", relative_path));
        if candidate.exists() {
            return Some(candidate);
        }
        let candidate = current.join(relative_path).join("mod.nit");
        if candidate.exists() {
            return Some(candidate);
        }
        None
    }
}

fn visibility_filter(item: Item, is_same_package: bool) -> Option<Item> {
    fn is_visible(vis: Option<Visibility>, is_same_package: bool) -> bool {
        match vis.unwrap_or(Visibility::Private) {
            Visibility::Public => true,
            Visibility::Protected => is_same_package,
            Visibility::Private => false,
        }
    }

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
            if !is_visible(node.visibility, is_same_package) {
                return None;
            }

            Some(Item::TypeAlias(node))
        }

        Item::Struct(node) => {
            if !is_visible(node.visibility, is_same_package) {
                return None;
            }

            Some(Item::Struct(node))
        }

        Item::Enum(node) => {
            if !is_visible(node.visibility, is_same_package) {
                return None;
            }

            Some(Item::Enum(node))
        }

        Item::Trait(node) => {
            if !is_visible(node.visibility, is_same_package) {
                return None;
            }

            Some(Item::Trait(node))
        }

        Item::Function(node) => {
            if !is_visible(node.visibility, is_same_package) {
                return None;
            }

            Some(Item::Function(node))
        }

        Item::Variable(node) => {
            if !is_visible(node.visibility, is_same_package) {
                return None;
            }

            Some(Item::Variable(node))
        }
    }
}

fn load_source_file(
    path: &std::path::Path,
    source_from_package_name: NString,
    is_same_package: bool,
    log: &CompilerLog,
) -> Option<Module> {
    let source_code = match std::fs::read_to_string(path) {
        Ok(code) => code,
        Err(err) => {
            log.report(&ResolveIssue::ImportNotFound((source_from_package_name.clone(), err)));
            return None;
        }
    };

    let lexer = match Lexer::new(source_code.as_bytes(), intern_file_id(&path.to_string_lossy())) {
        Ok(lex) => lex,
        Err(LexerError::SourceTooBig) => {
            log.report(&ResolveIssue::ImportSourceCodeSizeLimitExceeded(path.to_path_buf()));

            return None;
        }
    };

    let mut module = Parser::new(lexer, log).parse_source(source_from_package_name.clone());
    module.name = source_from_package_name;

    module.items = module
        .items
        .into_iter()
        .filter_map(|item| visibility_filter(item, is_same_package))
        .collect();

    Some(module)
}

fn decide_what_to_import(ctx: &ImportContext, import_name: NString, log: &CompilerLog) -> Option<ImportContext> {
    let folder = ctx.source_filepath.parent()?;

    let source_filepath = folder.join(format!("{}.nit", import_name));
    if source_filepath.exists() {
        return Some(ImportContext {
            package_name: ctx.package_name.clone(),
            source_filepath,
            package_search_paths: ctx.package_search_paths.clone(),
            package_root: ctx.package_root.clone(),
        });
    }

    let source_filepath = folder.join(import_name.to_string()).join("mod.nit");
    if source_filepath.exists() {
        return Some(ImportContext {
            package_name: ctx.package_name.clone(),
            source_filepath,
            package_search_paths: ctx.package_search_paths.clone(),
            package_root: ctx.package_root.clone(),
        });
    }

    if let Some(source_filepath) = ctx.find_package(&import_name)
        && source_filepath.exists()
    {
        return Some(ImportContext {
            package_name: import_name,
            source_filepath,
            package_search_paths: ctx.package_search_paths.clone(),
            package_root: ctx.package_root.clone(),
        });
    }

    log.report(&ResolveIssue::ImportNotFound((
        import_name.clone(),
        std::io::Error::from(std::io::ErrorKind::NotFound),
    )));

    None
}

fn resolve_import(
    ctx: &ImportContext,
    import: &mut Import,
    log: &CompilerLog,
    visited: &mut HashSet<NString>,
    depth: &mut Vec<NString>,
) {
    let import_path = import.use_tree.path();
    let first_seg = match import_path.segments.first() {
        Some(seg) => seg,
        None => return,
    };

    // Determine the import name and whether we have a path prefix
    let import_name: NString = first_seg.segment.clone().into();

    // Build the item name from remaining segments (after the module name)
    let item_name: Option<String> = if import_path.segments.len() > 1 {
        Some(
            import_path.segments[1..]
                .iter()
                .map(|s| s.segment.clone())
                .collect::<Vec<_>>()
                .join("::"),
        )
    } else {
        None
    };

    const MAX_IMPORT_DEPTH: usize = 256;
    if depth.len() >= MAX_IMPORT_DEPTH {
        log.report(&ResolveIssue::ImportDepthLimitExceeded(import_name.clone()));
        return;
    } else {
        depth.push(import_name.clone());
    }

    if visited.contains(&import_name) {
        log.report(&ResolveIssue::CircularImport {
            path: import_name.clone(),
            depth: depth.clone(),
        });
        depth.pop();
        return;
    } else {
        visited.insert(import_name.clone());
    }

    // Resolve based on the path prefix
    let what: Option<ImportContext> = match first_seg.prefix {
        Some(PathPrefix::Crate) => {
            // `use crate::path` or bare `use crate;`
            if let Some(item_name) = &item_name {
                ctx.resolve_crate_path(item_name).map(|path| ImportContext {
                    package_name: ctx.package_name.clone(),
                    source_filepath: path,
                    package_search_paths: ctx.package_search_paths.clone(),
                    package_root: ctx.package_root.clone(),
                })
            } else {
                // Bare `use crate;` — resolve to the package root (entry.nit)
                ctx.package_root.clone().map(|root| ImportContext {
                    package_name: ctx.package_name.clone(),
                    source_filepath: root,
                    package_search_paths: ctx.package_search_paths.clone(),
                    package_root: ctx.package_root.clone(),
                })
            }
        }

        Some(PathPrefix::Super) => {
            // Count how many `super` segments we have (consecutive from the start)
            let super_count = import_path.segments.iter().take_while(|s| s.segment == "super").count();

            // Remaining segments after the super chain form the relative path
            let relative_path: String = import_path.segments[super_count..]
                .iter()
                .map(|s| s.segment.clone())
                .collect::<Vec<_>>()
                .join("::");

            if relative_path.is_empty() {
                // Bare `super` or `super::super` — resolve to parent entry.nit
                let parent = ctx.source_filepath.parent();
                if parent.is_none() {
                    None
                } else {
                    let mut current = parent.unwrap().to_path_buf();
                    let mut valid = true;
                    for _ in 0..super_count {
                        match current.parent() {
                            Some(p) => current = p.to_path_buf(),
                            None => {
                                valid = false;
                                break;
                            }
                        }
                    }
                    if !valid {
                        None
                    } else {
                        let candidate = current.join("entry.nit");
                        if !candidate.exists() {
                            let src_dir = current.join("src");
                            if src_dir.exists() {
                                let entry = src_dir.join("entry.nit");
                                if entry.exists() {
                                    Some(ImportContext {
                                        package_name: ctx.package_name.clone(),
                                        source_filepath: entry,
                                        package_search_paths: ctx.package_search_paths.clone(),
                                        package_root: ctx.package_root.clone(),
                                    })
                                } else {
                                    None
                                }
                            } else {
                                None
                            }
                        } else {
                            Some(ImportContext {
                                package_name: ctx.package_name.clone(),
                                source_filepath: candidate,
                                package_search_paths: ctx.package_search_paths.clone(),
                                package_root: ctx.package_root.clone(),
                            })
                        }
                    }
                }
            } else {
                ctx.resolve_super_path(super_count, &relative_path)
                    .map(|path| ImportContext {
                        package_name: ctx.package_name.clone(),
                        source_filepath: path,
                        package_search_paths: ctx.package_search_paths.clone(),
                        package_root: ctx.package_root.clone(),
                    })
            }
        }

        Some(PathPrefix::SelfPath) => {
            // `use self::path` or bare `use self;`
            if let Some(item_name) = &item_name {
                ctx.resolve_self_path(item_name).map(|path| ImportContext {
                    package_name: ctx.package_name.clone(),
                    source_filepath: path,
                    package_search_paths: ctx.package_search_paths.clone(),
                    package_root: ctx.package_root.clone(),
                })
            } else {
                // Bare `use self;` — reference the current module (same file)
                Some(ImportContext {
                    package_name: ctx.package_name.clone(),
                    source_filepath: ctx.source_filepath.clone(),
                    package_search_paths: ctx.package_search_paths.clone(),
                    package_root: ctx.package_root.clone(),
                })
            }
        }

        None => {
            // No prefix — standard module import (existing logic)
            decide_what_to_import(ctx, import_name.clone(), log)
        }
    };

    if let Some(what) = what {
        let inside = ctx.package_name == what.package_name;
        let content = load_source_file(&what.source_filepath, what.package_name.clone(), inside, log);

        if let Some(mut module) = content {
            resolve_imports_guarded(&what, &mut module, log, visited, depth);
            module.visibility = import.visibility;

            if import_path.segments.len() > 1 {
                // Multi-segment path: extract the named item from the module
                let items = std::mem::take(&mut module.items);

                // Build the target item name from segments after the module name
                // For `use crate::foo::bar`, after resolving `crate::foo`, look for `bar`
                let target_segments: Vec<&str> = import_path
                    .segments
                    .iter()
                    .skip_while(|s| s.prefix.is_some())
                    .map(|s| s.segment.as_str())
                    .collect();

                let target: NString = if target_segments.len() > 1 {
                    target_segments[1..].join("::").into()
                } else if let Some(ref item_name) = item_name {
                    item_name.as_str().into()
                } else {
                    // Only one non-prefix segment — it IS the module we imported
                    module.items = items;
                    import.resolved = Some(vec![Item::Module(Box::new(module))]);
                    visited.remove(&import_name);
                    depth.pop();
                    return;
                };

                let found: Vec<Item> = items
                    .into_iter()
                    .filter(|item| match item {
                        Item::Function(f) => f.name == target,
                        Item::Struct(s) => s.name == target,
                        Item::Enum(e) => e.name == target,
                        Item::Trait(t) => t.name == target,
                        Item::TypeAlias(t) => t.name == target,
                        Item::Variable(v) => v.name == target,
                        _ => false,
                    })
                    .collect();

                if found.is_empty() {
                    log.report(&ResolveIssue::ImportNotFound((
                        format!("{}::{}", import_name, target).into(),
                        std::io::Error::from(std::io::ErrorKind::NotFound),
                    )));
                } else {
                    import.resolved = Some(found);
                }
            } else {
                // Single segment path: import the whole module
                import.resolved = Some(vec![Item::Module(Box::new(module))]);
            }
        } else {
            log.report(&ResolveIssue::ImportNotFound((
                import_name.clone(),
                std::io::Error::from(std::io::ErrorKind::NotFound),
            )));
        }
    }

    visited.remove(&import_name);
    depth.pop();
}

fn resolve_imports_guarded(
    ctx: &ImportContext,
    module: &mut Module,
    log: &CompilerLog,
    visited: &mut HashSet<NString>,
    depth: &mut Vec<NString>,
) {
    module.depth_first_iter_mut(&mut |order, node| {
        if order == Order::Leave
            && let RefNodeMut::ItemImport(import) = node
        {
            resolve_import(ctx, import, log, visited, depth);
        }
    });
}

pub fn resolve_imports(ctx: &ImportContext, module: &mut Module, log: &CompilerLog) {
    let mut visited = HashSet::new();
    let mut depth = Vec::new();

    resolve_imports_guarded(ctx, module, log, &mut visited, &mut depth);
}
