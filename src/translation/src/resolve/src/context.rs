use nitrate_diagnosis::{
    DiagnosticCollector, DiagnosticGroupId, DiagnosticInfo, FormattableDiagnosticGroup,
};
use nitrate_parsetree::{
    Order, RefNodeMut, item_depth_first_iter_mut,
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

pub fn resolve(module: Module, _bugs: &DiagnosticCollector) -> Module {
    let mut name_scope = Vec::new();
    let mut item = Item::Module(Box::new(module));

    item_depth_first_iter_mut(&mut item, &mut |order, node| {
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

        println!("Current scope: {}", name_scope.join("::"));
    });

    match item {
        Item::Module(module) => *module,
        _ => unreachable!(),
    }
}
