use nitrate_diagnosis::{
    DiagnosticGroupId, DiagnosticInfo, FormattableDiagnosticGroup, Origin, SourcePosition,
};

pub(crate) enum SyntaxBug {
    GenericParameterMissingName(SourcePosition),
    ExpectedCommaOrClosingAngleBracket(SourcePosition),
    ModuleMissingName(SourcePosition),
    ExpectedOpeningBrace(SourcePosition),
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
            SyntaxBug::ModuleMissingName(_) => 2,
            SyntaxBug::ExpectedOpeningBrace(_) => 3,
            SyntaxBug::Test => 4,
        }
    }

    fn format(&self) -> nitrate_diagnosis::DiagnosticInfo {
        match self {
            SyntaxBug::GenericParameterMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected a name for the generic parameter".into(),
            },

            SyntaxBug::ExpectedCommaOrClosingAngleBracket(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected a comma or closing angle bracket".into(),
            },

            SyntaxBug::ModuleMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected a name for the module".into(),
            },

            SyntaxBug::ExpectedOpeningBrace(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected an opening brace".into(),
            },

            SyntaxBug::Test => DiagnosticInfo {
                message: "this is a test syntax error".into(),
                origin: Origin::None,
            },
        }
    }
}
