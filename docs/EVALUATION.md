# Constant Evaluation Subsystem

## Overview and Purpose

Constant evaluation (const eval) is the process of computing the values of compile-time constant expressions. When the compiler encounters expressions that can be evaluated entirely at compile time — such as initializers for global variables, struct field default values, enum variant defaults, and static constant declarations — it invokes the constant evaluator to compute the concrete result. This serves several critical purposes in the compiler pipeline:

First, **global variable initialization**: Global variables must have concrete initial values that can be embedded in the compiled binary. The constant evaluator computes these initial values from the HIR expression graph, producing concrete `Lit` values that the LLVM codegen can emit as global initializers.

Second, **default values**: Struct fields and function parameters can have default values specified in source code. The constant evaluator computes these defaults, ensuring they are valid constant expressions and producing the concrete values that will be used when no explicit value is provided.

Third, **optimization**: When the solver or optimizer encounters expressions that are entirely composed of constant sub-expressions, constant evaluation can replace the entire expression tree with its concrete result, enabling further optimizations through constant propagation.

## Architecture

**Crate**: `nitrate_hir_evaluate`  
**Key types**: Evaluation functions  
**Key files**: `src/translation/src/hir_evaluate/src/lib.rs` (public API), `src/translation/src/hir_evaluate/src/eval.rs` (core evaluation dispatch), `src/translation/src/hir_evaluate/src/expr.rs` (expression-specific evaluation logic)

## Evaluation Capabilities

The constant evaluator can handle the following categories of expressions:

### Literal Identity

Literals evaluate to themselves: `Value::I32(42)` → `Lit::I32(42)`, `Value::Bool(true)` → `Lit::Bool(true)`, etc. This is the base case for all recursive evaluation.

### Arithmetic Operations

Basic integer arithmetic: addition (`+`), subtraction (`-`), multiplication (`*`), division (`/`), and remainder (`%`). The evaluator performs the operation using the `Lit` values' underlying integer types, checking for overflow and division by zero.

### Bitwise Operations

Bitwise AND (`&`), OR (`|`), XOR (`^`), left shift (`<<`), and right shift (`>>`). These operations are performed on the integer representations of the `Lit` values.

### Comparison Operations

Equality (`==`), inequality (`!=`), less than (`<`), greater than (`>`), less than or equal (`<=`), and greater than or equal (`>=`). Comparisons produce `Lit::Bool` results.

### Logical Operations

Logical AND (`&&`), logical OR (`||`), and logical NOT (`!`). These operate on `Lit::Bool` values and produce `Lit::Bool` results.

### Type Casts

Converting between compatible numeric types: e.g., casting `Lit::I32(42)` to `Lit::U8(42)`, or casting `Lit::F64(3.14)` to `Lit::F32(3.140000104904175)` (with potential precision loss).

### Struct and Enum Construction

Constructing struct values from their field values and enum variants from their payload values. The evaluator recursively evaluates each field value, producing the final composite value.

### Array and List Literals

Computing element values for array and tuple literals, ensuring all elements are evaluable.

## Evaluation Algorithm

The core evaluation function works by recursively walking the HIR value tree:

```rust
pub fn evaluate(value: &Value) -> Option<Lit> {
    match value {
        // Base cases: literals evaluate to themselves
        Value::Unit => Some(Lit::Unit),
        Value::Bool(b) => Some(Lit::Bool(*b)),
        Value::I8(v) => Some(Lit::I8(*v)),
        Value::I16(v) => Some(Lit::I16(*v)),
        Value::I32(v) => Some(Lit::I32(*v)),
        Value::I64(v) => Some(Lit::I64(*v)),
        Value::I128(v) => Some(Lit::I128(**v)),
        Value::U8(v) => Some(Lit::U8(*v)),
        Value::U16(v) => Some(Lit::U16(*v)),
        Value::U32(v) => Some(Lit::U32(*v)),
        Value::U64(v) => Some(Lit::U64(*v)),
        Value::U128(v) => Some(Lit::U128(**v)),
        Value::F32(v) => Some(Lit::F32(*v)),
        Value::F64(v) => Some(Lit::F64(*v)),
        Value::USize(bits, v) => Some(Lit::USize(*bits, *v)),

        // Recursive cases: evaluate children, then combine
        Value::Binary { left, op, right } => {
            let l = evaluate(&left.borrow())?;  // Recursively evaluate left
            let r = evaluate(&right.borrow())?; // Recursively evaluate right
            eval_binary_op(op, &l, &r)          // Combine via the operation
        }

        Value::Unary { op, operand } => {
            let val = evaluate(&operand.borrow())?;
            eval_unary_op(op, &val)
        }

        Value::Cast { value, target_type } => {
            let val = evaluate(&value.borrow())?;
            eval_cast(&val, target_type)
        }

        // Non-evaluable expressions: return None
        _ => None,
    }
}
```

The function returns `Option<Lit>`:

- `Some(lit)` if the expression is fully evaluable and the computation succeeded
- `None` if the expression contains non-evaluable components (function calls, control flow, etc.) or if the evaluation failed (overflow, division by zero, etc.)

## Integration Points

The constant evaluator is invoked at several points in the compiler pipeline:

### 1. During HIR Lowering

When the HIR lowerer processes struct field definitions and function parameters that have default values, it uses the constant evaluator to verify that the defaults are valid constant expressions and to store the computed `Lit` values.

### 2. During Type Solving

The solver may encounter constant sub-expressions during type inference. When it detects that an expression is fully composed of constants, it can use the evaluator to compute the concrete value, potentially resolving type constraints that depend on the value (e.g., array lengths, refinement type bounds).

### 3. During LLVM Code Generation

The LLVM codegen uses constant evaluation to compute initial values for global variables. When the codegen encounters a `GlobalVariable` with an initializer expression, it evaluates the expression to produce a `Lit` value, then translates that `Lit` to an LLVM constant for the global's initializer.

### 4. Global Constructor Functions

For global variables whose initializers are too complex for constant evaluation (e.g., involving function calls), the codegen falls back to generating constructor functions. The constant evaluator is tried first; if it returns `None`, the constructor approach is used instead.

## Current Limitations

The constant evaluator has several important limitations in the current implementation:

- **Function calls are not evaluable**: Even `const`-qualified function calls are not evaluated. Extending the evaluator to support const function evaluation is a planned enhancement.
- **Control flow is not evaluable**: Expressions involving `if`, `while`, `loop`, `match`, `return`, `break`, or `continue` cannot be evaluated. Only simple expression trees are supported.
- **Memory operations are not evaluable**: Borrows (`&expr`), dereferences (`*expr`), and field/index access through references are not supported.
- **String operations are not evaluable**: String concatenation, slicing, and other string operations are not evaluated at compile time.
- **No cross-function evaluation**: The evaluator works within a single expression tree; it cannot follow function calls to evaluate them.

These limitations mean that the constant evaluator is currently only suitable for simple constant expressions — arithmetic on literals, casts, and struct/enum construction from literal fields. Full compile-time function evaluation is a significant feature that would require expanding the evaluator to handle function call resolution, control flow, and potentially a mini-interpreter for Nitrate expressions.
