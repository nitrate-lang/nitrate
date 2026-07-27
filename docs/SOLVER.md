# Trait Solving and Constraint Propagation

## Overview

The trait solver resolves trait constraints and propagates type constraints through the expression graph. It is the engine behind Nitrate's Hindley-Milner type inference, monomorphization, and refinement type checking. The solver operates as part of the `nitrate_hir_solve` crate.

## Architecture

**Crate**: `nitrate_hir_solve`  
**Key types**: `Solver`, `TypeConstraint`, `NodeAction`, substitution machinery  
**Key files**: `solver.rs`, `substitution.rs`, `monomorphize.rs`, `diagnosis.rs`

## The Solver Structure

```rust
pub(crate) struct Solver<'m> {
    // Per-value-id type constraints
    constraints: HashMap<ValueId, HashSet<TypeConstraint>>,
    // Mutable reference to symbol table
    m: &'m mut SymbolTab,
    // Accumulated type errors
    errors: HashSet<TypeErr>,
    // The return type of the current function
    function_return_type: Option<TypeId>,
    // Counter for naming monomorphized copies
    mono_counter: u32,
    // Dedup cache: (generic_function_index, sorted_type_args) → monomorphized FunctionId
    mono_cache: HashMap<(usize, Vec<(u32, TypeId)>), FunctionId>,
}
```

## Type Constraints

```rust
pub(crate) enum TypeConstraint {
    Equal(TypeId),
}
```

Currently, all constraints are equality constraints. Each expresses that a value's type must equal a specific `TypeId`.

## Node Actions

```rust
pub(crate) enum NodeAction {
    NoChange,              // Keep the current value as-is
    Replace(Value),        // Replace with a new value (e.g., resolved literal)
}
```

## Constraint Propagation

The solver propagates constraints through the expression tree:

### Binary Operations

```rust
Binary { left, op, right }:
    // Propagate parent type constraints to children
    // For arithmetic ops: both operands have the same type as the result
    // For comparison ops: operands have the same type, result is Bool
    // For Refine types: propagate the base type
```

### Lists/Arrays

```rust
List { elements }:
    // If one element has a concrete type, propagate to inferred siblings
    // If parent constraint is Array(T, N) or SliceRef(T), propagate T to elements
```

### Conditionals

```rust
If { condition, true_branch, false_branch }:
    // Condition must be Bool
    // Both branches must have the same type (unified)
```

### Calls

```rust
Call { callee, args }:
    // For generic callees: infer type args, monomorphize
    // For non-generic callees: propagate parameter types to arguments
```

## Fixed-Point Iteration

The solver uses a fixed-point loop for each function:

```rust
loop {
    let prev_len = self.constraints.len();
    for element in body.iter_mut() {
        self.visit_block_element(element);
    }
    if self.constraints.len() == prev_len {
        break;
    }
}
```

This ensures:

1. Transitive constraint propagation
2. Nested monomorphization detection
3. Inferred type variable resolution

## Monomorphization

When a generic function call is detected:

1. **Infer type arguments**: Match actual argument types against parameter types containing `GenericParam`
2. **Clone and substitute**: Clone the generic function, replace all `GenericParam` with concrete types
3. **Cache and register**: Store in `mono_cache` and register in symbol table
4. **Redirect**: Replace the call site's callee with the monomorphized copy

## Refinement Type Checking

The solver tracks value bounds through arithmetic operations:

- For `BinaryOp::Add`: result bounds = [a_min + b_min, a_max + b_max]
- For `BinaryOp::Sub`: result bounds = [a_min - b_max, a_max - b_min]
- For `BinaryOp::Mul`: result bounds = [min(a*b), max(a*b)] across all combinations
- etc.

When a result is assigned to a refinement type, the computed bounds are checked against the refinement range.

## Error Accumulation

Errors are stored in a `HashSet<TypeErr>` for deduplication:

- `IntegerLiteralUnsatisfiable`
- `IntegerLiteralOutsizeRange`
- `FloatLiteralUnsatisfiable`
- `IntegerLiteralOutOfRefinementBounds`
- `OperationResultOutOfRefinementBounds`

## Integration

The solver is invoked by the translation pipeline after HIR lowering:

```
HIR → [Solver] → Solved HIR → [Validator] → Validated HIR
```
