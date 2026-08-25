use nitrate_diagnosis::{DiagnosticExplanation, DiagnosticGroupId, DiagnosticInfo, FormattableDiagnosticGroup, Origin, SourcePosition};
use nitrate_token::LexPos;

#[allow(dead_code)]
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

/// Static explanations for every parser error code, used by `no3 --explain`.
pub fn explanations() -> &'static [DiagnosticExplanation] {
    &[
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 0,
            explanation: "A generic parameter list (e.g. `fn foo<T, U>`) is missing a parameter name. \
                           Every generic parameter must have a name so it can be referred to elsewhere in the declaration.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 1,
            explanation: "The generic parameter list exceeded the 65,536-parameter limit. \
                           Generics lists are bounded to prevent pathological inputs; split the declaration into fewer parameters.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 2,
            explanation: "The parser expected `>` or `,` to terminate a generic parameter list. \
                           Generic parameters are separated by commas and the list must be closed with `>`.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 20,
            explanation: "A `module` declaration is missing its name. Write the module name after the `module` keyword.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 21,
            explanation: "A module declaration exceeded the 65,536-item limit. \
                           Split the module into multiple modules and import them.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 22,
            explanation: "The parser expected `}` at the end of a `module` block. \
                           Every module body must be closed with a closing brace.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 41,
            explanation: "A `use` import declaration is missing its alias name. \
                           An alias is written as `use path as Alias;`.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 42,
            explanation: "The parser expected `*` or `{` after `::` in a `use` declaration. \
                           Glob imports use `use path::*;` and grouped imports use `use path::{a, b};`.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 43,
            explanation: "The parser expected `}` at the end of a grouped `use` declaration. \
                           Close the import group with a closing brace.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 60,
            explanation: "A `type` alias declaration is missing its name. \
                           Write the alias name after the `type` keyword, e.g. `type MyInt = i32;`.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 80,
            explanation: "An `enum` declaration is missing its name. \
                           Write the enum name after the `enum` keyword.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 81,
            explanation: "An enum declaration exceeded the 65,536-variant limit. \
                           Split the enum into multiple types.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 82,
            explanation: "An enum variant is missing its name. \
                           Every variant must have a name, e.g. `enum Color { Red, Green, Blue }`.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 83,
            explanation: "The parser expected `}` or `,` inside an enum body. \
                           Variants are separated by commas and the body is closed with `}`.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 100,
            explanation: "A `struct` declaration is missing its name. \
                           Write the struct name after the `struct` keyword.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 101,
            explanation: "A struct declaration exceeded the 65,536-field limit. \
                           Split the struct into nested structs.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 102,
            explanation: "A struct field is missing its name. \
                           Every field must have a name, e.g. `struct Point { x: i32, y: i32 }`.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 103,
            explanation: "The parser expected `}` or `,` inside a struct body. \
                           Fields are separated by commas and the body is closed with `}`.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 120,
            explanation: "A `fn` declaration is missing its name. \
                           Write the function name after the `fn` keyword.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 122,
            explanation: "A function declaration exceeded the 65,536-parameter limit. \
                           Split the function into multiple functions or pass a struct of parameters.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 123,
            explanation: "A function parameter is missing its name. \
                           Every parameter needs a name, e.g. `fn foo(x: i32, y: i32)`.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 124,
            explanation: "The parser expected `)` or `,` in a function parameter list. \
                           Parameters are separated by commas and the list is closed with `)`.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 126,
            explanation: "A variadic function parameter must use the `...` syntax, e.g. `fn printf(fmt: *const u8, ...)`. \
                           The `...` marks the point where additional variadic arguments are accepted.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 140,
            explanation: "A variable declaration is missing its name. \
                           Write the variable name after `let` or `var`.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 180,
            explanation: "A `trait` declaration is missing its name. \
                           Write the trait name after the `trait` keyword.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 181,
            explanation: "A trait declaration exceeded the 65,536-item limit. \
                           Split the trait into multiple traits.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 182,
            explanation: "Traits may only contain associated constants, type aliases, and function signatures. \
                           Definitions with bodies or other item kinds are not allowed inside a trait.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 183,
            explanation: "The parser expected `}` at the end of a trait body. \
                           Close the trait body with a closing brace.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 200,
            explanation: "An `impl` block is missing its `for` clause. \
                           Write `impl Trait for Type { ... }`.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 201,
            explanation: "The parser expected `}` at the end of an `impl` block. \
                           Close the impl body with a closing brace.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 202,
            explanation: "An `impl` block exceeded the 65,536-item limit. \
                           Split the impl into multiple impl blocks.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 203,
            explanation: "`impl` blocks cannot have visibility modifiers. \
                           Remove the `pub`/`private` keyword from the impl declaration.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 222,
            explanation: "The parser expected `>` or `,` inside a generic argument list. \
                           Generic arguments are separated by commas and closed with `>`.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 223,
            explanation: "A generic argument list exceeded the 65,536-argument limit.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 224,
            explanation: "The parser expected an identifier or `::` in a path. \
                           Path segments are separated by `::`, e.g. `std::io::println`.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 225,
            explanation: "A path exceeded the 65,536-segment limit.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 226,
            explanation: "A path segment is missing its name. \
                           Every segment in a path must be a valid identifier.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 240,
            explanation: "A reference type is missing its lifetime name. \
                           Write the lifetime after the `&` marker, e.g. `&'a T`.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 260,
            explanation: "The parser expected a field name or `}` inside a struct literal. \
                           Struct literals list fields as `Name: value` separated by commas.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 261,
            explanation: "The parser expected a field name in a struct literal. \
                           Every entry must name the field it initializes.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 280,
            explanation: "The parser expected `)` or `,` inside a tuple type. \
                           Tuple types list element types separated by commas and close with `)`.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 281,
            explanation: "A tuple type exceeded the 65,536-element limit.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 300,
            explanation: "The parser expected `,` or `]` inside a list. \
                           Lists separate elements with commas and close with `]`.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 301,
            explanation: "A list exceeded the 65,536-element limit.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 320,
            explanation: "The parser expected `,` or `]` inside an attribute list. \
                           Attributes are written as `[attr]` or `[attr1, attr2]` before an item.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 321,
            explanation: "An attribute list exceeded the 65,536-element limit.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 340,
            explanation: "The parser expected `}` at the end of a block. \
                           Every block body must be closed with a closing brace.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 341,
            explanation: "A block exceeded the 65,536-statement limit.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 360,
            explanation: "A `break` statement is missing its label. \
                           When breaking out of a labeled loop, the label must follow `break`, e.g. `break 'outer`.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 380,
            explanation: "A `continue` statement is missing its label. \
                           When continuing a labeled loop, the label must follow `continue`, e.g. `continue 'outer`.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 400,
            explanation: "The parser expected `)` at the end of a function call. \
                           Call arguments are closed with `)`.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 401,
            explanation: "A function call exceeded the 65,536-argument limit.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 402,
            explanation: "Positional arguments cannot follow named arguments in a function call. \
                           Place all positional arguments before any `name: value` arguments.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 440,
            explanation: "A `for`-loop binding is missing its variable name. \
                           Write `for x in iterable { ... }`.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 441,
            explanation: "The parser expected the end of the `for`-loop binding list. \
                           Bindings are separated by commas, e.g. `for (a, b) in pairs`.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 442,
            explanation: "A `for`-loop binding list exceeded the 65,536-binding limit.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 443,
            explanation: "The parser expected the `in` keyword after the `for`-loop bindings. \
                           Write `for x in iterable { ... }`.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 500,
            explanation: "The parser expected a field or method name after a `.` in an access expression. \
                           Write the field or method name after the dot.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 1000,
            explanation: "The parser expected `(` here. \
                           Check the expression or declaration for a missing opening parenthesis.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 1001,
            explanation: "The parser expected `)` here. \
                           Check the expression or parameter list for a missing closing parenthesis.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 1002,
            explanation: "The parser expected `{` here. \
                           Check the block, struct literal, or declaration for a missing opening brace.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 1003,
            explanation: "The parser expected `}` here. \
                           Check the block or declaration for a missing closing brace.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 1004,
            explanation: "The parser expected `[` here. \
                           Check the list, array type, or index expression for a missing opening bracket.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 1005,
            explanation: "The parser expected `]` here. \
                           Check the list or array type for a missing closing bracket.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 1007,
            explanation: "The parser expected `>` here. \
                           Check the generic argument list or comparison expression for a missing closing angle bracket.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 1008,
            explanation: "The parser expected `;` here. \
                           Most statements must be terminated with a semicolon.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 1009,
            explanation: "The parser expected `:` here. \
                           Check the type annotation, struct literal, or named argument for a missing colon.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 1010,
            explanation: "The parser expected `->` here. \
                           Function return types are written after an arrow, e.g. `fn foo() -> i32`.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 2000,
            explanation: "The parser expected an item here (a function, struct, enum, module, etc.). \
                           Top-level declarations must be valid items.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 2001,
            explanation: "The parser expected a type here. \
                           Check the type annotation or type argument for a missing or malformed type.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 2002,
            explanation: "The parser expected an expression here. \
                           Check the statement or operand for a missing or malformed expression.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::Parse,
            variant_id: 2020,
            explanation: "The parser encountered syntax that is recognized but not yet supported. \
                           This usually means the feature exists in the grammar but is not implemented; \
                           check the Nitrate documentation or remove the offending construct.",
        },
    ]
}

