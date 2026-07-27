# Trait Solving and Constraint Propagation

## Overview

The trait solver resolves type constraints and propagates type information through the expression graph. It is the engine behind Nitrate's Hindley-Milner type inference, monomorphization, and refinement type checking. The solver operates as part of the `nitrate_hir_solve` crate, serving as the core analysis pass that transforms unresolved HIR (with `Inferred` type variables and `GenericParam` references) into fully resolved, concrete type assignments ready for validation and code generation.

## Architecture

**Crate**: `nitrate_hir_solve`  
**Key types**: `Solver`, `TypeConstraint`, `NodeAction`, substitution machinery  
**Key files**: `solver.rs`, `substitution.rs`, `monomorphize.rs`, `diagnosis.rs`

## The Solver Structure

The `Solver` struct maintains per-expression type constraints, a mutable symbol table reference for registering monomorphized copies, accumulated error state, the current function's return type, a monotonically increasing counter for naming monomorphized functions, and a deduplication cache that prevents redundant generic instantiations.

## Type Constraints

```rust
pub(crate) enum TypeConstraint {
    Equal(TypeId),
}
```

Currently, all constraints are equality constraints — each states that a value's type must equal a specific `TypeId`. This simple constraint system is sufficient for Hindley-Milner inference because all type relationships in the base system are equality-based. Future extensions could add subtyping constraints for variance or trait bound constraints for more expressive dispatch.

## Constraint Propagation

The solver propagates constraints through the expression tree by visiting each node and extending child constraints based on parent constraints. For binary operations, arithmetic operands inherit the result type while comparison operands must match but produce `Bool`. For lists, concrete element types are propagated to inferred siblings. For calls, parameter types from the callee are propagated to arguments, enabling inference of argument types from the function signature.

## Fixed-Point Iteration

The solver uses a fixed-point loop for each function, repeatedly visiting all block elements until constraint accumulation reaches a steady state:

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

This ensures transitive constraint propagation (constraints flow through intermediate values), nested monomorphization detection (inner generics resolved before outer), and inference variable resolution (literals constrained by their usage context).

## Monomorphization

When a generic function call is detected, the solver infers type arguments by matching actual argument types against parameter types containing `GenericParam`, clones the generic function and applies type substitution, caches the monomorphized copy in `mono_cache` and registers it in the symbol table, and redirects the call site's callee to the monomorphized copy.

## Refinement Type Checking

The solver tracks value bounds through arithmetic operations: addition bounds are the sum of min and max; subtraction bounds account for the range of both operands; multiplication evaluates all four combinations of min/max products. When a result is assigned to a refinement type, the computed bounds are checked against the declared refinement range, producing errors if values could fall outside the valid range.

## Error Accumulation

Errors are stored in a `HashSet<TypeErr>` for deduplication, preventing multiple identical error reports for the same type mismatch.

## Integration

The solver is invoked by the translation pipeline after HIR lowering:

```
HIR → [Solver] → Solved HIR → [Validator] → Validated HIR
```
