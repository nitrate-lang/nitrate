use nitrate_diagnosis::{DiagnosticGroupId, DiagnosticInfo, FormattableDiagnosticGroup, Origin, SourcePosition, Span};
use nitrate_hir::{Lit, Type, TypeId};
use nitrate_hir_dump::Dump;
use nitrate_tree::ByteSpan;
use std::{format, ops::Deref};

fn byte_span_to_origin(span: ByteSpan) -> Origin {
    if span.is_empty() {
        Origin::Point(SourcePosition {
            line: 0,
            column: 0,
            offset: span.start,
            fileid: None,
        })
    } else {
        Origin::Span(Span {
            start: SourcePosition {
                line: 0,
                column: 0,
                offset: span.start,
                fileid: None,
            },
            end: SourcePosition {
                line: 0,
                column: 0,
                offset: span.end,
                fileid: None,
            },
        })
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub(crate) enum TypeErr {
    IntegerLiteralOutOfRange {
        span: ByteSpan,
        value: u128,
        target_type: TypeId,
    },
    IntegerLiteralOutOfRefinementBounds {
        span: ByteSpan,
        value: u128,
        refinement_type: TypeId,
    },
    OperationResultOutOfRefinementBounds {
        span: ByteSpan,
        refinement_type: TypeId,
        computed_min: u128,
        computed_max: u128,
    },
    MismatchedBranchTypes {
        span: ByteSpan,
        true_type: TypeId,
        false_type: TypeId,
    },
    CannotInferTypeArgs {
        span: ByteSpan,
        generic_name: String,
        reason: String,
    },
    AmbiguousType {
        span: ByteSpan,
        description: String,
    },
    UnboundGenericParam {
        span: ByteSpan,
        param_name: String,
        generic_name: String,
    },
    MethodNotFound {
        span: ByteSpan,
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
                origin: byte_span_to_origin(*span),
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
                    origin: byte_span_to_origin(*span),
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
                    origin: byte_span_to_origin(*span),
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
                origin: byte_span_to_origin(*span),
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
                origin: byte_span_to_origin(*span),
                message: format!("cannot infer type arguments for `{}`: {}", generic_name, reason),
            },
            TypeErr::AmbiguousType { span, description } => DiagnosticInfo {
                origin: byte_span_to_origin(*span),
                message: format!("ambiguous type: {}", description),
            },
            TypeErr::UnboundGenericParam {
                span,
                param_name,
                generic_name,
            } => DiagnosticInfo {
                origin: byte_span_to_origin(*span),
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
                origin: byte_span_to_origin(*span),
                message: format!(
                    "method `{}` not found on type `{}`",
                    method_name,
                    receiver_type.to_string()
                ),
            },
        }
    }
}
