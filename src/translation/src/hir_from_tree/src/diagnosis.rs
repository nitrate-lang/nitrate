use nitrate_diagnosis::{DiagnosticGroupId, DiagnosticInfo, FormattableDiagnosticGroup, Origin, SourcePosition};
use nitrate_hir_evaluate::EvalError;
use nitrate_tree::{SrcPos, SrcSpan};

/// Comprehensive error codes for the HIR lowering stage (group H).
///
/// Error codes follow the pattern HXYZ where:
/// - H = HIR lowering group
/// - X  = category (1=attributes, 2=names/symbols, 3=types, 4=expressions,
///                  5=control flow, 6=refinement, 7=unimplemented, 8=definitions,
///                  9=syntax/other)
/// - YZ = specific error variant
///
/// Error code ranges (with large margins for future expansion):
///   H001-H049: Attribute/annotation errors
///   H050-H099: Name/symbol errors
///   H100-H149: Type errors
///   H150-H199: Expression and literal errors
///   H200-H249: Statement and control flow errors
///   H250-H299: Refinement type errors
///   H300-H349: Unimplemented features
///   H350-H399: Item and definition errors
///   H400-H449: Security and safety errors
#[derive(Debug, Clone)]
pub(crate) enum HirErr {
    // ── Attribute Errors (H001-H049) ──
    /// An attribute was used on a module item that is not recognized.
    UnrecognizedModuleAttribute { span: SrcPos, name: String },
    /// An attribute was used on a global variable that is not recognized.
    UnrecognizedGlobalVarAttribute { span: SrcPos, name: String },
    /// An attribute was used on a function that is not recognized.
    UnrecognizedFunctionAttribute { span: SrcPos, name: String },
    /// An attribute was used on a function parameter that is not recognized.
    UnrecognizedFunctionParamAttribute { span: SrcPos, name: String },
    /// An attribute was used on a type alias that is not recognized.
    UnrecognizedTypeAliasAttribute { span: SrcPos, name: String },
    /// An attribute was used on a struct definition that is not recognized.
    UnrecognizedStructAttribute { span: SrcPos, name: String },
    /// An attribute was used on a struct field that is not recognized.
    UnrecognizedStructFieldAttribute { span: SrcPos, name: String },
    /// An attribute was used on an enum definition that is not recognized.
    UnrecognizedEnumAttribute { span: SrcPos, name: String },
    /// An attribute was used on an enum variant that is not recognized.
    UnrecognizedEnumVariantAttribute { span: SrcPos, name: String },
    /// An attribute was used on a local variable that is not recognized.
    UnrecognizedLocalVarAttribute { span: SrcPos, name: String },
    /// An attribute was used on a trait definition that is not recognized.
    UnrecognizedTraitAttribute { span: SrcPos, name: String },

    // ── Name/Symbol Errors (H050-H099) ──
    /// A symbol in an expression path could not be resolved.
    UnresolvedSymbol { span: SrcPos, name: String },
    /// A type path could not be resolved to any known type.
    UnresolvedTypePath { span: SrcPos, name: String },
    /// An entity with the same name was already defined in this scope.
    DuplicateEntity { span: SrcPos, name: String },
    /// A lifetime name was used that is not recognized or defined.
    UnrecognizedLifetime { span: SrcPos, name: String },

    // ── Type Errors (H100-H149) ──
    /// An integer literal value cannot fit in the target integer type.
    IntegerCastOutOfRange {
        span: SrcPos,
        value: String,
        target_type: String,
    },
    /// A global variable declaration had no initializer expression.
    GlobalVariableMustHaveInitializer { span: SrcPos, name: String },
    /// A local variable declaration had no initializer expression.
    LocalVariableMissingInitializer { span: SrcPos, name: String },
    /// A type alias definition is missing its right-hand side type.
    TypeAliasMustHaveType { span: SrcPos, name: String },
    /// An array type length expression did not evaluate to a usize value.
    ArrayLengthExpectedUSize { span: SrcPos },
    /// An array type length expression could not be evaluated at compile time.
    ArrayTypeLengthEvalError { span: SrcPos, err: EvalError },
    /// Slice types ([T]) can only appear behind references (&[T]) or pointers (*[T]).
    SliceTypesMustBeInRefOrPtr { span: SrcPos },
    /// A type alias's type evaluation failed.
    TypeAliasEvalError { span: SrcPos, name: String },

    // ── Expression and Literal Errors (H150-H199) ──
    /// The `match` expression is not yet implemented.
    MatchNotImplemented { span: SrcPos },
    /// The `for` loop is not yet implemented.
    ForLoopNotImplemented { span: SrcPos },
    /// The `await` expression is not yet implemented.
    AwaitNotImplemented { span: SrcPos },
    /// The `typeof` operator is not yet implemented.
    TypeofNotImplemented { span: SrcPos },
    /// Type reflection via `typeinfo` is not yet implemented.
    TypeReflectionNotImplemented { span: SrcPos },
    /// Closure expressions are not yet implemented.
    ClosureNotImplemented { span: SrcPos },
    /// Type potentials are not yet implemented.
    TypePotentialNotImplemented { span: SrcPos },
    /// Lifetime as standalone type is not yet implemented.
    LifetimeTypeNotImplemented { span: SrcPos },
    /// Generic type arguments in intermediate path segments are not yet supported.
    IntermediateGenericArgsNotSupported { span: SrcPos, path: String },

    // ── Statement/Control Flow Errors (H200-H249) ──
    /// A non-unit function body does not end with a return expression.
    MissingReturnStatement { span: SrcPos, name: String },
    /// Unsafe expression body is not yet implemented.
    UnsafeExprBodyNotImplemented { span: SrcPos },

    // ── Refinement Type Errors (H250-H299) ──
    /// A refinement type bound expression could not be evaluated to a constant.
    RefinementBoundNotConstant { span: SrcPos },
    /// A refinement type was applied to a non-integer base type.
    RefinementTypeOnNonInteger { span: SrcPos, base_type: String },
    /// The refinement type width must be between 1 and 128 (inclusive).
    RefinementWidthOutOfRange { span: SrcPos, width: String },
    /// The refinement type had no bounds specified and at least one is required.
    RefinementTypeEmpty { span: SrcPos },
    /// The refinement type width was zero or negative.
    RefinementWidthNotPositive { span: SrcPos, width: String },

    // ── Item/Definition Errors (H350-H399) ──
    /// The function's return type is non-unit but the body is missing.
    MissingFunctionBody { span: SrcPos, name: String },
}

/// Convert a `SrcPos` into an `Origin` for diagnostic output.
fn byte_span_to_origin(span: SrcPos) -> Origin {
    Origin::Point(SourcePosition {
        line: span.line as u32,
        column: span.column as u32,
        offset: span.offset,
        fileid: span.fileid,
    })
}

impl FormattableDiagnosticGroup for HirErr {
    fn group_id(&self) -> DiagnosticGroupId {
        DiagnosticGroupId::Hir
    }

    fn variant_id(&self) -> u16 {
        match self {
            // ── Attribute errors (H001-H049) ──
            HirErr::UnrecognizedModuleAttribute { .. } => 1,
            HirErr::UnrecognizedGlobalVarAttribute { .. } => 2,
            HirErr::UnrecognizedFunctionAttribute { .. } => 3,
            HirErr::UnrecognizedFunctionParamAttribute { .. } => 4,
            HirErr::UnrecognizedTypeAliasAttribute { .. } => 5,
            HirErr::UnrecognizedStructAttribute { .. } => 6,
            HirErr::UnrecognizedStructFieldAttribute { .. } => 7,
            HirErr::UnrecognizedEnumAttribute { .. } => 8,
            HirErr::UnrecognizedEnumVariantAttribute { .. } => 9,
            HirErr::UnrecognizedLocalVarAttribute { .. } => 10,
            HirErr::UnrecognizedTraitAttribute { .. } => 11,

            // ── Name/Symbol errors (H050-H099) ──
            HirErr::UnresolvedSymbol { .. } => 50,
            HirErr::UnresolvedTypePath { .. } => 51,
            HirErr::DuplicateEntity { .. } => 52,
            HirErr::UnrecognizedLifetime { .. } => 53,

            // ── Type errors (H100-H149) ──
            HirErr::IntegerCastOutOfRange { .. } => 100,
            HirErr::GlobalVariableMustHaveInitializer { .. } => 101,
            HirErr::LocalVariableMissingInitializer { .. } => 102,
            HirErr::TypeAliasMustHaveType { .. } => 103,
            HirErr::ArrayLengthExpectedUSize { .. } => 104,
            HirErr::ArrayTypeLengthEvalError { .. } => 105,
            HirErr::SliceTypesMustBeInRefOrPtr { .. } => 106,
            HirErr::TypeAliasEvalError { .. } => 107,

            // ── Expression/literal errors (H150-H199) ──
            HirErr::MatchNotImplemented { .. } => 150,
            HirErr::ForLoopNotImplemented { .. } => 151,
            HirErr::AwaitNotImplemented { .. } => 152,
            HirErr::TypeofNotImplemented { .. } => 153,
            HirErr::TypeReflectionNotImplemented { .. } => 155,
            HirErr::ClosureNotImplemented { .. } => 156,
            HirErr::TypePotentialNotImplemented { .. } => 157,
            HirErr::LifetimeTypeNotImplemented { .. } => 158,
            HirErr::IntermediateGenericArgsNotSupported { .. } => 159,

            // ── Control flow errors (H200-H249) ──
            HirErr::MissingReturnStatement { .. } => 200,
            HirErr::UnsafeExprBodyNotImplemented { .. } => 201,

            // ── Refinement type errors (H250-H299) ──
            HirErr::RefinementBoundNotConstant { .. } => 250,
            HirErr::RefinementTypeOnNonInteger { .. } => 251,
            HirErr::RefinementWidthOutOfRange { .. } => 252,
            HirErr::RefinementTypeEmpty { .. } => 253,
            HirErr::RefinementWidthNotPositive { .. } => 254,

            // ── Item/definition errors (H350-H399) ──
            HirErr::MissingFunctionBody { .. } => 350,
        }
    }

    fn format(&self) -> DiagnosticInfo {
        match self {
            // ════════════════════════════════════════════════════════════════
            // ATTRIBUTE ERRORS (H001-H049)
            // ════════════════════════════════════════════════════════════════
            HirErr::UnrecognizedModuleAttribute { span, name } => DiagnosticInfo {
                message: format!(
                    "unrecognized module attribute `{name}`\n\
                     \n  = note: module attributes are placed before the `mod` keyword\n\
                     \n  = help: module items do not support custom attributes. Remove the attribute.\n\
                     \n  = example:\n           mod my_module {{\n               // ... module contents\n           }}"
                ),
                origin: byte_span_to_origin(*span),
            },

            HirErr::UnrecognizedGlobalVarAttribute { span, name } => DiagnosticInfo {
                message: format!(
                    "unrecognized global variable attribute `{name}`\n\
                     \n  = note: global variables support only `#[no_mangle]`\n\
                     \n  = help: remove this attribute or use a recognized one\n\
                     \n  = example:\n           #[no_mangle]\n           static MY_CONST: i32 = 42;"
                ),
                origin: byte_span_to_origin(*span),
            },

            HirErr::UnrecognizedFunctionAttribute { span, name } => DiagnosticInfo {
                message: format!(
                    "unrecognized function attribute `{name}`\n\
                     \n  = note: functions support `#[no_mangle]` and `#[extern(abi)]`\n\
                     \n  = help: remove this attribute or use a recognized one\n\
                     \n  = example:\n           #[no_mangle]\n           fn my_function() {{ }}\n\
                     \n           #[extern(\"C\")]\n           fn external_fn() {{ }}"
                ),
                origin: byte_span_to_origin(*span),
            },

            HirErr::UnrecognizedFunctionParamAttribute { span, name } => DiagnosticInfo {
                message: format!(
                    "unrecognized function parameter attribute `{name}`\n\
                     \n  = note: function parameters do not support custom attributes\n\
                     \n  = help: remove the attribute from this parameter\n\
                     \n  = example:\n           fn foo(x: i32, y: bool) {{ }}"
                ),
                origin: byte_span_to_origin(*span),
            },

            HirErr::UnrecognizedTypeAliasAttribute { span, name } => DiagnosticInfo {
                message: format!(
                    "unrecognized type alias attribute `{name}`\n\
                     \n  = note: type aliases do not support custom attributes\n\
                     \n  = help: remove the attribute from this type alias\n\
                     \n  = example:\n           type MyInt = i32;"
                ),
                origin: byte_span_to_origin(*span),
            },

            HirErr::UnrecognizedStructAttribute { span, name } => DiagnosticInfo {
                message: format!(
                    "unrecognized struct attribute `{name}`\n\
                     \n  = note: struct definitions do not support custom attributes\n\
                     \n  = help: remove the attribute from this struct definition\n\
                     \n  = example:\n           struct Point {{\n               x: i32,\n               y: i32,\n           }}"
                ),
                origin: byte_span_to_origin(*span),
            },

            HirErr::UnrecognizedStructFieldAttribute { span, name } => DiagnosticInfo {
                message: format!(
                    "unrecognized struct field attribute `{name}`\n\
                     \n  = note: struct fields do not support custom attributes\n\
                     \n  = help: remove the attribute from this struct field\n\
                     \n  = example:\n           struct Point {{\n               x: i32,\n               y: i32,\n           }}"
                ),
                origin: byte_span_to_origin(*span),
            },

            HirErr::UnrecognizedEnumAttribute { span, name } => DiagnosticInfo {
                message: format!(
                    "unrecognized enum attribute `{name}`\n\
                     \n  = note: enum definitions do not support custom attributes\n\
                     \n  = help: remove the attribute from this enum definition\n\
                     \n  = example:\n           enum Color {{\n               Red,\n               Green,\n               Blue,\n           }}"
                ),
                origin: byte_span_to_origin(*span),
            },

            HirErr::UnrecognizedEnumVariantAttribute { span, name } => DiagnosticInfo {
                message: format!(
                    "unrecognized enum variant attribute `{name}`\n\
                     \n  = note: enum variants do not support custom attributes\n\
                     \n  = help: remove the attribute from this enum variant\n\
                     \n  = example:\n           enum Option<T> {{\n               Some(T),\n               None,\n           }}"
                ),
                origin: byte_span_to_origin(*span),
            },

            HirErr::UnrecognizedLocalVarAttribute { span, name } => DiagnosticInfo {
                message: format!(
                    "unrecognized local variable attribute `{name}`\n\
                     \n  = note: local variables do not support custom attributes\n\
                     \n  = help: remove the attribute from this local variable\n\
                     \n  = example:\n           fn foo() {{\n               let x = 42;\n               var y = 10;\n           }}"
                ),
                origin: byte_span_to_origin(*span),
            },

            HirErr::UnrecognizedTraitAttribute { span, name } => DiagnosticInfo {
                message: format!(
                    "unrecognized trait attribute `{name}`\n\
                     \n  = note: trait definitions do not support custom attributes\n\
                     \n  = help: remove the attribute from this trait definition\n\
                     \n  = example:\n           trait MyTrait {{\n               fn method(&self);\n           }}"
                ),
                origin: byte_span_to_origin(*span),
            },

            // ════════════════════════════════════════════════════════════════
            // NAME/SYMBOL ERRORS (H050-H099)
            // ════════════════════════════════════════════════════════════════
            HirErr::UnresolvedSymbol { span, name } => DiagnosticInfo {
                message: format!(
                    "cannot resolve symbol `{name}` in this context\n\
                     \n  = note: the name `{name}` could not be found in the current scope\n\
                     \n  = help: make sure the name is spelled correctly and is in scope.\n\
                     \n         If this is a type, use it in a type position (after `:` or as a type argument).\n\
                     \n         If this is a value, make sure it has been defined before use.\n\
                     \n  = example:\n           fn bar() {{\n               let x = foo(); // `foo` must be defined or imported\n           }}"
                ),
                origin: byte_span_to_origin(*span),
            },

            HirErr::UnresolvedTypePath { span, name } => DiagnosticInfo {
                message: format!(
                    "cannot resolve type path `{name}`\n\
                     \n  = note: the type `{name}` could not be found in the current scope\n\
                     \n  = help: ensure the type is spelled correctly, is in scope, and is a type\n\
                     \n         (struct, enum, type alias) rather than a value or function.\n\
                     \n         You may need to import it with `use` at the top of the module.\n\
                     \n  = example:\n           use std::collections::HashMap;\n\
                     \n           fn foo(map: HashMap<String, i32>) {{ }}"
                ),
                origin: byte_span_to_origin(*span),
            },

            HirErr::DuplicateEntity { span, name } => DiagnosticInfo {
                message: format!(
                    "duplicate definition of `{name}`\n\
                     \n  = note: an entity with the name `{name}` has already been defined in this scope.\n\
                     \n        Names must be unique within a module.\n\
                     \n  = help: rename one of the definitions to avoid the conflict.\n\
                     \n  = example:\n           // This is not allowed:\n           fn foo() {{ }}\n           fn foo() {{ }} // error: duplicate definition\n\
                     \n           // Instead, use different names:\n           fn foo() {{ }}\n           fn foo_v2() {{ }}"
                ),
                origin: byte_span_to_origin(*span),
            },

            HirErr::UnrecognizedLifetime { span, name } => DiagnosticInfo {
                message: format!(
                    "unrecognized lifetime name `'{name}`\n\
                     \n  = note: lifetimes in this language are one of: `'static`, `'gc`,\n\
                     \n         `'thread`, `'task`, or `'_` (inferred). Got `'{name}`.\n\
                     \n  = help: use one of the valid lifetime names or `'_` for an inferred lifetime.\n\
                     \n  = example:\n           fn foo(x: &'static i32) {{ }}\n           fn bar(x: &'_ i32) {{ }}"
                ),
                origin: byte_span_to_origin(*span),
            },

            // ════════════════════════════════════════════════════════════════
            // TYPE ERRORS (H100-H149)
            // ════════════════════════════════════════════════════════════════
            HirErr::IntegerCastOutOfRange {
                span,
                value,
                target_type,
            } => DiagnosticInfo {
                message: format!(
                    "integer literal `{value}` cannot be represented in type `{target_type}`\n\
                     \n  = note: the value `{value}` is outside the valid range for `{target_type}`.\n\
                     \n  = help: use a smaller integer literal or a wider integer type.\n\
                     \n  = example:\n           let a: i8 = 127;   // valid: i8 ranges from -128 to 127\n           let b: i8 = 128;   // error: 128 does not fit in i8\n           let c: i16 = 128;  // ok: 128 fits in i16"
                ),
                origin: byte_span_to_origin(*span),
            },

            HirErr::GlobalVariableMustHaveInitializer { span, name } => DiagnosticInfo {
                message: format!(
                    "global variable `{name}` must have an initializer\n\
                     \n  = note: global variables at module level must be initialized at the point of declaration.\n\
                     \n        Unlike local variables, globals cannot remain uninitialized.\n\
                     \n  = help: provide an initializer expression:\n\
                     \n  = example:\n           static MAX_SIZE: i32 = 1024;     // ok\n           static MIN_SIZE: i32;           // error: missing initializer"
                ),
                origin: byte_span_to_origin(*span),
            },

            HirErr::LocalVariableMissingInitializer { span, name } => DiagnosticInfo {
                message: format!(
                    "local variable `{name}` must have an initializer\n\
                     \n  = note: variables declared with `let` or `var` must be initialized.\n\
                     \n  = help: provide an initializer expression:\n\
                     \n  = example:\n           fn foo() {{\n               let x = 42;     // ok\n               let y;             // error: missing initializer\n               var z = vec![1];   // ok\n           }}"
                ),
                origin: byte_span_to_origin(*span),
            },

            HirErr::TypeAliasMustHaveType { span, name } => DiagnosticInfo {
                message: format!(
                    "type alias `{name}` must have a type on the right-hand side\n\
                     \n  = note: a type alias requires an equals sign followed by a type expression.\n\
                     \n  = help: provide the type that this alias refers to.\n\
                     \n  = example:\n           type MyInt = i32;                 // ok\n           type MyInt;                       // error\n           type MyInt<T> = Result<T, Error>; // ok"
                ),
                origin: byte_span_to_origin(*span),
            },

            HirErr::ArrayLengthExpectedUSize { span } => DiagnosticInfo {
                message: format!(
                    "array length must evaluate to a `usize` value\n\
                     \n  = note: the length expression in an array type `[T; N]` must evaluate\n\
                     \n        to a non-negative usize value at compile time.\n\
                     \n  = help: use a constant usize expression:\n\
                     \n  = example:\n           let arr: [i32; 5] = [0; 5];        // ok: literal usize\n           const N: usize = 10;\n           let arr: [i32; N] = [0; N];      // ok: const usize\n           let arr: [i32; \"foo\"] = [];     // error: string is not usize"
                ),
                origin: byte_span_to_origin(*span),
            },

            HirErr::ArrayTypeLengthEvalError { span, err } => DiagnosticInfo {
                message: format!(
                    "failed to evaluate array length expression: {err}\n\
                     \n  = note: the length expression in an array type `[T; N]` must be a constant\n\
                     \n        expression that can be evaluated at compile time.\n\
                     \n  = help: ensure the length expression is a valid constant expression:\n\
                     \n  = example:\n           const N: usize = 10;\n           let arr: [i32; N] = [0; N];      // ok\n           let arr: [i32; some_var] = [];   // error: `some_var` is not const"
                ),
                origin: byte_span_to_origin(*span),
            },

            HirErr::SliceTypesMustBeInRefOrPtr { span } => DiagnosticInfo {
                message: format!(
                    "slice types `[T]` cannot appear outside references or pointers\n\
                     \n  = note: bare slice types like `[i32]` have no compile-time known size\n\
                     \n        and cannot be used as variable types. They must be wrapped in\n\
                     \n        a reference (`&[T]`) or pointer (`*[T]`).\n\
                     \n  = help: use `&[T]` or `*[T]` instead of bare `[T]`.\n\
                     \n  = example:\n           fn foo(slice: &[i32]) {{ }}  // ok\n           fn bar(slice: [i32]) {{ }}  // error: bare slice"
                ),
                origin: byte_span_to_origin(*span),
            },

            HirErr::TypeAliasEvalError { span, name } => DiagnosticInfo {
                message: format!(
                    "failed to evaluate the type for type alias `{name}`\n\
                     \n  = note: the type expression on the right-hand side of this type alias\n\
                     \n        could not be fully resolved during HIR lowering.\n\
                     \n  = help: check that the type expression is valid and all referenced types are in scope."
                ),
                origin: byte_span_to_origin(*span),
            },

            // ════════════════════════════════════════════════════════════════
            // EXPRESSION/LITERAL ERRORS (H150-H199)
            // ════════════════════════════════════════════════════════════════
            HirErr::MatchNotImplemented { span } => DiagnosticInfo {
                message: format!(
                    "`match` expressions are not yet implemented\n\
                     \n  = note: pattern matching with `match` is planned but not yet available.\n\
                     \n  = help: use `if`/`else if` chains as a workaround.\n\
                     \n  = example:\n           // Workaround for match:\n           if x == 1 {{\n               // handle case 1\n           }} else if x == 2 {{\n               // handle case 2\n           }} else {{\n               // default case\n           }}"
                ),
                origin: byte_span_to_origin(*span),
            },

            HirErr::ForLoopNotImplemented { span } => DiagnosticInfo {
                message: format!(
                    "`for` loops are not yet implemented\n\
                     \n  = note: `for` .. `in` loops are planned but not yet available.\n\
                     \n  = help: use `while` loops or manual iteration as a workaround.\n\
                     \n  = example:\n           // Workaround for for loop:\n           let mut i = 0;\n           while i < 10 {{\n               // ... loop body\n               i += 1;\n           }}"
                ),
                origin: byte_span_to_origin(*span),
            },

            HirErr::AwaitNotImplemented { span } => DiagnosticInfo {
                message: format!(
                    "`await` expressions are not yet implemented\n\
                     \n  = note: async/await is planned but not yet available.\n\
                     \n  = help: use synchronous blocking calls as a workaround."
                ),
                origin: byte_span_to_origin(*span),
            },

            HirErr::TypeofNotImplemented { span } => DiagnosticInfo {
                message: format!(
                    "`typeof` operator is not yet implemented\n\
                     \n  = note: the `typeof` reflection operator is planned but not yet available.\n\
                     \n  = help: specify the type explicitly instead of using typeof."
                ),
                origin: byte_span_to_origin(*span),
            },

            HirErr::TypeReflectionNotImplemented { span } => DiagnosticInfo {
                message: format!(
                    "type reflection is not yet implemented\n\
                     \n  = note: compile-time type inspection (e.g., `typeinfo`, `typeof`) is\n\
                     \n        planned but not yet available.\n\
                     \n  = help: use explicit type annotations instead."
                ),
                origin: byte_span_to_origin(*span),
            },

            HirErr::ClosureNotImplemented { span } => DiagnosticInfo {
                message: format!(
                    "closure expressions are not yet implemented\n\
                     \n  = note: anonymous functions (closures) are planned but not yet available.\n\
                     \n  = help: define a named function instead.\n\
                     \n  = example:\n           fn my_callback(x: i32) -> i32 {{ x * 2 }}\n           process(my_callback);"
                ),
                origin: byte_span_to_origin(*span),
            },

            HirErr::TypePotentialNotImplemented { span } => DiagnosticInfo {
                message: format!(
                    "type potentials are not yet implemented\n\
                     \n  = note: type potentials (type computed from a block expression) are planned\n\
                     \n        but not yet available.\n\
                     \n  = help: specify the type explicitly."
                ),
                origin: byte_span_to_origin(*span),
            },

            HirErr::LifetimeTypeNotImplemented { span } => DiagnosticInfo {
                message: format!(
                    "lifetimes as standalone types are not yet implemented\n\
                     \n  = note: using a lifetime `'a` as a type expression is planned but not\n\
                     \n        yet available.\n\
                     \n  = help: lifetimes can only be used in reference type positions currently:\n\
                     \n         `&'a i32`, not `'a` as a standalone type."
                ),
                origin: byte_span_to_origin(*span),
            },

            HirErr::IntermediateGenericArgsNotSupported { span, path } => DiagnosticInfo {
                message: format!(
                    "generic type arguments in intermediate path segments are not supported: `{path}`\n\
                     \n  = note: the compiler encountered a type path like `Foo<i32>::Bar` where\n\
                     \n        a generic argument appears on a non-final segment.\n\
                     \n  = help: this syntax is not yet supported. Try restructuring the type path\n\
                     \n         so that generic arguments only appear on the final segment:\n\
                     \n  = example:\n           // Not yet supported:\n           Foo<i32>::Bar\n           // Use instead:\n           Foo::Bar<i32>  // if applicable"
                ),
                origin: byte_span_to_origin(*span),
            },

            // ════════════════════════════════════════════════════════════════
            // CONTROL FLOW ERRORS (H200-H249)
            // ════════════════════════════════════════════════════════════════
            HirErr::MissingReturnStatement { span, name } => DiagnosticInfo {
                message: format!(
                    "missing return statement in function `{name}`\n\
                     \n  = note: this function has a non-unit return type but its body does not\n\
                     \n        end with a `return` expression. Every code path must return a value.\n\
                     \n  = help: add a `return` expression at the end of the function body, or\n\
                     \n         change the return type to `()` (unit) if nothing should be returned.\n\
                     \n  = example:\n           fn add(a: i32, b: i32) -> i32 {{\n               return a + b;  // explicit return\n           }}\n\
                     \n           // The final expression is also returned implicitly:\n           fn add_implicit(a: i32, b: i32) -> i32 {{\n               a + b  // no semicolon; this is the return value\n           }}"
                ),
                origin: byte_span_to_origin(*span),
            },

            HirErr::UnsafeExprBodyNotImplemented { span } => DiagnosticInfo {
                message: format!(
                    "unsafe expressions with bodies are not yet implemented\n\
                     \n  = note: `unsafe {{ ... }}` blocks with an expression body that computes\n\
                     \n        a value are not yet supported.\n\
                     \n  = help: use a statement-level `unsafe` block instead.\n\
                     \n  = example:\n           unsafe {{  // this is supported\n               // ... unsafe operations\n           }}\n           unsafe expr  // not yet supported"
                ),
                origin: byte_span_to_origin(*span),
            },

            // ════════════════════════════════════════════════════════════════
            // REFINEMENT TYPE ERRORS (H250-H299)
            // ════════════════════════════════════════════════════════════════
            HirErr::RefinementBoundNotConstant { span } => DiagnosticInfo {
                message: format!(
                    "refinement type bound must be a constant expression\n\
                     \n  = note: the bounds in a refinement type (e.g., `i32: [0:100]`) must be\n\
                     \n        computable at compile time.\n\
                     \n  = help: use literal values, constants, or expressions that can be fully\n\
                     \n         evaluated during compilation.\n\
                     \n  = example:\n           type SmallInt = i32: [0:255];   // ok: literal bounds\n           const MAX: i32 = 100;\n           type Range = i32: [0:MAX];   // ok: const bound\n           fn foo(limit: i32) {{\n               type Dynamic = i32: [0:limit]; // error: limit is not constant\n           }}"
                ),
                origin: byte_span_to_origin(*span),
            },

            HirErr::RefinementTypeOnNonInteger { span, base_type } => DiagnosticInfo {
                message: format!(
                    "refinement types can only be applied to integer types, not `{base_type}`\n\
                     \n  = note: refinement types (using `:`) restrict the range of integer types.\n\
                     \n        They can only be applied to integer types like `i8`, `u8`, `i32`, `u64`, etc.\n\
                     \n  = help: use a valid integer type as the base of this refinement type.\n\
                     \n  = example:\n           type Small = i32: [0:255];    // ok\n           type Small = f32: [0:1];     // error: float is not an integer\n           type Small = bool: [0:1];    // error: bool is not an integer"
                ),
                origin: byte_span_to_origin(*span),
            },

            HirErr::RefinementWidthOutOfRange { span, width } => DiagnosticInfo {
                message: format!(
                    "refinement type width `{width}` is out of valid range (1 to 128)\n\
                     \n  = note: the width parameter in a refinement type (e.g., `u8: 6` for 6-bit values)\n\
                     \n        must be between 1 and 128 bits inclusive.\n\
                     \n  = help: use a width value between 1 and 128.\n\
                     \n  = example:\n           type HalfByte = u8: 4;        // ok: 4-bit values [0:15]\n           type TripleWord = u128: 96;  // ok: 96-bit values\n           type Invalid = u8: 0;        // error: width must be >= 1\n           type Invalid = u8: 200;       // error: width must be <= 128"
                ),
                origin: byte_span_to_origin(*span),
            },

            HirErr::RefinementTypeEmpty { span } => DiagnosticInfo {
                message: format!(
                    "refinement type must have at least one bound\n\
                     \n  = note: a refinement type requires either a width, a minimum, or a maximum\n\
                     \n        bound to restrict the integer range.\n\
                     \n  = help: add a width, minimum bound, maximum bound, or range to the type.\n\
                     \n  = example:\n           type Small = u8: 4;           // ok: width only\n           type Range = i32: [0:100];   // ok: min and max\n           type MinOnly = i32: [0:];    // ok: min only\n           type MaxOnly = i32: [:100];  // ok: max only\n           type Empty = i32:;           // error: no bounds"
                ),
                origin: byte_span_to_origin(*span),
            },

            HirErr::RefinementWidthNotPositive { span, width } => DiagnosticInfo {
                message: format!(
                    "refinement type width must be a positive integer, got `{width}`\n\
                     \n  = note: the width parameter in a refinement type must be greater than zero.\n\
                     \n        A width of 0 bits would result in an uninhabitable type.\n\
                     \n  = help: use a positive integer value for the width.\n\
                     \n  = example:\n           type HalfByte = u8: 4;  // ok\n           // The following would be invalid:\n           type Empty = u8: 0;    // width must be > 0"
                ),
                origin: byte_span_to_origin(*span),
            },

            // ════════════════════════════════════════════════════════════════
            // ITEM/DEFINITION ERRORS (H350-H399)
            // ════════════════════════════════════════════════════════════════
            HirErr::MissingFunctionBody { span, name } => DiagnosticInfo {
                message: format!(
                    "function `{name}` has a non-unit return type but no body\n\
                     \n  = note: functions that are declared without a body (extern declarations)\n\
                     \n        must have the `extern` attribute or return unit `()`.\n\
                     \n  = help: add a function body, change the return type to `()`, or mark\n\
                     \n         it as `extern`.\n\
                     \n  = example:\n           fn foo() -> i32 {{ 42 }}         // ok: has body\n           fn bar() -> i32;               // error: no body and non-unit return\n           extern fn baz() -> i32;         // ok: extern with no body\n           fn qux() -> ();                 // ok: unit return"
                ),
                origin: byte_span_to_origin(*span),
            },
        }
    }
}
