use nitrate_diagnosis::{DiagnosticGroupId, DiagnosticInfo, FormattableDiagnosticGroup, Origin};

pub(crate) enum HirErr {
    UnspecifiedError,
}

impl FormattableDiagnosticGroup for HirErr {
    fn group_id(&self) -> DiagnosticGroupId {
        DiagnosticGroupId::Hir
    }

    fn variant_id(&self) -> u16 {
        match self {
            HirErr::UnspecifiedError => 0,
        }
    }

    fn format(&self) -> nitrate_diagnosis::DiagnosticInfo {
        match self {
            HirErr::UnspecifiedError => DiagnosticInfo {
                message: "unspecified error".to_string(),
                origin: Origin::None,
            },
            /* ------------------------------------------------------------------------- */
        }
    }
}
