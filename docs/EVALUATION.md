# Constant Evaluation / Interpreter Subsystem

## Overview and Purpose

The `nitrate_hir_evaluate` crate provides a **tree-walking interpreter** for the HIR that serves dual purposes:

1. **Compile-time constant evaluation** — computing the values of expressions that must be known at compile time, such as:
   - Global variable initializers
   - Struct field default values
   - Enum variant default values
   - Array type length expressions
   - Refinement type bounds (range expressions)

2. **Full interpreter-like evaluation** — executing function bodies at compile time, enabling `const fn`-style computation for array sizes, refinement bounds, and other compile-time contexts. This allows user-defined functions to participate in constant contexts.

The interpreter models all values directly — it does not lower to a bytecode or use a VM. It recursively walks the HIR value tree, evaluating each node in place.

## Architecture

**Crate**: `nitrate_hir_evaluate`  
**Key files**:

| File           | Purpose                                                                                        |
| -------------- | ---------------------------------------------------------------------------------------------- |
| `lib.rs`       | Public API exports                                                                             |
| `error.rs`     | `EvalError` type — all errors the interpreter can produce                                      |
| `memory.rs`    | `Memory` — abstract heap for pointer/reference operations                                      |
| `evaluator.rs` | `Evaluator` struct — core interpreter with frame stack, limits, and built-in function dispatch |
| `value.rs`     | Expression evaluation — recursive match on all `Value` variants                                |
| `builtins.rs`  | Default built-in function registry (`std::math::abs`, `std::mem::size_of`, etc.)               |

## Core Types

### `Evaluator<'log>`

The main interpreter struct. Owns:

- **`memory: Memory`** — abstract heap for all pointer/reference operations
- **`log: &'log CompilerLog`** — diagnostic log (for compatibility with built-in functions)
- **`ptr_size: PtrSize`** — target pointer size (32-bit or 64-bit)
- **`frames: Vec<Frame>`** — call stack, with each `Frame` holding local variable and parameter bindings
- **`loop_limit`, `call_depth_limit`, `memory_limit`** — safety limits to prevent infinite loops/recursion
- **`current_safety: BlockSafety`** — tracks whether the current context is `safe` or `unsafe`
- **`added_builtins: HashMap<NString, BuiltinFn>`** — user-registered built-in functions

### `EvalError`

Errors that can arise during evaluation. Control-flow (`break`, `continue`, `return`) uses error-based unwinding for efficiency.

| Variant                     | Description                                    |
| --------------------------- | ---------------------------------------------- |
| `Break { label }`           | `break` statement                              |
| `Continue { label }`        | `continue` statement                           |
| `Return(Value)`             | `return` statement with value                  |
| `DivisionByZero`            | Integer or float division by zero              |
| `ModuloByZero`              | Integer modulo by zero                         |
| `ShiftAmountError`          | Shift amount out of range                      |
| `TypeError`                 | Runtime type mismatch                          |
| `LoopLimitExceeded`         | Loop iteration limit (prevents infinite loops) |
| `CallDepthExceeded`         | Recursion depth limit                          |
| `MemoryLimitExceeded`       | Abstract heap allocation limit                 |
| `InvalidPointer`            | Dereference of unknown pointer                 |
| `OutOfBoundsAccess`         | Memory access outside allocation               |
| `MisalignedAccess`          | Memory access not aligned                      |
| `Unsupported(&'static str)` | Unimplemented operation                        |
| `UnsafeInSafeContext`       | Unsafe operation in safe context               |

### `Memory` (Abstract Heap)

All pointer and reference operations go through the abstract heap, never touching real process memory. This enables safe evaluation of both safe and unsafe code at compile time.

- Allocations are keyed by base address (`u64`), each with data (`Vec<u8>`), size, and mutability flag
- All pointer values are validated: base must be in the allocation map, offset+size must be within bounds
- Invalid pointers produce `EvalError` variants rather than crashes
- The heap starts allocating at `0x1000` and grows linearly

## Evaluation Capabilities

### Supported (Implemented)

| Category         | Operations                                                                                                                        |
| ---------------- | --------------------------------------------------------------------------------------------------------------------------------- | --- | -------------------- |
| **Literals**     | All primitive types (`Unit`, `Bool`, `I8`–`I128`, `U8`–`U128`, `F32`, `F64`, `USize`), `StringLit`, `BStringLit`                  |
| **Arithmetic**   | `+`, `-`, `*`, `/`, `%` (all integer and float types, checked division/modulo)                                                    |
| **Bitwise**      | `&`, `\`, `^`, `<<`, `>>`, `<<<` (rotate left), `>>>` (rotate right)                                                              |
| **Comparison**   | `==`, `!=`, `<`, `>`, `<=`, `>=`                                                                                                  |
| **Logical**      | `&&`, `                                                                                                                           |     | `(short-circuit),`!` |
| **Unary**        | `+` (identity), `-` (negation), `!` (bitwise/logical not)                                                                         |
| **Type casts**   | All numeric-to-numeric casts via `Cast` expression                                                                                |
| **Control flow** | `if`/`else`, `while`, `loop`, `break` (with/without label), `continue` (with/without label), `return`, blocks with safety context |
| **Compounds**    | `StructObject`, `EnumVariant`, `List`, `Tuple` construction                                                                       |
| **Access**       | `FieldAccess` on structs, `IndexAccess` on lists and tuples                                                                       |
| **Assignment**   | `Assign` to local variables                                                                                                       |
| **Functions**    | `Call` — look up `FunctionSymbol`, push frame, bind params, evaluate body                                                         |
| **Built-in**     | `std::math::abs`, `std::math::max`, `std::math::min`, `std::mem::size_of` (stub)                                                  |

### Not Yet Implemented (Returns `EvalError::Unsupported`)

| Category             | Notes                                                               |
| -------------------- | ------------------------------------------------------------------- |
| **MethodCall**       | Trait method dispatch requires full type resolution at eval time    |
| **Borrow/Deref**     | Memory model is in place; borrow/deref implementation is planned    |
| **Range**            | Range construction in const-eval contexts                           |
| **Global variables** | Cross-global evaluation (use `evaluate_global_initializer` instead) |

## Evaluation Algorithm

The core evaluation loop is a recursive tree walk:

```
evaluate_value(evaluator, value) -> Result<Value, EvalError>
  match value:
    Literal variants -> identity
    Binary -> eval_binary (short-circuit for &&, ||; Lit-ops for rest)
    Unary -> eval_unary
    Cast -> eval_cast (via CastBridge numeric conversion)
    If -> eval_if (evaluate condition, execute branch)
    While -> eval_while (loop with iteration limit)
    Loop -> eval_infinite_loop
    Break/Continue/Return -> emit EvalError for stack unwinding
    Block -> evaluate_block (safety context push/pop, iterate elements)
    StructObject/EnumVariant/List/Tuple -> evaluate fields, construct
    FieldAccess -> evaluate object, look up field by name
    IndexAccess -> evaluate collection + index, bounds-checked access
    Assign -> evaluate RHS, bind to place in current frame
    Call -> evaluate callee to FunctionSymbol, evaluate args, evaluate_function
    LocalVariableSymbol -> lookup_binding in frame stack
    ParameterSymbol -> lookup_binding in frame stack
    MethodCall/Borrow/Deref/Range/GlobalVariable -> Unsupported (for now)
```

## Public API

```rust
// Create an evaluator
let mut eval = Evaluator::new(log, PtrSize::U64);

// Evaluate a single expression
let result: Value = eval.evaluate(&some_value)?;

// Evaluate and destructure to a Lit (for compile-time contexts)
let lit: Lit = eval.evaluate_to_literal(&some_value)?;

// Evaluate a function body with arguments
let result: Value = eval.evaluate_function(func_id, &args)?;

// Evaluate a block
let result: Value = eval.evaluate_block(&block)?;

// Evaluate a global variable initializer
let lit: Lit = eval.evaluate_global_initializer(&global_var)?;

// Register a custom built-in function
eval.add_builtin_function("my::builtin".into(), my_builtin_fn);
```

## Integration Points

The evaluator is used in the pipeline at the following points:

1. **`hir_from_tree` (AST → HIR lowering)** — `Evaluator::new(log, ptr_size).evaluate_to_literal(...)` is called for:
   - Refinement type bound expressions (`lower_refinement_bound` in `ty.rs`)
   - Array type length expressions (`lower_array_type` in `ty.rs`)

2. **Future: `hir_validate`** — will use the evaluator for global variable initializer validation

3. **Future: pipeline integration** — for `const fn` evaluation at compile time

## Safety Limits

The interpreter enforces safety limits to prevent denial-of-service from malicious or buggy code:

| Limit             | Default   | Error                 |
| ----------------- | --------- | --------------------- |
| Loop iterations   | 1,000,000 | `LoopLimitExceeded`   |
| Call depth        | 1000      | `CallDepthExceeded`   |
| Memory allocation | 1 MiB     | `MemoryLimitExceeded` |

All limits are configurable via public fields on `Evaluator`.

## Design Decisions

1. **Tree-walking, not bytecode** — The interpreter walks the HIR directly. This is simpler, more debuggable, and sufficient for compile-time evaluation where performance is bounded by compilation time, not runtime throughput.

2. **Error-based control flow** — `break`, `continue`, and `return` are implemented as `Result::Err` variants rather than explicit state machines. This keeps the evaluator's recursive structure simple and readable.

3. **Abstract memory, not real memory** — All memory operations go through the `Memory` struct, which validates all pointer accesses. This prevents the compiler process from crashing or exhibiting undefined behavior when evaluating code with pointers.

4. **Built-in functions via static registry** — Built-in functions are stored in a `LazyLock<HashMap>` and merged with user-registered builtins. This avoids depending on the entire standard library while still supporting common operations.

5. **No extra checking** — The evaluator assumes the HIR has already been validated by `hir_validate`. It does not perform type checking or borrow checking; it simply evaluates what it's given.
