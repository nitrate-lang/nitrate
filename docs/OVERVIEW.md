# Nitrate Compiler: Architectural Overview

## Abstract

This document provides the definitive architectural overview of the Nitrate compiler — a modern systems programming language compiler written entirely in Rust. Nitrate transforms source code (`.nit` files) into optimized native machine code through a sophisticated multi-stage pipeline. Each stage operates on a distinct intermediate representation with rigorously defined boundaries, enabling independent testing, clear separation of concerns, and the ability to modify individual stages without affecting the rest of the compiler.

The complete pipeline encompasses ten stages: lexical analysis (source text to token stream), syntactic parsing (tokens to Abstract Syntax Tree called the Parse Tree), name resolution (resolving all named references to their declarations), HIR lowering (AST to High-Level Intermediate Representation), type inference via Hindley-Milner constraint-based unification with monomorphization of generic instantiations, HIR validation with semantic correctness checks, name mangling for deterministic linker symbol generation, LLVM IR code generation (HIR to LLVM IR), and finally LLVM optimization and native code emission through its mature, battle-tested backend infrastructure.

This document describes each pipeline stage in detail, the data flow between stages, the four core architectural principles that shape the entire codebase, how the pieces of the compilation pipeline fit together to form a coherent system, and the rationale behind every major architectural decision. Understanding this document is the essential first step for any developer working on the Nitrate compiler.

## Table of Contents

- [Nitrate Compiler: Architectural Overview](#nitrate-compiler-architectural-overview)
  - [Abstract](#abstract)
  - [Table of Contents](#table-of-contents)
  - [Core Architectural Principles](#core-architectural-principles)
    - [Principle 1: Immutable Interning with Thread-Local Storage](#principle-1-immutable-interning-with-thread-local-storage)
    - [Principle 2: Diagnostic Accumulation](#principle-2-diagnostic-accumulation)
    - [Principle 3: Pass-Based Pipeline with Fixed-Point Iteration](#principle-3-pass-based-pipeline-with-fixed-point-iteration)
    - [Principle 4: Reentrant Store Access](#principle-4-reentrant-store-access)
  - [Detailed Compilation Pipeline](#detailed-compilation-pipeline)
    - [Stage 0: Source Loading and Package Management](#stage-0-source-loading-and-package-management)
    - [Stage 1: Lexical Analysis](#stage-1-lexical-analysis)
    - [Stage 2: Syntactic Parsing](#stage-2-syntactic-parsing)
    - [Stage 3: Name Resolution](#stage-3-name-resolution)
    - [Stage 4: HIR Lowering](#stage-4-hir-lowering)
    - [Stage 5: Type Inference and Solving](#stage-5-type-inference-and-solving)
    - [Stage 6: Type Determination](#stage-6-type-determination)
    - [Stage 7: HIR Validation](#stage-7-hir-validation)
    - [Stage 8: Name Mangling](#stage-8-name-mangling)
    - [Stages 9-10: LLVM Code Generation and Optimization](#stages-9-10-llvm-code-generation-and-optimization)
  - [Design Rationale](#design-rationale)

## Core Architectural Principles

The Nitrate compiler is built on four fundamental architectural principles that permeate every subsystem and guide all implementation decisions. These principles manifest directly in the codebase structure, the API design of every crate, and the patterns used throughout the compilation pipeline. Understanding these principles is essential for working effectively with the codebase.

### Principle 1: Immutable Interning with Thread-Local Storage

The compiler employs a sophisticated storage architecture centered around the `Store` type in the `nitrate_hir` crate. This architecture recognizes that different categories of data within a compiler have fundamentally different access patterns, mutation requirements, and performance characteristics. The architecture divides data into two categories: immutable, deduplicated data (types and literals) and mutable, append-only data (functions, structs, values, blocks, and all other compilation artifacts).

**Immutable data** — specifically types stored in `TypeStore` and literals stored in `ExprLiteralStore` — uses a BiMap-backed deduplication strategy. Each time a new `Type` is created, the store checks if an identical type already exists using a `RwLock<BiMap<Arc<Type>, TypeId>>`. If found, the existing `TypeId` handle is returned, avoiding duplication entirely. If not found, the type is added to an `AppendOnlyVec` and registered in the BiMap for future lookups. This design ensures that structural type comparison reduces to O(1) handle equality comparison — two `TypeId` values are equal if and only if they point to the same deduplicated `Type` in the store. This O(1) guarantee is critical because type comparison occurs millions of times during type inference and validation; any O(n) structural comparison at this scale would become a severe performance bottleneck that would slow compilation significantly.

**Mutable data** — functions in `FunctionStore`, struct definitions in `StructDefStore`, values in `ExprValueStore`, blocks in `ExprBlockStore`, and all other mutable items — uses a simpler pattern: each new item is appended to an `AppendOnlyVec<RefCell<T>>`. The `RefCell` provides interior mutability, enabling passes to modify stored items without requiring exclusive `&mut Store` access. The `AppendOnlyVec` ensures that existing handles remain valid indefinitely — new items are always appended to the end, never invalidating existing references. This is essential because the solver, for example, needs to create new monomorphized functions while still holding references to existing functions. The `RefCell` allows it to borrow existing items immutably while storing new items through a shared reference to the store.

Access to the store is mediated through a Thread-Local Storage (TLS) pattern defined in `store.rs`. The `using_storage()` function saves a raw pointer to the store in TLS, executes the compilation pass, and restores the previous TLS value on completion. This RAII pattern supports nested store usage — if a compilation pass internally needs to invoke another pass that uses `using_storage()`, the outer store is saved and restored correctly. Any handle's `Deref` implementation calls `get_storage()` to retrieve the store pointer and then indexes into the appropriate sub-store using the handle's inner `NonZeroU32` index.

This architecture provides several interrelated benefits:

- **Type deduplication**: Identical types share storage, enabling O(1) comparison via handle equality rather than O(n) structural recursion. Since type comparison is the most frequent operation during inference, this is the single most important optimization in the compiler.
- **No global locks**: TLS avoids synchronization overhead entirely in the single-threaded pipeline; the `RwLock` in `TypeStore` exists only as a safeguard for potential future parallelism.
- **Append-only growth**: Adding items never invalidates existing references, so handles remain valid across the entire compilation session. This is critical because handles are stored in HashMaps throughout the compiler.
- **Panic-safety**: The `Cell` is restored on panic via the RAII pattern in `using_storage()`, preventing TLS corruption even if a compilation pass panics unexpectedly.
- **Lifetime-free handles**: Because the store pointer is always accessible via TLS, handles do not need lifetime annotations, making them easy to store in collections, pass between functions, and use from any context without borrow-checker complexity.

### Principle 2: Diagnostic Accumulation

Rather than aborting on the first error — which would force users into frustrating edit-compile-debug cycles where they fix one error only to discover another — the compiler employs a comprehensive diagnostic accumulation strategy. Each compilation stage defines its own diagnostic types that implement the `FormattableDiagnosticGroup` trait. Diagnostics are collected in a `CompilerLog` instance that is passed through the entire pipeline. This design ensures that users see as many errors as possible in a single compilation, enabling batch fixes and faster iteration.

Diagnostics are organized into seven groups based on the compilation stage that produced them: Scanner (Group 0), Lexical (Group 1), Syntax/Parse (Group 2), Resolution (Group 3), HIR (Group 4), Type (Group 5), and Semantic (Group 6). Each diagnostic carries a unique 16-bit ID encoding both the group (4 bits) and variant (12 bits). The ID format `[XNNNN]` enables precise error code referencing in documentation, error messages, and the `no3 explain` command.

### Principle 3: Pass-Based Pipeline with Fixed-Point Iteration

The compiler is organized as a sequence of passes over progressively lower-level IRs. Each pass has a well-defined input type and output type, enabling independent testing and clear separation of concerns. The `Pass` trait in `nitrate_hir` formalizes this contract. Several critical passes — most notably the Hindley-Milner constraint solver — use fixed-point iteration, repeatedly visiting every expression in a function body until no new type constraints are added. This pattern is essential for transitive constraint propagation (if `x = y` and `y = 42`, then `x = i32`), nested monomorphization (when a generic function calls another generic function, the first pass monomorphizes the inner call and the second pass discovers the outer call), and incremental type resolution (type variables are resolved as constraints accumulate).

### Principle 4: Reentrant Store Access

Because all store handles dereference through TLS, any function that has access to a handle can read from the store regardless of where it was called from. This enables handles to be created, stored in collections, passed across module boundaries, and used from any context without lifetime annotations. This pattern enables symbol table entries that hold handles into the global store, codegen contexts that reference types and functions without lifetime parameters, and a clean separation between storage (in the Store) and data access (through handles).

## Detailed Compilation Pipeline

### Stage 0: Source Loading and Package Management

Compilation begins with the `no3` binary parsing command-line arguments and loading the package manifest. The driver subsystem in `nitrate_driver` handles discovering the `no3.xml` package manifest, parsing metadata (name, version, dependencies), resolving the dependency graph (downloading missing packages, checking version compatibility), finding all source files in the `src/` directory, assigning `FileId` identifiers, and creating the `CompilerLog` instance. This stage is the only one that performs I/O operations directly; all subsequent stages operate purely on in-memory data structures.

### Stage 1: Lexical Analysis

The lexer (`nitrate_token_lexer`) reads raw source bytes (`&[u8]`) and produces a stream of `AnnotatedToken` values. Each annotated token carries the token variant plus precise source position information — start line, start column, start offset, end line, end column, end offset, and `FileId`. The lexer operates byte-by-byte using a dispatch-on-first-byte strategy: alphabetic characters trigger identifier/keyword parsing, digits trigger numeric literal parsing, double quotes trigger string literal parsing with full escape sequence processing, and special characters trigger operator or comment parsing.

The lexer supports two modes: with trivia enabled (whitespace, newlines, and comments emitted as explicit tokens for pretty-printing and LSP features) and with trivia disabled (the default, where trivia is silently skipped). A source size limit of 4 GiB (`u32::MAX` bytes) ensures the `offset` field in `SourcePosition` never overflows.

### Stage 2: Syntactic Parsing

The parser (`nitrate_tree_parse`) consumes the token stream and produces a Parse Tree (AST) using a hand-written recursive-descent strategy with one token of lookahead. Operator precedence is managed using a precedence climbing algorithm with 10 levels from logical OR (lowest) through exponentiation (highest). The parser is organized into distinct functions for item parsing, expression parsing, and type parsing.

Item parsing handles all top-level declarations: functions, structs, enums, classes, unions, contracts, traits, impl blocks, type aliases, modules, use declarations, extern blocks, and global variables. Expression parsing handles control flow constructs (`if`/`while`/`loop`/`for`/`match`/`break`/`continue`/`return`), binary operations, unary operations, calls, field access, index access, literals, blocks, and special forms. Type parsing handles primitives, arrays, tuples, references, pointers, slices, generic parameterized types, function types, trait objects, and inferred types.

The parser performs no semantic analysis — it is purely syntactic, validating only that the token sequence conforms to the grammar. All semantic interpretation is deferred to later stages.

### Stage 3: Name Resolution

The resolver (`nitrate_tree_resolve`) processes `use` declarations and resolves all name paths in the parse tree to their fully qualified equivalents. It builds a symbol map through depth-first traversal of the AST, discovering all declared symbols and their scopes. The resolver handles absolute paths, relative paths, self references, super references, and wildcard imports. It detects and reports unknown names, ambiguous references, cyclic imports, and visibility violations.

### Stage 4: HIR Lowering

The lowerer (`nitrate_hir_from_tree`) transforms the resolved parse tree into the HIR — the most complex transformation in the pipeline. Item lowering creates `FunctionId`, `StructDefId`, `EnumDefId`, `TraitId`, and `ModuleId` handles stored in the TLS Store. Expression lowering recursively transforms every AST expression into `Value` nodes stored in `ExprValueStore`. Type lowering converts type annotations to `Type` nodes interned in `TypeStore` through the deduplication mechanism, with generic parameters becoming `Type::GenericParam` and omitted annotations creating `Type::Inferred` variables.

### Stage 5: Type Inference and Solving

The solver (`nitrate_hir_solve`) performs Hindley-Milner constraint-based type inference with monomorphization. It maintains a constraint map and walks the expression tree repeatedly in a fixed-point loop. When a generic function call is detected, the solver infers concrete type arguments, creates a monomorphized copy via type substitution, registers it in the symbol table, and redirects the call site. The solver also handles method call resolution, generic struct instantiation, and refinement type bounds checking.

### Stage 6: Type Determination

The `HirGetType` trait (`nitrate_hir_get_type`) determines the type of any `Value` expression at any point during compilation, handling all 30+ `Value` variants from simple literals to complex control flow, function calls, borrows, and casts.

### Stage 7: HIR Validation

The validator (`nitrate_hir_validate`) performs final semantic checks verifying that all `Inferred` variables are resolved, all `GenericParam` instances are substituted, function signatures are well-formed, control flow is valid, and field/index accesses target valid types. Successful validation produces a `ValidHir<T>` wrapper providing a type-level guarantee of semantic correctness.

### Stage 8: Name Mangling

The mangler (`nitrate_hir_mangle`) produces unique, deterministic LLVM linkage names encoding the package name, symbol name, and type signature. Generic instantiations append a monomorphization counter suffix. Functions with the `NoMangle` attribute preserve their original name.

### Stages 9-10: LLVM Code Generation and Optimization

The code generator (`nitrate_llvm_from_hir`) translates validated HIR into LLVM IR in three passes: global variable generation (with constructor functions for complex initializers), function declarations (ensuring symbols are available for mutual recursion), and function definitions. The LLVM crate then runs optimization passes based on the configured level (0-3) and emits the result as an object file, assembly, or LLVM IR text.

## Design Rationale

**Why HIR instead of direct AST-to-LLVM?** The HIR carries resolved type information expensive to recompute, serves as the monomorphization target (simpler than LLVM IR level), abstracts backend-specific details for alternative backends, and provides a validation boundary.

**Why monomorphization instead of generics via dynamic dispatch?** Monomorphization produces zero runtime overhead, full optimization visibility, and integrates naturally with LLVM's type system. Code size tradeoffs are mitigated by cache deduplication and link-time optimization.

**Why TLS store instead of arena allocation?** The TLS pattern provides O(1) handle comparison equaling structural equality, eliminates lifetime parameters on handles, enables storage in any collection without borrow-checker complexity, and provides deterministic destruction per compilation session through RAII.
