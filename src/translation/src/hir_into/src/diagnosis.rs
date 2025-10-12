use nitrate_diagnosis::{DiagnosticGroupId, DiagnosticInfo, FormattableDiagnosticGroup, Origin};

pub(crate) enum HirErr {
    UnspecifiedError,
    UnrecognizedModuleAttribute,
    UnimplementedFeature(String),
    UnrecognizedGlobalVariableAttribute,
    GlobalVariableMustBeConstOrStatic,
    GlobalVariableMustHaveInitializer,
}

impl FormattableDiagnosticGroup for HirErr {
    fn group_id(&self) -> DiagnosticGroupId {
        DiagnosticGroupId::Hir
    }

    fn variant_id(&self) -> u16 {
        match self {
            HirErr::UnspecifiedError => 0,
            HirErr::UnrecognizedModuleAttribute => 1,
            HirErr::UnimplementedFeature(_) => 2,
            HirErr::UnrecognizedGlobalVariableAttribute => 3,
            HirErr::GlobalVariableMustBeConstOrStatic => 4,
            HirErr::GlobalVariableMustHaveInitializer => 5,
        }
    }

    fn format(&self) -> nitrate_diagnosis::DiagnosticInfo {
        match self {
            HirErr::UnspecifiedError => DiagnosticInfo {
                message: "unspecified error".to_string(),
                origin: Origin::None,
            },

            HirErr::UnrecognizedModuleAttribute => DiagnosticInfo {
                message: "unrecognized module attribute".to_string(),
                origin: Origin::None,
            },

            HirErr::UnimplementedFeature(feature) => DiagnosticInfo {
                message: format!("unimplemented feature: {}", feature),
                origin: Origin::None,
            },

            HirErr::UnrecognizedGlobalVariableAttribute => DiagnosticInfo {
                message: "unrecognized global variable attribute".to_string(),
                origin: Origin::None,
            },

            HirErr::GlobalVariableMustBeConstOrStatic => DiagnosticInfo {
                message: "global variable must be 'const' or 'static'".to_string(),
                origin: Origin::None,
            },

            HirErr::GlobalVariableMustHaveInitializer => DiagnosticInfo {
                message: "global variable must have an initializer".to_string(),
                origin: Origin::None,
            },
        }
    }
}
