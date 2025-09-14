use nitrate_diagnosis::{DiagnosticGroupId, FormattableDiagnosticGroup, Origin};

pub(crate) enum SyntaxError {
    Test,
}

impl FormattableDiagnosticGroup for SyntaxError {
    fn group_id(&self) -> DiagnosticGroupId {
        DiagnosticGroupId::SyntaxError
    }

    fn variant_id(&self) -> u16 {
        match self {
            SyntaxError::Test => 0,
        }
    }

    fn format(&self) -> nitrate_diagnosis::DiagnosticInfo {
        match self {
            SyntaxError::Test => nitrate_diagnosis::DiagnosticInfo {
                message: "This is a test syntax error".into(),
                origin: Origin::None,
            },
        }
    }
}
