# Nitrate Compiler Documentation Index

## Overview

This documentation provides comprehensive coverage of the Nitrate compiler, a modern systems programming language compiler built in Rust. The compiler architecture follows a multi-stage pipeline: source text → tokens → AST (Parse Tree) → HIR (High-Level IR) → LLVM IR → machine code.

## Documentation Files

| Document                                             | Description                                                    | Audience                |
| ---------------------------------------------------- | -------------------------------------------------------------- | ----------------------- |
| [OVERVIEW.md](OVERVIEW.md)                           | High-level compiler architecture and pipeline                  | All developers          |
| [LEXER.md](LEXER.md)                                 | Tokenization: source text to token stream                      | Lexer developers        |
| [PARSER.md](PARSER.md)                               | AST parsing: tokens to parse tree                              | Parser developers       |
| [RESOLVER.md](RESOLVER.md)                           | Name resolution and symbol table construction                  | Resolver developers     |
| [HIR.md](HIR.md)                                     | High-Level IR: types, expressions, store architecture          | HIR developers          |
| [TYPE_SYSTEM.md](TYPE_SYSTEM.md)                     | Type representation, semantics, and theory                     | Type system developers  |
| [HINDLEY_MILNER.md](HINDLEY_MILNER.md)               | Type inference via Hindley-Milner constraint solving           | Inference developers    |
| [GENERICS_ARCHITECTURE.md](GENERICS_ARCHITECTURE.md) | Generics and monomorphization architecture                     | Generics developers     |
| [VALIDATION.md](VALIDATION.md)                       | HIR validation passes and semantic checks                      | Validation developers   |
| [EVALUATION.md](EVALUATION.md)                       | Constant evaluation and compile-time computation               | Evaluation developers   |
| [SOLVER.md](SOLVER.md)                               | Trait solving and type constraint propagation                  | Solver developers       |
| [MANGLE.md](MANGLE.md)                               | Name mangling for symbol identification                        | Codegen developers      |
| [LLVM_CODEGEN.md](LLVM_CODEGEN.md)                   | LLVM IR generation and backend codegen                         | Codegen developers      |
| [OPTIMIZATION.md](OPTIMIZATION.md)                   | Optimization passes and transformation framework               | Optimization developers |
| [TRANSLATION.md](TRANSLATION.md)                     | Translation pipeline orchestration and wiring                  | Pipeline developers     |
| [DIAGNOSTICS.md](DIAGNOSTICS.md)                     | Error reporting, diagnostics, and user-facing messages         | All developers          |
| [DRIVER.md](DRIVER.md)                               | CLI driver, subcommands, and compiler invocation               | Tooling developers      |
| [PACKAGE_MANAGER.md](PACKAGE_MANAGER.md)             | Package management, dependencies, and publishing               | Package developers      |
| [LSP.md](LSP.md)                                     | Language Server Protocol implementation                        | LSP/IDE developers      |
| [BUILD_SYSTEM.md](BUILD_SYSTEM.md)                   | Build system, crate dependencies, and toolchain                | Build/CI developers     |
| [NSTRING.md](NSTRING.md)                             | Interned string system for memory-efficient identifier storage | Core developers         |
| [REFERENCE_SEMANTICS.md](REFERENCE_SEMANTICS.md)     | Reference semantics, borrowing, lifetimes, and memory safety   | Language developers     |

## Quick Start

For new developers, the recommended reading order is:

1. [OVERVIEW.md](OVERVIEW.md) — understand the big picture
2. [TRANSLATION.md](TRANSLATION.md) — understand how stages connect
3. [HIR.md](HIR.md) — understand the central IR
4. [TYPE_SYSTEM.md](TYPE_SYSTEM.md) — understand types
5. [DIAGNOSTICS.md](DIAGNOSTICS.md) — understand error handling
6. [DRIVER.md](DRIVER.md) — understand how compilation is invoked

Then read specific documents as needed for the subsystem you're working on.

> **Total**: 22 comprehensive reference documents covering every compiler subsystem, approximately 5,600+ lines of documentation.

## Key Crate Map

The compiler is organized as a Rust workspace with the following core crates:

| Crate                   | Path                                 | Role                           |
| ----------------------- | ------------------------------------ | ------------------------------ |
| `nitrate`               | `src/lib.rs`                         | Top-level re-exports           |
| `nitrate_diagnosis`     | `src/diagnosis/`                     | Error reporting infrastructure |
| `nitrate_driver`        | `src/driver/`                        | CLI, package management, LSP   |
| `nitrate_translation`   | `src/translation/`                   | Translation pipeline root      |
| `nitrate_token`         | `src/translation/src/token/`         | Token types and definitions    |
| `nitrate_token_lexer`   | `src/translation/src/token_lexer/`   | Lexer implementation           |
| `nitrate_tree`          | `src/translation/src/tree/`          | Parse tree (AST) types         |
| `nitrate_tree_parse`    | `src/translation/src/tree_parse/`    | Parser implementation          |
| `nitrate_tree_resolve`  | `src/translation/src/tree_resolve/`  | Name resolution                |
| `nitrate_hir`           | `src/translation/src/hir/`           | HIR types, storage, passes     |
| `nitrate_hir_from_tree` | `src/translation/src/hir_from_tree/` | AST→HIR lowering               |
| `nitrate_hir_solve`     | `src/translation/src/hir_solve/`     | Type inference + solver        |
| `nitrate_hir_validate`  | `src/translation/src/hir_validate/`  | HIR validation                 |
| `nitrate_hir_evaluate`  | `src/translation/src/hir_evaluate/`  | Constant evaluation            |
| `nitrate_hir_get_type`  | `src/translation/src/hir_get_type/`  | Type determination             |
| `nitrate_hir_mangle`    | `src/translation/src/hir_mangle/`    | Name mangling                  |
| `nitrate_hir_dump`      | `src/translation/src/hir_dump/`      | HIR pretty-printing            |
| `nitrate_llvm`          | `src/translation/src/llvm/`          | LLVM context wrapper           |
| `nitrate_llvm_from_hir` | `src/translation/src/llvm_from_hir/` | HIR→LLVM IR codegen            |
| `nitrate_nstring`       | `src/translation/src/nstring/`       | Interned string system         |
| `nitrate_optimization`  | `src/optimization/`                  | Optimization passes            |

## Compilation Pipeline (Data Flow)

```
Source Code (.nit)
    │
    ▼ [Lexer]
Token Stream ──────► Trivia (whitespace, comments)
    │
    ▼ [Parser]
Parse Tree (AST) ──► Item tree + Expression tree
    │
    ▼ [Resolver]
Resolved AST ──────► Symbol table populated
    │
    ▼ [HIR Lowering]
High-Level IR ─────► Type-level representation
    │
    ▼ [Type Inference / Solver]
Solved HIR ────────► Constraints resolved, generics monomorphized
    │
    ▼ [Validation]
Validated HIR ─────► Semantic correctness verified
    │
    ▼ [Codegen]
LLVM IR ───────────► Module + Functions + Globals
    │
    ▼ [LLVM Backend]
Machine Code ──────► Object file / executable
```

## Common Patterns

Throughout the compiler, several architectural patterns are consistently applied:

1. **Thread-Local Store**: HIR elements use TLS-based storage with `AppendOnlyVec` and `BiMap` for deduplication
2. **Diagnostic Accumulation**: Errors are collected via `CompilerLog` rather than panicking, enabling multi-error reporting
3. **Pass-Based Architecture**: Each compilation stage is a pass over the IR, often with fixed-point iteration
4. **Immutable Core + Mutable Edges**: Types are immutable and interned; values/items are stored in `RefCell` for mutation
5. **Symbol Table Agnosticism**: Most passes work through `SymbolTab` which provides iteration over all defined symbols

## Contributing to Documentation

When adding or modifying documentation:

1. Follow the existing style: theoretical foundation first, then practical implementation details
2. Include architectural diagrams using ASCII art where appropriate
3. Reference type names, function names, and crate names but avoid line numbers
4. Explain _why_ design decisions were made, not just _what_ was implemented
5. Cross-reference related documents using relative links
