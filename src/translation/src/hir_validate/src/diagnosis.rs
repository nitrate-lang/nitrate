use std::ops::Deref;

use nitrate_diagnosis::{DiagnosticGroupId, FormattableDiagnosticGroup};
use nitrate_hir::{TraitId, TypeId};
use nitrate_hir_dump::Dump;

pub(crate) enum Issue {
    TypeDoesNotImplementTrait { type_id: TypeId, trait_id: TraitId },
    TypeMismatch { expected: TypeId, found: TypeId },
}

impl FormattableDiagnosticGroup for Issue {
    fn group_id(&self) -> DiagnosticGroupId {
        DiagnosticGroupId::Semantic
    }

    fn variant_id(&self) -> u16 {
        match self {
            Issue::TypeDoesNotImplementTrait { .. } => 0,
            Issue::TypeMismatch { .. } => 1,
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
        }
    }
}
