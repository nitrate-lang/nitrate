use nitrate_diagnosis::{DiagnosticGroupId, FormattableDiagnosticGroup};

pub(crate) enum Issue {
    Mismatch,
}

impl FormattableDiagnosticGroup for Issue {
    fn group_id(&self) -> DiagnosticGroupId {
        DiagnosticGroupId::Semantic
    }

    fn variant_id(&self) -> u16 {
        match self {
            Issue::Mismatch => 0,
        }
    }

    fn format(&self) -> nitrate_diagnosis::DiagnosticInfo {
        match self {
            Issue::Mismatch => nitrate_diagnosis::DiagnosticInfo {
                origin: nitrate_diagnosis::Origin::Unknown,
                message: "Type mismatch".into(),
            },
        }
    }
}
