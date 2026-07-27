# Hindley-Milner Type Inference and Monomorphization

## Theoretical Foundation

The Hindley-Milner (HM) type system is the foundation of Nitrate's type inference. Originally developed by Roger Hindley and later extended by Robin Milner, HM provides two fundamental guarantees: **principal types** (every well-typed expression has a unique most general type) and **complete inference** (types can be inferred without explicit annotations, though annotations are supported for documentation and constraint). The classic HM system also provides **let-polymorphism**, where `let` bindings can be polymorphic — a property that Nitrate's monomorphization approach handles through code cloning rather than runtime polymorphism.

Nitrate's implementation extends classical HM in several significant ways. Instead of the classic unification algorithm with destructive substitution, Nitrate uses **constraint accumulation with fixed-point solving** — the solver walks the expression tree repeatedly, adding equality constraints until no new constraints are discovered. This approach enables **refinement type propagation**, where value bounds are tracked through arithmetic operations and checked against declared refinement ranges. The solver also performs **monomorphization of generic instantiations** through code cloning and type substitution, **method call resolution** integrated with the trait system, and **struct generic inference** where type arguments are inferred from struct field values.

## Architecture

**Crate**: `nitrate_hir_solve`  
**Key types**: `Solver`, `TypeConstraint`, `Substitution`, `NodeAction`  
**Key files**: `solver.rs` (core solving algorithm), `substitution.rs` (type substitution), `monomorphize.rs` (generic instantiation), `diagnosis.rs` (error types)  
**Entry points**: `resolve_function()`, `resolve_global()`

## The Solver Struct

The `Solver` struct is the core of the type inference engine:

```rust
pub(crate) struct Solver<'m> {
    // Per-expression constraint map: each ValueId maps to its expected types
    constraints: HashMap<ValueId, HashSet<TypeConstraint>>,

    // Mutable symbol table reference (for registering monomorphized functions)
    m: &'m mut SymbolTab,

    // Accumulated type errors (HashSet for deduplication)
    errors: HashSet<TypeErr>,

    // Return type of the current function being solved
    function_return_type: Option<TypeId>,

    // Monotonically increasing counter for naming monomorphized copies
    mono_counter: u32,

    // Deduplication cache: (generic_func_store_index, sorted_type_args) -> monomorphized_id
    mono_cache: HashMap<(usize, Vec<(u32, TypeId)>), FunctionId>,
}
```

### Key Fields Explained

- **`constraints`**: A `HashMap<ValueId, HashSet<TypeConstraint>>` mapping each expression to its expected types. The `HashSet` ensures deduplication — multiple sources may assert the same constraint for the same expression. Each constraint is an equality: the expression must equal a specific `TypeId`. This map grows during the fixed-point iteration as new constraints are discovered.

- **`m`**: Mutable reference to the `SymbolTab`. The solver registers monomorphized functions and structs here, making them visible to downstream passes including the LLVM codegen phase, which iterates the symbol table to find all functions to compile.

- **`mono_cache`**: The cache key is a tuple of `(original_function_store_index, sorted_type_args)`. The `store_index` is obtained from `FunctionId::as_usize()`, which returns the underlying `NonZeroU32` as a `usize`. The type arguments are sorted by parameter index to ensure canonical cache keys regardless of inference order — the solver might discover type arguments in any order, but the cache key is always normalized.

## Constraint Types

```rust
pub(crate) enum TypeConstraint {
    Equal(TypeId),  // This value's type must equal TypeId
}
```

Currently, all constraints are equality constraints. Future extensions could add subtyping (`SubtypeOf`) for safe variance in parameter types, or trait bounds (`Implies(TraitId)`) for constraint-based trait resolution rather than monomorphization-based dispatch.

## Node Action Types

```rust
pub(crate) enum NodeAction {
    NoChange,       // Keep the current value, continue visiting children
    Replace(Value), // Replace this value with a new one (e.g., InferredInteger -> I32)
}
```

## The Solving Algorithm

### Entry Points

```rust
pub fn resolve_function(function: &mut Function, m: &mut SymbolTab, log: &CompilerLog) -> Result<(), ()> {
    let mut hm = Solver::new(m);
    hm.solve_function(function, log)
}

pub fn resolve_global(global: &mut GlobalVariable, m: &mut SymbolTab, log: &CompilerLog) -> Result<(), ()> {
    let mut hm = Solver::new(m);
    hm.solve_global_variable(global, log)
}
```

Each function is solved independently — the solver is instantiated per-function, ensuring that type variables from different functions don't interfere. Global variables are solved through a simpler path without return type tracking.

### Constraint Accumulation

The `visit()` method walks each expression and dispatches:

```rust
fn visit(&mut self, e: &ValueId) {
    let action = {
        let current_value = e.borrow();
        self.determine_action(&current_value, e)
    };
    match action {
        NodeAction::Replace(new_value) => {
            e.replace(new_value);  // In-place mutation via RefCell
        }
        NodeAction::NoChange => self.visit_children(e),
    }
}
```

The `determine_action()` method checks if the value is an unresolved literal that can be resolved immediately based on existing constraints:

```rust
fn determine_action(&mut self, value: &Value, id: &ValueId) -> NodeAction {
    match value {
        Value::InferredInteger(integer) => self.solve_inferred_integer(id, **integer),
        Value::InferredFloat(float) => self.solve_inferred_float(id, *float),
        _ => NodeAction::NoChange,  // All other values need child visiting
    }
}
```

### Binary Operations and Constraint Propagation

When visiting binary operations, the solver propagates type constraints from the parent expression to its children. For arithmetic operations (`Add`, `Sub`, `Mul`, `Div`, `Mod`), both operands must have the same type as the result. For comparison operations (`Lt`, `Gt`, `Eq`, etc.), operands must have the same type but the result is `Bool`. The solver also handles refinement type bounds: before propagating constraints, it checks if the parent's type constraint includes refinement bounds, and if so, extracts the bounds to propagate only the base type.

### Generic Call Detection and Monomorphization

When visiting a `Call` value, the solver checks whether the callee is a generic function (has non-empty `generics` field). If so, it performs four steps:

1. **Infer type arguments**: `infer_generic_args_from_call()` matches actual argument types against parameter types, recording mappings from `GenericParam` indices to concrete types
2. **Create monomorphized copy**: `monomorphize_function()` clones the entire function definition and applies type substitution
3. **Register in symbol table**: The copy is registered so codegen can find it
4. **Redirect the call site**: The callee reference is updated to point to the monomorphized copy

### Fixed-Point Iteration

```rust
fn solve_function(&mut self, function: &mut Function, log: &CompilerLog) -> Result<(), ()> {
    if let Some(body) = &mut function.body {
        self.function_return_type = Some(function.return_type);
        loop {
            let prev_len = self.constraints.len();
            for element in body.iter_mut() {
                self.visit_block_element(element);
            }
            if self.constraints.len() == prev_len {
                break;  // Fixed point reached
            }
        }
    }
    // Report errors
    for error in &self.errors { log.report(error); }
    if self.errors.is_empty() { Ok(()) } else { Err(()) }
}
```

## Design Rationale

### Why Constraint-Based Instead of Classical Unification?

Traditional HM uses destructive unification with type variable substitution. Constraints provide better error messages (the constraint history reveals why a type was expected), enable refinement type bounds tracking (which requires range constraints, not just equality), support monomorphization ordering (the solver needs to detect when arguments have concrete types before cloning), and work with the mutable store pattern where values are in `RefCell`s.

### Why Per-Function Solver Instead of Global?

Function bodies don't share type variables — each function is an independent inference problem. Per-function solving enables modularity, monomorphization independence (generic function bodies are cloned so their type variables are independent), potential parallelism (functions could be solved concurrently), and incremental compilation (only changed functions need re-solving).

### Why Fixed-Point Loop?

The fixed-point loop handles mutual dependencies (`let x = y; let y = 42;` where x depends on y, resolved in the second pass), nested generics (`map(list, fn(x) -> identity(x))` where first pass monomorphizes `identity<i32>` and second pass monomorphizes `map<List<i32>, fn(i32) -> i32>`), and default type inference where seeing that a parameter is unconstrained may require visiting all uses.

### Why Separate Monomorphization Cache?

Without caching, each call site of `identity::<i32>(x)` produces a separate monomorphized copy. The cache reduces code bloat (one copy per unique type argument combination), prevents infinite recursion in recursive generic instantiation, and speeds compilation by avoiding redundant cloning and substitution.
