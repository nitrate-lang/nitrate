# Constant Evaluation Subsystem

## Overview and Purpose

Constant evaluation (const eval) computes the values of compile-time constant expressions. When the compiler encounters expressions that can be evaluated entirely at compile time — such as initializers for global variables, struct field default values, enum variant defaults, and static constant declarations — it invokes the constant evaluator to compute the concrete `Lit` result. This serves several critical purposes:

- **Global variable initialization**: Globals must have concrete initial values embedded in the compiled binary
- **Default values**: Struct fields and function parameters with defaults need compile-time computation
- **Optimization**: Constant sub-expressions can be replaced with their computed results, enabling further optimizations

## Architecture

**Crate**: `nitrate_hir_evaluate`  
**Key files**: `lib.rs` (public API), `eval.rs` (core evaluation dispatch), `expr.rs` (expression-specific logic)

## Evaluation Capabilities

The evaluator handles: literal identity (literals evaluate to themselves), arithmetic operations (`+`, `-`, `*`, `/`, `%`), bitwise operations (`&`, `|`, `^`, `<<`, `>>`), comparison operations (`==`, `!=`, `<`, `>`, `<=`, `>=`), logical operations (`&&`, `||`, `!`), type casts between compatible numeric types, and struct/enum construction from literal field values.

## Evaluation Algorithm

The core function recursively walks the HIR value tree:

```rust
pub fn evaluate(value: &Value) -> Option<Lit> {
    match value {
        Value::Unit => Some(Lit::Unit),
        Value::Bool(b) => Some(Lit::Bool(*b)),
        Value::I8(v) => Some(Lit::I8(*v)),
        // ... all literal variants
        Value::Binary { left, op, right } => {
            let l = evaluate(&left.borrow())?;
            let r = evaluate(&right.borrow())?;
            eval_binary_op(op, &l, &r)
        }
        // ... unary, cast, other recursive cases
        _ => None,  // Non-evaluable expressions
    }
}
```

The function returns `Option<Lit>`: `Some(lit)` if fully evaluable, `None` if the expression contains non-evaluable components (function calls, control flow, memory operations).

## Current Limitations

Function calls are not evaluable (even `const`-qualified ones), control flow expressions cannot be evaluated, memory operations (borrows, dereferences) are not supported, string operations are not evaluated at compile time, and no cross-function evaluation is performed. The evaluator currently only handles simple constant expressions — arithmetic on literals, casts, and struct/enum construction from literal fields.
