# Build System, Dependencies, and Toolchain

## Overview

The Nitrate compiler uses Rust's Cargo build system with a workspace structure containing multiple crates. The build system handles compilation of the compiler itself, linking with LLVM, and producing the `no3` binary.

## Workspace Structure

The project is a Cargo workspace with the following crate layout:

```
nitrate/                    # Root workspace
├── Cargo.toml             # Workspace definition
├── rustfmt.toml           # Code formatting configuration
├── src/
│   ├── lib.rs             # Top-level re-exports
│   └── bin/
│       └── no3.rs         # Main binary entry point
├── src/diagnosis/         # Diagnostic system crate
├── src/driver/            # Driver and CLI crate
├── src/optimization/      # Optimization crate
└── src/translation/       # Translation pipeline crate
    └── src/
        ├── token/         # Token types
        ├── token_lexer/   # Lexer
        ├── tree/          # Parse tree types
        ├── tree_parse/    # Parser
        ├── tree_resolve/  # Name resolver
        ├── nstring/       # Interned strings
        ├── hir/           # HIR types and store
        ├── hir_from_tree/ # AST→HIR lowering
        ├── hir_solve/     # Type inference
        ├── hir_get_type/  # Type determination
        ├── hir_validate/  # HIR validation
        ├── hir_mangle/    # Name mangling
        ├── hir_evaluate/  # Constant evaluation
        ├── hir_dump/      # HIR pretty-printing
        ├── llvm/          # LLVM context wrapper
        └── llvm_from_hir/ # HIR→LLVM codegen
```

## Key Dependencies

### LLVM

LLVM is the primary backend dependency. The compiler uses the `inkwell` crate for LLVM bindings:

```toml
[dependencies]
inkwell = { version = "0.4", features = ["llvm18-0"] }
```

The LLVM dependency requires the LLVM 18 development libraries to be installed on the build system.

### Other Core Dependencies

- `append_only_vec`: Append-only vector for lock-free concurrent growth
- `bimap`: Bidirectional map for type deduplication
- `thin-vec`: Memory-efficient vector type
- `thin-str`: Memory-efficient string type
- `ordered-float`: Ordered float wrappers (NotNan, OrderedFloat)
- `enum-iterator`: Enum iteration for tests
- `serde`, `serde_json`: Serialization for diagnostics and error codes
- `log`: Logging facade

## Building the Compiler

```bash
# Build the compiler
cargo build

# Build in release mode
cargo build --release

# Run tests
cargo test

# Run specific crate tests
cargo test -p nitrate_hir_solve
```

## Testing

Tests are organized per crate, with each crate containing a `tests/` directory where applicable:

- `nitrate_token_lexer`: Extensive lexer test suite (identifiers, literals, errors, etc.)
- `nitrate_tree_parse`: Parser test suite (functions, structs, enums, expressions, etc.)
- `nitrate_hir_from_tree`: Lowering test suite
- `nitrate_hir_solve`: Solver test suite

## Formatting

Code formatting follows the configuration in `rustfmt.toml`:

```bash
cargo fmt
```

## Extensions

The VS Code extensions are located in `extensions/`:

- `extensions/vscode/nitrate-syntax/`: TextMate grammar for syntax highlighting
- `extensions/vscode/nitrate-lsp/`: LSP client extension
- `extensions/vscode/nitrate/`: Meta-package bundling both extensions

## System Dependencies

Building the compiler requires:

1. **Rust toolchain**: Latest stable Rust (edition 2021)
2. **LLVM 18 development libraries**: For the `inkwell` LLVM bindings
3. **C++ compiler**: For linking LLVM (g++, clang++, or MSVC)
4. **cmake**: Required by some LLVM build configurations

On Ubuntu/Debian:

```bash
apt install llvm-18-dev libclang-18-dev
```

On macOS (Homebrew):

```bash
brew install llvm@18
```
