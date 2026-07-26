# Nitrate Compiler - Comprehensive Implementation Plan

## Overview

This task covers: (1) Rust-compatible traits, trait impls, supertraits, and type bounds, (2) Unmanaged rust-like lifetimes/pointers, (3) `iso`/`poly` modifier support, (4) Generics fixes and completion.

## Phase 1: HIR Type System Extensions ✅ (COMPLETE)

- [x] **1.1 Add lifetime to Pointer/SlicePtr types**
  - Added `lifetime: Lifetime` field to `Type::Pointer` and `Type::SlicePtr`
  - Updated all match patterns across the codebase (13 files)
  - Updated `Substitution::apply` to handle lifetime in pointers
  - Updated `HirGetType`, `gen_ty`, validation, dump, mangle

- [x] **1.2 Add Trait bounds/generics support**
  - Added `generics` field to `Trait` struct
  - Added `supertraits: Vec<TraitId>` field to `Trait` struct
  - Added `Type::TraitObject { bounds: Vec<TypeBound> }` variant for trait object types
  - Added `TypeBound` enum: `Trait(TraitId)`, `Lifetime(Lifetime)`

- [x] **1.3 Add where clause support**
  - Added `WhereClause` struct with `type_id` and `bounds` fields
  - Added `where_clause: Option<Vec<WhereClause>>` to `Trait`

- [x] **1.4 Add associated types to traits**
  - Added `associated_types: Vec<NString>` to `Trait`

## Phase 2: HIR Lowering (hir_from_tree) [PARTIAL]

- [ ] **2.1 Fix lower_trait_definition** - TODO: generic traits, supertraits, where clauses, associated types
- [ ] **2.2 Fix lower_implementation** - TODO: generic impl blocks, associated types
- [x] **2.3 Fix lower_type_path for disambiguation** - Records type args for later resolution
- [x] **2.4 Fix lower_expr_path for generic disambiguation** - Parses and stores explicit type args from `foo::<i32>` expressions
- [ ] **2.5 Handle trait bounds on generic parameters** - TODO: parse and store type bounds
- [x] **2.6 Lower pointer types with lifetime annotations** - Lifetime field added to Pointer/SlicePtr

## Phase 3: Type System Updates (HIR core) ✅ (COMPLETE)

- [x] **3.1 Update all Type match arms** - Updated all 13 source files
- [x] **3.2 Fix Parameterized type handling** - Handled in Substitution

## Phase 4: Trait Resolution (hir_solve) [PARTIAL]

- [x] **4.1 Implement trait resolution for method calls** - Solver now resolves methods through SymbolTab and monomorphizes generic trait methods
- [ ] **4.2-4.4** - TODO: trait bound checking, supertrait resolution, bounds in solver

## Phase 5: Monomorphization Fixes [PARTIAL]

- [x] **5.1 expr_path with generic type args** - Lowering parses and stores explicit type args
- [ ] **5.2-5.4** - TODO: type_path resolution, struct/enum mono, Substitution completeness

## Phase 6: Tests ✅

- [x] **All 212 existing tests pass** - No regressions

## Phase 7: Codegen & Validation Updates ✅

- [x] **7.1 LLVM codegen for new types** - TraitObject as opaque ptr, pointer lifetimes handled
- [x] **7.2 Type validation** - TraitObject validated, pointer lifetimes handled
- [x] **7.3 Test stability** - All existing tests pass, build clean

## Implementation Order

1. Phase 1.1 (Pointer lifetime) - affects most files, foundation
2. Phase 1.2-1.4 (Trait extensions) - add HIR types
3. Phase 3.1 (Update match arms) - update all files for new variants
4. Phase 2.1-2.6 (Lowering fixes) - implement parsing
5. Phase 4 (Trait resolution) - the core logic
6. Phase 5 (Monomorphization fixes) - fix generics
7. Phase 6 (Tests) - verify everything works
8. Phase 7 (Codegen) - final integration
