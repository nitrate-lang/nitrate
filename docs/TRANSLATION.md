# Translation Pipeline

## Overview

The translation pipeline orchestrates the transformation of source code from tokens to LLVM IR. It connects all the individual compilation stages into a coherent, configurable pipeline. The `nitrate_translation` crate serves as the root of the translation subsystem, re-exporting all sub-crate public APIs and providing the top-level `TranslationOptions` configuration.

## Architecture

**Crate**: `nitrate_translation`  
**Key types**: `TranslationOptions`, `TranslationOptionsBuilder`  
**Re-exports**: All HIR sub-crates, parser, lexer, resolver, LLVM bindings

## Pipeline Components

The translation pipeline is composed of these stages, each implemented as a separate crate:

```
nitrate_translation (orchestrator)
├── nitrate_token              (token type definitions)
├── nitrate_token_lexer        (source → tokens)
├── nitrate_tree               (parse tree / AST types)
├── nitrate_tree_parse         (tokens → parse tree)
├── nitrate_tree_resolve       (parse tree → resolved parse tree)
├── nitrate_nstring            (interned string system)
├── nitrate_hir                (HIR types, store, passes)
├── nitrate_hir_from_tree      (resolved parse tree → HIR)
├── nitrate_hir_solve          (HIR → solved HIR, type inference)
├── nitrate_hir_get_type       (type determination for values)
├── nitrate_hir_validate       (HIR → validated HIR)
├── nitrate_hir_mangle         (name mangling)
├── nitrate_hir_evaluate       (constant evaluation)
├── nitrate_hir_dump           (HIR pretty-printing)
├── nitrate_llvm               (LLVM context wrapper)
└── nitrate_llvm_from_hir      (validated HIR → LLVM IR)
```

## TranslationOptions

The `TranslationOptions` struct configures the translation pipeline:

```rust
pub struct TranslationOptions {
    // Which optimization level to use (0-3)
    pub optimization_level: u8,

    // Whether to generate debug information
    pub debug_info: bool,

    // Target triple (e.g., "x86_64-unknown-linux-gnu")
    pub target_triple: Option<String>,

    // Whether to emit LLVM IR instead of machine code
    pub emit_llvm_ir: bool,

    // Whether to emit assembly instead of object code
    pub emit_assembly: bool,

    // Additional LLVM pass options
    pub llvm_options: Vec<String>,

    // Paths for output files
    pub output_path: Option<PathBuf>,
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

```
Input: Source file(s) + package manifest (no3.xml)

Step 1: Load source files
    ├── Read .nit files as byte slices
    ├── Assign FileId to each file
    └── Create CompilerLog for error accumulation

Step 2: Lexical Analysis
    ├── Create Lexer for each source file
    ├── Produce token stream per file
    └── Report lexer errors to CompilerLog

Step 3: Syntactic Parsing
    ├── Create Parser for each token stream
    ├── Produce Module (parse tree) per file
    └── Report parser errors to CompilerLog

Step 4: Name Resolution
    ├── Resolve imports (use declarations)
    ├── Resolve paths to fully qualified names
    ├── Build symbol table with preliminary entries
    └── Report resolution errors to CompilerLog

Step 5: HIR Lowering
    ├── Create Store for compilation session
    ├── Set up TLS storage via using_storage()
    ├── Lower each resolved Module to HIR items
    ├── Inter types in TypeStore
    ├── Store value expressions in ExprValueStore
    └── Report lowering errors to CompilerLog

Step 6: Type Solving (Hindley-Milner)
    ├── For each function: run fixed-point solver
    ├── Resolve inferred types
    ├── Monomorphize generic instantiations
    ├── Register monomorphized functions in symbol table
    └── Report type errors to CompilerLog

Step 7: HIR Validation
    ├── Walk entire HIR tree
    ├── Validate items, expressions, and types
    ├── Produce ValidHir wrapper
    └── Report validation errors to CompilerLog

Step 8: Name Mangling
    ├── For each function: compute mangled name
    ├── Encode package name, function name, type signature
    └── Store mangled name in Function::mangled_name

Step 9: LLVM Code Generation
    ├── Create LLVMContext
    ├── Generate LLVM Module from validated HIR
    ├── Verify LLVM module
    └── Output to object file, assembly, or LLVM IR

Step 10: Optimization (LLVM)
    ├── Run ModuleOptimizer with configured optimization level
    ├── Apply LLVM optimization passes
    └── Output the optimized module
```

## Pass Manager Integration

The translation pipeline uses the `Pass` trait and `PassManager` from `nitrate_hir`:

```rust
pub trait Pass<T> {
    fn run(&mut self, input: T) -> T;
}

pub struct PassManager<T> {
    passes: Vec<Box<dyn Pass<T>>>,
}
```

The pipeline can be extended by registering additional passes:

```rust
let mut pass_manager = PassManager::new();
pass_manager.add_pass(Box::new(LoweringPass::new(options)));
pass_manager.add_pass(Box::new(SolvingPass::new(options)));
pass_manager.add_pass(Box::new(ValidationPass::new(options)));
let result = pass_manager.run(input);
```

## Compilation Modes

The translation pipeline supports several modes:

### Full Compilation

```
source → tokens → AST → resolved AST → HIR → solved HIR → validated HIR → LLVM IR → object file
```

Used for `no3 build` and `no3 run`.

### Check-Only

```
source → tokens → AST → resolved AST → HIR → solved HIR → validated HIR
```

Used for `no3 check` — validates code without producing output.

### Lex-Only

```
source → tokens
```

Used for `no3 lex` — debugging the lexer.

### Parse-Only

```
source → tokens → AST
```

Used for `no3 parse` — debugging the parser.

### LLVM IR Emission

```
source → tokens → AST → resolved AST → HIR → solved HIR → validated HIR → LLVM IR
```

Used for `no3 build --emit-llvm` — outputs LLVM IR for inspection.

## Error Handling

The pipeline checks for errors at each stage before proceeding:

```rust
fn compile(options: &TranslationOptions) -> Result<(), Vec<Diagnostic>> {
    let log = CompilerLog::new();

    let tokens = lex_source(&source, &log)?;
    if log.has_errors() { return Err(log.errors()); }

    let ast = parse_tokens(tokens, &log)?;
    if log.has_errors() { return Err(log.errors()); }

    let resolved_ast = resolve_names(ast, &log)?;
    if log.has_errors() { return Err(log.errors()); }

    let hir = lower_to_hir(resolved_ast, &log)?;
    if log.has_errors() { return Err(log.errors()); }

    let solved_hir = solve_types(hir, &log)?;
    if log.has_errors() { return Err(log.errors()); }

    let validated_hir = validate_hir(solved_hir, &log)?;
    if log.has_errors() { return Err(log.errors()); }

    let llvm_module = generate_code(validated_hir, options)?;
    emit_output(llvm_module, options)?;

    Ok(())
}
```

## Integration with Driver

The driver subsystem (`nitrate_driver`) invokes the translation pipeline based on user commands. The driver:

1. Parses command-line arguments
2. Discovers and loads the package manifest
3. Finds source files
4. Creates the `CompilerLog`
5. Creates the `TranslationOptions`
6. Invokes the appropriate compilation mode
7. Handles output (files, stdout)
8. Reports diagnostics to the user

## Extension Points

The translation pipeline is designed for extensibility:

- **Additional passes**: New passes can be added to the `PassManager`
- **Custom lowering**: Alternative lowerers can produce different IRs
- **Alternative backends**: While LLVM is the primary backend, the `ValidHir` boundary provides a clean interface for alternative backends
- **Plugin system**: Future plugins could intercept or modify the pipeline at any stage

## Design Rationale

### Why a Modular Pipeline?

The modular pipeline approach provides:

1. **Testability**: Each stage can be tested independently
2. **Reusability**: Stages can be combined in different ways for different compilation modes
3. **Parallelism potential**: Independent stages (like lexing different files) can be parallelized
4. **Clear boundaries**: Each stage has a well-defined input and output type

### Why Error Accumulation Between Stages?

Accumulating errors before proceeding ensures:

1. **Maximum error discovery**: All errors in the current stage are reported before moving on
2. **Staged recovery**: Later stages may still work with partial results from earlier stages
3. **User efficiency**: Multiple issues are reported in a single compiler invocation

### Why the ValidHir Wrapper?

The `ValidHir<T>` wrapper provides a type-level guarantee that the HIR has passed validation:

- Codegen only accepts `ValidHir<Module>`, ensuring validation is never skipped
- The wrapper can be unwrapped via `into_inner()` after validation
- The type system enforces the compilation order at compile time
