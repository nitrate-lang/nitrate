use nitrate_diagnosis::{DiagnosticGroupId, DiagnosticInfo, FormattableDiagnosticGroup, Origin, SourcePosition};
use nitrate_hir::{Lit, Type, TypeId};
use nitrate_hir_dump::Dump;
use nitrate_tree::ByteSpan;
use ordered_float::OrderedFloat;
use std::{format, ops::Deref};

/// Convert a `ByteSpan` into an `Origin` for diagnostic output.
fn byte_span_to_origin(span: ByteSpan) -> Origin {
    if span.is_empty() {
        Origin::Point(SourcePosition {
            line: 0,
            column: 0,
            offset: span.start,
            fileid: None,
        })
    } else {
        Origin::Span(nitrate_diagnosis::Span {
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

/// Comprehensive type error variants for the solver.
///
/// Every error variant includes a source location span for precise diagnostics.
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub(crate) enum TypeErr {
    IntegerLiteralOutOfRange {
        span: ByteSpan,
        value: u128,
        target_type: TypeId,
    },
    IntegerLiteralUnsatisfiable {
        span: ByteSpan,
        value: u128,
        unsatisfiable_type: TypeId,
    },
    FloatLiteralUnsatisfiable {
        span: ByteSpan,
        value: ordered_float::OrderedFloat<f64>,
        unsatisfiable_type: TypeId,
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
        callee_name: String,
        reason: String,
    },
    GenericArgCountMismatch {
        span: ByteSpan,
        expected: usize,
        provided: usize,
    },
    AmbiguousType {
        span: ByteSpan,
        name: String,
        reason: String,
    },
    MissingTypeAnnotation {
        span: ByteSpan,
        name: String,
    },
    UnresolvedInferredType {
        span: ByteSpan,
        id: u32,
        name: Option<String>,
    },
    UnboundGenericParam {
        span: ByteSpan,
        index: u32,
        name: String,
    },
    StructFieldTypeMismatch {
        span: ByteSpan,
        struct_name: String,
        field_name: String,
        expected_type: TypeId,
        actual_type: TypeId,
    },
    ArgumentTypeMismatch {
        span: ByteSpan,
        parameter_name: String,
        expected_type: TypeId,
        actual_type: TypeId,
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
            TypeErr::IntegerLiteralUnsatisfiable { .. } => 1,
            TypeErr::FloatLiteralUnsatisfiable { .. } => 2,
            TypeErr::IntegerLiteralOutOfRefinementBounds { .. } => 3,
            TypeErr::OperationResultOutOfRefinementBounds { .. } => 4,
            TypeErr::MismatchedBranchTypes { .. } => 5,
            TypeErr::CannotInferTypeArgs { .. } => 6,
            TypeErr::GenericArgCountMismatch { .. } => 7,
            TypeErr::AmbiguousType { .. } => 8,
            TypeErr::MissingTypeAnnotation { .. } => 9,
            TypeErr::UnresolvedInferredType { .. } => 10,
            TypeErr::UnboundGenericParam { .. } => 11,
            TypeErr::StructFieldTypeMismatch { .. } => 12,
            TypeErr::ArgumentTypeMismatch { .. } => 13,
            TypeErr::MethodNotFound { .. } => 14,
        }
    }

    fn format(&self) -> DiagnosticInfo {
        use std::string::ToString;
        match self {
            TypeErr::IntegerLiteralOutOfRange {
                span,
                value,
                target_type,
            } => DiagnosticInfo {
                origin: byte_span_to_origin(*span),
                message: format!(
                    "Integer literal value `{}` is outside the range of type `{}`",
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
                    "Integer literal `{}` cannot satisfy non-integer type constraint `{}`",
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
                    "Float literal `{}` cannot satisfy non-float type constraint `{}`",
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
                        "Integer literal `{}` does not satisfy refinement type `{}`{}",
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
                        "Arithmetic operation result range [{}, {}] cannot be guaranteed to satisfy refinement type `{}`{}",
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
                callee_name,
                reason,
            } => DiagnosticInfo {
                origin: byte_span_to_origin(*span),
                message: format!("Cannot infer type arguments for `{}`: {}", callee_name, reason),
            },
            TypeErr::GenericArgCountMismatch {
                span,
                expected,
                provided,
            } => DiagnosticInfo {
                origin: byte_span_to_origin(*span),
                message: format!("Expected {} type arguments, but {} were provided", expected, provided),
            },
            TypeErr::AmbiguousType { span, name, reason } => DiagnosticInfo {
                origin: byte_span_to_origin(*span),
                message: format!("Type of `{}` is ambiguous: {}", name, reason),
            },
            TypeErr::MissingTypeAnnotation { span, name } => DiagnosticInfo {
                origin: byte_span_to_origin(*span),
                message: format!("Type annotation required for `{}`: cannot infer from context", name),
            },
            TypeErr::UnresolvedInferredType { span, id, name } => {
                let name_str = name.as_deref().unwrap_or("<unnamed>");
                DiagnosticInfo {
                    origin: byte_span_to_origin(*span),
                    message: format!(
                        "Inferred type variable `{}` (`{}`) could not be resolved to a concrete type",
                        id, name_str
                    ),
                }
            }
            TypeErr::UnboundGenericParam { span, index, name } => DiagnosticInfo {
                origin: byte_span_to_origin(*span),
                message: format!("Generic parameter `{name}` (index {index}) could not be inferred from usage context"),
            },
            TypeErr::StructFieldTypeMismatch {
                span,
                struct_name,
                field_name,
                expected_type,
                actual_type,
            } => DiagnosticInfo {
                origin: byte_span_to_origin(*span),
                message: format!(
                    "Field `{}` of struct `{}` has type `{}`, but the provided expression has type `{}`",
                    field_name,
                    struct_name,
                    expected_type.to_string(),
                    actual_type.to_string()
                ),
            },
            TypeErr::ArgumentTypeMismatch {
                span,
                parameter_name,
                expected_type,
                actual_type,
            } => DiagnosticInfo {
                origin: byte_span_to_origin(*span),
                message: format!(
                    "Argument `{}` has type `{}`, but expected `{}`",
                    parameter_name,
                    actual_type.to_string(),
                    expected_type.to_string()
                ),
            },
            TypeErr::MethodNotFound {
                span,
                method_name,
                receiver_type,
            } => DiagnosticInfo {
                origin: byte_span_to_origin(*span),
                message: format!(
                    "Method `{}` not found on type `{}`",
                    method_name,
                    receiver_type.to_string()
                ),
            },
        }
    }
}
