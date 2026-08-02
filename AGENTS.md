# Nitrate Compiler: Complete Documentation Index

## About This Documentation Set

This index provides a structured portal into the complete Nitrate compiler documentation — a comprehensive reference for a modern systems programming language compiler written entirely in Rust. The Nitrate compiler transforms `.nit` source files into optimized native machine code through a sophisticated multi-stage pipeline, and these documents collectively form a complete reference for every subsystem within the compiler.

The documentation follows a coherent narrative that mirrors the natural flow of compilation: from raw source text through lexical analysis, syntactic parsing, name resolution, HIR construction, type inference, validation, code generation, and optimization. Supporting documents cover the diagnostic system, package management, LSP integration, build system configuration, string interning infrastructure, and reference semantics.

## Complete Document Index

| Document                                             | Description                                                                                               | Primary Audience                   |
| ---------------------------------------------------- | --------------------------------------------------------------------------------------------------------- | ---------------------------------- |
| [OVERVIEW.md](OVERVIEW.md)                           | High-level compiler architecture, pipeline stages, core design principles, data flow between subsystems   | All developers new to the codebase |
| [LEXER.md](LEXER.md)                                 | Lexical analysis: character-by-character tokenization, literal parsing, trivia management, error recovery | Lexer subsystem developers         |
| [PARSER.md](PARSER.md)                               | Syntactic parsing: recursive-descent AST construction, operator precedence, type and expression parsing   | Parser subsystem developers        |
| [RESOLVER.md](RESOLVER.md)                           | Name resolution: import processing, scope analysis, symbol table construction, path resolution            | Resolver subsystem developers      |
| [HIR.md](HIR.md)                                     | High-Level Intermediate Representation: type system, value representation, TLS storage architecture       | All HIR developers                 |
| [TYPE_SYSTEM.md](TYPE_SYSTEM.md)                     | Type system: type hierarchy, primitive/compound/nominal/refinement types, memory layout computations      | Type system and solver developers  |
| [HINDLEY_MILNER.md](HINDLEY_MILNER.md)               | Type inference engine: Hindley-Milner constraint solving, fixed-point iteration, monomorphization         | Inference and solver developers    |
| [GENERICS_ARCHITECTURE.md](GENERICS_ARCHITECTURE.md) | Generics: monomorphization mechanics, type substitution, template instantiation, caching                  | Generics and solver developers     |
| [VALIDATION.md](VALIDATION.md)                       | HIR validation: semantic checks, ValidHir wrapper pattern, expression and type validation                 | Validation subsystem developers    |
| [EVALUATION.md](EVALUATION.md)                       | Constant evaluation: compile-time expression computation, global initializers, defaults                   | Evaluation developers              |
| [SOLVER.md](SOLVER.md)                               | Constraint solver: engine architecture, type constraints, refinement bounds, monomorphization             | Type system and solver developers  |
| [MANGLE.md](MANGLE.md)                               | Name mangling: deterministic symbol names, type encoding, generic instantiation naming                    | Codegen developers                 |
| [LLVM_CODEGEN.md](LLVM_CODEGEN.md)                   | LLVM IR codegen: HIR-to-LLVM and MIR-to-LLVM translation, type mapping, control flow, calling conventions | Codegen backend developers         |
| [OPTIMIZATION.md](OPTIMIZATION.md)                   | Optimization: LLVM pass pipeline, optimization levels, Nitrate-specific pass infrastructure               | Optimization developers            |
| [MIR.md](MIR.md)                                     | MIR (Mid-level IR): basic block CFG, SSA locals, Places/Operands/Rvalues, TLS storage                     | MIR and codegen developers         |
| [TRANSLATION.md](TRANSLATION.md)                     | Pipeline orchestration: stage sequencing, configuration, compilation modes, error handling                | Integration developers             |
| [DIAGNOSTICS.md](DIAGNOSTICS.md)                     | Diagnostics: structured error types, accumulation, groups, error codes, display formatting                | All developers                     |
| [DRIVER.md](DRIVER.md)                               | CLI driver: `no3` binary, subcommands, package loading, compiler invocation                               | Tooling developers                 |
| [PACKAGE_MANAGER.md](PACKAGE_MANAGER.md)             | Package management: dependencies, manifest format, publishing, registry                                   | Package developers                 |
| [LSP.md](LSP.md)                                     | LSP server: JSON-RPC, document sync, diagnostics push, code completion                                    | IDE/tooling developers             |
| [BUILD_SYSTEM.md](BUILD_SYSTEM.md)                   | Build system: Cargo workspace, LLVM linkage, testing, CI dependencies                                     | Build/CI developers                |
| [NSTRING.md](NSTRING.md)                             | Interned strings: `NString` design, memory optimization, compiler-wide usage                              | Core infrastructure developers     |
| [REFERENCE_SEMANTICS.md](REFERENCE_SEMANTICS.md)     | Reference semantics: borrowing, lifetimes, pointer types, memory safety                                   | Language/type system developers    |

## Recommended Reading Path

For developers new to the Nitrate compiler codebase, we recommend the following graduated reading sequence:

1. **Start with the big picture**: [OVERVIEW.md](OVERVIEW.md) provides the essential high-level understanding of the compiler pipeline, core architectural principles (immutable interning with TLS, diagnostic accumulation, pass-based architecture, and reentrant store access), and how data flows between stages.

2. **Understand stage connectivity**: [TRANSLATION.md](TRANSLATION.md) describes how the pipeline orchestration works, how stages are sequenced and configured, and how errors propagate between them.

3. **Master the central IR**: [HIR.md](HIR.md) covers the High-Level Intermediate Representation — the central data structure that all analysis passes operate on. Understanding the TLS-based storage architecture, type deduplication, and handle-based access patterns is essential for any compiler development work.

4. **Study the type system**: [TYPE_SYSTEM.md](TYPE_SYSTEM.md) provides a complete reference for all type variants, their memory layouts, and classification rules. This knowledge is prerequisite for solver, codegen, or HIR development.

5. **Learn error handling**: [DIAGNOSTICS.md](DIAGNOSTICS.md) describes the structured diagnostic system, error accumulation patterns, error code conventions, and display formatting.

6. **Explore CLI invocation**: [DRIVER.md](DRIVER.md) explains how `no3` parses commands, loads packages, and invokes the pipeline, providing context for how the compiler is actually used.

## Complete Data Flow Diagram

The following diagram traces source code through the entire compilation pipeline, showing each stage's input/output and the progressive lowering through intermediate representations:

```
Source Code (.nit files) ──► [File Loading + Package Resolution]
    │  Output: Byte slices + FileIds + CompilerLog
    ▼
[Lexical Analysis — nitrate_token_lexer]
    │  Input: &[u8] source bytes
    │  Output: Token stream (AnnotatedToken values with source positions)
    │  Handling: Maximal munch, keyword recognition, literal parsing, trivia management
    ▼
[Syntactic Parsing — nitrate_tree_parse]
    │  Input: Token stream from lexer
    │  Output: Parse Tree (Module with Items: functions, structs, enums, etc.)
    │  Strategy: Hand-written recursive descent with precedence climbing
    ▼
[Name Resolution — nitrate_tree_resolve]
    │  Input: Unresolved Parse Tree
    │  Output: Resolved Parse Tree + populated SymbolTab
    │  Operations: Import expansion, scope analysis, path resolution, error detection
    ▼
[HIR Lowering — nitrate_hir_from_tree]
    │  Input: Resolved Parse Tree + SymbolTab
    │  Output: HIR items in TLS Store (TypeId, FunctionId, ValueId handles)
    │  Operations: Type interning, expression graph construction, symbol resolution
    ▼
[Type Inference + Solving — nitrate_hir_solve]
    │  Input: Unresolved HIR (Type::Inferred, Type::GenericParam present)
    │  Output: Solved HIR (all types resolved, generics monomorphized)
    │  Algorithm: Fixed-point constraint solving with monomorphization
    ▼
[HIR Validation — nitrate_hir_validate]
    │  Input: Solved HIR module
    │  Output: ValidHir<Module> (type-level guarantee of correctness)
    │  Checks: No unresolved types, valid control flow, sound expressions
    ▼
[Name Mangling — nitrate_hir_mangle]
    │  Input: Validated HIR with Function and GlobalVariable records
    │  Output: mangled_name fields populated on all symbols
    ▼
[LLVM IR Code Generation — nitrate_llvm_from_hir]
    │  Input: ValidHir<Module>
    │  Output: LLVM Module (verified, type-checked)
    │  Passes: (1) Globals (2) Declarations (3) Definitions
    ▼
[LLVM Optimization — nitrate_llvm / ModuleOptimizer]
    │  Input: Unoptimized LLVM Module
    │  Output: Optimized LLVM Module (based on optimization level 0-3)
    │  Passes: Mem2Reg, GVN, SCCP, inlining, loop opts, DCE
    ▼
[Native Code Emission]
    │  Output: Object file (.o), executable, assembly (.s), or LLVM IR (.ll)
```

## Foundational Architectural Patterns

Five architectural patterns recur consistently throughout the compiler. Understanding them will accelerate comprehension of any individual subsystem document:

### Pattern 1: Thread-Local Storage (TLS)

The compiler avoids both global mutable state and passing the Store through every function call. Instead, the `Store` type is placed in thread-local storage at the start of a compilation session. All handles (`TypeId`, `FunctionId`, `ValueId`, etc.) dereference through TLS to access their underlying data. This enables lifetime-free handles, reentrant access from any call site, and RAII-based deterministic cleanup when the compilation session ends.

### Pattern 2: Diagnostic Accumulation

Rather than aborting on the first error, every compilation stage collects diagnostics in a shared `CompilerLog`. Each stage defines its own structured error types implementing `FormattableDiagnosticGroup`. The pipeline checks for errors between stages, but within each stage, processing continues to maximize error discovery. This enables users to see all issues in a single compilation pass.

### Pattern 5: Immutable Core with Mutable Edges

Types are immutable, interned, and deduplicated: identical types always produce the same `TypeId`, and handle comparison equals structural comparison. Values and items are stored in `RefCell`-backed append-only vectors that allow interior mutability. This hybrid optimizes the most frequent operation (type comparison) while supporting the mutation patterns required by the solver.

### Pattern 6: Symbol Table as Compilation Hub

Most passes interact with the `SymbolTab`, which provides iteration over all defined symbols (functions, types, globals, methods). The symbol table is built during name resolution, extended during solving (with monomorphized copies), and iterated by codegen. This abstraction allows incremental population without requiring passes to understand symbol storage internals.

## Documentation Contribution Guidelines

When extending or modifying these documentation files, adhere to the following standards:

1. **Structure**: Present theoretical foundation first, then practical implementation details, then design rationale with tradeoffs considered
2. **Visual aids**: Use ASCII art diagrams for data flow, architecture, and relationships where they improve comprehension
3. **Conventions**: Reference type names (`TypeId`, `Value::Binary`), function names, and crate names with backtick formatting. Avoid line-number references
4. **Rationale-first**: Every design decision should explain why that approach was chosen over alternatives, including specific tradeoffs and limitations
5. **Cross-references**: Use relative links to related documents. Central concepts (Store, TLS, Monomorphization) should be linked to their primary documentation
6. **Audience targeting**: Each document specifies its target audience; foundational documents assume less prior knowledge than subsystem-specific ones

## Documentation Statistics

- **Total documents**: 22 comprehensive reference files
- **Scope**: Every compiler subsystem covered, from low-level byte scanning to high-level optimization pipelines
- **Target audience**: From new contributors to experienced subsystem maintainers
