//! Type error diagnostics for the suspended constraint solver.
//!
//! Provides structured error types with precise source spans, implementing
//! the `FormattableDiagnosticGroup` trait for integration with the Nitrate
//! diagnostic system.

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
    /// Integer literal value exceeds the range of the target primitive type.
    IntegerLiteralOutOfRange {
        span: ByteSpan,
        value: u128,
        target_type: TypeId,
    },
    /// Integer literal constrained to a non-integer type.
    IntegerLiteralUnsatisfiable {
        span: ByteSpan,
        value: u128,
        unsatisfiable_type: TypeId,
    },
    /// Float literal constrained to a non-float type.
    FloatLiteralUnsatisfiable {
        span: ByteSpan,
        value: ordered_float::OrderedFloat<f64>,
        unsatisfiable_type: TypeId,
    },
    /// Integer literal outside refinement type bounds.
    IntegerLiteralOutOfRefinementBounds {
        span: ByteSpan,
        value: u128,
        refinement_type: TypeId,
    },
    /// Arithmetic result cannot be guaranteed to satisfy refinement bounds.
    OperationResultOutOfRefinementBounds {
        span: ByteSpan,
        refinement_type: TypeId,
        computed_min: u128,
        computed_max: u128,
    },
    /// If/else branches have incompatible concrete types.
    MismatchedBranchTypes {
        span: ByteSpan,
        true_type: TypeId,
        false_type: TypeId,
    },
    /// Generic type arguments cannot be inferred from context.
    CannotInferTypeArgs {
        span: ByteSpan,
        generic_name: String,
        reason: String,
    },
    /// Expression type is ambiguous (cannot be determined).
    AmbiguousType { span: ByteSpan, description: String },
    /// A generic parameter could not be resolved.
    UnboundGenericParam {
        span: ByteSpan,
        param_name: String,
        generic_name: String,
    },
    /// Method not found on receiver type.
    MethodNotFound {
        span: ByteSpan,
        method_name: String,
        receiver_type: TypeId,
    },
    /// Cannot unify two distinct concrete types.
    TypeMismatch {
        span: ByteSpan,
        expected: TypeId,
        found: TypeId,
    },
    /// Occurs check failure — recursive type detected.
    RecursiveType { span: ByteSpan, description: String },
    /// Trait bound not satisfied.
    TraitNotSatisfied {
        span: ByteSpan,
        type_id: TypeId,
        trait_name: String,
    },
}

impl FormattableDiagnosticGroup for TypeErr {
    fn group_id(&self) -> DiagnosticGroupId {
        DiagnosticGroupId::Type
    }

    fn variant_id(&self) -> u16 {
        match self {
            TypeErr::IntegerLiteralOutOfRange { .. } => 0,
            TypeErr::IntegerLiteralUnsatisfiable { .. } => 1,
            TypeErr::FloatLiteralUnsatisfiable { .. } => 2,
            TypeErr::IntegerLiteralOutOfRefinementBounds { .. } => 3,
            TypeErr::OperationResultOutOfRefinementBounds { .. } => 4,
            TypeErr::MismatchedBranchTypes { .. } => 5,
            TypeErr::CannotInferTypeArgs { .. } => 6,
            TypeErr::AmbiguousType { .. } => 8,
            TypeErr::UnboundGenericParam { .. } => 11,
            TypeErr::MethodNotFound { .. } => 14,
            TypeErr::TypeMismatch { .. } => 16,
            TypeErr::RecursiveType { .. } => 17,
            TypeErr::TraitNotSatisfied { .. } => 18,
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
            TypeErr::IntegerLiteralUnsatisfiable {
                span,
                value,
                unsatisfiable_type,
            } => DiagnosticInfo {
                origin: byte_span_to_origin(*span),
                message: format!(
                    "integer literal `{}` cannot satisfy non-integer type constraint `{}`",
                    value,
                    unsatisfiable_type.to_string()
                ),
            },
            TypeErr::FloatLiteralUnsatisfiable {
                span,
                value,
                unsatisfiable_type,
            } => DiagnosticInfo {
                origin: byte_span_to_origin(*span),
                message: format!(
                    "float literal `{}` cannot satisfy non-float type constraint `{}`",
                    value,
                    unsatisfiable_type.to_string()
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
            TypeErr::TypeMismatch { span, expected, found } => DiagnosticInfo {
                origin: byte_span_to_origin(*span),
                message: format!(
                    "type mismatch: expected `{}`, found `{}`",
                    expected.to_string(),
                    found.to_string()
                ),
            },
            TypeErr::RecursiveType { span, description } => DiagnosticInfo {
                origin: byte_span_to_origin(*span),
                message: format!("recursive type detected: {}", description),
            },
            TypeErr::TraitNotSatisfied {
                span,
                type_id,
                trait_name,
            } => DiagnosticInfo {
                origin: byte_span_to_origin(*span),
                message: format!(
                    "type `{}` does not satisfy trait bound `{}`",
                    type_id.to_string(),
                    trait_name
                ),
            },
        }
    }
}
