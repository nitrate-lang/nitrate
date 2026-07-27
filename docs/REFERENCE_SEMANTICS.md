# Reference Semantics, Borrowing, and Lifetimes

## Overview

Nitrate's reference system provides safe, checked pointers through a combination of lifetime tracking, borrow checking, and reference types. The system distinguishes between safe references (guaranteed valid, non-null) and raw pointers (unsafe, potentially invalid). This design provides memory safety guarantees for safe code while allowing low-level pointer manipulation in `unsafe` contexts.

## Reference Types

The type system defines four pointer-like type categories, each with specific safety guarantees and usage patterns.

**Safe References** (`Type::Reference`) are guaranteed to be valid, non-null, and properly aligned. They come in four flavors defined by the `exclusive` and `mutable` flags:

- `&T` (`exclusive: false, mutable: false`): Shared read-only reference. Multiple concurrent readers are allowed, and the referenced data cannot be mutated through this reference.
- `&mut T` (shared, `exclusive: false, mutable: true`): Shared mutable reference that requires external synchronization (e.g., `Mutex` or `RefCell`).
- `&uniq T` (`exclusive: true, mutable: false`): Exclusive read-only reference with a guarantee that no other references to the same data exist.
- `&mut T` (exclusive, `exclusive: true, mutable: true`): Exclusive mutable reference — the typical mutable borrow. No other references to the same data can coexist.

**Slice References** (`Type::SliceRef`) are dynamically-sized views into contiguous sequences. At runtime, they are fat pointers containing both a data pointer and a length field.

**Raw Pointers** (`Type::Pointer`) are unsafe pointers without lifetime guarantees. They can be null, dangling, or misaligned, and are only usable in `unsafe` blocks.

**Slice Pointers** (`Type::SlicePtr`) are raw fat pointers to dynamically-sized sequences, combining pointer unsafety with the fat pointer representation.

## Lifetime System

The `Lifetime` enum tracks reference validity duration:

```rust
pub enum Lifetime {
    Static,       // 'static — valid for entire program duration
    Gc,           // Garbage-collected
    ThreadLocal,  // Per-thread lifetime
    TaskLocal,    // Per-task (async context) lifetime
    Inferred,     // To be inferred by the solver
}
```

Lifetimes determine how long references remain valid. `Static` references (to string literals, constant globals) last the entire program. `Gc` references are managed by the garbage collector. `ThreadLocal` references are valid only within one thread. `TaskLocal` references are valid within an async task context. `Inferred` is a placeholder that the solver must resolve.

## Borrow Expressions

The HIR represents borrows explicitly through `Value::Borrow { exclusive, mutable, place }`:

- `&expr` → `Borrow { exclusive: false, mutable: false, place: expr }`
- `&mut expr` → `Borrow { exclusive: true, mutable: true, place: expr }`
- `&uniq expr` → `Borrow { exclusive: true, mutable: false, place: expr }`

## Borrow Checking

The solver performs basic borrow checking: exclusive borrows prevent any other access to the borrowed place, mutable borrows require the borrowed place to be mutable, shared read-only borrows can coexist with each other, and assignment to a borrowed place is restricted while a borrow is active. Returning a reference from a function requires that the reference's lifetime is not longer than any of the input references' lifetimes.

## Integration

References are created by the HIR lowerer when `&` operators are encountered. The solver propagates reference types through constraints, and the codegen generates LLVM pointer operations for reference creation, dereference, and field access through references.
