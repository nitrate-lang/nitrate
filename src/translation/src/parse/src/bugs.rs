use nitrate_diagnosis::{
    DiagnosticGroupId, DiagnosticInfo, FormattableDiagnosticGroup, Origin, SourcePosition,
};

pub(crate) enum SyntaxBug {
    GenericParameterMissingName(SourcePosition),
    TooManyGenericParameters(SourcePosition),
    ExpectedCommaOrClosingAngleBracket(SourcePosition),
    ModuleMissingName(SourcePosition),
    TooManyModuleItems(SourcePosition),
    ExpectedOpeningBrace(SourcePosition),
    TypeAliasMissingName(SourcePosition),
    ExpectedEquals(SourcePosition),
    ExpectedSemicolon(SourcePosition),
    Test,
}

impl FormattableDiagnosticGroup for SyntaxBug {
    fn group_id(&self) -> DiagnosticGroupId {
        DiagnosticGroupId::SyntaxBug
    }

    fn variant_id(&self) -> u16 {
        match self {
            SyntaxBug::GenericParameterMissingName(_) => 0,
            SyntaxBug::TooManyGenericParameters(_) => 1,
            SyntaxBug::ExpectedCommaOrClosingAngleBracket(_) => 2,
            SyntaxBug::ModuleMissingName(_) => 3,
            SyntaxBug::TooManyModuleItems(_) => 4,
            SyntaxBug::ExpectedOpeningBrace(_) => 5,
            SyntaxBug::TypeAliasMissingName(_) => 6,
            SyntaxBug::ExpectedEquals(_) => 7,
            SyntaxBug::ExpectedSemicolon(_) => 8,
            SyntaxBug::Test => 9,
        }
    }

    fn format(&self) -> nitrate_diagnosis::DiagnosticInfo {
        match self {
            SyntaxBug::GenericParameterMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected a name for the generic parameter".into(),
            },

            SyntaxBug::TooManyGenericParameters(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "a type cannot have more than 65,536 generic parameters".into(),
            },

            SyntaxBug::ExpectedCommaOrClosingAngleBracket(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected a comma or closing angle bracket".into(),
            },

            SyntaxBug::ModuleMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected a name for the module".into(),
            },

            SyntaxBug::TooManyModuleItems(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "modules cannot have more than 65,536 immediate children".into(),
            },

            SyntaxBug::ExpectedOpeningBrace(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected an opening brace".into(),
            },

            SyntaxBug::TypeAliasMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected a type alias name".into(),
            },

            SyntaxBug::ExpectedEquals(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected '='".into(),
            },

            SyntaxBug::ExpectedSemicolon(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected ';'".into(),
            },

            SyntaxBug::Test => DiagnosticInfo {
                message: "this is a test syntax error".into(),
                origin: Origin::None,
            },
        }
    }
}
