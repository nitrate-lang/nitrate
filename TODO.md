# Nitrate Compiler - Visibility and Linkage Implementation

## Implementing Rust-compatible `extern` linkage syntax and visibility/linkage separation

### Completed

All items complete. The compiler now supports full Rust-compatible extern linkage syntax.

### Changes Summary

#### 1. Token Layer

- **token.rs**: Added `Token::Extern` enum variant with Display implementation
- **lex.rs**: Added `extern` keyword recognition → maps to `Token::Extern`

#### 2. Tree AST (tree crate)

- **item.rs**: Added `ExternAbi` struct with `name: NString` field
- **item.rs**: Added `abi: Option<ExternAbi>` field to `Function`

#### 3. Old Parser (tree_parse)

- **item.rs**: Added `parse_abi()` method to parse ABI string specifiers
- **item.rs**: Added `parse_extern_block()` method for `extern { ... }` blocks
- **item.rs**: Updated `parse_item()` to handle `extern` keyword before functions and extern blocks
- **item.rs**: Functions inside extern blocks inherit the block's ABI

#### 4. HIR Layer (hir crate)

- **ty.rs**: Added `ExternAbi` struct at HIR level
- **ty.rs**: Added `FunctionAttribute::ExternAbi(ExternAbi)` variant

#### 5. HIR Lowering (hir_from_tree)

- **item.rs**: Lower `abi` from AST Function into `FunctionAttribute::ExternAbi`

#### 6. LLVM Codegen (llvm_from_hir)

- **symbol.rs**: Added calling convention constants for common ABIs (C, stdcall, fastcall, thiscall, win64)
- **symbol.rs**: `get_abi_call_conv()` maps ABI strings to LLVM convention IDs
- **symbol.rs**: `gen_function_decl()` now sets call conventions for extern functions
- **symbol.rs**: Functions with `extern` ABI (without body) are emitted as declarations regardless of visibility

#### 7. Validation

- **ty.rs**: `FunctionAttribute::ExternAbi` accepted in HIR validation
- **dump_ty.rs**: `FunctionAttribute::ExternAbi` properly dumped

### Test Results

- All 211 existing tests pass with no regressions
- Build succeeds cleanly
- Test package compiles successfully
