use nitrate_diagnosis::{DiagnosticExplanation, DiagnosticGroupId, DiagnosticInfo, FormattableDiagnosticGroup, Origin};
use nitrate_nstring::NString;
use nitrate_tree::ast::{ExprPath, TypePath};

pub enum ResolveIssue {
    ExprPathUnresolved(ExprPath),

    TypePathUnresolved(TypePath),

    ImportNotFound((NString, std::io::Error)),

    CircularImport { path: NString, depth: Vec<NString> },

    ImportSourceCodeSizeLimitExceeded(std::path::PathBuf),
    ImportDepthLimitExceeded(NString),
}

impl FormattableDiagnosticGroup for ResolveIssue {
    fn group_id(&self) -> DiagnosticGroupId {
        DiagnosticGroupId::Resolve
    }

    fn variant_id(&self) -> u16 {
        match self {
            ResolveIssue::ExprPathUnresolved(_) => 1,
            ResolveIssue::TypePathUnresolved(_) => 20,
            ResolveIssue::ImportNotFound(_) => 40,
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

            ResolveIssue::ImportNotFound(path) => DiagnosticInfo {
                origin: Origin::None,
                message: format!("Module not found: {} ({})", path.0, path.1),
            },

            ResolveIssue::CircularImport { path, depth } => DiagnosticInfo {
                origin: Origin::None,
                message: format!(
                    "Circular import detected: {}\nImport depth:\n{}",
                    path,
                    depth.iter().map(|p| format!(" - {}", p)).collect::<Vec<_>>().join("\n")
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
                message: format!("Import depth limit of 256 exceeded while importing module: {}", path),
            },
        }
    }
}

/// Static explanations for every name-resolution error code, used by `no3 --explain`.
pub fn explanations() -> &'static [DiagnosticExplanation] {
    &[
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Resolve,
            variant_id: 1,
            explanation: "An expression path (e.g. `foo::bar`) could not be resolved to any known symbol. \
                           Make sure the name is spelled correctly, is in scope, and has been imported with `use` if it lives in another module.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Resolve,
            variant_id: 20,
            explanation: "A type path could not be resolved to any known type. \
                           Check the spelling of the type and make sure it is in scope (or imported).",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Resolve,
            variant_id: 40,
            explanation: "An imported module was not found. \
                           Verify that the module file exists at the expected path and that the import path is spelled correctly.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Resolve,
            variant_id: 41,
            explanation: "A circular import was detected: a module (transitively) imports itself. \
                           Break the cycle by moving the shared declarations into a separate module that both sides import.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Resolve,
            variant_id: 42,
            explanation: "An imported module exceeded the maximum source file size limit. \
                           Split the module into smaller files.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Resolve,
            variant_id: 43,
            explanation: "The import chain exceeded the maximum depth of 256 nested imports. \
                           Flatten the module hierarchy to reduce import nesting.",
        },
    ]
}
