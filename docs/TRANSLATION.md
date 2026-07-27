# Translation Pipeline Orchestration

## Overview

The translation pipeline orchestrates the transformation of source code from tokens to LLVM IR. It connects all individual compilation stages into a coherent, configurable pipeline that can operate in different modes depending on the user's request. The `nitrate_translation` crate serves as the root of the translation subsystem, re-exporting all sub-crate public APIs and providing the top-level `TranslationOptions` configuration that controls how the pipeline operates.

The pipeline is designed around the principle of progressive lowering: each stage transforms the input into a representation that is closer to machine code while preserving the semantic content of the original program. The stages are sequenced so that earlier stages perform purely syntactic transformations (lexing, parsing), middle stages perform semantic analysis (resolution, type inference), and later stages perform code generation and optimization.

## Architecture

**Crate**: `nitrate_translation`  
**Key types**: `TranslationOptions`, `TranslationOptionsBuilder`  
**Re-exports**: All HIR sub-crates (`nitrate_hir`, `nitrate_hir_from_tree`, `nitrate_hir_solve`, `nitrate_hir_validate`, `nitrate_hir_get_type`, `nitrate_hir_mangle`, `nitrate_hir_evaluate`, `nitrate_hir_dump`), the parser (`nitrate_tree_parse`), lexer (`nitrate_token_lexer`), resolver (`nitrate_tree_resolve`), LLVM bindings (`nitrate_llvm`, `nitrate_llvm_from_hir`), and supporting infrastructure (`nitrate_token`, `nitrate_tree`, `nitrate_nstring`)

## Pipeline Components

The translation pipeline is composed of 16 sub-crates, each implementing one stage of the compilation process:

```
nitrate_translation (orchestrator crate - coordinates all stages)
├── nitrate_token              (Token type definitions - the output of lexing)
├── nitrate_token_lexer        (Source bytes → Token stream)
├── nitrate_tree               (Parse tree / AST type definitions)
├── nitrate_tree_parse         (Token stream → Parse tree/AST)
├── nitrate_tree_resolve       (Parse tree → Resolved parse tree with symbol table)
├── nitrate_nstring            (Interned string system - NString)
├── nitrate_hir                (HIR types, Store, Pass infrastructure)
├── nitrate_hir_from_tree      (Resolved parse tree → HIR in Store)
├── nitrate_hir_solve          (HIR → Solved HIR with type inference + monomorphization)
├── nitrate_hir_get_type       (HirGetType trait for determining expression types)
├── nitrate_hir_validate       (HIR → Validated HIR with ValidHir wrapper)
├── nitrate_hir_mangle         (Name mangling for LLVM linkage names)
├── nitrate_hir_evaluate       (Constant evaluation for compile-time expressions)
├── nitrate_hir_dump           (HIR pretty-printing for debugging)
├── nitrate_llvm               (LLVM context wrapper and type factories)
└── nitrate_llvm_from_hir      (Validated HIR → LLVM IR module)
```

## TranslationOptions

The `TranslationOptions` struct configures the translation pipeline's behavior:

```rust
pub struct TranslationOptions {
    pub optimization_level: u8,       // 0-3, controls LLVM optimization
    pub debug_info: bool,              // Generate DWARF debug information
    pub target_triple: Option<String>, // e.g., "x86_64-unknown-linux-gnu"
    pub emit_llvm_ir: bool,            // Emit .ll file instead of object file
    pub emit_assembly: bool,           // Emit .s assembly file instead of object file
    pub llvm_options: Vec<String>,     // Additional LLVM pass options
    pub output_path: Option<PathBuf>,  // Override default output location
}
```

The `TranslationOptionsBuilder` provides a builder pattern for constructing options:

```rust
let options = TranslationOptionsBuilder::new()
    .optimization_level(2)
    .debug_info(true)
    .target_triple("x86_64-unknown-linux-gnu")
    .build();
```

## Data Flow Through the Pipeline

The pipeline processes source code through 10 sequential stages, each of which transforms the data into a progressively lower-level representation:

**Stage 1 — Source Loading**: Read `.nit` files from the package's `src/` directory as byte slices. Each file is assigned a unique `FileId` for consistent source location tracking. A `CompilerLog` instance is created to accumulate diagnostics throughout the entire pipeline. The package manifest (`no3.xml`) is also loaded during this stage to determine the package name and dependencies.

**Stage 2 — Lexical Analysis**: Create a `Lexer` for each source file. The lexer reads source bytes and produces a flat stream of `AnnotatedToken` values, each with precise source position information. Lexer errors (invalid tokens, malformed literals) are reported to the `CompilerLog`.

**Stage 3 — Syntactic Parsing**: Create a `Parser` for each token stream. The parser uses recursive-descent parsing with precedence climbing to build a `Module` — the top-level AST node containing all items (functions, structs, enums, etc.) declared in the file. Parser errors (grammar violations, missing tokens) are reported to the `CompilerLog`.

**Stage 4 — Name Resolution**: Process `use` declarations and resolve all name paths to their fully qualified equivalents. The resolver builds a `SymbolTab` that maps names to their declarations. Resolution errors (unknown names, ambiguous references, cyclic imports) are reported.

**Stage 5 — HIR Lowering**: Create the `Store` for the compilation session and set up TLS storage via `using_storage()`. The lowerer transforms each resolved `Module` into HIR items, interning types in `TypeStore` and storing value expressions in `ExprValueStore`. Lowering errors (invalid type expressions, unresolved symbols) are reported.

**Stage 6 — Type Solving**: Run the Hindley-Milner fixed-point solver for each function. The solver resolves `Inferred` and `InferredInteger`/`InferredFloat` type variables through constraint propagation, monomorphizes generic function instantiations, and registers the resulting concrete functions in the symbol table. Type errors (mismatches, unsatisfiable constraints, refinement violations) are reported.

**Stage 7 — HIR Validation**: Walk the entire HIR tree to verify that all `Inferred` types have been resolved, all `GenericParam` instances have been substituted, control flow is valid, and all field/index accesses target valid types. Successful validation produces a `ValidHir<Module>` wrapper.

**Stage 8 — Name Mangling**: Compute deterministic LLVM linkage names for each function and global variable by encoding the package name, symbol name, and type signature into a compact string format.

**Stage 9 — LLVM Code Generation**: Create an `LLVMContext` and generate an LLVM `Module` from the validated HIR. The codegen runs three passes: global variable generation, function declarations, and function definitions. The resulting LLVM module is verified for correctness.

**Stage 10 — Optimization and Output**: Run the `ModuleOptimizer` with the configured optimization level. The optimized module is then emitted as an object file (`.o`), assembly file (`.s`), or LLVM IR text (`.ll`) depending on the compilation flags.

## Error Handling Between Stages

The pipeline checks for errors at each stage before proceeding to the next:

```rust
fn compile(options: &TranslationOptions) -> Result<(), Vec<Diagnostic>> {
    let log = CompilerLog::new();

    let tokens = lex_source(&source, &log)?;
    if log.has_errors() { return Err(log.errors()); }

    let ast = parse_tokens(tokens, &log)?;
    if log.has_errors() { return Err(log.errors()); }

    // ... continues through each stage
}
```

This pattern ensures that the pipeline stops as soon as a stage produces errors, because subsequent stages depend on correct output from earlier stages. However, within each stage, as many errors as possible are collected before reporting, maximizing the information available to the programmer.

## Compilation Modes

The pipeline supports different modes by stopping at different stages:

- **Full compilation** (stages 1-10): Used for `no3 build` and `no3 run`
- **Check-only** (stages 1-7): Used for `no3 check` — validates code without producing output
- **Lex-only** (stage 2): Used for `no3 lex` — debugging the lexer by inspecting the token stream
- **Parse-only** (stages 2-3): Used for `no3 parse` — debugging the parser by inspecting the AST
- **LLVM IR emission** (stages 1-9): Used for `no3 build --show-llvm` — outputs LLVM IR for inspection

## Pass Manager Integration

The pipeline uses the `Pass` trait and `PassManager` from `nitrate_hir` for extensibility. New passes can be added to the pipeline without modifying existing code:

```rust
let mut pass_manager = PassManager::new();
pass_manager.add_pass(Box::new(LoweringPass::new(options)));
pass_manager.add_pass(Box::new(SolvingPass::new(options)));
pass_manager.add_pass(Box::new(ValidationPass::new(options)));
let result = pass_manager.run(input);
```

## Design Rationale

**Why a modular pipeline?** The modular approach provides testability (each stage can be tested independently with its own test fixtures), reusability (stages can be combined in different ways for different compilation modes), parallelism potential (independent stages like lexing different files can be parallelized in the future), and clear boundaries with well-defined input and output types.

**Why error accumulation between stages?** Accumulating errors before proceeding ensures maximum error discovery in a single pass. Later stages may still be able to work with partial results from earlier stages (for example, the parser can produce a partial AST even if the lexer produced some errors).

**Why the ValidHir wrapper?** The `ValidHir<T>` wrapper provides a type-level guarantee that the HIR has passed validation. The codegen entry point accepts only `ValidHir<Module>`, making it impossible to generate code from invalid HIR. This pattern — using the type system to enforce phase ordering — prevents a class of bugs where code is generated from semantically incorrect input.
