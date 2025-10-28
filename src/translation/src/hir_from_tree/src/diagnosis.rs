use nitrate_diagnosis::{DiagnosticGroupId, DiagnosticInfo, FormattableDiagnosticGroup, Origin};

pub(crate) enum HirErr {
    UnrecognizedModuleAttribute,
    UnimplementedFeature(String),
    UnrecognizedGlobalVariableAttribute,
    GlobalVariableMustHaveInitializer,
    LatentTypeEvaluationError,
    FoundUSize32InNon32BitTarget,
    FoundUSize64InNon64BitTarget,
    ArrayLengthExpectedUSize,
    ArrayTypeLengthEvalError,
    TypeInferenceError,
    UnrecognizedFunctionAttribute,
    UnrecognizedFunctionParameterAttribute,
    IntegerCastOutOfRange,
    UnrecognizedLifetime,
    UnrecognizedTypeAliasAttribute,
    TypeAliasMustHaveType,
    UnrecognizedStructAttribute,
    UnrecognizedStructFieldAttribute,
    UnrecognizedEnumAttribute,
    UnrecognizedEnumVariantAttribute,
    UnresolvedTypePath,
    UnrecognizedLocalVariableAttribute,
    UnresolvedSymbol,
}

impl FormattableDiagnosticGroup for HirErr {
    fn group_id(&self) -> DiagnosticGroupId {
        DiagnosticGroupId::Hir
    }

    fn variant_id(&self) -> u16 {
        match self {
            HirErr::UnrecognizedModuleAttribute => 1,
            HirErr::UnimplementedFeature(_) => 2,
            HirErr::UnrecognizedGlobalVariableAttribute => 3,
            HirErr::GlobalVariableMustHaveInitializer => 5,
            HirErr::LatentTypeEvaluationError => 6,
            HirErr::FoundUSize32InNon32BitTarget => 7,
            HirErr::FoundUSize64InNon64BitTarget => 8,
            HirErr::ArrayLengthExpectedUSize => 9,
            HirErr::ArrayTypeLengthEvalError => 10,
            HirErr::TypeInferenceError => 11,
            HirErr::UnrecognizedFunctionAttribute => 14,
            HirErr::UnrecognizedFunctionParameterAttribute => 15,
            HirErr::IntegerCastOutOfRange => 17,
            HirErr::UnrecognizedLifetime => 21,
            HirErr::UnrecognizedTypeAliasAttribute => 22,
            HirErr::TypeAliasMustHaveType => 23,
            HirErr::UnrecognizedStructAttribute => 24,
            HirErr::UnrecognizedStructFieldAttribute => 25,
            HirErr::UnrecognizedEnumAttribute => 26,
            HirErr::UnrecognizedEnumVariantAttribute => 27,
            HirErr::UnresolvedTypePath => 28,
            HirErr::UnrecognizedLocalVariableAttribute => 30,
            HirErr::UnresolvedSymbol => 31,
        }
    }

    fn format(&self) -> nitrate_diagnosis::DiagnosticInfo {
        match self {
            HirErr::UnrecognizedModuleAttribute => DiagnosticInfo {
                message: "unrecognized module attribute".to_string(),
                origin: Origin::None,
            },

            HirErr::UnimplementedFeature(feature) => DiagnosticInfo {
                message: format!("unimplemented feature: {}", feature),
                origin: Origin::None,
            },

            HirErr::UnrecognizedGlobalVariableAttribute => DiagnosticInfo {
                message: "unrecognized global variable attribute".to_string(),
                origin: Origin::None,
            },

            HirErr::GlobalVariableMustHaveInitializer => DiagnosticInfo {
                message: "global variable must have an initializer".to_string(),
                origin: Origin::None,
            },

            HirErr::LatentTypeEvaluationError => DiagnosticInfo {
                message: "latent type evaluation error".to_string(),
                origin: Origin::None,
            },

            HirErr::FoundUSize32InNon32BitTarget => DiagnosticInfo {
                message: "found 32-bit 'usize' in non-32-bit target".to_string(),
                origin: Origin::None,
            },

            HirErr::FoundUSize64InNon64BitTarget => DiagnosticInfo {
                message: "found 64-bit 'usize' in non-64-bit target".to_string(),
                origin: Origin::None,
            },

            HirErr::ArrayLengthExpectedUSize => DiagnosticInfo {
                message: "array length expected to be 'usize'".to_string(),
                origin: Origin::None,
            },

            HirErr::ArrayTypeLengthEvalError => DiagnosticInfo {
                message: "array type length evaluation error".to_string(),
                origin: Origin::None,
            },

            HirErr::TypeInferenceError => DiagnosticInfo {
                message: "type inference error".to_string(),
                origin: Origin::None,
            },

            HirErr::UnrecognizedFunctionAttribute => DiagnosticInfo {
                message: "unrecognized function attribute".to_string(),
                origin: Origin::None,
            },

            HirErr::UnrecognizedFunctionParameterAttribute => DiagnosticInfo {
                message: "unrecognized function parameter attribute".to_string(),
                origin: Origin::None,
            },

            HirErr::IntegerCastOutOfRange => DiagnosticInfo {
                message: "integer cast out of range".to_string(),
                origin: Origin::None,
            },

            HirErr::UnrecognizedLifetime => DiagnosticInfo {
                message: "unrecognized lifetime".to_string(),
                origin: Origin::None,
            },

            HirErr::UnrecognizedTypeAliasAttribute => DiagnosticInfo {
                message: "unrecognized type alias attribute".to_string(),
                origin: Origin::None,
            },

            HirErr::TypeAliasMustHaveType => DiagnosticInfo {
                message: "type alias must have a type".to_string(),
                origin: Origin::None,
            },

            HirErr::UnrecognizedStructAttribute => DiagnosticInfo {
                message: "unrecognized struct attribute".to_string(),
                origin: Origin::None,
            },

            HirErr::UnrecognizedStructFieldAttribute => DiagnosticInfo {
                message: "unrecognized struct field attribute".to_string(),
                origin: Origin::None,
            },

            HirErr::UnrecognizedEnumAttribute => DiagnosticInfo {
                message: "unrecognized enum attribute".to_string(),
                origin: Origin::None,
            },

            HirErr::UnrecognizedEnumVariantAttribute => DiagnosticInfo {
                message: "unrecognized enum variant attribute".to_string(),
                origin: Origin::None,
            },

            HirErr::UnresolvedTypePath => DiagnosticInfo {
                message: "unresolved type path".to_string(),
                origin: Origin::None,
            },

            HirErr::UnrecognizedLocalVariableAttribute => DiagnosticInfo {
                message: "unrecognized local variable attribute".to_string(),
                origin: Origin::None,
            },

            HirErr::UnresolvedSymbol => DiagnosticInfo {
                message: "unresolved symbol".to_string(),
                origin: Origin::None,
            },
        }
    }
}
