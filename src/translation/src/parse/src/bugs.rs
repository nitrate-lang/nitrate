use nitrate_diagnosis::{
    DiagnosticGroupId, DiagnosticInfo, FormattableDiagnosticGroup, Origin, SourcePosition,
};

pub(crate) enum SyntaxBug {
    GenericParameterMissingName(SourcePosition),
    ExpectedCommaOrClosingAngleBracket(SourcePosition),
    Test,
}

impl FormattableDiagnosticGroup for SyntaxBug {
    fn group_id(&self) -> DiagnosticGroupId {
        DiagnosticGroupId::SyntaxBug
    }

    fn variant_id(&self) -> u16 {
        match self {
            SyntaxBug::GenericParameterMissingName(_) => 0,
            SyntaxBug::ExpectedCommaOrClosingAngleBracket(_) => 1,
            SyntaxBug::Test => 2,
        }
    }

    fn format(&self) -> nitrate_diagnosis::DiagnosticInfo {
        match self {
            SyntaxBug::GenericParameterMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "Expected a name for the generic parameter".into(),
            },

            SyntaxBug::ExpectedCommaOrClosingAngleBracket(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "Expected a comma or closing angle bracket".into(),
            },

            SyntaxBug::Test => DiagnosticInfo {
                message: "This is a test syntax error".into(),
                origin: Origin::None,
            },
        }
    }
}
