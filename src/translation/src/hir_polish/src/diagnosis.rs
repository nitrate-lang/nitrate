use nitrate_diagnosis::{DiagnosticGroupId, FormattableDiagnosticGroup};
use nitrate_hir::TypeId;

pub(crate) enum TypeErr {
    IntegerLiteralOutsizeRange { value: u128, target_type: TypeId },
}

impl FormattableDiagnosticGroup for TypeErr {
    fn group_id(&self) -> DiagnosticGroupId {
        DiagnosticGroupId::Type
    }

    fn variant_id(&self) -> u16 {
        match self {
            TypeErr::IntegerLiteralOutsizeRange { .. } => 0,
        }
    }

    fn format(&self) -> nitrate_diagnosis::DiagnosticInfo {
        match self {
            TypeErr::IntegerLiteralOutsizeRange { value, target_type } => {
                let message = format!(
                    "Integer literal value {} is outside the range of target type {:?}",
                    value, target_type
                );

                nitrate_diagnosis::DiagnosticInfo {
                    origin: nitrate_diagnosis::Origin::Unknown,
                    message,
                }
            }
        }
    }
}
