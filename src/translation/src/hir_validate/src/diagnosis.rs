use std::ops::Deref;

use nitrate_diagnosis::{DiagnosticGroupId, FormattableDiagnosticGroup};
use nitrate_hir::{LiteralId, TraitId, TypeId};
use nitrate_hir_dump::Dump;
use nitrate_nstring::NString;

pub(crate) enum Issue {
    TypeDoesNotImplementTrait {
        type_id: TypeId,
        trait_id: TraitId,
    },
    TypeMismatch {
        expected: TypeId,
        found: TypeId,
    },
    UnsupportedAlignment {
        alignment: u32,
        max_supported: u32,
    },
    UninferredTypeResidue,
    FunctionTypeDuplicateParameterName {
        name: NString,
        function: TypeId,
    },
    RefinementMinimumGreaterThanMaximum {
        min: LiteralId,
        max: LiteralId,
        type_id: TypeId,
    },
}

impl FormattableDiagnosticGroup for Issue {
    fn group_id(&self) -> DiagnosticGroupId {
        DiagnosticGroupId::Semantic
    }

    fn variant_id(&self) -> u16 {
        match self {
            Issue::TypeDoesNotImplementTrait { .. } => 0,
            Issue::TypeMismatch { .. } => 1,
            Issue::UnsupportedAlignment { .. } => 2,
            Issue::UninferredTypeResidue => 3,
            Issue::FunctionTypeDuplicateParameterName { .. } => 4,
            Issue::RefinementMinimumGreaterThanMaximum { .. } => 5,
        }
    }

    fn format(&self) -> nitrate_diagnosis::DiagnosticInfo {
        match self {
            Issue::TypeDoesNotImplementTrait { type_id, trait_id } => {
                let message = format!(
                    "Type {:?} does not implement trait {:?}.",
                    type_id.deref().to_string(),
                    trait_id.deref().borrow().name
                );

                nitrate_diagnosis::DiagnosticInfo {
                    origin: nitrate_diagnosis::Origin::Unknown,
                    message,
                }
            }

            Issue::TypeMismatch { expected, found } => {
                let message = format!(
                    "Type mismatch: expected {:?}, found {:?}.",
                    expected.deref().to_string(),
                    found.deref().to_string()
                );

                nitrate_diagnosis::DiagnosticInfo {
                    origin: nitrate_diagnosis::Origin::Unknown,
                    message,
                }
            }

            Issue::UnsupportedAlignment {
                alignment,
                max_supported,
            } => {
                let message = format!(
                    "Unsupported alignment: {} bytes. Maximum supported alignment is {} bytes.",
                    alignment, max_supported
                );

                nitrate_diagnosis::DiagnosticInfo {
                    origin: nitrate_diagnosis::Origin::Unknown,
                    message,
                }
            }

            Issue::UninferredTypeResidue => nitrate_diagnosis::DiagnosticInfo {
                origin: nitrate_diagnosis::Origin::Unknown,
                message: "Type residue was not inferred correctly.".to_string(),
            },

            Issue::FunctionTypeDuplicateParameterName { name, function } => {
                let message = format!(
                    "Function type {:?} has duplicate parameter name: '{}'.",
                    function.deref().to_string(),
                    name
                );

                nitrate_diagnosis::DiagnosticInfo {
                    origin: nitrate_diagnosis::Origin::Unknown,
                    message,
                }
            }

            Issue::RefinementMinimumGreaterThanMaximum { min, max, type_id } => {
                let message = format!(
                    "Refinement type {:?} has minimum {:?} greater than maximum {:?}.",
                    type_id.deref().to_string(),
                    min.deref(),
                    max.deref()
                );

                nitrate_diagnosis::DiagnosticInfo {
                    origin: nitrate_diagnosis::Origin::Unknown,
                    message,
                }
            }
        }
    }
}
