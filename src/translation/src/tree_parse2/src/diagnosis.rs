use nitrate_diagnosis::{
    DiagnosticGroupId, DiagnosticInfo, FormattableDiagnosticGroup, Origin, SourcePosition,
};

pub(crate) enum SyntaxErr {
    ModuleExpectedName { pos: Option<SourcePosition> },
    ExpectedOpenBrace { pos: Option<SourcePosition> },
    ExpectedCloseBrace { pos: Option<SourcePosition> },
}

impl FormattableDiagnosticGroup for SyntaxErr {
    fn group_id(&self) -> DiagnosticGroupId {
        DiagnosticGroupId::Parse
    }

    fn variant_id(&self) -> u16 {
        match self {
            SyntaxErr::ModuleExpectedName { .. } => 0,

            SyntaxErr::ExpectedOpenBrace { .. } => 100,
            SyntaxErr::ExpectedCloseBrace { .. } => 101,
        }
    }

    fn format(&self) -> nitrate_diagnosis::DiagnosticInfo {
        match self {
            SyntaxErr::ModuleExpectedName { pos } => DiagnosticInfo {
                origin: pos.to_owned().map(Origin::Point).unwrap_or(Origin::None),
                message: "Expected module name after 'mod'".to_string(),
            },

            SyntaxErr::ExpectedOpenBrace { pos } => DiagnosticInfo {
                origin: pos.to_owned().map(Origin::Point).unwrap_or(Origin::None),
                message: "Expected '{'".to_string(),
            },

            SyntaxErr::ExpectedCloseBrace { pos } => DiagnosticInfo {
                origin: pos.to_owned().map(Origin::Point).unwrap_or(Origin::None),
                message: "Expected '}'".to_string(),
            },
        }
    }
}
