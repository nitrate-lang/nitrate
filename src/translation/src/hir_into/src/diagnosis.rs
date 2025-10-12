use nitrate_diagnosis::{DiagnosticGroupId, DiagnosticInfo, FormattableDiagnosticGroup, Origin};

pub(crate) enum HirErr {
    UnspecifiedError,
    UnrecognizedModuleAttribute,
    UnimplementedFeature(String),
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
        }
    }
}
