use nitrate_diagnosis::{
    DiagnosticGroupId, DiagnosticInfo, FormattableDiagnosticGroup, Origin, 
};
use nitrate_tree::{ast::{ExprPath, TypePath}, tag::ImportNameId};
use nitrate_token::SourcePosition;

pub(crate) enum SyntaxErr {
    GenericMissingParameterName(SourcePosition),
    GenericParameterLimit(SourcePosition),
    GenericParameterExpectedEnd(SourcePosition),

    ModuleMissingName(SourcePosition),
    ModuleItemLimit(SourcePosition),
    ModuleExpectedEnd(SourcePosition),

    ImportMissingName(SourcePosition),

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
    FunctionParameterExpectedType(SourcePosition),

    VariableMissingName(SourcePosition),

    TraitMissingName(SourcePosition),
    TraitItemLimit(SourcePosition),
    TraitDoesNotAllowItem(SourcePosition),
    TraitExpectedEnd(SourcePosition),

    ImplMissingFor(SourcePosition),
    ImplExpectedEnd(SourcePosition),
    ImplItemLimit(SourcePosition),
    ImplCannotBeVisible(SourcePosition),
    
    PathGenericArgumentExpectedEnd(SourcePosition),
    PathGenericArgumentLimit(SourcePosition),
    PathExpectedNameOrSeparator(SourcePosition),
    PathSegmentLimit(SourcePosition),
    PathExpectedName(SourcePosition),

    ReferenceTypeExpectedLifetimeName(SourcePosition),


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
    FunctionCallPositionFollowsNamed(SourcePosition),

    ForVariableBindingMissingName(SourcePosition),
    ForVariableBindingExpectedEnd(SourcePosition),
    ForVariableBindingLimit(SourcePosition),
    ForExpectedInKeyword(SourcePosition),

    ExpectedFieldOrMethodName(SourcePosition),

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

            SyntaxErr::ImportMissingName(_) => 40,

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
            SyntaxErr::FunctionParameterExpectedType(_) => 125,

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

            SyntaxErr::ImportMissingName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "import package name is missing".into(),
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

            SyntaxErr::FunctionParameterExpectedType(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "function parameter type is missing".into(),
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

            SyntaxErr::ImplCannotBeVisible(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "impl blocks cannot have visibility modifiers".into(),
            },

            /* ------------------------------------------------------------------------- */

            SyntaxErr::PathGenericArgumentExpectedEnd(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "expected '>' or ',' in generic arguments".into(),
            },

            SyntaxErr::PathGenericArgumentLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "generic argument limit of 65,536 exceeded".into(),
            },

            SyntaxErr::PathExpectedNameOrSeparator(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "expected identifier or '::' in path".into(),
            },

            SyntaxErr::PathSegmentLimit(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "path segment limit of 65,536 exceeded".into(),
            },

            SyntaxErr::PathExpectedName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "path segment name is missing".into(),
            },

            /* ------------------------------------------------------------------------- */

            SyntaxErr::ReferenceTypeExpectedLifetimeName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "reference lifetime is missing after '".into(),
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

            SyntaxErr::FunctionCallPositionFollowsNamed(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "positional argument cannot follow named argument".into(),
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

            SyntaxErr::ExpectedFieldOrMethodName(pos) => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned().into()),
                message: "expected field or method name".into(),
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


pub enum ResolveIssue {
    ExprPathUnresolved(ExprPath),

    TypePathUnresolved(TypePath),

    ImportNotFound((String, std::io::Error)),
    
    CircularImport {
        path: ImportNameId,
        depth: Vec<ImportNameId>,
    },

    ImportSourceCodeSizeLimitExceeded(std::path::PathBuf),
    ImportDepthLimitExceeded(String),
}

impl FormattableDiagnosticGroup for ResolveIssue {
    fn group_id(&self) -> DiagnosticGroupId {
        DiagnosticGroupId::Resolve
    }

    fn variant_id(&self) -> u16 {
        match self {
            ResolveIssue::ExprPathUnresolved(_) => 1,
            ResolveIssue::TypePathUnresolved(_) => 20,
            ResolveIssue::ImportNotFound(_) => 40,
            ResolveIssue::CircularImport { .. } => 41,
            ResolveIssue::ImportSourceCodeSizeLimitExceeded(_) => 42,
            ResolveIssue::ImportDepthLimitExceeded(_) => 43,
        }
    }

    fn format(&self) -> nitrate_diagnosis::DiagnosticInfo {
        match self {
            ResolveIssue::ExprPathUnresolved(path) => DiagnosticInfo {
                origin: Origin::None,
                message: format!(
                    "Unresolved expression path: {}",
                    path.segments
                        .iter()
                        .map(|s| s.name.to_owned())
                        .collect::<Vec<_>>()
                        .join("::"),
                ),
            },

            ResolveIssue::TypePathUnresolved(path) => DiagnosticInfo {
                origin: Origin::None,
                message: format!(
                    "Unresolved type path: {}",
                    path.segments
                        .iter()
                        .map(|s| s.name.to_owned())
                        .collect::<Vec<_>>()
                        .join("::"),
                ),
            },

            ResolveIssue::ImportNotFound(path) => DiagnosticInfo {
                origin: Origin::None,
                message: format!("Module not found: {} ({})", path.0, path.1),
            },

            ResolveIssue::CircularImport { path, depth } => DiagnosticInfo {
                origin: Origin::None,
                message: format!(
                    "Circular import detected: {}\nImport depth:\n{}",
                    path,
                    depth
                        .iter()
                        .map(|p| format!(" - {}", p))
                        .collect::<Vec<_>>()
                        .join("\n")
                ),
            },

            ResolveIssue::ImportSourceCodeSizeLimitExceeded(path) => DiagnosticInfo {
                origin: Origin::None,
                message: format!(
                    "Imported module ({}) exceeded the source code file size limit.",
                    path.display()
                ),
            },

            ResolveIssue::ImportDepthLimitExceeded(path) => DiagnosticInfo {
                origin: Origin::None,
                message: format!(
                    "Import depth limit of 256 exceeded while importing module: {}",
                    path
                ),
            },
        }
    }
}

