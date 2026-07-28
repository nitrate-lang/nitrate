# Constraint Solver and Type Inference Engine

## Overview

The constraint solver resolves type constraints and propagates type information through the expression graph. It is the engine behind Nitrate's bidirectional Hindley-Milner type inference, monomorphization, and refinement type checking. The solver operates as part of the `nitrate_hir_solve` crate, serving as the core analysis pass that transforms unresolved HIR (with `Inferred` type variables and `GenericParam` references) into fully resolved, concrete type assignments ready for validation and code generation.

The solver has been comprehensively rewritten to maximize inference power, improve readability through modular decomposition, and provide immaculate error reporting with source locations.

## Architecture

**Crate**: `nitrate_hir_solve`  
**Key types**: `Solver`, `TypeConstraint`, `NodeAction`, `Substitution`  
**Modules**:

| Module         | Path              | Purpose                                                                                          |
| -------------- | ----------------- | ------------------------------------------------------------------------------------------------ |
| `solver`       | `solver.rs`       | Core `Solver` struct, fixed-point iteration, literal resolution (InferredInteger → concrete)     |
| `visit`        | `visit.rs`        | Per-variant visitor handlers for all 22 Value expression nodes, constraint propagation           |
| `bounds`       | `bounds.rs`       | Value range analysis for integer/refinement type checking with arithmetic bounds propagation     |
| `constraints`  | `constraints.rs`  | Constraint types (`TypeConstraint`, `NodeAction`), propagation helpers, operation classification |
| `substitution` | `substitution.rs` | Type substitution for generic instantiation (monomorphization)                                   |
| `monomorphize` | `monomorphize.rs` | Generic function/struct instantiation with deduplication caching                                 |
| `diagnosis`    | `diagnosis.rs`    | 15 comprehensive error variants with `ByteSpan` source locations                                 |

## The Solver Structure

```rust
pub(crate) struct Solver<'m> {
    constraints: HashMap<ValueId, HashSet<TypeConstraint>>,
    m: &'m mut SymbolTab,
    errors: HashSet<TypeErr>,
    function_return_type: Option<TypeId>,
    mono_counter: u32,
    mono_cache: HashMap<(usize, Vec<(u32, TypeId)>), FunctionId>,
    struct_mono_cache: HashMap<(usize, Vec<(u32, TypeId)>), StructDefId>,
}
```

The `Solver` struct maintains per-expression type constraints, a mutable symbol table reference for registering monomorphized copies, accumulated error state (deduplicated via `HashSet`), the current function's return type for backward inference from return statements, a monotonically increasing counter for naming monomorphized functions, and deduplication caches for both function and struct monomorphization.

## Type Constraints

```rust
pub(crate) enum TypeConstraint {
    Equal(TypeId),
    Unify(TypeId),
    SubtypeOf(TypeId),
}
```

Currently, all constraints are equality constraints — each states that a value's type must equal a specific `TypeId`. The additional `Unify` and `SubtypeOf` variants are reserved for future variance and subtyping extensions. This simple constraint system is sufficient for Hindley-Milner inference because all type relationships in the base system are equality-based.

## Bidirectional Constraint Propagation

The solver propagates constraints in **both directions** through the expression tree, maximizing inference power:

### Backward Inference (Parent → Child)

- **Binary operations**: If `a + b` must be `i32`, then both `a` and `b` are constrained to `i32`.
- **Unary operations**: Parent constraints propagate to the operand.
- **List literals**: If `[a, b, c]` is assigned to `[i32; 3]`, all elements are constrained to `i32`.
- **Function calls**: Parameter types from the callee signature are propagated to each argument.
- **Return statements**: The return value is constrained to match the function's declared return type.
- **Local variables**: The initializer is constrained to match the declared type (`let x: i32 = val` constrains `val` to `i32`).
- **Assignments**: The RHS is constrained to match the LHS type.
- **Cast expressions**: The cast source is constrained to match the target type.

### Forward Inference (Child → Parent)

- **Binary operations**: If both operands have the same concrete type, that type propagates to the result.
- **Comparison operations**: Automatically produce `Bool` type.
- **List literals**: If one element has a concrete type (e.g., `1_i32`), that type propagates to inferred siblings.
- **Local variables**: If type is inferred (`let x = ...`), the initializer's type becomes the variable's type.
- **Struct fields**: Field types from the struct definition constrain the field values.

### Branch Type Unification (If/Else)

Both branches of an if/else expression are checked for type compatibility. Mismatched concrete types produce a `MismatchedBranchTypes` error unless one branch diverges (`Type::Never`).

## Fixed-Point Iteration

The solver uses a fixed-point loop for each function, repeatedly visiting all block elements until constraint accumulation reaches a steady state:

```rust
loop {
    let prev_len = self.constraints.len();
    let prev_mono_count = self.mono_counter;
    for element in body.iter_mut() {
        self.visit_block_element(element);
    }
    if self.constraints.len() == prev_len && self.mono_counter == prev_mono_count {
        break;
    }
}
```

This ensures:

- **Transitive constraint propagation**: Constraints flow through intermediate values.
- **Nested monomorphization detection**: Inner generics resolved before outer.
- **Inference variable resolution**: Literals constrained by their usage context.

## Monomorphization

When a generic function call or struct literal is detected, the solver:

1. Infers type arguments by matching actual argument/field types against parameter types containing `GenericParam`.
2. Clones the generic function/struct and applies type substitution.
3. Caches the monomorphized copy in `mono_cache` / `struct_mono_cache`.
4. Registers it in the symbol table for codegen to find.
5. Redirects the call site's callee to the monomorphized copy.

Both function and struct monomorphization are cached keyed by `(original_id, sorted_type_args)` to avoid redundant work.

## Refinement Type Checking

The solver tracks value bounds through arithmetic operations:

- Addition bounds are the sum of min and max.
- Subtraction bounds account for the range of both operands.
- Multiplication evaluates all four combinations of min/max products.
- Division accounts for sign changes and zero divisors.

When a result is assigned to a refinement type, the computed bounds are checked against the declared refinement range, producing `OperationResultOutOfRefinementBounds` errors if values could fall outside the valid range.

## Comprehensive Error Diagnostics

The solver reports 15 error variants, each with precise `ByteSpan` source locations:

| Code | Error                                  | Description                                                    |
| ---- | -------------------------------------- | -------------------------------------------------------------- |
| 0    | `IntegerLiteralOutOfRange`             | Value doesn't fit in target primitive (e.g. 256 into u8)       |
| 1    | `IntegerLiteralUnsatisfiable`          | Integer constrained to non-integer type                        |
| 2    | `FloatLiteralUnsatisfiable`            | Float constrained to non-float type                            |
| 3    | `IntegerLiteralOutOfRefinementBounds`  | Literal outside refinement type bounds                         |
| 4    | `OperationResultOutOfRefinementBounds` | Arithmetic result can't be guaranteed within refinement bounds |
| 5    | `MismatchedBranchTypes`                | If/else branches have incompatible types                       |
| 6    | `CannotInferTypeArgs`                  | Generic type arguments can't be inferred                       |
| 7    | `GenericArgCountMismatch`              | Wrong number of type arguments                                 |
| 8    | `AmbiguousType`                        | Variable/expression type can't be inferred                     |
| 9    | `MissingTypeAnnotation`                | Parameter needs explicit type annotation                       |
| 10   | `UnresolvedInferredType`               | Inference variable remained unresolved                         |
| 11   | `UnboundGenericParam`                  | Generic param couldn't be inferred from context                |
| 12   | `StructFieldTypeMismatch`              | Field value type doesn't match declaration                     |
| 13   | `ArgumentTypeMismatch`                 | Call argument type doesn't match parameter                     |
| 14   | `MethodNotFound`                       | Method not found on receiver type                              |

Errors are collected in a `HashSet<TypeErr>` to prevent duplicate reports of the same type mismatch.

## Key Design Patterns

### Pattern 1: Per-Variant Visitor Handlers

The original monolithic `visit_children` method (500+ lines matching on 50+ variants) has been decomposed into per-variant handler methods:

- `visit_struct_object` — Generic struct monomorphization or field type constraint
- `visit_enum_variant` — Variant payload type constraint
- `visit_binary` — Refinement bounds checking + bidirectional inference
- `visit_unary` — Bounds checking + constraint propagation
- `visit_index_access` — Element type inference + index type constraint
- `visit_list` — Element type propagation (forward + backward)
- `visit_if` — Condition type constraint + branch type unification
- `visit_call` — Generic function monomorphization + parameter constraints
- `visit_method_call` — Method resolution + generic instantiation
- `visit_return` — Return value constrained to function return type

### Pattern 2: Decoupled Helper Modules

Bounds analysis and constraint manipulation are factored into separate modules (`bounds.rs`, `constraints.rs`) with pure functions, making them testable independently of the solver's mutable state.

### Pattern 3: Deduplication Everywhere

Both errors (via `HashSet`) and monomorphization results (via `HashMap` cache) are deduplicated to avoid redundant work and duplicate diagnostics.

## Integration

The solver is invoked by the translation pipeline after HIR lowering:

```
HIR → [Solver] → Solved HIR → [Validator] → Validated HIR
```

Each function and global variable in the module is solved independently via `resolve_function` and `resolve_global`.
