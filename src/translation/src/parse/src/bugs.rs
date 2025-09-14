use nitrate_diagnosis::{
    DiagnosticGroupId, DiagnosticInfo, FormattableDiagnosticGroup, Origin, SourcePosition,
};

pub(crate) enum SyntaxBug {
    GenericMissingParameterName(SourcePosition),
    GenericParameterLimit(SourcePosition),
    GenericParameterExpectedEnd(SourcePosition),

    TooManyModuleItems(SourcePosition),
    ExpectedOpeningBrace(SourcePosition),
    ExpectedEquals(SourcePosition),
    ExpectedSemicolon(SourcePosition),
    ExpectedItem(SourcePosition),
    ItemMissingName(SourcePosition),
    ExpectedImportAliasName(SourcePosition),
    Test,
}

impl FormattableDiagnosticGroup for SyntaxBug {
    fn group_id(&self) -> DiagnosticGroupId {
        DiagnosticGroupId::SyntaxBug
    }

    fn variant_id(&self) -> u16 {
        match self {
            SyntaxBug::GenericMissingParameterName(_) => 0,
            SyntaxBug::GenericParameterLimit(_) => 1,
            SyntaxBug::GenericParameterExpectedEnd(_) => 2,

            SyntaxBug::TooManyModuleItems(_) => 4,
            SyntaxBug::ExpectedOpeningBrace(_) => 5,
            SyntaxBug::ExpectedEquals(_) => 7,
            SyntaxBug::ExpectedSemicolon(_) => 8,
            SyntaxBug::ExpectedItem(_) => 9,
            SyntaxBug::ItemMissingName(_) => 10,
            SyntaxBug::ExpectedImportAliasName(_) => 11,
            SyntaxBug::Test => 12,
        }
    }

    fn format(&self) -> nitrate_diagnosis::DiagnosticInfo {
        match self {
            SyntaxBug::GenericMissingParameterName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected a name for the generic parameter".into(),
            },

            SyntaxBug::GenericParameterLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "a type cannot have more than 65,536 generic parameters".into(),
            },

            SyntaxBug::GenericParameterExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected a comma or closing angle bracket".into(),
            },

            SyntaxBug::TooManyModuleItems(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "modules cannot have more than 65,536 immediate children".into(),
            },

            SyntaxBug::ExpectedOpeningBrace(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected '{'".into(),
            },

            SyntaxBug::ExpectedEquals(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected '='".into(),
            },

            SyntaxBug::ExpectedSemicolon(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected ';'".into(),
            },

            SyntaxBug::ExpectedItem(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected an item".into(),
            },

            SyntaxBug::ItemMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "item name is missing".into(),
            },

            SyntaxBug::ExpectedImportAliasName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected a name for the import alias".into(),
            },

            SyntaxBug::Test => DiagnosticInfo {
                message: "this is a test syntax error".into(),
                origin: Origin::None,
            },
        }
    }
}
