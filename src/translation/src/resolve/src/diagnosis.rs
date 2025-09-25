use crate::Symbol;

use nitrate_diagnosis::{DiagnosticGroupId, DiagnosticInfo, FormattableDiagnosticGroup, Origin};
use nitrate_parsetree::kind::{ExprPath, TypePath};

pub enum ResolveIssue {
    ExprPathUnresolved(ExprPath),
    ExprPathAmbiguous(ExprPath, Vec<Symbol>),

    TypePathUnresolved(TypePath),
    TypePathAmbiguous(TypePath, Vec<Symbol>),

    ModuleNotFound((std::path::PathBuf, std::io::Error)),
    CircularImport {
        path: std::path::PathBuf,
        depth: Vec<std::path::PathBuf>,
    },
    ImportSourceCodeSizeLimitExceeded(std::path::PathBuf),
    ImportDepthLimitExceeded(std::path::PathBuf),
}

impl FormattableDiagnosticGroup for ResolveIssue {
    fn group_id(&self) -> DiagnosticGroupId {
        DiagnosticGroupId::Resolve
    }

    fn variant_id(&self) -> u16 {
        match self {
            ResolveIssue::ExprPathUnresolved(_) => 1,
            ResolveIssue::ExprPathAmbiguous(_, _) => 2,

            ResolveIssue::TypePathUnresolved(_) => 20,
            ResolveIssue::TypePathAmbiguous(_, _) => 21,

            ResolveIssue::ModuleNotFound(_) => 40,
            ResolveIssue::CircularImport { .. } => 41,
            ResolveIssue::ImportSourceCodeSizeLimitExceeded(_) => 42,
            ResolveIssue::ImportDepthLimitExceeded(_) => 43,
        }
    }

    fn format(&self) -> nitrate_diagnosis::DiagnosticInfo {
        match self {
            ResolveIssue::ExprPathUnresolved(path) => DiagnosticInfo {
                origin: Origin::None,
                message: format!(
                    "Unresolved expression path: {}",
                    path.segments
                        .iter()
                        .map(|s| s.name.to_owned())
                        .collect::<Vec<_>>()
                        .join("::"),
                ),
            },

            ResolveIssue::ExprPathAmbiguous(path, _) => DiagnosticInfo {
                origin: Origin::None,
                message: format!(
                    "Ambiguous expression path: {}",
                    path.segments
                        .iter()
                        .map(|s| s.name.to_owned())
                        .collect::<Vec<_>>()
                        .join("::"),
                ),
            },

            ResolveIssue::TypePathUnresolved(path) => DiagnosticInfo {
                origin: Origin::None,
                message: format!(
                    "Unresolved type path: {}",
                    path.segments
                        .iter()
                        .map(|s| s.name.to_owned())
                        .collect::<Vec<_>>()
                        .join("::"),
                ),
            },

            ResolveIssue::TypePathAmbiguous(path, _) => DiagnosticInfo {
                origin: Origin::None,
                message: format!(
                    "Ambiguous type path: {}",
                    path.segments
                        .iter()
                        .map(|s| s.name.to_owned())
                        .collect::<Vec<_>>()
                        .join("::"),
                ),
            },

            ResolveIssue::ModuleNotFound(path) => DiagnosticInfo {
                origin: Origin::None,
                message: format!("Module not found: {} ({})", path.0.display(), path.1),
            },

            ResolveIssue::CircularImport { path, depth } => DiagnosticInfo {
                origin: Origin::None,
                message: format!(
                    "Circular import detected: {}\nImport depth:\n{}",
                    path.display(),
                    depth
                        .iter()
                        .map(|p| format!(" - {}", p.display()))
                        .collect::<Vec<_>>()
                        .join("\n")
                ),
            },

            ResolveIssue::ImportSourceCodeSizeLimitExceeded(path) => DiagnosticInfo {
                origin: Origin::None,
                message: format!(
                    "Imported module ({}) exceeded the source code file size limit.",
                    path.display()
                ),
            },

            ResolveIssue::ImportDepthLimitExceeded(path) => DiagnosticInfo {
                origin: Origin::None,
                message: format!(
                    "Import depth limit of 256 exceeded while importing module: {}",
                    path.display()
                ),
            },
        }
    }
}
