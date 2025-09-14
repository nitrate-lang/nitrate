use nitrate_diagnosis::{
    DiagnosticGroupId, DiagnosticInfo, FormattableDiagnosticGroup, Origin, SourcePosition,
};

pub(crate) enum SyntaxBug {
    GenericMissingParameterName(SourcePosition),
    GenericParameterLimit(SourcePosition),
    GenericParameterExpectedEnd(SourcePosition),

    ModuleMissingName(SourcePosition),
    ModuleItemLimit(SourcePosition),

    ImportMissingAliasName(SourcePosition),

    TypeAliasMissingName(SourcePosition),

    EnumMissingName(SourcePosition),
    EnumVariantLimit(SourcePosition),
    EnumMissingVariantName(SourcePosition),
    EnumExpectedEnd(SourcePosition),

    StructureMissingName(SourcePosition),
    StructureFieldLimit(SourcePosition),
    StructureMissingFieldName(SourcePosition),
    StructureExpectedEnd(SourcePosition),

    FunctionMissingName(SourcePosition),
    FunctionExpectedBody(SourcePosition),

    StaticVariableMissingName(SourcePosition),

    ExpectedOpeningBrace(SourcePosition),
    ExpectedEquals(SourcePosition),
    ExpectedSemicolon(SourcePosition),
    ExpectedColon(SourcePosition),
    ExpectedItem(SourcePosition),

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

            SyntaxBug::ModuleMissingName(_) => 20,
            SyntaxBug::ModuleItemLimit(_) => 21,

            SyntaxBug::ImportMissingAliasName(_) => 40,

            SyntaxBug::TypeAliasMissingName(_) => 60,

            SyntaxBug::EnumMissingName(_) => 80,
            SyntaxBug::EnumVariantLimit(_) => 81,
            SyntaxBug::EnumMissingVariantName(_) => 82,
            SyntaxBug::EnumExpectedEnd(_) => 83,

            SyntaxBug::StructureMissingName(_) => 100,
            SyntaxBug::StructureFieldLimit(_) => 101,
            SyntaxBug::StructureMissingFieldName(_) => 102,
            SyntaxBug::StructureExpectedEnd(_) => 103,

            SyntaxBug::FunctionMissingName(_) => 120,
            SyntaxBug::FunctionExpectedBody(_) => 121,

            SyntaxBug::StaticVariableMissingName(_) => 140,

            SyntaxBug::ExpectedOpeningBrace(_) => 1000,
            SyntaxBug::ExpectedEquals(_) => 1001,
            SyntaxBug::ExpectedSemicolon(_) => 1002,
            SyntaxBug::ExpectedColon(_) => 1003,
            SyntaxBug::ExpectedItem(_) => 1004,

            SyntaxBug::Test => 9999,
        }
    }

    fn format(&self) -> nitrate_diagnosis::DiagnosticInfo {
        match self {
            SyntaxBug::GenericMissingParameterName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "generic parameter name is missing".into(),
            },

            SyntaxBug::GenericParameterLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "generic parameter limit of 65,536 exceeded".into(),
            },

            SyntaxBug::GenericParameterExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected a '>' or ','".into(),
            },

            SyntaxBug::ModuleMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "module name is missing".into(),
            },

            SyntaxBug::ModuleItemLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "module item limit of 65,536 exceeded".into(),
            },

            SyntaxBug::ImportMissingAliasName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "import alias name is missing".into(),
            },

            SyntaxBug::TypeAliasMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "type alias name is missing".into(),
            },

            SyntaxBug::EnumMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "enum name is missing".into(),
            },

            SyntaxBug::EnumVariantLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "enum variant limit of 65,536 exceeded".into(),
            },

            SyntaxBug::EnumMissingVariantName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "enum variant name is missing".into(),
            },

            SyntaxBug::EnumExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected '}' or ','".into(),
            },

            SyntaxBug::StructureMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "structure name is missing".into(),
            },

            SyntaxBug::StructureFieldLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "structure field limit of 65,536 exceeded".into(),
            },

            SyntaxBug::StructureMissingFieldName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "structure field name is missing".into(),
            },

            SyntaxBug::StructureExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected '}' or ','".into(),
            },

            SyntaxBug::FunctionMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "function name is missing".into(),
            },

            SyntaxBug::FunctionExpectedBody(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected function body".into(),
            },

            SyntaxBug::StaticVariableMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "static variable name is missing".into(),
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

            SyntaxBug::ExpectedColon(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected ':'".into(),
            },

            SyntaxBug::ExpectedItem(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected an item".into(),
            },

            SyntaxBug::Test => DiagnosticInfo {
                message: "this is a test syntax error".into(),
                origin: Origin::None,
            },
        }
    }
}
