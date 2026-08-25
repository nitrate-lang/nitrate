# Build System, Dependencies, and Toolchain

## Overview

The Nitrate compiler uses Rust's Cargo build system with a workspace structure containing multiple crates. The build system handles compilation of the compiler itself, linking with LLVM, and producing the `no3` binary.

## Workspace Structure

The project is a Cargo workspace with crates organized under `src/`:

```
src/
├── lib.rs                   # Top-level re-exports
├── bin/no3.rs               # Main binary
├── diagnosis/               # Diagnostic system (nitrate_diagnosis)
├── driver/                  # Driver and CLI (nitrate_driver)
└── translation/             # Translation pipeline (nitrate_translation)
    └── src/
        ├── token/           # Token types
        ├── token_lexer/     # Lexer
        ├── tree/            # Parse tree types
        ├── tree_parse/      # Parser
        ├── tree_resolve/    # Name resolver
        ├── nstring/         # Interned strings
        ├── hir/             # HIR types and store
        ├── hir_from_tree/   # AST→HIR lowering
        ├── hir_solve/       # Type inference
        ├── hir_get_type/    # Type determination
        ├── hir_validate/    # HIR validation
        ├── hir_mangle/      # Name mangling
        ├── hir_evaluate/    # Constant evaluation
        ├── hir_dump/        # HIR pretty-printing
        ├── llvm/            # LLVM context wrapper
        └── llvm_from_mir/   # MIR→LLVM codegen
```

## Key Dependencies

LLVM is the primary backend dependency, accessed through the `inkwell` crate (version 0.4 with LLVM 18 features). Other core dependencies include `append_only_vec`, `bimap`, `thin-vec`, `thin-str`, `ordered-float`, `enum-iterator`, `serde`/`serde_json`, and `log`.

## Building the Compiler

```bash
cargo build              # Debug build
cargo build --release    # Release build
cargo test               # Run all tests
cargo test -p nitrate_hir_solve  # Test specific crate
```

## Testing

Tests are organized per crate. Each crate contains targeted tests for its functionality: `nitrate_token_lexer` tests lexical analysis, `nitrate_tree_parse` tests parsing, `nitrate_hir_from_tree` tests lowering, and `nitrate_hir_solve` tests type inference.

## System Dependencies

Building requires the Rust toolchain (latest stable, edition 2021), LLVM 18 development libraries, a C++ compiler for LLVM linkage, and cmake for some LLVM build configurations.
