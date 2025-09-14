use nitrate_diagnosis::{
    DiagnosticGroupId, DiagnosticInfo, FormattableDiagnosticGroup, Origin, SourcePosition,
};

pub(crate) enum SyntaxError {
    NamelessGenericParameter(SourcePosition),
    Test,
}

impl FormattableDiagnosticGroup for SyntaxError {
    fn group_id(&self) -> DiagnosticGroupId {
        DiagnosticGroupId::SyntaxError
    }

    fn variant_id(&self) -> u16 {
        match self {
            SyntaxError::NamelessGenericParameter(_) => 0,
            SyntaxError::Test => 1,
        }
    }

    fn format(&self) -> nitrate_diagnosis::DiagnosticInfo {
        match self {
            SyntaxError::NamelessGenericParameter(pos) => DiagnosticInfo {
                message: "Generic parameters must have a name".into(),
                origin: Origin::Point(pos.to_owned()),
            },

            SyntaxError::Test => DiagnosticInfo {
                message: "This is a test syntax error".into(),
                origin: Origin::None,
            },
        }
    }
}
