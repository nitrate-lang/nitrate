use nitrate_diagnosis::{
    DiagnosticGroupId, DiagnosticInfo, FormattableDiagnosticGroup, Origin, SourcePosition,
};

pub(crate) enum SyntaxBug {
    GenericMissingParameterName(SourcePosition),
    GenericParameterLimit(SourcePosition),
    GenericParameterExpectedEnd(SourcePosition),

    ModuleMissingName(SourcePosition),
    ModuleItemLimit(SourcePosition),
    ModuleExpectedEnd(SourcePosition),

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
    FunctionParameterLimit(SourcePosition),
    FunctionParameterMissingName(SourcePosition),
    FunctionParametersExpectedEnd(SourcePosition),

    StaticVariableMissingName(SourcePosition),

    ConstVariableMissingName(SourcePosition),

    TraitMissingName(SourcePosition),
    TraitItemLimit(SourcePosition),
    TraitDoesNotAllowItem(SourcePosition),
    TraitExpectedEnd(SourcePosition),

    ImplMissingFor(SourcePosition),
    ImplExpectedEnd(SourcePosition),
    ImplItemLimit(SourcePosition),

    ExpectedGenericArgumentEnd(SourcePosition),
    GenericArgumentLimit(SourcePosition),

    ReferenceTypeExpectedLifetimeName(SourcePosition),

    OpaqueTypeMissingName(SourcePosition),

    TupleTypeExpectedEnd(SourcePosition),
    TupleTypeElementLimit(SourcePosition),

   
    ExpectedOpenParen(SourcePosition),
    ExpectedCloseParen(SourcePosition),
    ExpectedOpenBrace(SourcePosition),
    ExpectedCloseBrace(SourcePosition),
    ExpectedOpenBracket(SourcePosition),
    ExpectedCloseBracket(SourcePosition),
    ExpectedOpenAngle(SourcePosition),
    ExpectedCloseAngle(SourcePosition),
    ExpectedSemicolon(SourcePosition),
    ExpectedColon(SourcePosition),
    ExpectedArrow(SourcePosition),
    ExpectedBlockArrow(SourcePosition),

    ExpectedItem(SourcePosition),
    ExpectedType(SourcePosition),
    ExpectedExpr(SourcePosition),
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
            SyntaxBug::ModuleExpectedEnd(_) => 22,

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
            SyntaxBug::FunctionParameterLimit(_) => 122,
            SyntaxBug::FunctionParameterMissingName(_) => 123,
            SyntaxBug::FunctionParametersExpectedEnd(_) => 124,

            SyntaxBug::StaticVariableMissingName(_) => 140,

            SyntaxBug::ConstVariableMissingName(_) => 160,

            SyntaxBug::TraitMissingName(_) => 180,
            SyntaxBug::TraitItemLimit(_) => 181,
            SyntaxBug::TraitDoesNotAllowItem(_) => 182,
            SyntaxBug::TraitExpectedEnd(_) => 183,

            SyntaxBug::ImplMissingFor(_) => 200,
            SyntaxBug::ImplExpectedEnd(_) => 201,
            SyntaxBug::ImplItemLimit(_) => 202,

            SyntaxBug::ExpectedGenericArgumentEnd(_) => 220,
            SyntaxBug::GenericArgumentLimit(_) => 221,

            SyntaxBug::ReferenceTypeExpectedLifetimeName(_) => 240,

            SyntaxBug::OpaqueTypeMissingName(_) => 260,

            SyntaxBug::TupleTypeExpectedEnd(_) => 280,
            SyntaxBug::TupleTypeElementLimit(_) => 281,

            SyntaxBug::ExpectedOpenParen(_) => 1000,
            SyntaxBug::ExpectedCloseParen(_) => 1001,
            SyntaxBug::ExpectedOpenBrace(_) => 1002,
            SyntaxBug::ExpectedCloseBrace(_) => 1003,
            SyntaxBug::ExpectedOpenBracket(_) => 1004,
            SyntaxBug::ExpectedCloseBracket(_) => 1005,
            SyntaxBug::ExpectedOpenAngle(_) => 1006,
            SyntaxBug::ExpectedCloseAngle(_) => 1007,
            SyntaxBug::ExpectedSemicolon(_) => 1008,
            SyntaxBug::ExpectedColon(_) => 1009,
            SyntaxBug::ExpectedArrow(_) => 1010,
            SyntaxBug::ExpectedBlockArrow(_) => 1011,

            SyntaxBug::ExpectedItem(_) => 2000,
            SyntaxBug::ExpectedType(_) => 2001,
            SyntaxBug::ExpectedExpr(_) => 2002,
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

            /* ------------------------------------------------------------------------- */

            SyntaxBug::ModuleMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "module name is missing".into(),
            },

            SyntaxBug::ModuleItemLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "module item limit of 65,536 exceeded".into(),
            },

            SyntaxBug::ModuleExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected '}'".into(),
            },

            /* ------------------------------------------------------------------------- */

            SyntaxBug::ImportMissingAliasName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "import alias name is missing".into(),
            },

            /* ------------------------------------------------------------------------- */

            SyntaxBug::TypeAliasMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "type alias name is missing".into(),
            },

            /* ------------------------------------------------------------------------- */

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

            /* ------------------------------------------------------------------------- */

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

            /* ------------------------------------------------------------------------- */

            SyntaxBug::FunctionMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "function name is missing".into(),
            },

            SyntaxBug::FunctionExpectedBody(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected function body".into(),
            },

            SyntaxBug::FunctionParameterLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "function parameter limit of 65,536 exceeded".into(),
            },

            SyntaxBug::FunctionParameterMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "function parameter name is missing".into(),
            },

            SyntaxBug::FunctionParametersExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected ')' or ','".into(),
            },

            /* ------------------------------------------------------------------------- */

            SyntaxBug::StaticVariableMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "static variable name is missing".into(),
            },

            /* ------------------------------------------------------------------------- */

            SyntaxBug::ConstVariableMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "const variable name is missing".into(),
            },

            /* ------------------------------------------------------------------------- */

            SyntaxBug::TraitMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "trait name is missing".into(),
            },

            SyntaxBug::TraitItemLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "trait item limit of 65,536 exceeded".into(),
            },

            SyntaxBug::TraitDoesNotAllowItem(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "only associated constants, type aliases, and function signatures are allowed in traits"
                    .into(),
            },

            SyntaxBug::TraitExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected '}'".into(),
            },

            /* ------------------------------------------------------------------------- */

            SyntaxBug::ImplMissingFor(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected 'for' in impl declaration".into(),
            },

            SyntaxBug::ImplExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected '}'".into(),
            },

            SyntaxBug::ImplItemLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "impl item limit of 65,536 exceeded".into(),
            },

            /* ------------------------------------------------------------------------- */

            SyntaxBug::ExpectedGenericArgumentEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected a '>' or ','".into(),
            },

            SyntaxBug::GenericArgumentLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "generic argument limit of 65,536 exceeded".into(),
            },

            /* ------------------------------------------------------------------------- */

            SyntaxBug::ReferenceTypeExpectedLifetimeName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "reference lifetime is missing after '".into(),
            },

            /* ------------------------------------------------------------------------- */

            SyntaxBug::OpaqueTypeMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "opaque type name is missing".into(),
            },
            
            /* ------------------------------------------------------------------------- */

            SyntaxBug::TupleTypeExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected ')' or ','".into(),
            },

            SyntaxBug::TupleTypeElementLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "tuple element limit of 65,536 exceeded".into(),
            },

            /* ------------------------------------------------------------------------- */

            SyntaxBug::ExpectedOpenParen(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected '('".into(),
            },

            SyntaxBug::ExpectedCloseParen(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected ')'".into(),
            },

            SyntaxBug::ExpectedOpenBrace(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected '{'".into(),
            },

            SyntaxBug::ExpectedCloseBrace(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected '}'".into(),
            },

            SyntaxBug::ExpectedOpenBracket(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected '['".into(),
            },

            SyntaxBug::ExpectedCloseBracket(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected ']'".into(),
            },

            SyntaxBug::ExpectedOpenAngle(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected '<'".into(),
            },

            SyntaxBug::ExpectedCloseAngle(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected '>'".into(),
            },

            SyntaxBug::ExpectedSemicolon(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected ';'".into(),
            },

            SyntaxBug::ExpectedColon(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected ':'".into(),
            },

            SyntaxBug::ExpectedArrow(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected '->'".into(),
            },

            SyntaxBug::ExpectedBlockArrow(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected '=>''".into(),
            },

            /* ------------------------------------------------------------------------- */

            SyntaxBug::ExpectedItem(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected an item".into(),
            },

            SyntaxBug::ExpectedType(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected a type".into(),
            },

            SyntaxBug::ExpectedExpr(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: "expected an expression".into(),
            },
        }
    }
}
