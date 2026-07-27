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

    /// A literal value falls outside the declared refinement bounds.
    /// The target_type is the Refine type with its bounds.
    IntegerLiteralOutOfRefinementBounds {
        value: u128,
        refinement_type: TypeId,
    },

    /// An arithmetic operation produces a result that cannot be guaranteed
    /// to satisfy the refinement bounds of the result type.
    OperationResultOutOfRefinementBounds {
        refinement_type: TypeId,
        computed_min: u128,
        computed_max: u128,
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
            TypeErr::IntegerLiteralOutOfRefinementBounds { .. } => 3,
            TypeErr::OperationResultOutOfRefinementBounds { .. } => 4,
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

            TypeErr::IntegerLiteralOutOfRefinementBounds { value, refinement_type } => {
                let message = format!(
                    "Integer literal value {} does not satisfy refinement type bounds {}",
                    value,
                    refinement_type.to_string()
                );
                nitrate_diagnosis::DiagnosticInfo {
                    origin: nitrate_diagnosis::Origin::None,
                    message,
                }
            }

            TypeErr::OperationResultOutOfRefinementBounds {
                refinement_type,
                computed_min,
                computed_max,
            } => {
                let message = format!(
                    "Arithmetic operation result [{}, {}] cannot be guaranteed to satisfy refinement type bounds {}",
                    computed_min,
                    computed_max,
                    refinement_type.to_string()
                );
                nitrate_diagnosis::DiagnosticInfo {
                    origin: nitrate_diagnosis::Origin::None,
                    message,
                }
            }
        }
    }
}
