use nitrate_diagnosis::{
    DiagnosticGroupId, DiagnosticInfo, FormattableDiagnosticGroup, Origin, 
};
use nitrate_tokenize::SourcePosition;

pub(crate) enum SyntaxErr {
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
    FunctionParameterLimit(SourcePosition),
    FunctionParameterMissingName(SourcePosition),
    FunctionParametersExpectedEnd(SourcePosition),

    VariableMissingName(SourcePosition),

    TraitMissingName(SourcePosition),
    TraitItemLimit(SourcePosition),
    TraitDoesNotAllowItem(SourcePosition),
    TraitExpectedEnd(SourcePosition),

    ImplMissingFor(SourcePosition),
    ImplExpectedEnd(SourcePosition),
    ImplItemLimit(SourcePosition),

    
    PathIsEmpty(SourcePosition),
    PathUnexpectedScopeSeparator(SourcePosition),
    PathTrailingScopeSeparator(SourcePosition),
    PathGenericArgumentExpectedEnd(SourcePosition),
    PathGenericArgumentLimit(SourcePosition),

    ReferenceTypeExpectedLifetimeName(SourcePosition),

    OpaqueTypeMissingName(SourcePosition),

    TupleTypeExpectedEnd(SourcePosition),
    TupleTypeElementLimit(SourcePosition),

    ListExpectedEnd(SourcePosition),
    ListElementLimit(SourcePosition),

    AttributesExpectedEnd(SourcePosition),
    AttributesElementLimit(SourcePosition),

    BlockExpectedEnd(SourcePosition),
    BlockElementLimit(SourcePosition),

    BreakMissingLabel(SourcePosition),

    ContinueMissingLabel(SourcePosition),

    FunctionCallExpectedEnd(SourcePosition),
    FunctionCallArgumentLimit(SourcePosition),

    DoWhileExpectedWhileKeyword(SourcePosition),

    ForVariableBindingMissingName(SourcePosition),
    ForVariableBindingExpectedEnd(SourcePosition),
    ForVariableBindingLimit(SourcePosition),
    ForExpectedInKeyword(SourcePosition),

    ExpectedOpenParen(SourcePosition),
    ExpectedCloseParen(SourcePosition),
    ExpectedOpenBrace(SourcePosition),
    #[allow(dead_code)]
    ExpectedCloseBrace(SourcePosition),
    ExpectedOpenBracket(SourcePosition),
    ExpectedCloseBracket(SourcePosition),
    #[allow(dead_code)]
    ExpectedOpenAngle(SourcePosition),
    ExpectedCloseAngle(SourcePosition),
    ExpectedSemicolon(SourcePosition),
    ExpectedColon(SourcePosition),
    ExpectedArrow(SourcePosition),

    ExpectedItem(SourcePosition),
    ExpectedType(SourcePosition),
    ExpectedExpr(SourcePosition),

    #[allow(dead_code)]
    SyntaxNotSupported(SourcePosition),
}

impl FormattableDiagnosticGroup for SyntaxErr {
    fn group_id(&self) -> DiagnosticGroupId {
        DiagnosticGroupId::Syntax
    }

    fn variant_id(&self) -> u16 {
        match self {
            SyntaxErr::GenericMissingParameterName(_) => 0,
            SyntaxErr::GenericParameterLimit(_) => 1,
            SyntaxErr::GenericParameterExpectedEnd(_) => 2,

            SyntaxErr::ModuleMissingName(_) => 20,
            SyntaxErr::ModuleItemLimit(_) => 21,
            SyntaxErr::ModuleExpectedEnd(_) => 22,

            SyntaxErr::ImportMissingAliasName(_) => 40,

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

            SyntaxErr::VariableMissingName(_) => 140,

            SyntaxErr::TraitMissingName(_) => 180,
            SyntaxErr::TraitItemLimit(_) => 181,
            SyntaxErr::TraitDoesNotAllowItem(_) => 182,
            SyntaxErr::TraitExpectedEnd(_) => 183,

            SyntaxErr::ImplMissingFor(_) => 200,
            SyntaxErr::ImplExpectedEnd(_) => 201,
            SyntaxErr::ImplItemLimit(_) => 202,

            SyntaxErr::PathIsEmpty(_) => 220,
            SyntaxErr::PathTrailingScopeSeparator(_) => 221,
            SyntaxErr::PathGenericArgumentExpectedEnd(_) => 222,
            SyntaxErr::PathGenericArgumentLimit(_) => 223,
            SyntaxErr::PathUnexpectedScopeSeparator(_) => 224,

            SyntaxErr::ReferenceTypeExpectedLifetimeName(_) => 240,

            SyntaxErr::OpaqueTypeMissingName(_) => 260,

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

            SyntaxErr::DoWhileExpectedWhileKeyword(_) => 420,

            SyntaxErr::ForVariableBindingMissingName(_) => 440,
            SyntaxErr::ForVariableBindingExpectedEnd(_) => 441,
            SyntaxErr::ForVariableBindingLimit(_) => 442,
            SyntaxErr::ForExpectedInKeyword(_) => 443,

            SyntaxErr::ExpectedOpenParen(_) => 1000,
            SyntaxErr::ExpectedCloseParen(_) => 1001,
            SyntaxErr::ExpectedOpenBrace(_) => 1002,
            SyntaxErr::ExpectedCloseBrace(_) => 1003,
            SyntaxErr::ExpectedOpenBracket(_) => 1004,
            SyntaxErr::ExpectedCloseBracket(_) => 1005,
            SyntaxErr::ExpectedOpenAngle(_) => 1006,
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

    fn format(&self) -> nitrate_diagnosis::DiagnosticInfo {
        match self {
            SyntaxErr::GenericMissingParameterName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "generic parameter name is missing".into(),
            },

            SyntaxErr::GenericParameterLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "generic parameter limit of 65,536 exceeded".into(),
            },

            SyntaxErr::GenericParameterExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "expected a '>' or ','".into(),
            },

            /* ------------------------------------------------------------------------- */

            SyntaxErr::ModuleMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "module name is missing".into(),
            },

            SyntaxErr::ModuleItemLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "module item limit of 65,536 exceeded".into(),
            },

            SyntaxErr::ModuleExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "expected '}'".into(),
            },

            /* ------------------------------------------------------------------------- */

            SyntaxErr::ImportMissingAliasName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "import alias name is missing".into(),
            },

            /* ------------------------------------------------------------------------- */

            SyntaxErr::TypeAliasMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "type alias name is missing".into(),
            },

            /* ------------------------------------------------------------------------- */

            SyntaxErr::EnumMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "enum name is missing".into(),
            },

            SyntaxErr::EnumVariantLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "enum variant limit of 65,536 exceeded".into(),
            },

            SyntaxErr::EnumMissingVariantName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "enum variant name is missing".into(),
            },

            SyntaxErr::EnumExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "expected '}' or ','".into(),
            },

            /* ------------------------------------------------------------------------- */

            SyntaxErr::StructureMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "structure name is missing".into(),
            },

            SyntaxErr::StructureFieldLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "structure field limit of 65,536 exceeded".into(),
            },

            SyntaxErr::StructureMissingFieldName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "structure field name is missing".into(),
            },

            SyntaxErr::StructureExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "expected '}' or ','".into(),
            },

            /* ------------------------------------------------------------------------- */

            SyntaxErr::FunctionMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "function name is missing".into(),
            },

            SyntaxErr::FunctionParameterLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "function parameter limit of 65,536 exceeded".into(),
            },

            SyntaxErr::FunctionParameterMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "function parameter name is missing".into(),
            },

            SyntaxErr::FunctionParametersExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "expected ')' or ','".into(),
            },

            /* ------------------------------------------------------------------------- */

            SyntaxErr::VariableMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "variable name is missing".into(),
            },

            /* ------------------------------------------------------------------------- */

            SyntaxErr::TraitMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "trait name is missing".into(),
            },

            SyntaxErr::TraitItemLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "trait item limit of 65,536 exceeded".into(),
            },

            SyntaxErr::TraitDoesNotAllowItem(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "only associated constants, type aliases, and function signatures are allowed in traits"
                    .into(),
            },

            SyntaxErr::TraitExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "expected '}'".into(),
            },

            /* ------------------------------------------------------------------------- */

            SyntaxErr::ImplMissingFor(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "expected 'for' in impl declaration".into(),
            },

            SyntaxErr::ImplExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "expected '}'".into(),
            },

            SyntaxErr::ImplItemLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "impl item limit of 65,536 exceeded".into(),
            },

            /* ------------------------------------------------------------------------- */

            SyntaxErr::PathIsEmpty(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "path is empty".into(),
            },

            SyntaxErr::PathUnexpectedScopeSeparator(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "unexpected '::' in path".into(),
            },

            SyntaxErr::PathTrailingScopeSeparator(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "trailing '::' in path".into(),
            },

            SyntaxErr::PathGenericArgumentExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "expected '>' or ',' in generic arguments".into(),
            },

            SyntaxErr::PathGenericArgumentLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "generic argument limit of 65,536 exceeded".into(),
            },

            /* ------------------------------------------------------------------------- */

            SyntaxErr::ReferenceTypeExpectedLifetimeName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "reference lifetime is missing after '".into(),
            },

            /* ------------------------------------------------------------------------- */

            SyntaxErr::OpaqueTypeMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "opaque type name is missing".into(),
            },
            
            /* ------------------------------------------------------------------------- */

            SyntaxErr::TupleTypeExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "expected ')' or ','".into(),
            },

            SyntaxErr::TupleTypeElementLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "tuple element limit of 65,536 exceeded".into(),
            },

            /* ------------------------------------------------------------------------- */

            SyntaxErr::ListExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "expected ',' or ']'".into(),
            },

            SyntaxErr::ListElementLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "list element limit of 65,536 exceeded".into(),
            },

            /* ------------------------------------------------------------------------- */

            SyntaxErr::AttributesExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "expected ',' or ']'".into(),
            },

            SyntaxErr::AttributesElementLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "attributes element limit of 65,536 exceeded".into(),
            },

            /* ------------------------------------------------------------------------- */

            SyntaxErr::BlockExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "expected ';' or '}'".into(),
            },

            SyntaxErr::BlockElementLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "block element limit of 65,536 exceeded".into(),
            },

            /* ------------------------------------------------------------------------- */

            SyntaxErr::BreakMissingLabel(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "break statement is missing a label".into(),
            },

            /* ------------------------------------------------------------------------- */

            SyntaxErr::ContinueMissingLabel(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "continue statement is missing a label".into(),
            },

            /* ------------------------------------------------------------------------- */

            SyntaxErr::FunctionCallExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "expected ',' or ')' after function argument".into(),
            },

            SyntaxErr::FunctionCallArgumentLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "function call argument limit of 65,536 exceeded".into(),
            },

            /* ------------------------------------------------------------------------- */

            SyntaxErr::DoWhileExpectedWhileKeyword(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "expected 'while' after 'do' block".into(),
            },

            /* ------------------------------------------------------------------------- */

            SyntaxErr::ForVariableBindingMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "missing name for for-loop binding".into(),
            },

            SyntaxErr::ForVariableBindingExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "expected ',' or ')' after for-loop binding".into(),
            },

            SyntaxErr::ForVariableBindingLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "for-loop binding limit of 65,536 exceeded".into(),
            },

            SyntaxErr::ForExpectedInKeyword(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "expected 'in' after for-loop binding(s)".into(),
            },

            /* ------------------------------------------------------------------------- */

            SyntaxErr::ExpectedOpenParen(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "expected '('".into(),
            },

            SyntaxErr::ExpectedCloseParen(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "expected ')'".into(),
            },

            SyntaxErr::ExpectedOpenBrace(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "expected '{'".into(),
            },

            SyntaxErr::ExpectedCloseBrace(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "expected '}'".into(),
            },

            SyntaxErr::ExpectedOpenBracket(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "expected '['".into(),
            },

            SyntaxErr::ExpectedCloseBracket(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "expected ']'".into(),
            },

            SyntaxErr::ExpectedOpenAngle(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "expected '<'".into(),
            },

            SyntaxErr::ExpectedCloseAngle(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "expected '>'".into(),
            },

            SyntaxErr::ExpectedSemicolon(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "expected ';'".into(),
            },

            SyntaxErr::ExpectedColon(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "expected ':'".into(),
            },

            SyntaxErr::ExpectedArrow(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "expected '->'".into(),
            },

            /* ------------------------------------------------------------------------- */

            SyntaxErr::ExpectedItem(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "expected an item".into(),
            },

            SyntaxErr::ExpectedType(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "expected a type".into(),
            },

            SyntaxErr::ExpectedExpr(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "expected an expression".into(),
            },

            /* ------------------------------------------------------------------------- */

            SyntaxErr::SyntaxNotSupported(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "this syntax is not supported".into(),
            }
        }
    }
}
