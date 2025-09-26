use crate::ResolveIssue;

use nitrate_diagnosis::CompilerLog;
use nitrate_parsetree::{
    Order, ParseTreeIterMut, RefNodeMut,
    kind::{Import, Module},
    tag::PackageNameId,
};

use std::collections::HashSet;

pub struct ImportContext<'a> {
    pub load_package: &'a dyn Fn(PackageNameId, &CompilerLog) -> Result<Module, ResolveIssue>,
    pub this_package_name: Option<PackageNameId>,
}

fn resolve_import_item(
    context: &ImportContext,
    import: &mut Import,
    log: &CompilerLog,
    visited: &mut HashSet<PackageNameId>,
    depth: &mut Vec<PackageNameId>,
) {
    let package_name = import.package_name.clone();

    const MAX_IMPORT_DEPTH: usize = 256;
    if depth.len() >= MAX_IMPORT_DEPTH {
        // This prevents stack overflow and other bugs.
        // For example, prevents crashes when cyclic importing symlinked files.
        log.report(&ResolveIssue::ImportDepthLimitExceeded(package_name));
        return;
    }

    depth.push(package_name.clone());

    if visited.contains(&package_name) {
        log.report(&ResolveIssue::CircularImport {
            path: package_name,
            depth: depth.clone(),
        });
        depth.pop();
        return;
    }

    visited.insert(package_name.clone());

    let mut module = match (context.load_package)(package_name.clone(), log) {
        Ok(m) => m,
        Err(err) => {
            log.report(&err);
            visited.remove(&package_name);
            depth.pop();
            return;
        }
    };

    module.visibility = import.visibility;
    resolve_imports_guarded(context, &mut module, log, visited, depth);

    import.module = Some(module);

    visited.remove(&package_name);
    depth.pop();
}

fn resolve_imports_guarded(
    context: &ImportContext,
    module: &mut Module,
    log: &CompilerLog,
    visited: &mut HashSet<PackageNameId>,
    depth: &mut Vec<PackageNameId>,
) {
    module.depth_first_iter_mut(&mut |order, node| {
        if order == Order::Leave {
            if let RefNodeMut::ItemImport(import) = node {
                resolve_import_item(context, import, log, visited, depth);
            }
        }
    });
}

pub fn resolve_imports(context: &ImportContext, module: &mut Module, log: &CompilerLog) {
    let mut visited = HashSet::new();
    let mut depth = Vec::new();

    resolve_imports_guarded(context, module, log, &mut visited, &mut depth);
}
