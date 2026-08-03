use nitrate_diagnosis::{DiagnosticGroupId, DiagnosticInfo, FormattableDiagnosticGroup, Origin, SourcePosition};
use nitrate_token::LexPos;

pub(crate) enum SyntaxErr {
    GenericMissingParameterName(LexPos),
    GenericParameterLimit(LexPos),
    GenericParameterExpectedEnd(LexPos),

    ModuleMissingName(LexPos),
    ModuleItemLimit(LexPos),
    ModuleExpectedEnd(LexPos),

    ImportAliasMissingName(LexPos),
    ImportExpectedStarOrGroup(LexPos),
    ImportGroupExpectedEnd(LexPos),

    TypeAliasMissingName(LexPos),

    EnumMissingName(LexPos),
    EnumVariantLimit(LexPos),
    EnumMissingVariantName(LexPos),
    EnumExpectedEnd(LexPos),

    StructureMissingName(LexPos),
    StructureFieldLimit(LexPos),
    StructureMissingFieldName(LexPos),
    StructureExpectedEnd(LexPos),

    FunctionMissingName(LexPos),
    FunctionParameterLimit(LexPos),
    FunctionParameterMissingName(LexPos),
    FunctionParametersExpectedEnd(LexPos),
    FunctionParameterVariadicExpected(LexPos),

    VariableMissingName(LexPos),

    TraitMissingName(LexPos),
    TraitItemLimit(LexPos),
    TraitDoesNotAllowItem(LexPos),
    TraitExpectedEnd(LexPos),

    ImplMissingFor(LexPos),
    ImplExpectedEnd(LexPos),
    ImplItemLimit(LexPos),
    ImplCannotBeVisible(LexPos),

    PathGenericArgumentExpectedEnd(LexPos),
    PathGenericArgumentLimit(LexPos),
    PathExpectedNameOrSeparator(LexPos),
    PathSegmentLimit(LexPos),
    PathExpectedName(LexPos),

    ReferenceTypeExpectedLifetimeName(LexPos),

    StructExpectedFieldOrEnd(LexPos),
    StructExpectedFieldName(LexPos),

    TupleTypeExpectedEnd(LexPos),
    TupleTypeElementLimit(LexPos),

    ListExpectedEnd(LexPos),
    ListElementLimit(LexPos),

    AttributesExpectedEnd(LexPos),
    AttributesElementLimit(LexPos),

    BlockExpectedEnd(LexPos),
    BlockElementLimit(LexPos),

    BreakMissingLabel(LexPos),

    ContinueMissingLabel(LexPos),

    FunctionCallExpectedEnd(LexPos),
    FunctionCallArgumentLimit(LexPos),
    FunctionCallPositionFollowsNamed(LexPos),

    ForVariableBindingMissingName(LexPos),
    ForVariableBindingExpectedEnd(LexPos),
    ForVariableBindingLimit(LexPos),
    ForExpectedInKeyword(LexPos),

    ExpectedFieldOrMethodName(LexPos),

    ExpectedOpenParen(LexPos),
    ExpectedCloseParen(LexPos),
    ExpectedOpenBrace(LexPos),
    ExpectedCloseBrace(LexPos),
    ExpectedOpenBracket(LexPos),
    ExpectedCloseBracket(LexPos),
    ExpectedCloseAngle(LexPos),
    ExpectedSemicolon(LexPos),
    ExpectedColon(LexPos),
    ExpectedArrow(LexPos),
    SyntaxNotSupported(LexPos),

    ExpectedItem(LexPos),
    ExpectedType(LexPos),
    ExpectedExpr(LexPos),
}

fn lexpos_to_source_position(pos: &LexPos) -> SourcePosition {
    SourcePosition {
        line: u32::from(pos.line),
        column: u32::from(pos.column),
        offset: pos.offset,
        fileid: pos.fileid,
    }
}

impl FormattableDiagnosticGroup for SyntaxErr {
    fn group_id(&self) -> DiagnosticGroupId {
        DiagnosticGroupId::Parse
    }

    fn variant_id(&self) -> u16 {
        match self {
            SyntaxErr::GenericMissingParameterName(_) => 0,
            SyntaxErr::GenericParameterLimit(_) => 1,
            SyntaxErr::GenericParameterExpectedEnd(_) => 2,

            SyntaxErr::ModuleMissingName(_) => 20,
            SyntaxErr::ModuleItemLimit(_) => 21,
            SyntaxErr::ModuleExpectedEnd(_) => 22,

            SyntaxErr::ImportAliasMissingName(_) => 41,
            SyntaxErr::ImportExpectedStarOrGroup(_) => 42,
            SyntaxErr::ImportGroupExpectedEnd(_) => 43,

            SyntaxErr::TypeAliasMissingName(_) => 60,

            SyntaxErr::EnumMissingName(_) => 80,
            SyntaxErr::EnumVariantLimit(_) => 81,
            SyntaxErr::EnumMissingVariantName(_) => 82,
            SyntaxErr::EnumExpectedEnd(_) => 83,

            SyntaxErr::StructureMissingName(_) => 100,
            SyntaxErr::StructureFieldLimit(_) => 101,
            SyntaxErr::StructureMissingFieldName(_) => 102,
            SyntaxErr::StructureExpectedEnd(_) => 103,

            SyntaxErr::FunctionMissingName(_) => 120,
            SyntaxErr::FunctionParameterLimit(_) => 122,
            SyntaxErr::FunctionParameterMissingName(_) => 123,
            SyntaxErr::FunctionParametersExpectedEnd(_) => 124,
            SyntaxErr::FunctionParameterVariadicExpected(_) => 126,

            SyntaxErr::VariableMissingName(_) => 140,

            SyntaxErr::TraitMissingName(_) => 180,
            SyntaxErr::TraitItemLimit(_) => 181,
            SyntaxErr::TraitDoesNotAllowItem(_) => 182,
            SyntaxErr::TraitExpectedEnd(_) => 183,

            SyntaxErr::ImplMissingFor(_) => 200,
            SyntaxErr::ImplExpectedEnd(_) => 201,
            SyntaxErr::ImplItemLimit(_) => 202,
            SyntaxErr::ImplCannotBeVisible(_) => 203,

            SyntaxErr::PathGenericArgumentExpectedEnd(_) => 222,
            SyntaxErr::PathGenericArgumentLimit(_) => 223,
            SyntaxErr::PathExpectedNameOrSeparator(_) => 224,
            SyntaxErr::PathSegmentLimit(_) => 225,
            SyntaxErr::PathExpectedName(_) => 226,

            SyntaxErr::ReferenceTypeExpectedLifetimeName(_) => 240,

            SyntaxErr::StructExpectedFieldOrEnd(_) => 260,
            SyntaxErr::StructExpectedFieldName(_) => 261,

            SyntaxErr::TupleTypeExpectedEnd(_) => 280,
            SyntaxErr::TupleTypeElementLimit(_) => 281,

            SyntaxErr::ListExpectedEnd(_) => 300,
            SyntaxErr::ListElementLimit(_) => 301,

            SyntaxErr::AttributesExpectedEnd(_) => 320,
            SyntaxErr::AttributesElementLimit(_) => 321,

            SyntaxErr::BlockExpectedEnd(_) => 340,
            SyntaxErr::BlockElementLimit(_) => 341,

            SyntaxErr::BreakMissingLabel(_) => 360,

            SyntaxErr::ContinueMissingLabel(_) => 380,

            SyntaxErr::FunctionCallExpectedEnd(_) => 400,
            SyntaxErr::FunctionCallArgumentLimit(_) => 401,
            SyntaxErr::FunctionCallPositionFollowsNamed(_) => 402,

            SyntaxErr::ForVariableBindingMissingName(_) => 440,
            SyntaxErr::ForVariableBindingExpectedEnd(_) => 441,
            SyntaxErr::ForVariableBindingLimit(_) => 442,
            SyntaxErr::ForExpectedInKeyword(_) => 443,

            SyntaxErr::ExpectedFieldOrMethodName(_) => 500,

            SyntaxErr::ExpectedOpenParen(_) => 1000,
            SyntaxErr::ExpectedCloseParen(_) => 1001,
            SyntaxErr::ExpectedOpenBrace(_) => 1002,
            SyntaxErr::ExpectedCloseBrace(_) => 1003,
            SyntaxErr::ExpectedOpenBracket(_) => 1004,
            SyntaxErr::ExpectedCloseBracket(_) => 1005,
            SyntaxErr::ExpectedCloseAngle(_) => 1007,
            SyntaxErr::ExpectedSemicolon(_) => 1008,
            SyntaxErr::ExpectedColon(_) => 1009,
            SyntaxErr::ExpectedArrow(_) => 1010,

            SyntaxErr::ExpectedItem(_) => 2000,
            SyntaxErr::ExpectedType(_) => 2001,
            SyntaxErr::ExpectedExpr(_) => 2002,
            SyntaxErr::SyntaxNotSupported(_) => 2020,
        }
    }

    fn format(&self) -> DiagnosticInfo {
        match self {
            SyntaxErr::GenericMissingParameterName(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "generic parameter name is missing".into(),
            },
            SyntaxErr::GenericParameterLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "generic parameter limit of 65,536 exceeded".into(),
            },
            SyntaxErr::GenericParameterExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "expected a '>' or ','".into(),
            },
            SyntaxErr::ModuleMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "module name is missing".into(),
            },
            SyntaxErr::ModuleItemLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "module item limit of 65,536 exceeded".into(),
            },
            SyntaxErr::ModuleExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "expected '}'".into(),
            },
            SyntaxErr::ImportAliasMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "use alias name is missing".into(),
            },
            SyntaxErr::ImportExpectedStarOrGroup(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "expected '*' or '{' after '::' in use statement".into(),
            },
            SyntaxErr::ImportGroupExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "expected '}' at the end of use group".into(),
            },
            SyntaxErr::TypeAliasMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "type alias name is missing".into(),
            },
            SyntaxErr::EnumMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "enum name is missing".into(),
            },
            SyntaxErr::EnumVariantLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "enum variant limit of 65,536 exceeded".into(),
            },
            SyntaxErr::EnumMissingVariantName(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "enum variant name is missing".into(),
            },
            SyntaxErr::EnumExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "expected '}' or ','".into(),
            },
            SyntaxErr::StructureMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "structure name is missing".into(),
            },
            SyntaxErr::StructureFieldLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "structure field limit of 65,536 exceeded".into(),
            },
            SyntaxErr::StructureMissingFieldName(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "structure field name is missing".into(),
            },
            SyntaxErr::StructureExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "expected '}' or ','".into(),
            },
            SyntaxErr::FunctionMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "function name is missing".into(),
            },
            SyntaxErr::FunctionParameterLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "function parameter limit of 65,536 exceeded".into(),
            },
            SyntaxErr::FunctionParameterMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "function parameter name is missing".into(),
            },
            SyntaxErr::FunctionParametersExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "expected ')' or ','".into(),
            },
            SyntaxErr::FunctionParameterVariadicExpected(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "expected '...' for variadic function parameter".into(),
            },
            SyntaxErr::VariableMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "variable name is missing".into(),
            },
            SyntaxErr::TraitMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "trait name is missing".into(),
            },
            SyntaxErr::TraitItemLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "trait item limit of 65,536 exceeded".into(),
            },
            SyntaxErr::TraitDoesNotAllowItem(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "only associated constants, type aliases, and function signatures are allowed in traits"
                    .into(),
            },
            SyntaxErr::TraitExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "expected '}'".into(),
            },
            SyntaxErr::ImplMissingFor(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "expected 'for' in impl declaration".into(),
            },
            SyntaxErr::ImplExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "expected '}'".into(),
            },
            SyntaxErr::ImplItemLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "impl item limit of 65,536 exceeded".into(),
            },
            SyntaxErr::ImplCannotBeVisible(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "impl blocks cannot have visibility modifiers".into(),
            },
            SyntaxErr::PathGenericArgumentExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "expected '>' or ',' in generic arguments".into(),
            },
            SyntaxErr::PathGenericArgumentLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "generic argument limit of 65,536 exceeded".into(),
            },
            SyntaxErr::PathExpectedNameOrSeparator(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "expected identifier or '::' in path".into(),
            },
            SyntaxErr::PathSegmentLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "path segment limit of 65,536 exceeded".into(),
            },
            SyntaxErr::PathExpectedName(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "path segment name is missing".into(),
            },
            SyntaxErr::ReferenceTypeExpectedLifetimeName(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "reference lifetime is missing after '".into(),
            },
            SyntaxErr::StructExpectedFieldOrEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "expected field name or '}'".into(),
            },
            SyntaxErr::StructExpectedFieldName(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "expected field name".into(),
            },
            SyntaxErr::TupleTypeExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "expected ')' or ','".into(),
            },
            SyntaxErr::TupleTypeElementLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "tuple element limit of 65,536 exceeded".into(),
            },
            SyntaxErr::ListExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "expected ',' or ']'".into(),
            },
            SyntaxErr::ListElementLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "list element limit of 65,536 exceeded".into(),
            },
            SyntaxErr::AttributesExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "expected ',' or ']'".into(),
            },
            SyntaxErr::AttributesElementLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "attributes element limit of 65,536 exceeded".into(),
            },
            SyntaxErr::BlockExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "expected ';' or '}'".into(),
            },
            SyntaxErr::BlockElementLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "block element limit of 65,536 exceeded".into(),
            },
            SyntaxErr::BreakMissingLabel(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "break statement is missing a label".into(),
            },
            SyntaxErr::ContinueMissingLabel(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "continue statement is missing a label".into(),
            },
            SyntaxErr::FunctionCallExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "expected ',' or ')' after function argument".into(),
            },
            SyntaxErr::FunctionCallArgumentLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "function call argument limit of 65,536 exceeded".into(),
            },
            SyntaxErr::FunctionCallPositionFollowsNamed(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "positional argument cannot follow named argument".into(),
            },
            SyntaxErr::ForVariableBindingMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "missing name for for-loop binding".into(),
            },
            SyntaxErr::ForVariableBindingExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "expected ',' or ')' after for-loop binding".into(),
            },
            SyntaxErr::ForVariableBindingLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "for-loop binding limit of 65,536 exceeded".into(),
            },
            SyntaxErr::ForExpectedInKeyword(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "expected 'in' after for-loop binding(s)".into(),
            },
            SyntaxErr::ExpectedFieldOrMethodName(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "expected field or method name".into(),
            },
            SyntaxErr::ExpectedOpenParen(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "expected '('".into(),
            },
            SyntaxErr::ExpectedCloseParen(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "expected ')'".into(),
            },
            SyntaxErr::ExpectedOpenBrace(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "expected '{'".into(),
            },
            SyntaxErr::ExpectedOpenBracket(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "expected '['".into(),
            },
            SyntaxErr::ExpectedCloseBracket(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "expected ']'".into(),
            },
            SyntaxErr::ExpectedCloseAngle(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "expected '>'".into(),
            },
            SyntaxErr::ExpectedSemicolon(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "expected ';'".into(),
            },
            SyntaxErr::ExpectedColon(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "expected ':'".into(),
            },
            SyntaxErr::ExpectedArrow(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "expected '->'".into(),
            },
            SyntaxErr::ExpectedItem(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "expected an item".into(),
            },
            SyntaxErr::ExpectedType(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "expected a type".into(),
            },
            SyntaxErr::ExpectedExpr(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "expected an expression".into(),
            },
            SyntaxErr::ExpectedCloseBrace(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "expected '}'".into(),
            },
            SyntaxErr::SyntaxNotSupported(pos) => DiagnosticInfo {
                origin: Origin::Point(lexpos_to_source_position(pos)),
                message: "this syntax is not supported".into(),
            },
        }
    }
}
