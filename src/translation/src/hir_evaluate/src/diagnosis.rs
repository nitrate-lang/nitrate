use nitrate_diagnosis::{DiagnosticExplanation, DiagnosticGroupId, DiagnosticInfo, FormattableDiagnosticGroup, Origin};

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

/// Static explanations for every evaluator diagnostic code, used by `no3 --explain`.
pub fn explanations() -> &'static [DiagnosticExplanation] {
    &[DiagnosticExplanation {
        group_id: DiagnosticGroupId::Hir,
        variant_id: 1000,
        explanation: "A user-emitted message produced during constant evaluation (for example, by the `message!` \
                       compile-time builtin). This is not a compiler error; it carries a message supplied by user code.",
    }]
}

