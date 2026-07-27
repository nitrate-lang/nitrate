# HIR Validation Subsystem

## Overview

HIR validation is a semantic checking pass that ensures the HIR is well-formed before code generation. It walks the entire HIR tree, verifying items, expressions, and types for correctness. Successful validation produces a `ValidHir<T>` wrapper that signals the HIR is ready for codegen.

## Architecture

**Crate**: `nitrate_hir_validate`  
**Key types**: `ValidHir`, validation visitors  
**Key files**: `lib.rs`, `validate_hir.rs`, `item.rs`, `expr.rs`, `ty.rs`, `diagnosis.rs`

## The ValidHir Wrapper

```rust
pub struct ValidHir<T>(T);

impl<T> ValidHir<T> {
    pub fn new(inner: T) -> Self {
        ValidHir(inner)
    }

    pub fn into_inner(self) -> T {
        self.0
    }
}
```

The wrapper provides a type-level guarantee that the HIR has been validated. The codegen entry point `generate_llvmir` accepts `ValidHir<hir::Module>`, ensuring validation is never skipped.

## Validation Passes

The validator runs multiple passes over the HIR:

### Item Validation

Validates top-level declarations:

- **Functions**:
  - Return type is a valid type
  - Parameter types are valid
  - Body (if present) is well-formed
  - Generic parameters (if any) have valid bounds
  - `NoMangle` functions have external linkage
  - `extern` functions with bodies are valid
- **Structs**:
  - Field types are valid
  - Fields have unique names
  - `Packed` structs have valid layouts
  - Generic parameters have valid bounds

- **Enums**:
  - Variant types are valid
  - Variants have unique names
  - Generic parameters have valid bounds

- **Traits**:
  - Method definitions are valid function signatures (no bodies)
  - Associated type names are unique
  - Supertrait references are valid
  - No circular trait inheritance

- **Type Aliases**:
  - The aliased type is valid
  - Generic parameters match usage

### Expression Validation

Validates expression nodes in function bodies:

- **Type correctness**: All expressions have compatible types
- **Control flow**: `break`/`continue` only inside loops, `return` only inside functions
- **Borrow checking**: `&mut` requires mutable place, no conflicting borrows
- **Call validation**: Arguments match parameter count and types
- **Field access**: Fields exist on the target struct type
- **Index access**: Target is array/slice type, index is integer type
- **Assignment**: Left side is a place expression, types match
- **Cast validation**: Cast is between compatible types

### Type Validation

Validates type expressions:

- **No unresolved types**: All `Type::Inferred` variables must be resolved
- **No remaining GenericParams**: After monomorphization, no `Type::GenericParam` should remain
- **Array length**: Non-zero, fits in a `u32`
- **Reference validation**: Valid lifetime references
- **Struct fields**: Field types in the struct definition match the layout

## Validation Error Types

Defined in `diagnosis.rs`:

- `InvalidFunctionSignature`: Function parameter or return type error
- `MismatchedTypes`: Type mismatch in expression
- `InvalidExpression`: Malformed expression
- `InvalidStructDef`: Struct definition error
- `InvalidEnumDef`: Enum definition error
- `UnresolvedTypeVariable`: `Type::Inferred` or `Type::GenericParam` still present
- `InvalidBorrow`: Invalid borrow (e.g., borrowing a non-place expression)
- `InvalidControlFlow`: Misplaced break/continue/return
- `MissingField`: Referencing a nonexistent struct field
- `TypeMismatch`: Expression type doesn't match expected type

## Validation Traversal

The validator traverses the HIR using a visitor pattern:

```rust
pub fn validate(module: hir::Module, log: &CompilerLog) -> Result<ValidHir<hir::Module>, ()> {
    let validator = HirValidator::new(log);
    validator.validate_module(&module)?;
    Ok(ValidHir::new(module))
}

struct HirValidator<'log> {
    log: &'log CompilerLog,
    errors: Vec<ValidationError>,
}

impl HirValidator {
    fn validate_module(&mut self, module: &hir::Module) -> Result<(), ()> {
        for item in &module.items {
            self.validate_item(item)?;
        }
        Ok(())
    }

    fn validate_item(&mut self, item: &hir::Item) -> Result<(), ()> {
        match item {
            Item::Function(id) => self.validate_function(id),
            Item::StructDef(id) => self.validate_struct_def(id),
            Item::EnumDef(id) => self.validate_enum_def(id),
            Item::Trait(id) => self.validate_trait(id),
            Item::TypeAliasDef(id) => self.validate_type_alias(id),
            Item::Module(id) => self.validate_module(&id.borrow()),
            Item::GlobalVariable(id) => self.validate_global(id),
        }
    }
}
```

## Integration

Validation is the final semantic checking pass before code generation:

```
Solved HIR → [Validator] → ValidHir → [Codegen] → LLVM IR
```

If validation fails, the compilation stops with error messages. No code generation is attempted for invalid HIR.
