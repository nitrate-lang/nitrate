# Nitrate Programming Language

<div align="center">
    <img src="https://github.com/Kracken256/nitrate/blob/main/extensions/media/logo-dark-1024x1024.png?raw=true" width="300" height="300" alt="css-in-readme">
</div>

_This project is in rapid development. It will not compile into anything useful yet. Check back soon for updates on the project._

Here is a detailed structural summary of the Nitrate Compiler architecture as described in the documentation:

### Architectural Overview & Core Principles

The Nitrate compiler is a multi-stage systems language compiler written in Rust. Its design centers around four foundational principles:

1. **Immutable Interning with Thread-Local Storage (TLS):** Uses a TLS-based `Store` model to manage handles without lifetime annotations. Types in `TypeStore` and literals in `ExprLiteralStore` use a `BiMap`-backed deduplication strategy, guaranteeing $O(1)$ structural equality comparison via handle equality. Mutable items (functions, structs, etc.) are stored in `AppendOnlyVec<RefCell<T>>` to support interior mutability during monomorphization.

2. **Diagnostic Accumulation:** Errors are gathered across 7 distinct diagnostic groups into a `CompilerLog` rather than aborting on the first error.

3. **Pass-Based Pipeline with Fixed-Point Iteration:** Compilation phases operate as standalone passes. Certain passes—notably Hindley-Milner type inference—iteratively execute until reaching a fixed point.

4. **Reentrant Store Access:** Dereferencing handles retrieves the store pointer via TLS, allowing store lookup across any scope without explicit lifetimes.

---

### End-to-End Compilation Pipeline

The translation pipeline executes 10 sequential stages to lower source code into native machine code:

```
[.nit Source Files + no3.xml Manifest]
                   │
                   ▼
  Stage 0: Package & Source Loading (no3 driver, CompilerLog creation)
                   │
                   ▼
  Stage 1: Lexical Analysis (Maximal munch scanning -> AnnotatedToken stream)
                   │
                   ▼
  Stage 2: Syntactic Parsing (Hand-written recursive descent -> AST Parse Tree)
                   │
                   ▼
  Stage 3: Name Resolution (Path expansion, module symbol table construction)
                   │
                   ▼
  Stage 4: HIR Lowering (AST -> High-Level IR in TLS Store)
                   │
                   ▼
  Stage 5: Type Inference & Monomorphization (Hindley-Milner constraint solving)
                   │
                   ▼
  Stage 6: Type Determination (HirGetType expression typing)
                   │
                   ▼
  Stage 7: HIR Validation (Inferred/Generic check -> ValidHir<T> wrapper)
                   │
                   ▼
  Stage 8: Name Mangling (Deterministic LLVM linkage symbols)
                   │
                   ▼
  Stage 9: LLVM Code Generation (Global vars, Function decls & defs)
                   │
                   ▼
  Stage 10: LLVM Optimization & Emission (.o, .s, or .ll output)

```

---

### Lexical Analysis Subsystem Summary

#### Core Lexer Characteristics

- **Input/Output:** Consumes source byte slices (`&[u8]`) and produces `AnnotatedToken` instances carrying explicit start/end line, column, byte offset, and `FileId` spans for caret diagnostic reporting.

- **Position Tracking:** Accounts for multi-byte UTF-8 sequences (continuation bytes `0x80`–`0xBF` do not increment the column counter) to match user editor code points.

- **Source Size & Trivia Limits:** Enforces a 4 GiB (`u32::MAX` byte) source limit. Operates in either trivia-enabled (preserves whitespace/comments) or trivia-disabled (default) modes.

#### Parsing Algorithms & Token Structure

- **Lexing Algorithm:** Employs a single-byte dispatch pattern at the entry point (`parse_next_token`).

- **Identifiers:** Standard identifiers match alphanumeric/underscore runs against a 52-keyword table. Atypical identifiers enclosed in backticks (`identifier`) allow keyword usage as identifiers for FFI scenarios.

- **Integer Literals:** Processed via `parse_number()` using maximal munch. Supports prefix-based radix decoding (`0b`, `0o`, `0d`, `0x`), strips visual underscore separators (`_`), and decodes up to `u128` values.

---

### Type System Reference Summary

#### Type Representation & Layout

- **Storage & Interning:** The core `Type` enum has 37+ variants. Types are interned in `TypeStore` behind `Arc` pointers inside a `BiMap`, allowing instant handle equality checks.

- **Primitive Types:** Include `Never` (bottom type for diverging paths like `return`/`panic`), `Unit` (`()`, 0 bytes), `Bool` (1 byte), fixed-width signed/unsigned integers (`I8`–`I128`, `U8`–`U128`), floating-point types (`F32`, `F64` wrapped in `NotNan`), and target-dependent `USize`.

- **Compound & Advanced Types:**
- **Arrays & Tuples:** Fixed-size arrays (`len * stride`) and alignment-padded heterogeneous tuples.

- **Structs & Enums:** Structs support standard and packed memory layouts. Enums use a rear-discriminant tagged union layout, sizing the discriminant byte width based on total variant count.

- **Refinement Types:** `Refine { base, min, max }` constrains integer ranges and propagates bounds through arithmetic for compile-time safety verification.

- **Reference/Pointer Families:** Differentiates `Reference`, `SliceRef`, `Pointer`, and `SlicePtr` across four permission flags (`&T`, `&mut T` shared, `&uniq T`, and `&mut T` exclusive).

---

### Subsystem Reference Guide

| Subsystem / Module | Primary Crate            | Primary Role & Responsibilities                                                                                                           |
| ------------------ | ------------------------ | ----------------------------------------------------------------------------------------------------------------------------------------- |
| **CLI Driver**     | `nitrate_driver` / `no3` | Binary entry point handling subcommands (`build`, `run`, `check`, `lex`, `parse`), manifest parsing (`no3.xml`), and package resolution.  |
| **Lexer**          | `nitrate_token_lexer`    | Byte-slice dispatch scanner implementing maximal munch, backtick raw identifiers, and token location tracking.                            |
| **Parser**         | `nitrate_tree_parse`     | Hand-written recursive descent parser with precedence climbing (10 precedence levels) producing the Parse Tree AST.                       |
| **Name Resolver**  | `nitrate_tree_resolve`   | Resolves module paths, `use` statements, relative/absolute paths, and builds symbol tables via depth-first AST traversal.                 |
| **HIR Lowering**   | `nitrate_hir_from_tree`  | Desugars complex AST constructs, lowers types into the interned `TypeStore`, and emits value graphs into `ExprValueStore`.                |
| **Type Solver**    | `nitrate_hir_solve`      | Hindley-Milner constraint-based inference engine utilizing a fixed-point iteration loop and on-the-fly monomorphization.                  |
| **HIR Validation** | `nitrate_hir_validate`   | Semantic pass ensuring all type variables and generics are substituted; emits the type-safe `ValidHir<T>` wrapper.                        |
| **Diagnostics**    | `nitrate_diagnosis`      | Structured error accumulation framework (`CompilerLog`) with 16-bit IDs, group prefixes (`[S0...]`–`[S6...]`), and source caret printing. |
| **LLVM Codegen**   | `nitrate_llvm_from_mir`  | Multi-pass translator emitting global variables, function declarations, and definitions into LLVM IR.                                     |
