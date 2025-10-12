use nitrate_diagnosis::{DiagnosticGroupId, DiagnosticInfo, FormattableDiagnosticGroup, Origin};

pub(crate) enum HirErr {
    UnsupportedConstruct,
}

impl FormattableDiagnosticGroup for HirErr {
    fn group_id(&self) -> DiagnosticGroupId {
        DiagnosticGroupId::Syntax
    }

    fn variant_id(&self) -> u16 {
        match self {
            HirErr::UnsupportedConstruct => 0,
        }
    }

    fn format(&self) -> nitrate_diagnosis::DiagnosticInfo {
        match self {
            HirErr::UnsupportedConstruct => DiagnosticInfo {
                message: "unsupported construct".to_string(),
                origin: Origin::None,
            },
            /* ------------------------------------------------------------------------- */
        }
    }
}
