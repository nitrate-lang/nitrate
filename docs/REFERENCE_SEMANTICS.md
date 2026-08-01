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

Borrow checking is a dedicated pass that runs after type inference (the solver) but before validation and code generation. The borrow checker operates as a dataflow analysis over a function body, tracking:

1. **Place decomposition**: Converting HIR value expressions into `Place` paths that represent memory location paths (e.g., `x.f.g` becomes `Projection(Projection(Local(x), .f), .g)`)

2. **Active borrow tracking**: Each borrow expression (`&expr`, `&mut expr`) is recorded as an active borrow on the borrowed place. The borrow remains active for its lexical scope.

3. **Conflict detection**: At each program point, the checker verifies:
   - **Aliasing XOR Mutation**: No write can occur while any shared borrow is active; no read or write can occur while a mutable/exclusive borrow is active
   - **Use-after-move**: Values cannot be used after they have been moved
   - **Use-before-initialization**: Variables must be initialized before use
   - **Mutable borrow targets**: Mutable borrows require the target to be mutable
   - **Assignment to borrowed places**: Writing to a place while it is borrowed
   - **Returning local references**: References to local variables cannot be returned

4. **Control flow handling**:
   - **if/else**: Borrows from both branches are merged at the join point
   - **while/loop**: Borrows created inside a loop must be released before the next iteration
   - **return**: All borrows of local variables are invalidated on return

The borrow checker is designed to be conservatively sound: it may reject valid programs (false positives) but will never accept invalid programs (no false negatives). This is the same trade-off Rust made in its lexical borrow checker (pre-NLL). A future upgrade to full NLL (Non-Lexical Lifetimes) region inference will reduce false positives.

## Code Generation Semantics

### Place-Based Memory Model

Codegen follows Rust's place-expression model. A _place_ is a memory location; a borrow of a place produces the address of that location. The codegen invariant is:

> **`gen_place(value)` returns the address of `value`'s storage — never a copy.**

This is what makes `&arr[i]` reference the actual array element rather than a temporary copy. Dereferencing `*p` for a borrow `p: &T` produces the same address as `p` — zero-cost aliasing. Assignment `*p = v` writes through that address.

### Field and Index Access Through References

Access expressions (`FieldAccess`, `IndexAccess`) automatically dereference one layer of reference/pointer:

- `p_ref.x` where `p_ref: &Point` computes a GEP on the pointee of the reference, producing the address of the actual struct field — no struct copy.
- `arr_ref[i]` where `arr_ref: &[i32; 5]` computes a GEP into the array pointee, producing the address of the actual element — no element copy.

Slice indexing (`SliceRef`/`SlicePtr`) extracts the data pointer from the fat pointer, then GEPs the element by the index, yielding a reference into the slice's backing storage.

### Method Call Receivers

When a method takes `&self` or `&mut self`:

- If the receiver expression is already a reference/pointer type (e.g., `p_ref.method()` where `p_ref: &Point`), the pointer value is passed directly — not the address of the reference slot.
- Otherwise, the receiver's place (address) is passed.

### Raw Pointer Assignment

Assignments through dereferenced pointers (`*ptr = v`) compile to a store through the pointer's address. The dereference place is the pointer value itself, so the store writes directly to the pointee — no intermediate temporaries.

### IR-Level Aliasing Guarantees

The zero-cost dereference ensures LLVM sees the alias relation directly. There is no `alloca` + `load` + `store` chain that would hide the data flow, so LLVM's mem2reg, GVN, and alias analysis passes can optimize through borrows as effectively as through direct variable access.

## Integration

References are created by the HIR lowerer when `&` operators are encountered. The solver propagates reference types through constraints, and the codegen generates LLVM pointer operations for reference creation, dereference, and field access through references. The codegen's place-based memory model ensures that all reference operations alias their targets correctly, providing Rust-compatible semantics for borrowed data.
