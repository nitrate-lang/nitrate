use nitrate_diagnosis::{DiagnosticGroupId, DiagnosticInfo, FormattableDiagnosticGroup, Origin, SourcePosition};
use nitrate_hir::{Lit, Type, TypeId};
use nitrate_hir_dump::Dump;
use nitrate_tree::SrcPos;
use std::{format, ops::Deref};

fn srcpos_to_origin(span: SrcPos) -> Origin {
    Origin::Point(SourcePosition {
        line: span.line as u32,
        column: span.column as u32,
        offset: span.offset.to_u32(),
        fileid: span.fileid,
    })
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub(crate) enum TypeErr {
    IntegerLiteralOutOfRange {
        span: SrcPos,
        value: u128,
        target_type: TypeId,
    },
    IntegerLiteralOutOfRefinementBounds {
        span: SrcPos,
        value: u128,
        refinement_type: TypeId,
    },
    OperationResultOutOfRefinementBounds {
        span: SrcPos,
        refinement_type: TypeId,
        computed_min: u128,
        computed_max: u128,
    },
    MismatchedBranchTypes {
        span: SrcPos,
        true_type: TypeId,
        false_type: TypeId,
    },
    CannotInferTypeArgs {
        span: SrcPos,
        generic_name: String,
        reason: String,
    },
    AmbiguousType {
        span: SrcPos,
        description: String,
    },
    UnboundGenericParam {
        span: SrcPos,
        param_name: String,
        generic_name: String,
    },
    MethodNotFound {
        span: SrcPos,
        method_name: String,
        receiver_type: TypeId,
    },
}

impl FormattableDiagnosticGroup for TypeErr {
    fn group_id(&self) -> DiagnosticGroupId {
        DiagnosticGroupId::Type
    }

    fn variant_id(&self) -> u16 {
        match self {
            TypeErr::IntegerLiteralOutOfRange { .. } => 0,
            TypeErr::IntegerLiteralOutOfRefinementBounds { .. } => 1,
            TypeErr::OperationResultOutOfRefinementBounds { .. } => 2,
            TypeErr::MismatchedBranchTypes { .. } => 3,
            TypeErr::CannotInferTypeArgs { .. } => 4,
            TypeErr::AmbiguousType { .. } => 5,
            TypeErr::UnboundGenericParam { .. } => 6,
            TypeErr::MethodNotFound { .. } => 7,
        }
    }

    fn format(&self) -> DiagnosticInfo {
        match self {
            TypeErr::IntegerLiteralOutOfRange {
                span,
                value,
                target_type,
            } => DiagnosticInfo {
                origin: srcpos_to_origin(*span),
                message: format!(
                    "integer literal value `{}` is outside the range of type `{}`",
                    value,
                    target_type.to_string()
                ),
            },
            TypeErr::IntegerLiteralOutOfRefinementBounds {
                span,
                value,
                refinement_type,
            } => {
                let bounds_info = match &**refinement_type {
                    Type::Refine { min, max, .. } => {
                        format!(" (expected {}-{})", min.deref() as &Lit, max.deref() as &Lit)
                    }
                    _ => String::new(),
                };
                DiagnosticInfo {
                    origin: srcpos_to_origin(*span),
                    message: format!(
                        "integer literal `{}` does not satisfy refinement type `{}`{}",
                        value,
                        refinement_type.to_string(),
                        bounds_info
                    ),
                }
            }
            TypeErr::OperationResultOutOfRefinementBounds {
                span,
                refinement_type,
                computed_min,
                computed_max,
            } => {
                let bounds_info = match &**refinement_type {
                    Type::Refine { min, max, .. } => {
                        format!(" (expected {}-{})", min.deref() as &Lit, max.deref() as &Lit)
                    }
                    _ => String::new(),
                };
                DiagnosticInfo {
                    origin: srcpos_to_origin(*span),
                    message: format!(
                        "arithmetic operation result range [{}, {}] cannot be guaranteed to satisfy refinement type `{}`{}",
                        computed_min,
                        computed_max,
                        refinement_type.to_string(),
                        bounds_info
                    ),
                }
            }
            TypeErr::MismatchedBranchTypes {
                span,
                true_type,
                false_type,
            } => DiagnosticInfo {
                origin: srcpos_to_origin(*span),
                message: format!(
                    "'if' and 'else' branches have incompatible types: `{}` vs `{}`",
                    true_type.to_string(),
                    false_type.to_string()
                ),
            },
            TypeErr::CannotInferTypeArgs {
                span,
                generic_name,
                reason,
            } => DiagnosticInfo {
                origin: srcpos_to_origin(*span),
                message: format!("cannot infer type arguments for `{}`: {}", generic_name, reason),
            },
            TypeErr::AmbiguousType { span, description } => DiagnosticInfo {
                origin: srcpos_to_origin(*span),
                message: format!("ambiguous type: {}", description),
            },
            TypeErr::UnboundGenericParam {
                span,
                param_name,
                generic_name,
            } => DiagnosticInfo {
                origin: srcpos_to_origin(*span),
                message: format!(
                    "generic parameter `{}` on `{}` could not be inferred from context",
                    param_name, generic_name
                ),
            },
            TypeErr::MethodNotFound {
                span,
                method_name,
                receiver_type,
            } => DiagnosticInfo {
                origin: srcpos_to_origin(*span),
                message: format!(
                    "method `{}` not found on type `{}`",
                    method_name,
                    receiver_type.to_string()
                ),
            },
        }
    }
}

// ── Tests ────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;
    use nitrate_diagnosis::DiagnosticGroupId;
    use nitrate_hir::{Lit, LiteralId, Store, Type, TypeId, using_storage};
    use nitrate_tree::SrcPos;

    fn sp() -> SrcPos {
        SrcPos::default()
    }

    fn with_store<R>(f: impl FnOnce() -> R) -> R {
        let store = Store::new();
        using_storage(&store, f)
    }

    #[test]
    fn srcpos_to_origin_conversion() {
        let pos = SrcPos::default();
        let origin = super::srcpos_to_origin(pos);
        if let Origin::Point(sp) = origin {
            assert_eq!(sp.line, 0);
            assert_eq!(sp.column, 0);
        } else {
            panic!("expected Point");
        }
    }

    #[test]
    fn all_variant_ids_are_unique() {
        with_store(|| {
            let ids: Vec<u16> = vec![
                TypeErr::IntegerLiteralOutOfRange {
                    span: sp(),
                    value: 1,
                    target_type: TypeId::from(Type::U8 { span: sp() }),
                }
                .variant_id(),
                TypeErr::IntegerLiteralOutOfRefinementBounds {
                    span: sp(),
                    value: 1,
                    refinement_type: TypeId::from(Type::I32 { span: sp() }),
                }
                .variant_id(),
                TypeErr::OperationResultOutOfRefinementBounds {
                    span: sp(),
                    refinement_type: TypeId::from(Type::I32 { span: sp() }),
                    computed_min: 0,
                    computed_max: 1,
                }
                .variant_id(),
                TypeErr::MismatchedBranchTypes {
                    span: sp(),
                    true_type: TypeId::from(Type::I32 { span: sp() }),
                    false_type: TypeId::from(Type::F64 { span: sp() }),
                }
                .variant_id(),
                TypeErr::CannotInferTypeArgs {
                    span: sp(),
                    generic_name: "".into(),
                    reason: "".into(),
                }
                .variant_id(),
                TypeErr::AmbiguousType {
                    span: sp(),
                    description: "".into(),
                }
                .variant_id(),
                TypeErr::UnboundGenericParam {
                    span: sp(),
                    param_name: "".into(),
                    generic_name: "".into(),
                }
                .variant_id(),
                TypeErr::MethodNotFound {
                    span: sp(),
                    method_name: "".into(),
                    receiver_type: TypeId::from(Type::I32 { span: sp() }),
                }
                .variant_id(),
            ];
            let mut seen = std::collections::HashSet::new();
            for id in &ids {
                assert!(seen.insert(*id), "duplicate variant_id: {id}");
            }
            assert_eq!(ids, vec![0, 1, 2, 3, 4, 5, 6, 7]);
        });
    }

    #[test]
    fn all_type_errs_have_type_group_id() {
        with_store(|| {
            let errors: Vec<TypeErr> = vec![
                TypeErr::IntegerLiteralOutOfRange {
                    span: sp(),
                    value: 1,
                    target_type: TypeId::from(Type::U8 { span: sp() }),
                },
                TypeErr::IntegerLiteralOutOfRefinementBounds {
                    span: sp(),
                    value: 1,
                    refinement_type: TypeId::from(Type::I32 { span: sp() }),
                },
                TypeErr::OperationResultOutOfRefinementBounds {
                    span: sp(),
                    refinement_type: TypeId::from(Type::I32 { span: sp() }),
                    computed_min: 0,
                    computed_max: 1,
                },
                TypeErr::MismatchedBranchTypes {
                    span: sp(),
                    true_type: TypeId::from(Type::I32 { span: sp() }),
                    false_type: TypeId::from(Type::F64 { span: sp() }),
                },
            ];
            for e in &errors {
                assert_eq!(e.group_id(), DiagnosticGroupId::Type);
            }
        });
    }

    #[test]
    fn format_integer_out_of_range() {
        with_store(|| {
            let e = TypeErr::IntegerLiteralOutOfRange {
                span: sp(),
                value: 999,
                target_type: TypeId::from(Type::U8 { span: sp() }),
            };
            let info = e.format();
            assert!(info.message.contains("999"));
            assert!(info.message.contains("outside the range"));
        });
    }

    #[test]
    fn format_refinement_out_of_bounds() {
        with_store(|| {
            let min_lit = LiteralId::from(Lit::I8(10));
            let max_lit = LiteralId::from(Lit::I8(20));
            let refine = Type::Refine {
                span: sp(),
                base: TypeId::from(Type::I32 { span: sp() }),
                min: min_lit,
                max: max_lit,
            };
            let e = TypeErr::IntegerLiteralOutOfRefinementBounds {
                span: sp(),
                value: 5,
                refinement_type: TypeId::from(refine),
            };
            let info = e.format();
            assert!(info.message.contains("does not satisfy refinement type"));
            assert!(info.message.contains("5"));
        });
    }

    #[test]
    fn format_operation_result_out_of_refinement() {
        with_store(|| {
            let min_lit = LiteralId::from(Lit::I8(0));
            let max_lit = LiteralId::from(Lit::I8(100));
            let refine = Type::Refine {
                span: sp(),
                base: TypeId::from(Type::I32 { span: sp() }),
                min: min_lit,
                max: max_lit,
            };
            let e = TypeErr::OperationResultOutOfRefinementBounds {
                span: sp(),
                refinement_type: TypeId::from(refine),
                computed_min: 50,
                computed_max: 200,
            };
            let info = e.format();
            assert!(info.message.contains("arithmetic operation result range"));
            assert!(info.message.contains("50"));
            assert!(info.message.contains("200"));
        });
    }

    #[test]
    fn format_mismatched_branch_types() {
        with_store(|| {
            let e = TypeErr::MismatchedBranchTypes {
                span: sp(),
                true_type: TypeId::from(Type::I32 { span: sp() }),
                false_type: TypeId::from(Type::F64 { span: sp() }),
            };
            let info = e.format();
            assert!(info.message.contains("'if' and 'else' branches"));
            assert!(info.message.contains("incompatible types"));
        });
    }

    #[test]
    fn format_cannot_infer_type_args() {
        let e = TypeErr::CannotInferTypeArgs {
            span: sp(),
            generic_name: "MyStruct".into(),
            reason: "no constraints on T".into(),
        };
        let info = e.format();
        assert!(info.message.contains("cannot infer type arguments"));
        assert!(info.message.contains("MyStruct"));
    }
}
