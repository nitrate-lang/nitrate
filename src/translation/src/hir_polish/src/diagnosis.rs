use nitrate_diagnosis::{DiagnosticGroupId, FormattableDiagnosticGroup};

pub(crate) enum TypeErr {
    Mismatch,
}

impl FormattableDiagnosticGroup for TypeErr {
    fn group_id(&self) -> DiagnosticGroupId {
        DiagnosticGroupId::Type
    }

    fn variant_id(&self) -> u16 {
        match self {
            TypeErr::Mismatch => 0,
        }
    }

    fn format(&self) -> nitrate_diagnosis::DiagnosticInfo {
        match self {
            TypeErr::Mismatch => nitrate_diagnosis::DiagnosticInfo {
                origin: nitrate_diagnosis::Origin::Unknown,
                message: "Type mismatch".into(),
            },
        }
    }
}
