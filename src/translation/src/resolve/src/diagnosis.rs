use crate::{Symbol, SymbolName};

use nitrate_diagnosis::{DiagnosticGroupId, DiagnosticInfo, FormattableDiagnosticGroup, Origin};
use nitrate_parsetree::kind::{ExprPath, TypePath};

pub enum ResolveIssue {
    ExprPathUnresolved(ExprPath, SymbolName),
    ExprPathAmbiguous(ExprPath, SymbolName, Vec<Symbol>),

    TypePathUnresolved(TypePath, SymbolName),
    TypePathAmbiguous(TypePath, SymbolName, Vec<Symbol>),
}

impl FormattableDiagnosticGroup for ResolveIssue {
    fn group_id(&self) -> DiagnosticGroupId {
        DiagnosticGroupId::Resolve
    }

    fn variant_id(&self) -> u16 {
        match self {
            ResolveIssue::ExprPathUnresolved(_, _) => 1,
            ResolveIssue::ExprPathAmbiguous(_, _, _) => 2,

            ResolveIssue::TypePathUnresolved(_, _) => 20,
            ResolveIssue::TypePathAmbiguous(_, _, _) => 21,
        }
    }

    fn format(&self) -> nitrate_diagnosis::DiagnosticInfo {
        match self {
            ResolveIssue::ExprPathUnresolved(path, candidate) => DiagnosticInfo {
                origin: Origin::None,
                message: format!(
                    "Unresolved expression path: {} (candidate: {})",
                    path.segments
                        .iter()
                        .map(|s| s.identifier.to_owned())
                        .collect::<Vec<_>>()
                        .join("::"),
                    candidate
                ),
            },

            ResolveIssue::ExprPathAmbiguous(path, candidate, _) => DiagnosticInfo {
                origin: Origin::None,
                message: format!(
                    "Ambiguous expression path: {} (candidate: {})",
                    path.segments
                        .iter()
                        .map(|s| s.identifier.to_owned())
                        .collect::<Vec<_>>()
                        .join("::"),
                    candidate
                ),
            },

            ResolveIssue::TypePathUnresolved(path, candidate) => DiagnosticInfo {
                origin: Origin::None,
                message: format!(
                    "Unresolved type path: {} (candidate: {})",
                    path.segments
                        .iter()
                        .map(|s| s.identifier.to_owned())
                        .collect::<Vec<_>>()
                        .join("::"),
                    candidate
                ),
            },

            ResolveIssue::TypePathAmbiguous(path, candidate, _) => DiagnosticInfo {
                origin: Origin::None,
                message: format!(
                    "Ambiguous type path: {} (candidate: {})",
                    path.segments
                        .iter()
                        .map(|s| s.identifier.to_owned())
                        .collect::<Vec<_>>()
                        .join("::"),
                    candidate
                ),
            },
        }
    }
}
