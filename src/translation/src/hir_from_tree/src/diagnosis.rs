use nitrate_diagnosis::{DiagnosticGroupId, DiagnosticInfo, FormattableDiagnosticGroup, Origin};

/// Comprehensive error codes for the HIR lowering stage (group H).
///
/// Error codes follow the pattern HXYZ where:
/// - H = HIR lowering group
/// - X  = category (1=attributes, 2=names/symbols, 3=types, 4=expressions,
///                  5=control flow, 6=refinement, 7=unimplemented, 8=definitions,
///                  9=syntax/other)
/// - YZ = specific error variant
///
/// Error code ranges:
///   H001-H019: Attribute/annotation errors
///   H020-H039: Name/symbol errors
///   H040-H059: Type errors
///   H060-H079: Expression and literal errors
///   H080-H099: Statement and control flow errors
///   H100-H119: Refinement type errors
///   H120-H139: Unimplemented features
///   H140-H159: Item and definition errors
///   H160-H179: Security and safety errors
#[derive(Debug, Clone)]
pub(crate) enum HirErr {
    // ── Attribute Errors (H001-H019) ──
    /// An attribute was used on a module item that is not recognized.
    UnrecognizedModuleAttribute(String),
    /// An attribute was used on a global variable that is not recognized.
    UnrecognizedGlobalVarAttribute(String),
    /// An attribute was used on a function that is not recognized.
    UnrecognizedFunctionAttribute(String),
    /// An attribute was used on a function parameter that is not recognized.
    UnrecognizedFunctionParamAttribute(String),
    /// An attribute was used on a type alias that is not recognized.
    UnrecognizedTypeAliasAttribute(String),
    /// An attribute was used on a struct definition that is not recognized.
    UnrecognizedStructAttribute(String),
    /// An attribute was used on a struct field that is not recognized.
    UnrecognizedStructFieldAttribute(String),
    /// An attribute was used on an enum definition that is not recognized.
    UnrecognizedEnumAttribute(String),
    /// An attribute was used on an enum variant that is not recognized.
    UnrecognizedEnumVariantAttribute(String),
    /// An attribute was used on a local variable that is not recognized.
    UnrecognizedLocalVarAttribute(String),
    /// An attribute was used on a trait definition that is not recognized.
    UnrecognizedTraitAttribute(String),

    // ── Name/Symbol Errors (H020-H039) ──
    /// A symbol in an expression path could not be resolved.
    UnresolvedSymbol(String),
    /// A type path could not be resolved to any known type.
    UnresolvedTypePath(String),
    /// An entity with the same name was already defined in this scope.
    DuplicateEntity(String),
    /// A lifetime name was used that is not recognized or defined.
    UnrecognizedLifetime(String),

    // ── Type Errors (H040-H059) ──
    /// An integer literal value cannot fit in the target integer type.
    IntegerCastOutOfRange { value: String, target_type: String },
    /// A global variable declaration had no initializer expression.
    GlobalVariableMustHaveInitializer(String),
    /// A local variable declaration had no initializer expression.
    LocalVariableMissingInitializer(String),
    /// A type alias definition is missing its right-hand side type.
    TypeAliasMustHaveType(String),
    /// An array type length expression did not evaluate to a usize value.
    ArrayLengthExpectedUSize,
    /// An array type length expression could not be evaluated at compile time.
    ArrayTypeLengthEvalError,
    /// Slice types ([T]) can only appear behind references (&[T]) or pointers (*[T]).
    SliceTypesMustBeInRefOrPtr,
    /// A type alias's type evaluation failed.
    TypeAliasEvalError(String),

    // ── Expression and Literal Errors (H060-H079) ──
    /// The `match` expression is not yet implemented.
    MatchNotImplemented,
    /// The `for` loop is not yet implemented.
    ForLoopNotImplemented,
    /// The `await` expression is not yet implemented.
    AwaitNotImplemented,
    /// The `typeof` operator is not yet implemented.
    TypeofNotImplemented,
    /// The `..` range operator is not yet implemented.
    RangeOperatorNotImplemented,
    /// Type reflection via `typeinfo` is not yet implemented.
    TypeReflectionNotImplemented,
    /// Closure expressions are not yet implemented.
    ClosureNotImplemented,
    /// Latent types are not yet implemented.
    LatentTypeNotImplemented,
    /// Lifetime as standalone type is not yet implemented.
    LifetimeTypeNotImplemented,
    /// Generic type arguments in intermediate path segments are not yet supported.
    IntermediateGenericArgsNotSupported(String),

    // ── Statement/Control Flow Errors (H080-H099) ──
    /// A non-unit function body does not end with a return expression.
    MissingReturnStatement(String),
    /// Unsafe expression body is not yet implemented.
    UnsafeExprBodyNotImplemented,

    // ── Refinement Type Errors (H100-H119) ──
    /// A refinement type bound expression could not be evaluated to a constant.
    RefinementBoundNotConstant,
    /// A refinement type was applied to a non-integer base type.
    RefinementTypeOnNonInteger(String),
    /// The refinement type width must be between 1 and 128 (inclusive).
    RefinementWidthOutOfRange(String),
    /// The refinement type had no bounds specified and at least one is required.
    RefinementTypeEmpty,
    /// The refinement type width was zero or negative.
    RefinementWidthNotPositive(String),

    // ── Item/Definition Errors (H140-H159) ──
    /// The function's return type is non-unit but the body is missing.
    MissingFunctionBody(String),
}

/// Represents the origin of an attribute error with the attribute name.
fn attr_origin(attr_name: &str) -> Origin {
    Origin::None
}

impl FormattableDiagnosticGroup for HirErr {
    fn group_id(&self) -> DiagnosticGroupId {
        DiagnosticGroupId::Hir
    }

    fn variant_id(&self) -> u16 {
        match self {
            // Attribute errors (H001-H019)
            HirErr::UnrecognizedModuleAttribute(_) => 1,
            HirErr::UnrecognizedGlobalVarAttribute(_) => 2,
            HirErr::UnrecognizedFunctionAttribute(_) => 3,
            HirErr::UnrecognizedFunctionParamAttribute(_) => 4,
            HirErr::UnrecognizedTypeAliasAttribute(_) => 5,
            HirErr::UnrecognizedStructAttribute(_) => 6,
            HirErr::UnrecognizedStructFieldAttribute(_) => 7,
            HirErr::UnrecognizedEnumAttribute(_) => 8,
            HirErr::UnrecognizedEnumVariantAttribute(_) => 9,
            HirErr::UnrecognizedLocalVarAttribute(_) => 10,
            HirErr::UnrecognizedTraitAttribute(_) => 11,

            // Name/Symbol errors (H020-H039)
            HirErr::UnresolvedSymbol(_) => 20,
            HirErr::UnresolvedTypePath(_) => 21,
            HirErr::DuplicateEntity(_) => 22,
            HirErr::UnrecognizedLifetime(..) => 23,

            // Type errors (H040-H059)
            HirErr::IntegerCastOutOfRange { .. } => 40,
            HirErr::GlobalVariableMustHaveInitializer(_) => 41,
            HirErr::LocalVariableMissingInitializer(_) => 42,
            HirErr::TypeAliasMustHaveType(_) => 43,
            HirErr::ArrayLengthExpectedUSize => 44,
            HirErr::ArrayTypeLengthEvalError => 45,
            HirErr::SliceTypesMustBeInRefOrPtr => 46,
            HirErr::TypeAliasEvalError(_) => 47,

            // Expression/literal errors (H060-H079)
            HirErr::MatchNotImplemented => 60,
            HirErr::ForLoopNotImplemented => 61,
            HirErr::AwaitNotImplemented => 62,
            HirErr::TypeofNotImplemented => 63,
            HirErr::RangeOperatorNotImplemented => 64,
            HirErr::TypeReflectionNotImplemented => 65,
            HirErr::ClosureNotImplemented => 66,
            HirErr::LatentTypeNotImplemented => 67,
            HirErr::LifetimeTypeNotImplemented => 68,
            HirErr::IntermediateGenericArgsNotSupported(_) => 69,

            // Control flow errors (H080-H099)
            HirErr::MissingReturnStatement(_) => 80,
            HirErr::UnsafeExprBodyNotImplemented => 81,

            // Refinement type errors (H100-H119)
            HirErr::RefinementBoundNotConstant => 100,
            HirErr::RefinementTypeOnNonInteger(_) => 101,
            HirErr::RefinementWidthOutOfRange(_) => 102,
            HirErr::RefinementTypeEmpty => 103,
            HirErr::RefinementWidthNotPositive(_) => 104,

            // Item/definition errors (H140-H159)
            HirErr::MissingFunctionBody(_) => 140,
        }
    }

    fn format(&self) -> DiagnosticInfo {
        match self {
            // ════════════════════════════════════════════════════════════════
            // ATTRIBUTE ERRORS (H001-H019)
            // ════════════════════════════════════════════════════════════════
            HirErr::UnrecognizedModuleAttribute(name) => DiagnosticInfo {
                message: format!(
                    "unrecognized module attribute `{name}`\n\
                     \n  = note: module attributes are placed before the `mod` keyword\n\
                     \n  = help: module items do not support custom attributes. Remove the attribute.\n\
                     \n  = example:\n           mod my_module {{\n               // ... module contents\n           }}"
                ),
                origin: Origin::None,
            },

            HirErr::UnrecognizedGlobalVarAttribute(name) => DiagnosticInfo {
                message: format!(
                    "unrecognized global variable attribute `{name}`\n\
                     \n  = note: global variables support only `#[no_mangle]`\n\
                     \n  = help: remove this attribute or use a recognized one\n\
                     \n  = example:\n           #[no_mangle]\n           static MY_CONST: i32 = 42;"
                ),
                origin: Origin::None,
            },

            HirErr::UnrecognizedFunctionAttribute(name) => DiagnosticInfo {
                message: format!(
                    "unrecognized function attribute `{name}`\n\
                     \n  = note: functions support `#[no_mangle]` and `#[extern(abi)]`\n\
                     \n  = help: remove this attribute or use a recognized one\n\
                     \n  = example:\n           #[no_mangle]\n           fn my_function() {{ }}\n\
                     \n           #[extern(\"C\")]\n           fn external_fn() {{ }}"
                ),
                origin: Origin::None,
            },

            HirErr::UnrecognizedFunctionParamAttribute(name) => DiagnosticInfo {
                message: format!(
                    "unrecognized function parameter attribute `{name}`\n\
                     \n  = note: function parameters do not support custom attributes\n\
                     \n  = help: remove the attribute from this parameter\n\
                     \n  = example:\n           fn foo(x: i32, y: bool) {{ }}"
                ),
                origin: Origin::None,
            },

            HirErr::UnrecognizedTypeAliasAttribute(name) => DiagnosticInfo {
                message: format!(
                    "unrecognized type alias attribute `{name}`\n\
                     \n  = note: type aliases do not support custom attributes\n\
                     \n  = help: remove the attribute from this type alias\n\
                     \n  = example:\n           type MyInt = i32;"
                ),
                origin: Origin::None,
            },

            HirErr::UnrecognizedStructAttribute(name) => DiagnosticInfo {
                message: format!(
                    "unrecognized struct attribute `{name}`\n\
                     \n  = note: struct definitions do not support custom attributes\n\
                     \n  = help: remove the attribute from this struct definition\n\
                     \n  = example:\n           struct Point {{\n               x: i32,\n               y: i32,\n           }}"
                ),
                origin: Origin::None,
            },

            HirErr::UnrecognizedStructFieldAttribute(name) => DiagnosticInfo {
                message: format!(
                    "unrecognized struct field attribute `{name}`\n\
                     \n  = note: struct fields do not support custom attributes\n\
                     \n  = help: remove the attribute from this struct field\n\
                     \n  = example:\n           struct Point {{\n               x: i32,\n               y: i32,\n           }}"
                ),
                origin: Origin::None,
            },

            HirErr::UnrecognizedEnumAttribute(name) => DiagnosticInfo {
                message: format!(
                    "unrecognized enum attribute `{name}`\n\
                     \n  = note: enum definitions do not support custom attributes\n\
                     \n  = help: remove the attribute from this enum definition\n\
                     \n  = example:\n           enum Color {{\n               Red,\n               Green,\n               Blue,\n           }}"
                ),
                origin: Origin::None,
            },

            HirErr::UnrecognizedEnumVariantAttribute(name) => DiagnosticInfo {
                message: format!(
                    "unrecognized enum variant attribute `{name}`\n\
                     \n  = note: enum variants do not support custom attributes\n\
                     \n  = help: remove the attribute from this enum variant\n\
                     \n  = example:\n           enum Option<T> {{\n               Some(T),\n               None,\n           }}"
                ),
                origin: Origin::None,
            },

            HirErr::UnrecognizedLocalVarAttribute(name) => DiagnosticInfo {
                message: format!(
                    "unrecognized local variable attribute `{name}`\n\
                     \n  = note: local variables do not support custom attributes\n\
                     \n  = help: remove the attribute from this local variable\n\
                     \n  = example:\n           fn foo() {{\n               let x = 42;\n               var y = 10;\n           }}"
                ),
                origin: Origin::None,
            },

            HirErr::UnrecognizedTraitAttribute(name) => DiagnosticInfo {
                message: format!(
                    "unrecognized trait attribute `{name}`\n\
                     \n  = note: trait definitions do not support custom attributes\n\
                     \n  = help: remove the attribute from this trait definition\n\
                     \n  = example:\n           trait MyTrait {{\n               fn method(&self);\n           }}"
                ),
                origin: Origin::None,
            },

            // ════════════════════════════════════════════════════════════════
            // NAME/SYMBOL ERRORS (H020-H039)
            // ════════════════════════════════════════════════════════════════
            HirErr::UnresolvedSymbol(name) => DiagnosticInfo {
                message: format!(
                    "cannot resolve symbol `{name}` in this context\n\
                     \n  = note: the name `{name}` could not be found in the current scope\n\
                     \n  = help: make sure the name is spelled correctly and is in scope.\n\
                     \n         If this is a type, use it in a type position (after `:` or as a type argument).\n\
                     \n         If this is a value, make sure it has been defined before use.\n\
                     \n  = example:\n           fn bar() {{\n               let x = foo(); // `foo` must be defined or imported\n           }}"
                ),
                origin: Origin::None,
            },

            HirErr::UnresolvedTypePath(name) => DiagnosticInfo {
                message: format!(
                    "cannot resolve type path `{name}`\n\
                     \n  = note: the type `{name}` could not be found in the current scope\n\
                     \n  = help: ensure the type is spelled correctly, is in scope, and is a type\n\
                     \n         (struct, enum, type alias) rather than a value or function.\n\
                     \n         You may need to import it with `use` at the top of the module.\n\
                     \n  = example:\n           use std::collections::HashMap;\n\
                     \n           fn foo(map: HashMap<String, i32>) {{ }}"
                ),
                origin: Origin::None,
            },

            HirErr::DuplicateEntity(name) => DiagnosticInfo {
                message: format!(
                    "duplicate definition of `{name}`\n\
                     \n  = note: an entity with the name `{name}` has already been defined in this scope.\n\
                     \n        Names must be unique within a module.\n\
                     \n  = help: rename one of the definitions to avoid the conflict.\n\
                     \n  = example:\n           // This is not allowed:\n           fn foo() {{ }}\n           fn foo() {{ }} // error: duplicate definition\n\
                     \n           // Instead, use different names:\n           fn foo() {{ }}\n           fn foo_v2() {{ }}"
                ),
                origin: Origin::None,
            },

            HirErr::UnrecognizedLifetime(name) => DiagnosticInfo {
                message: format!(
                    "unrecognized lifetime name `'{name}`\n\
                     \n  = note: lifetimes in this language are one of: `'static`, `'gc`,\n\
                     \n         `'thread`, `'task`, or `'_` (inferred). Got `'{name}`.\n\
                     \n  = help: use one of the valid lifetime names or `'_` for an inferred lifetime.\n\
                     \n  = example:\n           fn foo(x: &'static i32) {{ }}\n           fn bar(x: &'_ i32) {{ }}"
                ),
                origin: Origin::None,
            },

            // ════════════════════════════════════════════════════════════════
            // TYPE ERRORS (H040-H059)
            // ════════════════════════════════════════════════════════════════
            HirErr::IntegerCastOutOfRange { value, target_type } => DiagnosticInfo {
                message: format!(
                    "integer literal `{value}` cannot be represented in type `{target_type}`\n\
                     \n  = note: the value `{value}` is outside the valid range for `{target_type}`.\n\
                     \n  = help: use a smaller integer literal or a wider integer type.\n\
                     \n  = example:\n           let a: i8 = 127;   // valid: i8 ranges from -128 to 127\n           let b: i8 = 128;   // error: 128 does not fit in i8\n           let c: i16 = 128;  // ok: 128 fits in i16"
                ),
                origin: Origin::None,
            },

            HirErr::GlobalVariableMustHaveInitializer(name) => DiagnosticInfo {
                message: format!(
                    "global variable `{name}` must have an initializer\n\
                     \n  = note: global variables at module level must be initialized at the point of declaration.\n\
                     \n        Unlike local variables, globals cannot remain uninitialized.\n\
                     \n  = help: provide an initializer expression:\n\
                     \n  = example:\n           static MAX_SIZE: i32 = 1024;     // ok\n           static MIN_SIZE: i32;           // error: missing initializer"
                ),
                origin: Origin::None,
            },

            HirErr::LocalVariableMissingInitializer(name) => DiagnosticInfo {
                message: format!(
                    "local variable `{name}` must have an initializer\n\
                     \n  = note: variables declared with `let` or `var` must be initialized.\n\
                     \n  = help: provide an initializer expression:\n\
                     \n  = example:\n           fn foo() {{\n               let x = 42;     // ok\n               let y;             // error: missing initializer\n               var z = vec![1];   // ok\n           }}"
                ),
                origin: Origin::None,
            },

            HirErr::TypeAliasMustHaveType(name) => DiagnosticInfo {
                message: format!(
                    "type alias `{name}` must have a type on the right-hand side\n\
                     \n  = note: a type alias requires an equals sign followed by a type expression.\n\
                     \n  = help: provide the type that this alias refers to.\n\
                     \n  = example:\n           type MyInt = i32;                 // ok\n           type MyInt;                       // error\n           type MyInt<T> = Result<T, Error>; // ok"
                ),
                origin: Origin::None,
            },

            HirErr::ArrayLengthExpectedUSize => DiagnosticInfo {
                message: format!(
                    "array length must evaluate to a `usize` value\n\
                     \n  = note: the length expression in an array type `[T; N]` must evaluate\n\
                     \n        to a non-negative usize value at compile time.\n\
                     \n  = help: use a constant usize expression:\n\
                     \n  = example:\n           let arr: [i32; 5] = [0; 5];        // ok: literal usize\n           const N: usize = 10;\n           let arr: [i32; N] = [0; N];      // ok: const usize\n           let arr: [i32; \"foo\"] = [];     // error: string is not usize"
                ),
                origin: Origin::None,
            },

            HirErr::ArrayTypeLengthEvalError => DiagnosticInfo {
                message: format!(
                    "array length expression could not be evaluated at compile time\n\
                     \n  = note: the length of an array type `[T; N]` must be a constant expression\n\
                     \n        that can be fully evaluated during compilation.\n\
                     \n  = help: use a literal, a constant, or an expression that the compiler can evaluate.\n\
                     \n  = example:\n           const SIZE: usize = 100;\n           type Buffer = [u8; SIZE];  // ok\n           fn foo(n: usize) {{\n               let arr: [i32; n];       // error: n is not constant\n           }}"
                ),
                origin: Origin::None,
            },

            HirErr::SliceTypesMustBeInRefOrPtr => DiagnosticInfo {
                message: format!(
                    "slice types `[T]` cannot appear outside references or pointers\n\
                     \n  = note: bare slice types like `[i32]` have no compile-time known size\n\
                     \n        and cannot be used as variable types. They must be wrapped in\n\
                     \n        a reference (`&[T]`) or pointer (`*[T]`).\n\
                     \n  = help: use `&[T]` or `*[T]` instead of bare `[T]`.\n\
                     \n  = example:\n           fn foo(slice: &[i32]) {{ }}  // ok\n           fn bar(slice: [i32]) {{ }}  // error: bare slice"
                ),
                origin: Origin::None,
            },

            HirErr::TypeAliasEvalError(name) => DiagnosticInfo {
                message: format!(
                    "failed to evaluate the type for type alias `{name}`\n\
                     \n  = note: the type expression on the right-hand side of this type alias\n\
                     \n        could not be fully resolved during HIR lowering.\n\
                     \n  = help: check that the type expression is valid and all referenced types are in scope."
                ),
                origin: Origin::None,
            },

            // ════════════════════════════════════════════════════════════════
            // EXPRESSION/LITERAL ERRORS (H060-H079)
            // ════════════════════════════════════════════════════════════════
            HirErr::MatchNotImplemented => DiagnosticInfo {
                message: format!(
                    "`match` expressions are not yet implemented\n\
                     \n  = note: pattern matching with `match` is planned but not yet available.\n\
                     \n  = help: use `if`/`else if` chains as a workaround.\n\
                     \n  = example:\n           // Workaround for match:\n           if x == 1 {{\n               // handle case 1\n           }} else if x == 2 {{\n               // handle case 2\n           }} else {{\n               // default case\n           }}"
                ),
                origin: Origin::None,
            },

            HirErr::ForLoopNotImplemented => DiagnosticInfo {
                message: format!(
                    "`for` loops are not yet implemented\n\
                     \n  = note: `for` .. `in` loops are planned but not yet available.\n\
                     \n  = help: use `while` loops or manual iteration as a workaround.\n\
                     \n  = example:\n           // Workaround for for loop:\n           let mut i = 0;\n           while i < 10 {{\n               // ... loop body\n               i += 1;\n           }}"
                ),
                origin: Origin::None,
            },

            HirErr::AwaitNotImplemented => DiagnosticInfo {
                message: format!(
                    "`await` expressions are not yet implemented\n\
                     \n  = note: async/await is planned but not yet available.\n\
                     \n  = help: use synchronous blocking calls as a workaround."
                ),
                origin: Origin::None,
            },

            HirErr::TypeofNotImplemented => DiagnosticInfo {
                message: format!(
                    "`typeof` operator is not yet implemented\n\
                     \n  = note: the `typeof` reflection operator is planned but not yet available.\n\
                     \n  = help: specify the type explicitly instead of using typeof."
                ),
                origin: Origin::None,
            },

            HirErr::RangeOperatorNotImplemented => DiagnosticInfo {
                message: format!(
                    "the range operator `..` is not yet implemented\n\
                     \n  = note: range expressions like `0..10` or `start..end` are planned\n\
                     \n        but not yet available.\n\
                     \n  = help: use explicit bounds instead of range syntax."
                ),
                origin: Origin::None,
            },

            HirErr::TypeReflectionNotImplemented => DiagnosticInfo {
                message: format!(
                    "type reflection is not yet implemented\n\
                     \n  = note: compile-time type inspection (e.g., `typeinfo`, `typeof`) is\n\
                     \n        planned but not yet available.\n\
                     \n  = help: use explicit type annotations instead."
                ),
                origin: Origin::None,
            },

            HirErr::ClosureNotImplemented => DiagnosticInfo {
                message: format!(
                    "closure expressions are not yet implemented\n\
                     \n  = note: anonymous functions (closures) are planned but not yet available.\n\
                     \n  = help: define a named function instead.\n\
                     \n  = example:\n           fn my_callback(x: i32) -> i32 {{ x * 2 }}\n           process(my_callback);"
                ),
                origin: Origin::None,
            },

            HirErr::LatentTypeNotImplemented => DiagnosticInfo {
                message: format!(
                    "latent types are not yet implemented\n\
                     \n  = note: latent types (type computed from a block expression) are planned\n\
                     \n        but not yet available.\n\
                     \n  = help: specify the type explicitly."
                ),
                origin: Origin::None,
            },

            HirErr::LifetimeTypeNotImplemented => DiagnosticInfo {
                message: format!(
                    "lifetimes as standalone types are not yet implemented\n\
                     \n  = note: using a lifetime `'a` as a type expression is planned but not\n\
                     \n        yet available.\n\
                     \n  = help: lifetimes can only be used in reference type positions currently:\n\
                     \n         `&'a i32`, not `'a` as a standalone type."
                ),
                origin: Origin::None,
            },

            HirErr::IntermediateGenericArgsNotSupported(path) => DiagnosticInfo {
                message: format!(
                    "generic type arguments in intermediate path segments are not supported: `{path}`\n\
                     \n  = note: the compiler encountered a type path like `Foo<i32>::Bar` where\n\
                     \n        a generic argument appears on a non-final segment.\n\
                     \n  = help: this syntax is not yet supported. Try restructuring the type path\n\
                     \n         so that generic arguments only appear on the final segment:\n\
                     \n  = example:\n           // Not yet supported:\n           Foo<i32>::Bar\n           // Use instead:\n           Foo::Bar<i32>  // if applicable"
                ),
                origin: Origin::None,
            },

            // ════════════════════════════════════════════════════════════════
            // CONTROL FLOW ERRORS (H080-H099)
            // ════════════════════════════════════════════════════════════════
            HirErr::MissingReturnStatement(func_name) => DiagnosticInfo {
                message: format!(
                    "missing return statement in function `{func_name}`\n\
                     \n  = note: this function has a non-unit return type but its body does not\n\
                     \n        end with a `return` expression. Every code path must return a value.\n\
                     \n  = help: add a `return` expression at the end of the function body, or\n\
                     \n         change the return type to `()` (unit) if nothing should be returned.\n\
                     \n  = example:\n           fn add(a: i32, b: i32) -> i32 {{\n               return a + b;  // explicit return\n           }}\n\
                     \n           // The final expression is also returned implicitly:\n           fn add_implicit(a: i32, b: i32) -> i32 {{\n               a + b  // no semicolon; this is the return value\n           }}"
                ),
                origin: Origin::None,
            },

            HirErr::UnsafeExprBodyNotImplemented => DiagnosticInfo {
                message: format!(
                    "unsafe expressions with bodies are not yet implemented\n\
                     \n  = note: `unsafe {{ ... }}` blocks with an expression body that computes\n\
                     \n        a value are not yet supported.\n\
                     \n  = help: use a statement-level `unsafe` block instead.\n\
                     \n  = example:\n           unsafe {{  // this is supported\n               // ... unsafe operations\n           }}\n           unsafe expr  // not yet supported"
                ),
                origin: Origin::None,
            },

            // ════════════════════════════════════════════════════════════════
            // REFINEMENT TYPE ERRORS (H100-H119)
            // ════════════════════════════════════════════════════════════════
            HirErr::RefinementBoundNotConstant => DiagnosticInfo {
                message: format!(
                    "refinement type bound must be a constant expression\n\
                     \n  = note: the bounds in a refinement type (e.g., `i32: [0:100]`) must be\n\
                     \n        computable at compile time.\n\
                     \n  = help: use literal values, constants, or expressions that can be fully\n\
                     \n         evaluated during compilation.\n\
                     \n  = example:\n           type SmallInt = i32: [0:255];   // ok: literal bounds\n           const MAX: i32 = 100;\n           type Range = i32: [0:MAX];   // ok: const bound\n           fn foo(limit: i32) {{\n               type Dynamic = i32: [0:limit]; // error: limit is not constant\n           }}"
                ),
                origin: Origin::None,
            },

            HirErr::RefinementTypeOnNonInteger(base_type) => DiagnosticInfo {
                message: format!(
                    "refinement types can only be applied to integer types, not `{base_type}`\n\
                     \n  = note: refinement types (using `:`) restrict the range of integer types.\n\
                     \n        They can only be applied to integer types like `i8`, `u8`, `i32`, `u64`, etc.\n\
                     \n  = help: use a valid integer type as the base of this refinement type.\n\
                     \n  = example:\n           type Small = i32: [0:255];    // ok\n           type Small = f32: [0:1];     // error: float is not an integer\n           type Small = bool: [0:1];    // error: bool is not an integer"
                ),
                origin: Origin::None,
            },

            HirErr::RefinementWidthOutOfRange(width) => DiagnosticInfo {
                message: format!(
                    "refinement type width `{width}` is out of valid range (1 to 128)\n\
                     \n  = note: the width parameter in a refinement type (e.g., `u8: 6` for 6-bit values)\n\
                     \n        must be between 1 and 128 bits inclusive.\n\
                     \n  = help: use a width value between 1 and 128.\n\
                     \n  = example:\n           type HalfByte = u8: 4;        // ok: 4-bit values [0:15]\n           type TripleWord = u128: 96;  // ok: 96-bit values\n           type Invalid = u8: 0;        // error: width must be >= 1\n           type Invalid = u8: 200;       // error: width must be <= 128"
                ),
                origin: Origin::None,
            },

            HirErr::RefinementTypeEmpty => DiagnosticInfo {
                message: format!(
                    "refinement type must have at least one bound\n\
                     \n  = note: a refinement type requires either a width, a minimum, or a maximum\n\
                     \n        bound to restrict the integer range.\n\
                     \n  = help: add a width, minimum bound, maximum bound, or range to the type.\n\
                     \n  = example:\n           type Small = u8: 4;           // ok: width only\n           type Range = i32: [0:100];   // ok: min and max\n           type MinOnly = i32: [0:];    // ok: min only\n           type MaxOnly = i32: [:100];  // ok: max only\n           type Empty = i32:;           // error: no bounds"
                ),
                origin: Origin::None,
            },

            HirErr::RefinementWidthNotPositive(width) => DiagnosticInfo {
                message: format!(
                    "refinement type width must be a positive integer, got `{width}`\n\
                     \n  = note: the width parameter in a refinement type must be greater than zero.\n\
                     \n        A width of 0 bits would result in an uninhabitable type.\n\
                     \n  = help: use a positive integer value for the width.\n\
                     \n  = example:\n           type HalfByte = u8: 4;  // ok\n           // The following would be invalid:\n           type Empty = u8: 0;    // width must be > 0"
                ),
                origin: Origin::None,
            },

            // ════════════════════════════════════════════════════════════════
            // ITEM/DEFINITION ERRORS (H140-H159)
            // ════════════════════════════════════════════════════════════════
            HirErr::MissingFunctionBody(name) => DiagnosticInfo {
                message: format!(
                    "function `{name}` has a non-unit return type but no body\n\
                     \n  = note: functions that are declared without a body (extern declarations)\n\
                     \n        must have the `extern` attribute or return unit `()`.\n\
                     \n  = help: add a function body, change the return type to `()`, or mark\n\
                     \n         it as `extern`.\n\
                     \n  = example:\n           fn foo() -> i32 {{ 42 }}         // ok: has body\n           fn bar() -> i32;               // error: no body and non-unit return\n           extern fn baz() -> i32;         // ok: extern with no body\n           fn qux() -> ();                 // ok: unit return"
                ),
                origin: Origin::None,
            },
        }
    }
}
