# Borrow Checker Design Document

## Overview

This document describes Nitrate's borrow checker, which provides memory safety guarantees for safe code through a comprehensive NLL (Non-Lexical Lifetimes) style analysis. The borrow checker operates on **MIR** (Mid-level IR) — not HIR — because MIR's flattened, control-flow-graph form is the smallest representation on which liveness-based borrow regions, two-phase borrows, and move/init tracking are expressible.

The implementation lives in the `nitrate_mir_borrow_check` crate, which runs as part of the pipeline immediately after MIR lowering (see `HirMangled::lower_mir` in the pipeline). It replaces the former HIR borrow checker (`nitrate_hir_borrow_check`), which was disconnected from the pipeline because the tree-structured HIR cannot represent evaluation order, temporary lifetimes, or control-flow merges precisely enough for NLL.

## Core Invariants

The borrow checker enforces these invariants at all times:

1. **Aliasing XOR Mutation**: At any given program point, for any memory location, there exists either:
   - One or more shared reads (`&T` references) with no writes, OR
   - Exactly one mutable access (`&mut T` or direct write) with no reads
   - Exactly one exclusive immutable reference (`&uniq T`) with no other references

2. **Lifetime Validity**: A reference must never outlive the data it points to.

3. **No Dangling References**: References must always point to valid, live memory.

4. **Initialization**: All variables must be initialized before use.

## Architecture

The checker is implemented as a multi-pass analysis running on a per-function snapshot of the MIR:

### Pass 1: Liveness

A backward dataflow over the CFG computes, for every statement boundary, the set of locals whose current value may be used later. The `liveness` module also records per-local use counts and which locals are used directly as call arguments (needed for two-phase detection).

### Pass 2: Borrow Region Computation (forward dataflow)

A forward worklist dataflow maintains the set of **active borrows** at each program point. Each borrow carries:

- **`source`**: the place whose memory is borrowed,
- **`kind`**: shared or mutable,
- **`holders`**: the set of places currently storing the borrow's value.

A holder dies when (a) its root local is no longer live (the NLL last-use rule), (b) its local is overwritten by an assignment, or (c) its storage dies (`StorageDead`). When all holders die, the borrow dies. Holders propagate through:

- **Copies** (`Assign(dst, Use(Copy(holder)))` where the value is reference-typed),
- **Block arguments** (a borrow value passed as a block argument becomes a holder in the target block's argument local).

This is what makes borrow regions non-lexical: a borrow lasts exactly as long as its value is live, not until the end of its scope.

### Pass 3: Conflict Detection

At every statement, every read, write, move, and borrow creation is checked against the active borrows using the **place overlap** relation. `Deref` is a memory boundary: `x` and `*x` do not overlap, but `*x` and `(*x).f` do. Conflicts:

- **Write** while any borrow (shared or mutable) of an overlapping place is active,
- **Read** while an *activated* mutable borrow of an overlapping place is active,
- **Move** while any borrow of an overlapping place is active (moving deinitializes memory),
- **`&mut`** creation while any borrow of an overlapping place is active,
- **`&`** creation while an *activated* mutable borrow of an overlapping place is active.

### Pass 4: Two-Phase Borrows

A `&mut` borrow whose destination local is used **exactly once, as a direct call argument** is treated as a two-phase borrow:

- **Reserved** at creation: reads and shared borrows of the place are allowed (this enables `v.push(v.len())`-style patterns),
- **Activated** at the call: exclusivity is enforced against every other active borrow of overlapping memory.

### Pass 5: Move and Initialization Tracking

The checker tracks, per place, whether it is possibly moved-from or possibly-uninitialized (a union/may analysis over the CFG):

- `Move` operands deinitialize their place,
- reads and borrows of moved-from or never-initialized places are rejected,
- assignments re-initialize the place and everything reachable under it.

### Pass 6: Escaping Borrows

At a `Return`, any borrow whose holder is the returned value is checked: borrowing a local (or a field of a local) and returning it is rejected. Statics and reborrows (`&*p` where the first projection is a `Deref`) may escape.

## Region Representation

Borrow regions are represented implicitly: the forward dataflow state *is* the region. No explicit region variables are needed, because liveness already encodes the exact program points at which a borrow value can be observed.


## Soundness Guarantees

The analysis is a *may* (union) dataflow, so it over-approximates borrow activity and moved/uninitialized state: it may reject valid programs (false positives) but never accepts memory-unsafe ones (no false negatives).

1. **No escaped borrows**: All borrowed data must be valid for the entire borrow duration
2. **No interior aliasing without synchronization**: Shared mutable state requires explicit synchronization
3. **No reference invalidation**: References cannot be invalidated while borrowed
4. **Comprehensive coverage**: Every memory access in safe code is tracked

Known over-approximations and limitations:

- **Drop-liveness is not modeled** (MIR has no drops): a borrow dies at its last use even if the holding local's lexical scope extends further. This is sound (unused references are dead) and strictly more permissive than lexical borrow checking.
- **Points-to aliasing** through copied references is not tracked (the analysis is place-based); full soundness there requires move-only `&mut T`, which the MIR lowering does not yet enforce.
- **Partial moves** are tracked per-place; reading a whole struct after one field was moved is not rejected. The lowering emits `Move` for every by-value read of a non-copy type (structs and enums), so whole-value double moves are always rejected; field-level partial moves are recorded but a later whole-struct read is still permitted.
- **Global mutability** is not carried on `MirGlobal`; string-literal statics are treated as immutable, and unknown statics conservatively reject `&mut`.

## Place Representation

A `Place` is a path to a memory location (mirroring `nitrate_mir::Place`):

- `Place::Local(LocalId)` — a local variable
- `Place::Static(NString)` — a global/static
- `Place::Deref(Box<Place>)` — dereference of a pointer/reference
- `Place::Field(Box<Place>, NString)` — struct field access
- `Place::Index(Box<Place>, Box<Place>)` — array/slice index
- `Place::Downcast(Box<Place>, NString)` — enum variant downcast

