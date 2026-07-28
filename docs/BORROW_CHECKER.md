# Borrow Checker Design Document

## Overview

This document describes the design of Nitrate's borrow checker, which provides memory safety guarantees for safe code through a comprehensive NLL (Non-Lexical Lifetimes) style analysis.

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

The borrow checker is implemented as a multi-pass analysis running on HIR:

### Pass 1: Region Creation

- Assigns a `Region` (abstract program location set) to every reference type
- Creates `RegionVid` (region variables) for inferred lifetimes
- Maps named lifetime parameters to universal regions

### Pass 2: Constraint Collection

- Walks the HIR and collects outlives constraints of the form `'a: 'b` (region 'a outlives region 'b)
- Records where borrows are created (borrow regions)
- Tracks where borrows are used (use regions)
- Records where borrows end (kills)

### Pass 3: Region Solving

- Solves the constraint graph using fixed-point iteration
- Computes the minimal region for each region variable
- Checks that no borrow region outlives the region of its source

### Pass 4: Conflict Detection

- At each borrow site, computes the set of program points where the borrow is active
- For each active borrow, checks for conflicting accesses:
  - Shared borrow active → no writes to the borrowed place
  - Mutable/exclusive borrow active → no reads or writes to the borrowed place
- Handles "two-phase borrows" for function arguments

## Region Representation

```
Region = {
    kind: RegionKind,
    // For region variables, the solved value
    universe: Option<RegionVid>,
}
```

Where `RegionKind` is one of:

- `Static` - entire program
- `Named(NString)` - a named lifetime parameter `'a`
- `Var(RegionVid)` - an inference variable
- `Scope(RegionScopeId)` - a specific scope within a function

## Borrow State

At each program point, tracks:

- `ActiveBorrows`: Set of `(PlaceId, BorrowKind)` for currently active borrows
- `MoveState`: Whether each place has been moved from

## Place Representation

A `Place` is a path to a memory location:

- `Place::Local(LocalVar)` - a local variable
- `Place::Static(GlobalVar)` - a global/static
- `Place::Deref(Box<Place>)` - dereference of a pointer/reference
- `Place::Field(Box<Place>, NString)` - struct field access
- `Place::Index(Box<Place>, Box<Place>)` - array/slice index

## Soundness Guarantees

The borrow checker is designed to be provably sound:

1. **No escaped borrows**: All borrowed data must be valid for the entire borrow duration
2. **No interior aliasing without synchronization**: Shared mutable state requires explicit synchronization
3. **No reference invalidation**: References cannot be invalidated while borrowed
4. **Comprehensive coverage**: Every memory access in safe code is tracked

Unlike Rust's borrow checker (which has known soundness holes like self-referential structs with Pin, unsound lifetime coercions, etc.), Nitrate's borrow checker:

1. Tracks borrows through all levels of field and index access
2. Does not allow lifetime coercion that could shorten a borrow beyond its actual use
3. Has no escape hatches (like `unsafe`) that would bypass checks in safe code
4. Uses a conservatively sound region inference that rejects any ambiguous case
