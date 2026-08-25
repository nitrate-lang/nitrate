use std::format;

use nitrate_diagnosis::{DiagnosticGroupId, DiagnosticInfo, FormattableDiagnosticGroup, Origin};
use nitrate_nstring::NString;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub(crate) enum ValidateErr {
    /// A struct field doesn't exist on the struct.
    StructFieldDoesNotExist { struct_name: NString, field_name: NString },

    /// Field access visibility violation.
    FieldAccessVisibility { field_name: NString },

    /// Enum variant doesn't exist.
    EnumVariantDoesNotExist { enum_name: NString, variant_name: NString },

    /// Assignment target is not mutable.
    AssignmentTargetNotMutable,

    /// Mutable borrow target is not mutable.
    MutableBorrowTargetNotMutable,

    /// Function visibility violation.
    FunctionNotAccessible { function_name: NString },

    /// Global variable visibility violation.
    GlobalVariableNotAccessible { variable_name: NString },

    /// A divergent statement (break/continue/return) is not the last in a block.
    DivergentStatementHasSuccessors,

    /// A type is required to be Sized but is not.
    TypeNotSized { type_repr: String },

    /// A type mismatch between a declared type and the actual type of an initializer/expression.
    TypeMismatch { expected: String, actual: String },

    /// An inferred type (e.g., InferredInteger, InferredFloat, Inferred { .. }) was found where a concrete type was required.
    InferredTypeNotAllowed { type_repr: String },

    /// Alignment value exceeds the maximum supported alignment.
    AlignmentTooLarge {
        alignment: u32,
        max_supported: u32,
        context: String,
    },

    /// Invalid extern ABI name.
    InvalidExternAbi { abi_name: String },

    /// Duplicate parameter name in a function type.
    DuplicateParameterName { name: NString },

    /// Refinement type has min > max.
    RefinementBoundsInvalid { min: i128, max: i128 },

    /// Function body does not end with a return statement.
    FunctionBodyMissingReturn { function_name: NString },

    /// Return type of the function does not match the type of the returned expression.
    ReturnTypeMismatch {
        function_name: NString,
        expected: String,
        actual: String,
    },

    /// A module attribute is invalid.
    InvalidModuleAttribute,

    /// An enum attribute is invalid.
    InvalidEnumAttribute,

    /// An enum variant attribute is invalid.
    InvalidEnumVariantAttribute,

    /// An unsafe operation was performed outside an `unsafe` block.
    UnsafeOperationOutsideUnsafeBlock { operation: String },

    /// An `unsafe` function was called from safe code.
    UnsafeFnCallOutsideUnsafeBlock { function_name: NString },
}

impl FormattableDiagnosticGroup for ValidateErr {
    fn group_id(&self) -> DiagnosticGroupId {
        DiagnosticGroupId::Hir
    }

    fn variant_id(&self) -> u16 {
        match self {
            ValidateErr::StructFieldDoesNotExist { .. } => 0x000,
            ValidateErr::FieldAccessVisibility { .. } => 0x001,
            ValidateErr::EnumVariantDoesNotExist { .. } => 0x002,
            ValidateErr::AssignmentTargetNotMutable => 0x003,
            ValidateErr::MutableBorrowTargetNotMutable => 0x004,
            ValidateErr::FunctionNotAccessible { .. } => 0x005,
            ValidateErr::GlobalVariableNotAccessible { .. } => 0x006,
            ValidateErr::DivergentStatementHasSuccessors => 0x007,
            ValidateErr::TypeNotSized { .. } => 0x008,
            ValidateErr::TypeMismatch { .. } => 0x009,
            ValidateErr::InferredTypeNotAllowed { .. } => 0x00A,
            ValidateErr::AlignmentTooLarge { .. } => 0x00B,
            ValidateErr::InvalidExternAbi { .. } => 0x00C,
            ValidateErr::DuplicateParameterName { .. } => 0x00D,
            ValidateErr::RefinementBoundsInvalid { .. } => 0x00E,
            ValidateErr::FunctionBodyMissingReturn { .. } => 0x00F,
            ValidateErr::ReturnTypeMismatch { .. } => 0x010,
            ValidateErr::InvalidModuleAttribute => 0x011,
            ValidateErr::InvalidEnumAttribute => 0x012,
            ValidateErr::InvalidEnumVariantAttribute => 0x013,
            ValidateErr::UnsafeOperationOutsideUnsafeBlock { .. } => 0x015,
            ValidateErr::UnsafeFnCallOutsideUnsafeBlock { .. } => 0x016,
        }
    }

    fn format(&self) -> DiagnosticInfo {
        let message = match self {
            ValidateErr::StructFieldDoesNotExist {
                struct_name,
                field_name,
            } => {
                format!("struct `{}` does not have a field named `{}`", struct_name, field_name,)
            }

            ValidateErr::FieldAccessVisibility { field_name } => {
                format!("field `{}` is not accessible from the current module", field_name,)
            }

            ValidateErr::EnumVariantDoesNotExist {
                enum_name,
                variant_name,
            } => {
                format!("enum `{}` does not have a variant named `{}`", enum_name, variant_name,)
            }

            ValidateErr::AssignmentTargetNotMutable => "assignment target is not mutable".to_string(),

            ValidateErr::MutableBorrowTargetNotMutable => "mutable borrow target is not mutable".to_string(),

            ValidateErr::FunctionNotAccessible { function_name } => {
                format!("function `{}` is not accessible from the current module", function_name,)
            }

            ValidateErr::GlobalVariableNotAccessible { variable_name } => {
                format!(
                    "global variable `{}` is not accessible from the current module",
                    variable_name,
                )
            }

            ValidateErr::DivergentStatementHasSuccessors => {
                "divergent statement (break/continue/return) has successor statements in the same block".to_string()
            }

            ValidateErr::TypeNotSized { type_repr } => {
                format!("type `{}` does not have a known size at compile time", type_repr)
            }

            ValidateErr::TypeMismatch { expected, actual } => {
                format!("expected type `{}` but found type `{}`", expected, actual,)
            }

            ValidateErr::InferredTypeNotAllowed { type_repr } => {
                format!(
                    "inferred type `{}` is not allowed in this context; provide an explicit type annotation",
                    type_repr,
                )
            }

            ValidateErr::AlignmentTooLarge {
                alignment,
                max_supported,
                context,
            } => {
                format!(
                    "alignment value {} for {} exceeds the maximum supported alignment {}",
                    alignment, context, max_supported,
                )
            }

            ValidateErr::InvalidExternAbi { abi_name } => {
                format!("invalid extern ABI name `{}`", abi_name,)
            }

            ValidateErr::DuplicateParameterName { name } => {
                format!("duplicate parameter name `{}` in function type", name,)
            }

            ValidateErr::RefinementBoundsInvalid { min, max } => {
                format!("refinement type has invalid bounds: min={} > max={}", min, max,)
            }

            ValidateErr::FunctionBodyMissingReturn { function_name } => {
                format!("function `{}` body does not end with a return statement", function_name,)
            }

            ValidateErr::ReturnTypeMismatch {
                function_name,
                expected,
                actual,
            } => {
                format!(
                    "function `{}` return type mismatch: expected `{}` but body returns `{}`",
                    function_name, expected, actual,
                )
            }

            ValidateErr::InvalidModuleAttribute => "module has an invalid attribute".to_string(),

            ValidateErr::InvalidEnumAttribute => "enum has an invalid attribute".to_string(),

            ValidateErr::InvalidEnumVariantAttribute => "enum variant has an invalid attribute".to_string(),

            ValidateErr::UnsafeOperationOutsideUnsafeBlock { operation } => {
                format!(
                    "`{}` is unsafe and requires an `unsafe` block\n\
                     \n  = note: operations like raw pointer dereference, calls to `unsafe` functions,\n\
                     \n         and access to mutable statics must be performed inside an `unsafe` block.\n\
                     \n  = help: wrap this operation in an `unsafe {{ ... }}` block.",
                    operation,
                )
            }

            ValidateErr::UnsafeFnCallOutsideUnsafeBlock { function_name } => {
                format!(
                    "call to unsafe function `{}` requires an `unsafe` block\n\
                     \n  = note: `unsafe` functions may perform operations that violate memory safety.\n\
                     \n         The caller must explicitly opt into this by wrapping the call in `unsafe {{ ... }}`.\n\
                     \n  = help: wrap the call in an `unsafe {{ ... }}` block.",
                    function_name,
                )
            }
        };

        DiagnosticInfo {
            origin: Origin::None,
            message,
        }
    }
}
