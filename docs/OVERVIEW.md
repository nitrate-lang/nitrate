# Nitrate Compiler: Architectural Overview

## Introduction and Scope

Nitrate is a modern systems programming language compiler written in Rust. The compiler transforms Nitrate source code (`.nit` files) into optimized machine code through a sophisticated multi-stage pipeline that processes the source through progressively lower-level representations. The pipeline encompasses lexical analysis (source text to tokens), syntactic parsing (tokens to Abstract Syntax Tree), name resolution (resolving all named references), High-Level IR lowering (AST to HIR), type inference via Hindley-Milner constraint-based unification, monomorphization of generic instantiations, HIR validation with semantic checks, and finally LLVM IR code generation. The LLVM backend performs target-independent optimization and emits native machine code through its mature code generation infrastructure.

This document provides a comprehensive architectural overview of the entire compiler, describing each stage in detail, the data flow between stages, the key design decisions that shape the architecture, how the pieces of the compilation pipeline fit together, and the rationale behind the major architectural choices.

## Core Architectural Principles

The Nitrate compiler is built on four fundamental architectural principles that permeate every subsystem and guide all implementation decisions.

### Principle 1: Immutable Interning with Thread-Local Storage

The compiler employs a sophisticated storage architecture centered around the `Store` type in the `nitrate_hir` crate. This architecture distinguishes between two categories of data: immutable, deduplicated data (types and literals) and mutable, append-only data (functions, structs, values, blocks, and all other compilation artifacts).

**Immutable data** (types stored in `TypeStore`, literals stored in `ExprLiteralStore`) uses a BiMap-backed deduplication strategy. Each time a new `Type` is created, the store checks if an identical type already exists using a `RwLock<BiMap<Arc<Type>, TypeId>>`. If found, the existing `TypeId` handle is returned. If not found, the type is added to an `AppendOnlyVec` and registered in the BiMap. This ensures that structural type comparison reduces to O(1) handle equality comparison, which is critical for the millions of type comparisons that occur during type inference and validation.

**Mutable data** (functions in `FunctionStore`, struct definitions in `StructDefStore`, values in `ExprValueStore`, etc.) uses a simpler pattern: each new item is appended to an `AppendOnlyVec<RefCell<T>>`. The `RefCell` provides interior mutability, enabling passes to modify stored items without requiring exclusive `&mut Store` access. The `AppendOnlyVec` ensures that existing handles remain valid indefinitely — new items are always appended to the end, never invalidating existing references.

Access to the store is mediated through a Thread-Local Storage (TLS) pattern defined in `store.rs`:

```rust
thread_local! {
    static TLS_STORE: Cell<Option<*const Store>> = const { Cell::new(None) };
}

pub fn using_storage<R>(store: &Store, f: impl FnOnce() -> R) -> R {
    TLS_STORE.with(|tls| {
        let old = tls.take();
        tls.set(Some(store));
        let result = f();
        tls.set(old);  // RAII restore
        result
    })
}

pub fn get_storage<R>(f: impl FnOnce(&Store) -> R) -> R {
    TLS_STORE.with(|tls| {
        let store_ptr = tls.get()
            .expect("No Store found in TLS. Did you forget to call using_storage?");
        let store = unsafe { &*store_ptr };
        f(store)
    })
}
```

The `using_storage()` function saves a raw pointer to the store in TLS, executes the compilation pass, and restores the previous TLS value on completion (supporting nested store usage). Any handle's `Deref` implementation calls `get_storage()` to retrieve the store pointer and then indexes into the appropriate sub-store using the handle's inner `NonZeroU32` index. This architecture provides several benefits:

- **Type deduplication**: Identical types share storage, enabling O(1) comparison
- **No global locks**: TLS avoids synchronization overhead in the single-threaded pipeline
- **Append-only growth**: Adding items never invalidates existing references
- **Panic-safety**: The `Cell` is restored on panic via the RAII pattern in `using_storage()`

### Principle 2: Diagnostic Accumulation

Rather than aborting on the first error — which would force users into frustrating edit-compile-debug cycles where they fix one error only to discover another — the compiler employs a comprehensive diagnostic accumulation strategy. Each compilation stage (lexer, parser, resolver, solver, validator) defines its own diagnostic types that implement the `FormattableDiagnosticGroup` trait. Diagnostics are collected in a `CompilerLog` instance that is passed through the entire pipeline.

Diagnostics are organized into seven groups based on the compilation stage that produced them:

- **Scanner** (Group 0): File loading errors, source detection failures
- **Lexical** (Group 1): Invalid tokens, malformed literals, encoding errors
- **Syntax/Parse** (Group 2): Grammar violations, malformed constructs, missing tokens
- **Resolution** (Group 3): Unknown names, ambiguous references, cyclic imports
- **HIR** (Group 4): Lowering errors, invalid type expressions
- **Type** (Group 5): Type mismatch, unsatisfiable constraints, refinement range violations
- **Semantic** (Group 6): Semantic analysis violations, borrow checker errors

Each diagnostic carries a unique 16-bit ID encoding both the group (4 bits) and variant (12 bits). The ID format `[XNNNN]` (e.g., `[L0300]`, `[T0002]`) enables precise error code referencing in documentation and error messages.

### Principle 3: Pass-Based Pipeline with Fixed-Point Iteration

The compiler is organized as a sequence of passes over progressively lower-level IRs. Each pass has a well-defined input type and output type, enabling independent testing and clear separation of concerns. The `Pass` trait in `nitrate_hir` formalizes this:

```rust
pub trait Pass<T> {
    fn run(&mut self, input: T) -> T;
}

pub struct PassManager<T> {
    passes: Vec<Box<dyn Pass<T>>>,
}
```

Several critical passes, most notably the Hindley-Milner constraint solver in `nitrate_hir_solve`, use fixed-point iteration. The solver repeatedly visits every expression in a function body until no new type constraints are added:

```rust
loop {
    let prev_len = self.constraints.len();
    for element in body.iter_mut() {
        self.visit_block_element(element);
    }
    if self.constraints.len() == prev_len {
        break;
    }
}
```

This pattern is essential for:

- **Transitive constraint propagation**: If `x = y` and `y = 42`, then `x` must be `i32`. The first pass records `y: i32`, the second pass propagates this to `x`.
- **Nested monomorphization**: When a generic function calls another generic function, the first pass monomorphizes the inner call, and the second pass discovers the outer call now has concrete types.
- **Incremental type resolution**: `Inferred` type variables are resolved one at a time as constraints accumulate.

### Principle 4: Reentrant Store Access

Because all store handles dereference through TLS, any function that has access to a handle can read from the store regardless of where it was called from. This enables a powerful pattern where handles can be created, stored, and passed around without lifetime annotations:

```rust
// Create a function and immediately store it, getting back a handle
let func_id: FunctionId = Function { ... }.into();
// The handle can be cloned, stored in HashMaps, passed across functions
// All accesses go through TLS to the global store

// Types are deduplicated on creation
let ty_id: TypeId = Type::I32.into();
let same_ty_id: TypeId = Type::I32.into();
assert_eq!(ty_id, same_ty_id);  // Handle equality = structural equality
```

This pattern enables symbol table entries that hold handles into the global store, codegen contexts that reference types and functions without lifetime parameters, and a clean separation between storage (in the Store) and data access (through handles).

## Detailed Compilation Pipeline

### Stage 0: Source Loading and Package Management

The compilation begins with the `no3` binary parsing the command-line arguments and loading the package manifest. The driver subsystem in `nitrate_driver` handles:

- Discovering the `no3.xml` package manifest in the current or specified directory
- Parsing manifest metadata (name, version, dependencies)
- Resolving the dependency graph (downloading missing packages, checking version compatibility)
- Finding all source files in the `src/` directory and its subdirectories
- Assigning `FileId` identifiers to each source file
- Creating the `CompilerLog` instance that will collect diagnostics throughout compilation

### Stage 1: Lexical Analysis

The lexer (`nitrate_token_lexer`) reads raw source bytes (`&[u8]`) and produces a stream of `AnnotatedToken` values. Each annotated token carries the token variant plus precise source position information — start line, start column, start offset, end line, end column, end offset, and `FileId`. This precise position tracking enables accurate error messages and LSP diagnostics.

The lexer operates byte-by-byte through the source, using a dispatch-on-first-byte strategy:

- Alphabetic characters, underscores, and non-ASCII bytes trigger identifier/keyword parsing
- Digits trigger numeric literal parsing (integers and floats)
- Double quotes trigger string literal parsing with full escape sequence processing
- `#` and `//` trigger line comment parsing; `/*` triggers block comment parsing
- Single-character operators map directly to their token variants
- Backticks trigger atypical identifier parsing (e.g., \``keyword`\`)

The lexer supports two modes controlled by `enable_trivia()`/`disable_trivia()`: with trivia enabled, whitespace, newlines, and comments are emitted as explicit tokens; with trivia disabled (the default for parsing), they are silently skipped.

Source size is limited to 4 GiB (`u32::MAX` bytes) to ensure that the `offset` field in `SourcePosition` never overflows.

### Stage 2: Syntactic Parsing

The parser (`nitrate_tree_parse`) consumes the token stream and produces a Parse Tree (AST) defined in `nitrate_tree`. The parser is a hand-written recursive-descent parser with one token of lookahead, organized into distinct parsing functions for each grammatical construct:

- Item parsing: functions, structs, enums, classes, unions, contracts, traits, impl blocks, type aliases, modules, use declarations, extern blocks, global variables
- Expression parsing: all control flow constructs (if/while/loop/for/match/break/continue/return), binary operations with 10 levels of operator precedence, unary operations, calls, method calls, field access, index access, literals, blocks, and special forms
- Type parsing: primitives, arrays, tuples, references, pointers, slices, generic parameterized types, function types, trait objects, and inferred types

Operator precedence is handled using a precedence climbing algorithm (a streamlined variant of Pratt parsing) that associates each operator with a numeric precedence level and handles left/right associativity.

The parser does not perform any semantic analysis — it is purely syntactic, validating only that the token sequence conforms to the grammar. All semantic interpretation is deferred to later stages.

### Stage 3: Name Resolution

The resolver (`nitrate_tree_resolve`) processes `use` declarations and resolves all name paths in the parse tree to their fully qualified equivalents. It builds a symbol map through depth-first traversal of the AST:

```rust
pub fn discover_symbols(module: &mut Module) -> HashMap<NString, SymbolKind> {
    let mut symbol_map = HashMap::new();
    let mut scope_vec = Vec::new();

    module.depth_first_iter(&mut |order, node| {
        if order == Order::Enter {
            match node {
                RefNode::ItemFunction(sym) => {
                    let name = qualify_name(&scope_vec, &sym.name);
                    symbol_map.insert(name, SymbolKind::Function);
                    enumerate_generics(&mut scope_vec, sym.name.clone(), &sym.generics, &mut symbol_map);
                }
                RefNode::ItemStruct(sym) => {
                    let name = qualify_name(&scope_vec, &sym.name);
                    symbol_map.insert(name, SymbolKind::Struct);
                    // ... register generic parameters
                }
                // ... all other item types
            }
        }
        // Track scope enter/leave for qualification
    });
    symbol_map
}
```

The resolver handles absolute paths, relative paths, self references, super references, and wildcard imports. It detects errors like unknown names, ambiguous references, cyclic imports, and visibility violations.

### Stage 4: HIR Lowering

The lowerer (`nitrate_hir_from_tree`) transforms the resolved parse tree into the High-Level Intermediate Representation (HIR). This is the most complex transformation in the pipeline, converting source-level syntax into the typed, normalized representation that all subsequent analysis passes operate on.

The lowering process in detail:

**Item Lowering**: Each AST item declaration becomes a corresponding HIR item stored in the TLS Store. Functions produce `FunctionId` handles referencing `Function` structs with their name, parameters, return type, generic parameters, and body. Struct declarations produce `StructDefId` handles with field definitions and initial layout. Enums produce `EnumDefId` handles with variant definitions. Traits produce `TraitId` handles with method signatures and associated type/constant declarations.

**Expression Lowering**: Every AST expression is transformed into a `Value` node stored in `ExprValueStore`. The lowering is recursive: sub-expressions are lowered first, then combined into parent expressions. For example, `a + b * c` becomes `Value::Binary { left: a_id, op: Add, right: Value::Binary { left: b_id, op: Mul, right: c_id } }`.

**Type Lowering**: Type annotations are converted to `Type` nodes and interned in `TypeStore`. Generic parameters become `Type::GenericParam`, explicit type annotations become concrete types, and omitted type annotations (in `let` bindings) create `Type::Inferred` variables.

**Symbol Resolution**: Name references are resolved through the symbol table. Function calls to named functions become `Value::FunctionSymbol { id: func_id }`. Variable references become `Value::LocalVariableSymbol { id: local_id }` or `Value::ParameterSymbol { id: param_id }`.

### Stage 5: Type Inference and Solving

The solver (`nitrate_hir_solve`) is the intellectual core of the compiler. It performs Hindley-Milner constraint-based type inference with monomorphization of generics. The solver maintains a `HashMap<ValueId, HashSet<TypeConstraint>>` and walks the expression tree, adding equality constraints as it encounters each node.

When a call to a generic function is detected, the solver:

1. Infers concrete type arguments from the actual argument types using `infer_generic_args_from_call()`
2. Creates a monomorphized copy of the function via `monomorphize_function()`, which clones the function definition and applies type substitution
3. Registers the monomorphized copy in the symbol table
4. Redirects the call site to the new copy

The solver also handles method call resolution, generic struct instantiation, refinement type bounds checking, and all type error reporting.

### Stage 6: Type Determination

The `HirGetType` trait (`nitrate_hir_get_type`) provides the ability to determine the type of any `Value` expression at any point during compilation. This is used by the solver for constraint propagation and by codegen for type-appropriate code generation. The implementation handles all 30+ `Value` variants, from simple literals to complex control flow, calls, and borrows.

### Stage 7: HIR Validation

The validator (`nitrate_hir_validate`) performs final semantic checks on the solved HIR. It verifies that:

- All `Type::Inferred` variables have been resolved to concrete types
- All `Type::GenericParam` instances have been substituted (no remaining generics in monomorphized code)
- Function signatures are well-formed
- Control flow is valid (break/continue only in loops, return only in functions)
- Field accesses reference valid fields
- Index accesses target array or slice types

Successful validation produces a `ValidHir<T>` wrapper that type-level guarantees the HIR is ready for code generation.

### Stage 8: Name Mangling

The mangler (`nitrate_hir_mangle`) produces unique, deterministic LLVM linkage names for all symbols. The mangling encodes the package name, symbol name, and type signature to ensure uniqueness across generic instantiations and function overloading. Functions with the `NoMangle` attribute preserve their original name.

### Stage 9: LLVM IR Code Generation

The code generator (`nitrate_llvm_from_hir`) translates validated HIR into LLVM IR using the `inkwell` crate. It proceeds in multiple passes:

1. **Global variable generation**: Each HIR global variable creates an LLVM global with a constructor function registered via `llvm_appendToGlobalCtors`
2. **Function declaration pass**: All non-generic functions are declared first, establishing their signatures in the LLVM module
3. **Function definition pass**: Function bodies are compiled by generating LLVM instructions for each HIR `Value` node

The codegen handles type translation (HIR types to LLVM types), control flow (branches, phi nodes, loops), function calls with various calling conventions, and struct/enum construction with proper memory layout.

### Stage 10: LLVM Optimization and Code Emission

The LLVM crate (`nitrate_llvm`) wraps the verified LLVM module through an optimization pipeline based on the configured optimization level (0-3). The optimized module is then emitted as an object file, assembly file, or LLVM IR text, depending on the compilation flags.

## Extension Systems

### VS Code Extensions

The compiler ships with three VS Code extensions: `nitrate-syntax` (TextMate grammar for syntax highlighting), `nitrate-lsp` (LSP client communicating with the compiler's internal LSP server), and `nitrate` (a meta-package bundling both).

### LSP Server

The LSP server in `nitrate_driver` implements the Language Server Protocol over JSON-RPC on stdin/stdout. It provides real-time diagnostics, code completion, and document synchronization for VS Code and other LSP-compatible editors.

## Design Rationale

### Why HIR Instead of Direct AST-to-LLVM?

The HIR provides type information that would be expensive to recompute during codegen, serves as the monomorphization target (simpler than doing so at LLVM IR level), abstracts backend-specific details for potential alternative backends, and provides a validation boundary.

### Why Monomorphization Instead of Generics via Dynamic Dispatch?

Monomorphization produces zero runtime overhead, full optimization visibility, and no type erasure. The code size tradeoff is mitigated by monomorphization cache deduplication and link-time optimization.

### Why TLS Store Instead of Arena Allocation?

The TLS store pattern enables handle equality = pointer equality, eliminates lifetime parameters on handles, and provides deterministic destruction per compilation session.
