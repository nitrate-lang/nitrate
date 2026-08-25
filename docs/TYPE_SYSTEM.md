# Nitrate Type System: Complete Reference

## Type System Architecture

The Nitrate type system is a **sound, static, nominal type system** that combines several type-theoretic foundations into a unified framework for systems programming. Every expression in a Nitrate program has a type fully determined at compile time, with no implicit type conversions beyond the resolution of `InferredInteger` and `InferredFloat` literals to their concrete types. The type system encompasses a rich ecosystem of type categories: primitive types for raw data representation, compound types for aggregating data, named user-defined types for domain abstraction, reference and pointer types with fine-grained aliasing control, function types with calling convention specifications, refinement types for compile-time value range verification, and generic types resolved through monomorphization.

At the implementation level, the type system is built on a sophisticated storage architecture defined in `nitrate_hir`. The `Type` enum in `ty.rs` is the central representation of all type expressions. Each `Type` variant is stored in the `TypeStore`, an interned, deduplicated, immutable store: structurally identical types always map to the same `TypeId` handle, enabling O(1) type comparison via handle equality rather than O(n) structural recursion. The `TypeId` handle wraps a `NonZeroU32` that indexes into the store's `AppendOnlyVec` for O(1) retrieval. This architecture is critical because type comparison occurs millions of times during inference; O(1) comparison is essential for compilation performance.

The type system is complemented by several modules. `nitrate_hir_get_type` provides the `HirGetType` trait for determining expression types. `nitrate_hir_solve` implements Hindley-Milner constraint-based inference. The layout modules (`ty_size.rs`, `ty_alignment.rs`, `ty_stride.rs`) compute memory sizes and alignments for every type, essential for LLVM code generation and stack layout. The `LayoutCtx` struct coordinates these computations with the symbol table and pointer size configuration.

## Complete Type Enum

The `Type` enum is the single, unified representation of all type expressions, with 37 variants organized into logical groups:

**Primitive types (20 variants)**: `Never`, `Unit`, `Bool`, unsigned integers (`U8`-`U128`, `USize`), signed integers (`I8`-`I128`), floating-point types (`F32`, `F64`).

**Compound types (5 variants)**: `Array { element_type: TypeId, len: u32 }` for fixed-size homogeneous collections, `Tuple { element_types: ThinVec<TypeId> }` for heterogeneous collections, `Struct { def: StructDefId }` for named struct references, `Enum { def: EnumDefId }` for named enum references, `TypeAlias { def: TypeAliasDefId }` for transparent type aliases.

**Refinement types (1 variant)**: `Refine { base: TypeId, min: LiteralId, max: LiteralId }` constrains an integer base type to a compile-time verified value range.

**Function types (1 variant)**: `Function { function_type: Box<FunctionType> }` with attributes, named parameters, and return type.

**Reference and pointer types (4 variants)**: `Reference`, `SliceRef`, `Pointer`, `SlicePtr` — each with lifetime, exclusive/mutable flags, and pointed-to type.

**Trait object types (1 variant)**: `TraitObject { bounds: Vec<TypeBound> }` for dynamic dispatch through vtables.

**Generic and inference types (5 variants)**: `Parameterized`, `GenericParam`, `Inferred`, `InferredFloat`, `InferredInteger` — covering both declaration-site generics and local inference variables.

## Primitive Types in Detail

### Unit Type (Type::Unit)

The unit type `()` represents the absence of meaningful data. It is the implicit return type of side-effect-only functions, the type of blocks ending with a statement rather than a value expression, and the type of `if` expressions without an `else` branch. In memory, `()` occupies zero bytes — it is a zero-sized type (ZST). LLVM maps `()` to `void`.

### Boolean Type (Type::Bool)

The boolean type represents `true` and `false`. It occupies 1 byte in memory (LLVM `i1` in registers, zero-extended to `i8` for storage). The solver enforces `Bool` for all control flow conditions and logical operator results.

### Integer Types

Nitrate covers every standard integer width: `U8` through `U128`, `I8` through `I128`, and the architecture-dependent `USize`. Signed types use two's complement representation. `USize` is 32 bits on 32-bit architectures and 64 bits on 64-bit architectures, determined by `PtrSize` which queries the LLVM target data layout. This architecture dependency is critical for array indexing, pointer arithmetic, the `sizeof` operator, and heap allocation sizes.

### Floating-Point Types

Nitrate supports IEEE 754 single-precision `F32` (LLVM `float`) and double-precision `F64` (LLVM `double`). The implementation uses `ordered_float::OrderedFloat` and `ordered_float::NotNan` to ensure total ordering and NaN-free values, essential for deterministic float comparisons in constant evaluation.

### Never Type (Type::Never)

The never type `!` is the bottom type in Nitrate's type lattice, representing expressions that never produce a value: `return`, `break`, `continue`, `panic!()`, and infinite loops. `Never` can be coerced to any type because the value never exists at runtime — the `HirGetType` implementation returns `Never` for these control flow transfers, and the solver treats `Never` as compatible with all types during unification.

## Compound Types

**Arrays** (`Array { element_type, len }`) are fixed-size, homogeneous, inline value types. Size = element_stride × len. Zero-length arrays are permitted with alignment 1.

**Tuples** (`Tuple { element_types }`) are heterogeneous value types with alignment-aware layout: each element is placed at the next offset aligned to the element's alignment requirement. Total alignment is the maximum alignment of all elements.

**Structs** (`Struct { def }`) are named aggregates with named fields, each with its own visibility and optional default value. Layout is computed from `StructMemoryLayoutCell` entries, supporting both packed (no padding) and standard (alignment-padded) layouts.

**Enums** (`Enum { def }`) are tagged unions with a discriminant (tag) stored after the data payload. This rear-discriminant layout allows the data field to maintain natural alignment without being affected by the tag. Discriminant sizing scales with variant count: 0-1 variants (no discriminant), 2-256 (1 byte), 257-65536 (2 bytes), up to 2^32 (4 bytes), beyond (8 bytes).

## Reference and Pointer Types

The four pointer-like type families provide graduated safety guarantees:

- **References** (`Reference`): Safe, non-null, aligned pointers with exclusive/mutable semantics
- **Slice References** (`SliceRef`): Safe fat pointers (pointer + length) to dynamically-sized contiguous sequences
- **Pointers** (`Pointer`): Unsafe raw pointers, usable only in `unsafe` blocks
- **Slice Pointers** (`SlicePtr`): Unsafe fat pointers for low-level slice manipulation

The `exclusive` and `mutable` flags define access semantics: `&T` is shared read-only, `&mut T` (shared) requires synchronization, `&uniq T` is exclusive read-only, and `&mut T` (exclusive) is the standard mutable borrow with no aliasing.

## Function Types

Function types are **structural** — equivalence is defined by signature (parameter types and return type), not by name. This structural equivalence is essential for function pointers, callbacks, and FFI compatibility. The `FunctionAttribute` enum supports `CVariadic`, `NoMangle`, and `ExternAbi` for calling convention specification.

## Refinement Types

Refinement types constrain integer base types to compile-time verified ranges. The `Refine` variant stores `base: TypeId` (the underlying integer type), `min: LiteralId`, and `max: LiteralId` (inclusive bounds). The solver performs bounds extraction from concrete values and declared types, bounds propagation through arithmetic operations (addition sums bounds, multiplication evaluates all min/max combinations, division handles zero-case), and bounds checking against declared refinement ranges, producing errors for out-of-bounds values.

## Type Size and Alignment

The `LayoutCtx` computes sizes and alignments. Key sizes: `Never`/`Unit` (0 bytes), `Bool` (1), integer types (1-16 bytes by width), `USize` (4 or 8 by architecture), arrays (stride × length), tuples (aligned sum), enums (max variant size + discriminant), references/pointers (4 or 8 bytes), slice references (8 or 16 bytes). Alignments follow the same pattern: each type has a natural alignment equal to its size (up to the maximum alignment requirement of its components).

## Type Interning

The `TypeStore` uses a BiMap-backed deduplication strategy. The `impl_dedup_store!` macro generates a `TypeStore` with `RwLock<BiMap<Arc<Type>, TypeId>>` for deduplication lookup and `AppendOnlyVec<Arc<Type>>` for O(1) handle-to-value access. The Arc wrapping enables zero-cost cloning of type handles. When a new `Type` is stored, the BiMap is checked for an existing entry; if found, the existing `TypeId` is returned; otherwise, the type is appended and registered.

## Design Rationale

**Nominal vs structural typing**: Named types (structs, enums) use nominal typing because type identity by name provides intentional distinction — two identical structures with different names represent different concepts. Function types use structural typing because equivalence is defined by signature.

**Separation of Inferred vs GenericParam**: `Inferred` variables are resolved locally within a function through constraint propagation. `GenericParam` variables persist across function boundaries until a caller provides concrete types. This separation keeps local inference simple while enabling cross-function generics through monomorphization.
