use nitrate_diagnosis::{
    DiagnosticCollector, DiagnosticGroupId, DiagnosticInfo, FormattableDiagnosticGroup,
};
use nitrate_parsetree::{
    Order, ParseTreeIterMut, RefNodeMut,
    kind::{Item, Module, Path},
};

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

pub fn resolve(mut module: Module, _bugs: &DiagnosticCollector) -> Module {
    let mut name_scope = Vec::new();

    module.depth_first_iter_mut(&mut |order, node| {
        println!("Current scope: {}", name_scope.join("::"));

        if let RefNodeMut::ItemModule(module) = node {
            match order {
                Order::Pre => {
                    name_scope.push(module.name.to_string());
                }

                Order::Post => {
                    name_scope.pop();
                }
            }

            return;
        }

        if let RefNodeMut::ExprPath(path) = node {
            // TODO: Resolve the path here.
        }
    });

    module
}
