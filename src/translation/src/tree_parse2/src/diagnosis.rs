use nitrate_diagnosis::{
    DiagnosticGroupId, DiagnosticInfo, FormattableDiagnosticGroup, Origin, SourcePosition,
};
use nitrate_token::Token;

#[allow(dead_code)]
pub(crate) enum SyntaxErr {
    ModuleExpectedName { pos: Option<SourcePosition> },

    ExpectedAttributeDelimiter { pos: Option<SourcePosition> },
    ExpectedAttributeExpression { pos: Option<SourcePosition> },

    ExpectedOpenBrace { pos: Option<SourcePosition> },
    ExpectedCloseBrace { pos: Option<SourcePosition> },
    ExpectedOpenBracket { pos: Option<SourcePosition> },
    ExpectedCloseBracket { pos: Option<SourcePosition> },

    UnexpectedToken { token: Token, pos: SourcePosition },

    ExpectedItem { pos: Option<SourcePosition> },
}

impl FormattableDiagnosticGroup for SyntaxErr {
    fn group_id(&self) -> DiagnosticGroupId {
        DiagnosticGroupId::Parse
    }

    fn variant_id(&self) -> u16 {
        match self {
            SyntaxErr::ModuleExpectedName { .. } => 0,

            SyntaxErr::ExpectedAttributeDelimiter { .. } => 10,
            SyntaxErr::ExpectedAttributeExpression { .. } => 11,

            SyntaxErr::ExpectedOpenBrace { .. } => 100,
            SyntaxErr::ExpectedCloseBrace { .. } => 101,
            SyntaxErr::ExpectedOpenBracket { .. } => 102,
            SyntaxErr::ExpectedCloseBracket { .. } => 103,

            SyntaxErr::UnexpectedToken { .. } => 200,

            SyntaxErr::ExpectedItem { .. } => 300,
        }
    }

    fn format(&self) -> nitrate_diagnosis::DiagnosticInfo {
        match self {
            SyntaxErr::ModuleExpectedName { pos } => DiagnosticInfo {
                origin: pos.to_owned().map(Origin::Point).unwrap_or(Origin::None),
                message: "Expected module name after 'mod'".to_string(),
            },

            SyntaxErr::ExpectedAttributeDelimiter { pos } => DiagnosticInfo {
                origin: pos.to_owned().map(Origin::Point).unwrap_or(Origin::None),
                message: "Expected ',' or ']' in attribute list".to_string(),
            },

            SyntaxErr::ExpectedAttributeExpression { pos } => DiagnosticInfo {
                origin: pos.to_owned().map(Origin::Point).unwrap_or(Origin::None),
                message: "Expected expression in attribute list".to_string(),
            },

            SyntaxErr::ExpectedOpenBrace { pos } => DiagnosticInfo {
                origin: pos.to_owned().map(Origin::Point).unwrap_or(Origin::None),
                message: "Expected '{'".to_string(),
            },

            SyntaxErr::ExpectedCloseBrace { pos } => DiagnosticInfo {
                origin: pos.to_owned().map(Origin::Point).unwrap_or(Origin::None),
                message: "Expected '}'".to_string(),
            },

            SyntaxErr::ExpectedOpenBracket { pos } => DiagnosticInfo {
                origin: pos.to_owned().map(Origin::Point).unwrap_or(Origin::None),
                message: "Expected '['".to_string(),
            },

            SyntaxErr::ExpectedCloseBracket { pos } => DiagnosticInfo {
                origin: pos.to_owned().map(Origin::Point).unwrap_or(Origin::None),
                message: "Expected ']'".to_string(),
            },

            SyntaxErr::UnexpectedToken { token, pos } => DiagnosticInfo {
                origin: Origin::Point(pos.to_owned()),
                message: format!("Unexpected token: {}", token),
            },

            SyntaxErr::ExpectedItem { pos } => DiagnosticInfo {
                origin: pos.to_owned().map(Origin::Point).unwrap_or(Origin::None),
                message: "Expected item".to_string(),
            },
        }
    }
}
