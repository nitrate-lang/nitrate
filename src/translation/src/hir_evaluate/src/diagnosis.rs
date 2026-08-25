use nitrate_diagnosis::{DiagnosticGroupId, DiagnosticInfo, FormattableDiagnosticGroup, Origin};

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub(crate) enum Diagnostic {
    UserMessage { message: String },
}

impl FormattableDiagnosticGroup for Diagnostic {
    fn group_id(&self) -> DiagnosticGroupId {
        DiagnosticGroupId::Hir
    }

    fn variant_id(&self) -> u16 {
        match self {
            Diagnostic::UserMessage { .. } => 1000,
        }
    }

    fn format(&self) -> DiagnosticInfo {
        match self {
            Diagnostic::UserMessage { message } => DiagnosticInfo {
                origin: Origin::None,
                message: message.clone(),
            },
        }
    }
}
