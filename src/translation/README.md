# Nitrate Translation

This directory contains the Nitrate compiler's translation pipeline — the series of passes that transform source code into LLVM IR.

## Architecture

The translation pipeline is organized into several crate layers, all managed as Cargo workspace members:

```
src/translation/
├── Cargo.toml              # Top-level translation facade crate
├── src/nstring/            # String interning
├── src/token/              # Token types & diagnostics
├── src/token_lexer/        # Lexer: text → tokens
├── src/tree/               # Concrete Syntax Tree (CST) v1
├── src/tree_parse/         # Parser: tokens → tree (v1)
├── src/tree_resolve/       # Name resolution on the CST
├── src/hir/                # High-level IR data types
├── src/hir_dump/           # HIR debug formatting
├── src/hir_evaluate/       # HIR constant expression evaluation
├── src/hir_from_tree/      # Lowering: CST → HIR
├── src/hir_get_type/       # HIR type inference helper
├── src/hir_mangle/         # Name mangling
├── src/hir_solve/          # HIR type solving & monomorphization
├── src/hir_validate/       # HIR semantic validation
├── src/llvm/               # LLVM IR bindings (via inkwell)
└── src/llvm_from_hir/      # Lowering: HIR → LLVM IR
```

## Workspace Dependencies

All crates under `src/translation/` are **workspace members** of the root `Cargo.toml`. External dependencies (serde, inkwell, log, etc.) are defined centrally in `[workspace.dependencies]`, and each crate references them with e.g.:

```toml
serde_json.workspace = true
ordered-float = { workspace = true, features = ["serde"] }
```

This ensures all crate versions stay in sync and makes updates easier.

### Internal path dependencies

Inter-crate dependencies within the translation folder are also defined as workspace dependencies:

```toml
nitrate_hir.workspace = true
nitrate_nstring.workspace = true
nitrate_token.workspace = true
# ... etc
```

The actual path mappings live in the root `[workspace.dependencies]` section; individual crates never hardcode relative paths.
