# Reference Semantics, Borrowing, and Lifetimes

## Overview

Nitrate's reference system provides safe, checked pointers through a combination of lifetime tracking, borrow checking, and reference types. The system distinguishes between safe references (guaranteed valid, non-null) and raw pointers (unsafe, potentially invalid).

## Reference Types

The type system defines four pointer-like type categories:

### Safe References (`Type::Reference`)

```rust
Reference { lifetime: Lifetime, exclusive: bool, mutable: bool, to: TypeId }
```

Safe references are guaranteed to be valid, non-null, and properly aligned. They come in four flavors:

| exclusive | mutable | Semantics                                           |
| --------- | ------- | --------------------------------------------------- |
| false     | false   | `&T` — Shared, read-only reference                  |
| false     | true    | Shared mutable reference (requires synchronization) |
| true      | false   | `&uniq T` — Exclusive, read-only reference          |
| true      | true    | `&mut T` — Exclusive, mutable reference             |

### Slice References (`Type::SliceRef`)

```rust
SliceRef { lifetime: Lifetime, exclusive: bool, mutable: bool, element_type: TypeId }
```

Dynamically-sized views into contiguous sequences. At runtime, these are fat pointers (pointer + length).

### Raw Pointers (`Type::Pointer`)

```rust
Pointer { lifetime: Lifetime, exclusive: bool, mutable: bool, to: TypeId }
```

Unsafe pointers without lifetime guarantees. Can be null, dangling, or misaligned. Only usable in `unsafe` blocks.

### Slice Pointers (`Type::SlicePtr`)

```rust
SlicePtr { lifetime: Lifetime, exclusive: bool, mutable: bool, element_type: TypeId }
```

Raw fat pointers to dynamically-sized sequences.

## Lifetime System

```rust
pub enum Lifetime {
    Static,       // 'static — entire program duration
    Gc,           // Garbage-collected
    ThreadLocal,  // Per-thread lifetime
    TaskLocal,    // Per-task lifetime
    Inferred,     // To be inferred by the solver
}
```

Lifetimes track how long references are valid:

- **Static**: References to string literals, constant globals, etc.
- **Gc**: References managed by the garbage collector
- **ThreadLocal**: References valid only within one thread
- **TaskLocal**: References valid within an async task context
- **Inferred**: Placeholder resolved by the solver

## Borrow Expressions

The HIR represents borrows explicitly:

```rust
Value::Borrow { exclusive: bool, mutable: bool, place: ValueId }
```

- `&expr` → `Borrow { exclusive: false, mutable: false, place: expr }`
- `&mut expr` → `Borrow { exclusive: true, mutable: true, place: expr }`
- `&uniq expr` → `Borrow { exclusive: true, mutable: false, place: expr }`

## Deref Expressions

```rust
Value::Deref { place: ValueId }
```

The dereference operator `*expr` accesses the value pointed to by a reference or pointer.

## Borrow Checking

The solver performs basic borrow checking:

1. **Exclusive borrows** prevent any other access to the borrowed place
2. **Mutable borrows** require the borrowed place to be mutable
3. **Shared read-only borrows** can coexist with each other
4. **Assignment** to a borrowed place is restricted while a borrow is active

Returning a reference from a function requires that the reference's lifetime is not longer than any of the input references' lifetimes.

## Integration

References are created by the HIR lowerer when `&` operators are encountered. The solver propagates reference types through constraints. The codegen generates LLVM pointer operations for reference creation, dereference, and field access through references.
