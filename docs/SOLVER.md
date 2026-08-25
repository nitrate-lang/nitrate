# Constraint Solver and Type Inference Engine

## Overview

The constraint solver resolves type constraints and propagates type information through the expression graph. It is the engine behind Nitrate's bidirectional Hindley-Milner type inference, monomorphization, and refinement type checking. The solver operates as part of the `nitrate_hir_solve` crate, serving as the core analysis pass that transforms unresolved HIR (with `Inferred` type variables and `GenericParam` references) into fully resolved, concrete type assignments ready for validation and code generation.

## Architecture

**Crate**: `nitrate_hir_solve`  
**Key types**: `Solver`, `TypeConstraint`, `NodeAction`, `Substitution`  
**Modules**:

| Module         | Path              | Purpose                                                                                          |
| -------------- | ----------------- | ------------------------------------------------------------------------------------------------ |
| `solver`       | `solver.rs`       | Core `Solver` struct, fixed-point iteration with worklist, literal resolution, `MonoCacheKey`    |
| `visit`        | `visit.rs`        | Per-variant visitor handlers for all Value expression nodes, constraint propagation              |
| `bounds`       | `bounds.rs`       | Value range analysis for integer/refinement type checking with arithmetic bounds propagation     |
| `constraints`  | `constraints.rs`  | Constraint types (`TypeConstraint`, `NodeAction`), propagation helpers, operation classification |
| `substitution` | `substitution.rs` | Type substitution for generic instantiation (monomorphization)                                   |
| `monomorphize` | `monomorphize.rs` | Generic function/struct instantiation with deduplication caching                                 |
| `diagnosis`    | `diagnosis.rs`    | 16 comprehensive error variants with `ByteSpan` source locations                                 |

## The Solver Structure

```rust
pub(crate) struct Solver<'m> {
    constraints: HashMap<ValueId, HashSet<TypeConstraint>>,
    m: &'m mut SymbolTab,
    errors: HashSet<TypeErr>,
    function_return_type: Option<TypeId>,
    mono_counter: u32,
    mono_cache: HashMap<MonoCacheKey, FunctionId>,
    struct_mono_cache: HashMap<MonoCacheKey, StructDefId>,
    worklist: HashSet<ValueId>,
}
```

The `Solver` struct maintains per-expression type constraints, a mutable symbol table reference for registering monomorphized copies, accumulated error state (deduplicated via `HashSet`), the current function's return type for backward inference from return statements, a monotonically increasing counter for naming monomorphized functions, deduplication caches for both function and struct monomorphization (keyed by the compact `MonoCacheKey` hash), and a worklist for efficient iterative constraint propagation.

## MonoCacheKey

```rust
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
struct MonoCacheKey(u64);
```

A compact cache key for monomorphization that avoids heap allocation. Instead of `Vec<(u32, TypeId)>` (which allocates and sorts on every lookup), it hashes the (index, type_id) pairs directly into a single u64 via `std::hash::DefaultHasher`, with type IDs converted to their underlying usize value for hashing. This eliminates allocation overhead for every cache lookup.

## Type Constraints

```rust
pub(crate) enum TypeConstraint {
    Equal(TypeId),
}
```

Currently, all constraints are equality constraints — each states that a value's type must equal a specific `TypeId`. This simple constraint system is sufficient for Hindley-Milner inference because all type relationships in the base system are equality-based.

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

## Worklist-Based Fixed-Point Iteration

The solver uses a worklist-based fixed-point loop for each function, visiting only values that have pending changes:

```rust
self.add_all_elements_to_worklist(body);

loop {
    let pending: Vec<ValueId> = self.worklist.drain().collect();
    if pending.is_empty() {
        break;
    }

    let prev_len = self.constraints.len();
    let prev_mono_count = self.mono_counter;

    for value_id in &pending {
        self.visit(value_id);
    }

    if self.constraints.len() == prev_len && self.mono_counter == prev_mono_count {
        break;
    }
}
```

When a constraint is added to a value, that value is automatically added to the worklist via `add_constraint`/`add_constraints`, which call `add_to_worklist`. This ensures:

- **Transitive constraint propagation**: Constraints flow through intermediate values.
- **Nested monomorphization detection**: Inner generics resolved before outer.
- **Efficient iteration**: Only changed values are re-visited, rather than the entire expression tree.
- **Inference variable resolution**: Literals constrained by their usage context.

## Monomorphization

When a generic function call or struct literal is detected, the solver:

1. Infers type arguments by matching actual argument/field types against parameter types containing `GenericParam`.
2. Clones the generic function/struct and applies type substitution.
3. Caches the monomorphized copy in `mono_cache` / `struct_mono_cache` using compact `MonoCacheKey` hashes.
4. Registers it in the symbol table for codegen to find.
5. Redirects the call site's callee to the monomorphized copy.

Both function and struct monomorphization are cached keyed by hashed `(original_id, sorted_type_args)` to avoid redundant work, with no heap allocation per lookup.

### Improved Generic Parameter Detection

The `infer_generic_args_from_struct_fields` method uses a single-pass approach to detect which generic parameters appear in field types, avoiding the previous O(g × f) nested loop. An internal `GenericFieldInfo` struct tracks whether each generic parameter appears across all fields in a single iteration.

### Deduplicated Struct Object Visiting

The `visit_struct_object` handler has been refactored to extract a shared `apply_struct_field_constraints` helper, eliminating ~50 lines of duplicated code between the generic and non-generic code paths.

## Refinement Type Checking

The solver tracks value bounds through arithmetic operations:

- Addition bounds are the sum of min and max.
- Subtraction bounds account for the range of both operands.
- Multiplication evaluates all four combinations of min/max products.
- Division accounts for sign changes and zero divisors.

When a result is assigned to a refinement type, the computed bounds are checked against the declared refinement range, producing `OperationResultOutOfRefinementBounds` errors if values could fall outside the valid range.

## Literal Type Resolution

### Integer Literals

When resolving an `InferredInteger`, the solver now processes **all** constraints (not just the first one). It performs a two-phase approach:

1. **Validation phase**: Check all constraints for refinement bounds and non-integer type conflicts. Errors are collected for all violating constraints.
2. **Resolution phase**: Find the best common type across all integer constraints using `find_common_integer_type`, which selects the widest compatible integer type, preferring signed over unsigned (like Rust).

If no constraints are present, the literal retains its inferred state until the finalization pass, where it defaults to the narrowest fitting type (i32 → i64 → u64 → u128 cascade).

### Float Literals

Float literals follow a similar pattern, processing all constraints to find the widest float type (F64 preferred over F32), and reporting `FloatLiteralUnsatisfiable` errors for non-float constraints.

## Comprehensive Error Diagnostics

The solver reports 16 error variants, each with precise `ByteSpan` source locations:

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
| 15   | `InferredLiteralAmbiguous`             | Literal type ambiguous, annotation required                    |

Errors are collected in a `HashSet<TypeErr>` to prevent duplicate reports of the same type mismatch.

## Key Design Patterns

### Pattern 1: Per-Variant Visitor Handlers

The original monolithic `visit_children` method (500+ lines matching on 50+ variants) has been decomposed into per-variant handler methods:

- `visit_struct_object` — Generic struct monomorphization via `or_else` chain + shared field constraint helper
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

Both errors (via `HashSet`) and monomorphization results (via compact-keyed `HashMap` cache) are deduplicated to avoid redundant work and duplicate diagnostics.

### Pattern 4: Worklist-Based Iteration

The `worklist: HashSet<ValueId>` field enables efficient fixed-point iteration by only re-visiting values that have had new constraints added. The `add_constraint` and `add_constraints` helpers automatically add to the worklist.

### Pattern 5: Compact Cache Keys

`MonoCacheKey(u64)` replaces heap-allocated `Vec<(u32, TypeId)>` keys with a single u64 hash, computed by hashing the sorted (param_index, type_id) pairs. This eliminates allocation overhead on every monomorphization cache lookup.

## Integration

The solver is invoked by the translation pipeline after HIR lowering:

```
HIR → [Solver] → Solved HIR → [Validator] → Validated HIR
```

Each function and global variable in the module is solved independently via `resolve_function` and `resolve_global`.
