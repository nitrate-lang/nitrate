use nitrate_diagnosis::{
    DiagnosticCollector, DiagnosticGroupId, DiagnosticInfo, FormattableDiagnosticGroup,
};

use nitrate_parsetree::{
    Order, ParseTreeIterMut, RefNodeMut,
    kind::{Item, Module, Path},
};

use crate::build_symbol_table;

pub enum ResolveIssue {
    NotFound(Path),
    Ambiguous(Path, Vec<Item>),
}

impl FormattableDiagnosticGroup for ResolveIssue {
    fn group_id(&self) -> DiagnosticGroupId {
        DiagnosticGroupId::Resolve
    }

    fn variant_id(&self) -> u16 {
        match self {
            ResolveIssue::NotFound(_) => 1,
            ResolveIssue::Ambiguous(_, _) => 2,
        }
    }

    fn format(&self) -> DiagnosticInfo {
        match self {
            ResolveIssue::NotFound(path) => DiagnosticInfo {
                origin: nitrate_diagnosis::Origin::None,
                message: format!("Symbol not found: {}", path.path.join("::")),
            },

            ResolveIssue::Ambiguous(path, candidates) => {
                // FIXME: Improve formatting.

                let mut message = format!(
                    "Ambiguous symbol reference: {}\nCandidates:",
                    path.path.join("::")
                );

                for candidate in candidates {
                    message.push_str(&format!("\n - {:?}", candidate));
                }

                DiagnosticInfo {
                    origin: nitrate_diagnosis::Origin::None,
                    message,
                }
            }
        }
    }
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

        if let RefNodeMut::ExprPath(path) = node {
            let _ = &symbol_table;
            let _ = &scope_vec;
            let _ = &path;

            // TODO: Resolve the path using the symbol table.
        }
    });
}
