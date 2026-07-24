# Generics Implementation TODO

## Overall Goal

Implement full Rust-style generics support with monomorphization in the `hir_polish` crate (Hindley-Milner type inference pass).

## Implementation Status - COMPLETED ✅

### Phase 1: HIR Data Structures ✅

- [x] Add `Type::GenericParam { index: u32, name: NString }` variant
- [x] Add `generics` field to `Function`, `EnumDef`, `TypeAliasDef` structs
- [x] Fix exhaustive match patterns across 8 files for `Type::GenericParam`

### Phase 2: HIR Lowering (hir_from_tree crate) ✅

- [x] `lower_function` creates `Type::GenericParam` for each generic parameter
- [x] `lower_enum_definition` handles generics (creates GenericParam)
- [x] `lower_type_alias` handles generics (creates GenericParam)
- [x] `lower_type_path` creates `Type::Parameterized` when type args are present
- [x] `create_generic_placeholder` uses `Type::GenericParam` (not `Type::Inferred`)
- [x] `lower_expr_path` - no longer errors on generic type args
- [x] `lower_struct_init` - no longer errors on generic type args in struct paths
- [x] `lower_implementation` - no longer errors on generic impl blocks

### Phase 3: Type Inference (hir_get_type crate) ✅

- [x] `Type::GenericParam` and `Type::Parameterized` properly handled in HirGetType

### Phase 4: Hindley-Milner + Monomorphization (hir_polish crate) ✅

- [x] `Substitution` system: replaces `Type::GenericParam`, `Type::Inferred`, compound types
- [x] `infer_generic_args_from_call()`: infers concrete types from argument types
- [x] `monomorphize_function()`: creates concrete function copies with substituted types
- [x] Call site replacement: generic calls are replaced with monomorphized function references
- [x] Symbol table registration: monomorphized functions added to SymbolTab for LLVM codegen
- [x] LLVM codegen skips generic (uninstantiated) functions
- [x] RefCell double-borrow fixed by properly dropping borrows before mutation
- [x] Generic function body cloning with type substitution

### Phase 5: Verified Working ✅

- [x] `fn identity<T>(x: T) -> T` correctly monomorphized for i32, bool
- [x] Multiple instantiations at different call sites produce separate mono copies
- [x] Generic param type correctly inferred from argument type at each call site
- [x] Monomorphized functions are registered in SymbolTab
- [x] LLVM IR correctly generated for monomorphized instances

### Phase 6: Future Work (Not Yet Implemented) ❌

- [ ] Struct monomorphization: `StructDef` instances need type substitution
- [ ] Generic enum variant types
- [ ] Nested generic calls (generic function calling another generic)
- [ ] Generic impl blocks with method dispatch
- [ ] Proper mangling scheme for monomorphized symbols
- [ ] Test suite for generics
