# HIR Validation Subsystem

## Overview and Purpose

HIR validation is a semantic checking pass that ensures the HIR is well-formed before code generation. It walks the entire HIR tree — items, expressions, types, and all supporting data structures — verifying that the output of the type inference phase is complete, consistent, and ready for code generation. Validation serves as the final quality gate in the compiler's semantic analysis pipeline.

The key insight motivating validation as a separate pass: while the solver resolves types and monomorphizes generics, it does not exhaustively verify every semantic property of the HIR. The solver focuses on type constraints; validation checks everything else. This separation keeps each pass focused and manageable.

Successful validation produces a `ValidHir<T>` wrapper that provides a type-level guarantee that the HIR has passed all checks. The codegen entry point accepts only `ValidHir<Module>`, ensuring that validation can never be accidentally skipped.

## Architecture

**Crate**: `nitrate_hir_validate`  
**Key types**: `ValidHir`, `HirValidator` (internal visitor)  
**Key files**: `lib.rs`, `validate_hir.rs`, `item.rs`, `expr.rs`, `ty.rs`, `diagnosis.rs`

## The ValidHir Wrapper

```rust
pub struct ValidHir<T>(T);

impl<T> ValidHir<T> {
    pub fn new(inner: T) -> Self { ValidHir(inner) }
    pub fn into_inner(self) -> T { self.0 }
}
```

The wrapper provides a compile-time guarantee. The codegen entry point `generate_llvmir` accepts `ValidHir<hir::Module>`, making it impossible to generate code from unvalidated HIR. This pattern — using a newtype wrapper to encode phase ordering in the type system — prevents a class of bugs where code is generated from semantically invalid input.

## Validation Passes

### Item Validation

Validates all top-level declarations:

- **Functions**: Return type is valid; parameter types are valid; body (if present) is well-formed; generic parameters have valid bounds; `NoMangle` functions have external linkage
- **Structs**: Field types are valid; fields have unique names; `Packed` structs have valid layouts; generic parameters have valid bounds
- **Enums**: Variant types are valid; variants have unique names; generic parameters have valid bounds
- **Traits**: Method definitions are valid function signatures (no bodies); associated type names are unique; supertrait references are valid; no circular trait inheritance
- **Type Aliases**: The aliased type is valid; generic parameters match usage

### Expression Validation

Validates all expression nodes in function bodies:

- **Type correctness**: All expressions have compatible types (post-solver verification)
- **Control flow**: `break`/`continue` only inside loops; `return` only inside functions
- **Borrow checking**: `&mut` requires mutable place; no conflicting borrows
- **Call validation**: Argument count and types match parameter declarations
- **Field access**: Fields exist on the target struct type
- **Index access**: Target is array/slice type; index is integer type
- **Assignment**: Left side is a place expression; types match
- **Cast validation**: Cast is between compatible types

### Type Validation

- **No unresolved types**: All `Type::Inferred` variables must be resolved to concrete types
- **No remaining GenericParams**: After monomorphization, no `Type::GenericParam` should remain in any function body
- **Array length**: Non-zero; fits in a `u32`
- **Reference validation**: Valid lifetime references
- **Struct fields**: Field types in the struct definition match the computed layout

## Integration

Validation is the final semantic checking pass before code generation:

```
Solved HIR → [Validator] → ValidHir → [Codegen] → LLVM IR
```

If validation fails, compilation stops with error messages. No code generation is attempted for invalid HIR.
