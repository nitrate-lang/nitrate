use nitrate_diagnosis::{DiagnosticGroupId, FormattableDiagnosticGroup};
use nitrate_hir::TypeId;
use nitrate_hir_dump::Dump;
use ordered_float::OrderedFloat;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub(crate) enum TypeErr {
    IntegerLiteralOutsizeRange {
        value: u128,
        target_type: TypeId,
    },

    IntegerLiteralUnsatisfiable {
        value: u128,
        unsatisfiable_type: TypeId,
    },

    FloatLiteralUnsatisfiable {
        value: OrderedFloat<f64>,
        unsatisfiable_type: TypeId,
    },
}

impl FormattableDiagnosticGroup for TypeErr {
    fn group_id(&self) -> DiagnosticGroupId {
        DiagnosticGroupId::Type
    }

    fn variant_id(&self) -> u16 {
        match self {
            TypeErr::IntegerLiteralOutsizeRange { .. } => 0,
            TypeErr::IntegerLiteralUnsatisfiable { .. } => 1,
            TypeErr::FloatLiteralUnsatisfiable { .. } => 2,
        }
    }

    fn format(&self) -> nitrate_diagnosis::DiagnosticInfo {
        match self {
            TypeErr::IntegerLiteralOutsizeRange { value, target_type } => {
                let message = format!(
                    "Integer literal value {} is outside the range of target type {:?}",
                    value,
                    target_type.to_string()
                );

                nitrate_diagnosis::DiagnosticInfo {
                    origin: nitrate_diagnosis::Origin::None,
                    message,
                }
            }

            TypeErr::IntegerLiteralUnsatisfiable {
                value,
                unsatisfiable_type,
            } => {
                let message = format!(
                    "Integer literal value {} has unsatisfiable non-integer type constraint {}",
                    value,
                    unsatisfiable_type.to_string()
                );

                nitrate_diagnosis::DiagnosticInfo {
                    origin: nitrate_diagnosis::Origin::None,
                    message,
                }
            }

            TypeErr::FloatLiteralUnsatisfiable {
                value,
                unsatisfiable_type,
            } => {
                let message = format!(
                    "Float literal value {} has unsatisfiable non-float type constraint {}",
                    value,
                    unsatisfiable_type.to_string()
                );

                nitrate_diagnosis::DiagnosticInfo {
                    origin: nitrate_diagnosis::Origin::None,
                    message,
                }
            }
        }
    }
}
