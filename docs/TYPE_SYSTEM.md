# Nitrate Type System: Complete Reference

## Type System Architecture

The Nitrate type system is a **sound, static, nominal type system** that combines several type-theoretic foundations into a unified framework designed for systems programming. Every expression in a Nitrate program has a type that is fully determined at compile time, with no implicit type conversions. The type system encompasses a rich set of primitive types, compound types, named user-defined types, reference and pointer types with fine-grained aliasing control, function types with calling convention annotations, refinement types for compile-time value range verification, and generic types that are resolved through monomorphization.

At the implementation level, the type system is built on top of a sophisticated storage architecture defined in the `nitrate_hir` crate. The `Type` enum, defined in `src/translation/src/hir/src/ty.rs`, is the central representation of all type expressions. Each `Type` variant is stored in the `TypeStore`, which is an interned, deduplicated, immutable store. This means that structurally identical types always map to the same `TypeId` handle, enabling O(1) type comparison via handle equality rather than O(n) structural recursion. The `TypeId` handle wraps a `NonZeroU32` that indexes into the store's `AppendOnlyVec`.

The type system is complemented by several supporting modules. The `nitrate_hir_get_type` crate provides the `HirGetType` trait that can determine the type of any `Value` expression at any point during compilation. The `nitrate_hir_solve` crate implements the Hindley-Milner constraint-based type inference engine that resolves type variables and propagates type constraints. The layout modules (`ty_size.rs`, `ty_alignment.rs`, `ty_stride.rs` in `nitrate_hir`) compute memory sizes and alignment requirements for every type, which are essential for LLVM code generation.

## The Type Enum Hierarchy

The `Type` enum is defined as the single, unified representation of all type expressions in the compiler. It is a large enum with many variants that can be categorized into logical groups: primitive types (20 variants), compound types (3 variants), named types (3 variants), reference and pointer types (4 variants with 5 lifetime specifications), function types (1 variant with complex internal structure), trait object types (1 variant), refinement types (1 variant), and type inference/generic-related types (5 variants). Together these cover every possible type expression that can appear in a Nitrate program.

The complete `Type` enum as defined in `ty.rs`:

```rust
pub enum Type {
    // Primitive types (20)
    Never, Unit, Bool,
    U8, U16, U32, U64, U128, USize,
    I8, I16, I32, I64, I128,
    F32, F64,

    // Compound types (3)
    Array { element_type: TypeId, len: u32 },
    Tuple { element_types: ThinVec<TypeId> },

    // Named types (3)
    Struct { def: StructDefId },
    Enum { def: EnumDefId },
    TypeAlias { def: TypeAliasDefId },

    // Refinement type (1)
    Refine { base: TypeId, min: LiteralId, max: LiteralId },

    // Function type (1)
    Function { function_type: Box<FunctionType> },

    // Reference and pointer types (4)
    Reference  { lifetime: Lifetime, exclusive: bool, mutable: bool, to: TypeId },
    SliceRef   { lifetime: Lifetime, exclusive: bool, mutable: bool, element_type: TypeId },
    Pointer    { lifetime: Lifetime, exclusive: bool, mutable: bool, to: TypeId },
    SlicePtr   { lifetime: Lifetime, exclusive: bool, mutable: bool, element_type: TypeId },

    // Trait object (1)
    TraitObject { bounds: Vec<TypeBound> },

    // Generic and inference types (5)
    Parameterized   { base: TypeId, args: Arguments<TypeId> },
    GenericParam    { index: u32, name: NString },
    Inferred        { id: NonZeroU32, name: Option<NString> },
    InferredFloat,
    InferredInteger,
}
```

Each variant serves a distinct purpose in the type system. The `Never` type is the bottom type that represents computations that never complete (diverging functions, infinite loops, panics). The `Unit` type represents computations that produce no value (void functions). `Bool` represents truth values. The integer types cover every standard bit width from 8 to 128 bits, plus the pointer-sized `USize`. The floating-point types cover IEEE 754 single (`F32`) and double (`F64`) precision.

## Primitive Types in Detail

### Unit Type (Type::Unit)

The unit type `()` represents the absence of meaningful data. It is the return type of functions that execute for side effects only, such as `fn print_hello() { ... }` which implicitly returns `()`. In memory, the unit type occupies zero bytes — it is a zero-sized type (ZST). The solver treats `()` specially in several contexts: blocks that end with an expression statement instead of a value-returning expression produce `()`, `if` expressions without an `else` branch produce `()`, and `while`/`loop` expressions always produce `()`. The `HirGetType` implementation for `Block` checks if the last element is an expression; if so, it returns that expression's type; otherwise it returns `Type::Unit`.

### Boolean Type (Type::Bool)

The boolean type represents the two truth values `true` and `false`. It is a 1-byte value in memory, stored as LLVM `i1` (zero-extended to `i8` for storage in memory). The boolean type is required for:

- Condition expressions in `if`, `while`, and `for` control flow constructs
- Results of comparison operators (`==`, `!=`, `<`, `>`, `<=`, `>=`)
- Results of logical operators (`&&`, `||`, `!`)
- The solver enforces this by inserting `TypeConstraint::Equal(Type::Bool.into())` for the condition of `If` and `While` values

### Integer Types (U8-U128, I8-I128, USize)

Nitrate provides a comprehensive range of integer types covering every common bit width. The unsigned types are `U8`, `U16`, `U32`, `U64`, `U128`, and `USize`. The signed types are `I8`, `I16`, `I32`, `I64`, and `I128`. Each type has a specific bit width and corresponding value range. The signed types use two's complement representation, matching the underlying hardware representation.

The `USize` type is special: its size depends on the target architecture's pointer width. On a 32-bit architecture, `USize` is 32 bits (4 bytes). On a 64-bit architecture, `USize` is 64 bits (8 bytes). The `PtrSize` enum encapsulates this:

```rust
pub enum PtrSize {
    U32 = 4,  // 32-bit pointers, usize = u32
    U64 = 8,  // 64-bit pointers, usize = u64
}
```

The pointer size is obtained from the symbol table via `m.arch_ptr_size()`, which ultimately queries the LLVM target data layout. This architecture-dependent sizing is critical for correct code generation because:

- Array indexing uses `USize` for index values and offset computations
- Pointer arithmetic uses `USize` for byte offsets
- The `sizeof` operator returns `USize` values
- The heap allocation functions take `USize` size parameters

The type system provides several classification methods on `Type`:

```rust
impl Type {
    pub fn is_unsigned_primitive(&self) -> bool {
        matches!(self, Type::U8 | Type::U16 | Type::U32 | Type::U64 | Type::U128 | Type::USize)
    }
    pub fn is_signed_primitive(&self) -> bool {
        matches!(self, Type::I8 | Type::I16 | Type::I32 | Type::I64 | Type::I128)
    }
    pub fn is_integer_primitive(&self) -> bool {
        self.is_unsigned_primitive() || self.is_signed_primitive()
    }
    pub fn is_float_primitive(&self) -> bool {
        matches!(self, Type::F32 | Type::F64)
    }
}
```

### Floating-Point Types (F32, F64)

Nitrate supports IEEE 754 single-precision (`F32`, 32 bits, corresponding to LLVM `float`) and double-precision (`F64`, 64 bits, corresponding to LLVM `double`). The implementation uses `ordered_float::OrderedFloat` and `ordered_float::NotNan` wrappers to ensure that float values are ordered and never contain NaN. This is important because NaN is not ordered — `NaN < 1.0` is false, `NaN >= 1.0` is false, and `NaN == NaN` is false. By using `NotNan`, the compiler guarantees that float comparisons are well-defined. The `OrderedFloat` wrapper provides total ordering for float values (NaN is excluded), enabling floats to be used as keys in `HashMap` and `BTreeMap` during constant evaluation.

The `Value::F32` and `Value::F64` variants store `OrderedFloat<f32>` and `OrderedFloat<f64>` respectively. The `Lit` enum (for compile-time constant literals) similarly uses `OrderedFloat`. During LLVM codegen, these are translated directly to LLVM `float` and `double` types. Floating-point arithmetic operations generate LLVM `fadd`, `fsub`, `fmul`, `fdiv`, and `fneg` instructions.

### Never Type (Type::Never)

The never type `!` is the bottom type in Nitrate's type lattice. It is the type of expressions that never produce a value, including:

- `return` expressions (transfer control flow before producing a value)
- `break` and `continue` expressions (exit or restart a loop)
- `panic!()` or `exit()` calls that never return normally
- Infinite loops (`loop { ... }` without `break`)

The never type has the property that it can be coerced to any other type. This is implemented in the `HirGetType` trait: the `Value::Return`, `Value::Break`, and `Value::Continue` variants all return `Type::Never`. When the solver encounters `Never` in type unification (e.g., in the branches of an `if` expression), it is compatible with any type because the value never actually exists at runtime. The code generator handles `Never` by returning LLVM's `void` type, since no value is ever produced.

The `Type::is_diverging()` method checks for `Never`:

```rust
pub fn is_diverging(&self) -> bool {
    matches!(self, Type::Never)
}
```

## Compound Types

### Array Types (Type::Array)

Arrays are fixed-size, homogeneous collections where all elements have the same type and the length is known at compile time:

```rust
Array { element_type: TypeId, len: u32 }
```

The `element_type` is a `TypeId` handle to the interned element type, and `len` is a `u32` representing the number of elements. The length is stored as a `u32` because LLVM's array type uses a 32-bit length. Arrays are value types — the entire array is stored inline in its containing structure, not as a pointer to heap-allocated memory. This means that arrays in structs, function parameters, and local variables all allocate the full array storage inline.

The size computation for arrays in `ty_size.rs` is:

```rust
Type::Array { element_type, len } => {
    let element_stride = get_stride_of(element_type, ctx)?;
    Ok(element_stride * u64::from(*len))
}
```

The stride is the element size rounded up to the alignment boundary, ensuring that adjacent elements are properly aligned. Alignment for arrays delegates to the element type:

```rust
Type::Array { element_type, len } => {
    if *len == 0 { Ok(1) }        // Zero-length arrays have alignment 1
    else { get_align_of(element_type, ctx) }
}
```

### Tuple Types (Type::Tuple)

Tuples are heterogeneous collections of potentially different types:

```rust
Tuple { element_types: ThinVec<TypeId> }
```

The `ThinVec` used here is a space-efficient vector type from the `thin-vec` crate. It avoids the overhead of a standard `Vec` for small tuples by storing the inline data directly in the enum's allocation when possible. Tuples are value types stored inline. The empty tuple `()` is equivalent to the unit type. A single-element tuple `(T,)` is distinct from a plain `T` at the type level (there is no unwrapping of single-element tuples).

Size computation for tuples performs proper alignment-aware layout:

```rust
Type::Tuple { element_types: elements } => {
    let mut size = 0_u64;
    for element in elements {
        let element_size = get_size_of(element, ctx)?;
        let element_align = get_align_of(element, ctx)?;
        size = size.next_multiple_of(element_align);  // Align before each element
        size += element_size;
    }
    Ok(size)
}
```

Alignment is the maximum alignment of any element:

```rust
let mut max_align = 1;
for element in elements {
    let element_align = get_align_of(element, ctx)?;
    max_align = max(max_align, element_align);
}
Ok(max_align)
```

## Named Types (Struct, Enum, TypeAlias)

Named types in Nitrate are **nominal** — two types with different names are different types even if they have identical structure. This is different from structural typing (used in languages like TypeScript or Go's struct types) where two types with the same fields are interchangeable. Nominal typing provides intentional distinction: the programmer explicitly creates a new type to represent a different concept in the domain, and the compiler enforces this distinction.

### Struct Types (Type::Struct)

```rust
Struct { def: StructDefId }
```

The `StructDefId` handle references a `StructDef` in the store, which contains the struct's name, visibility, fields, generic parameters, and computed memory layout. The `StructDef` is defined in `item.rs`:

```rust
pub struct StructDef {
    pub visibility: Visibility,                    // Pub, Pro, or Sec
    pub name: NString,                             // The struct name
    pub attributes: BTreeSet<StructAttribute>,      // Currently only Packed
    pub fields: BTreeMap<NString, StructField>,     // Field definitions
    pub generics: Option<BTreeMap<NString, Option<TypeId>>>,  // Generic params with defaults
    pub layout: StructLayout,                       // Memory layout as cell list
}
```

The `StructField` struct:

```rust
pub struct StructField {
    pub visibility: Visibility,
    pub attributes: BTreeSet<StructFieldAttribute>,  // Align { alignment }
    pub name: NString,
    pub ty: TypeId,
    pub default_value: Option<ValueId>,
}
```

Struct layout computation handles both packed and unpacked layouts. For packed structs (`StructAttribute::Packed`), fields are laid out without padding:

```rust
if attributes.contains(&StructAttribute::Packed) {
    let mut total_size = 0_u64;
    for field in fields.values() {
        total_size += get_size_of(&field.ty, ctx)?;
    }
    return Ok(total_size);
}
```

For non-packed structs, proper alignment padding is inserted between fields:

```rust
let mut offset = 0_u64;
for field in fields.values() {
    let field_size = get_size_of(&field.ty, ctx)?;
    let field_align = get_align_of(&field.ty, ctx)?;
    offset = offset.next_multiple_of(field_align);
    offset += field_size;
}
Ok(offset)
```

The layout is also recorded as a `StructLayout` — a sequence of `StructMemoryLayoutCell` entries:

```rust
pub enum StructMemoryLayoutCell {
    Field { field_name: NString },
    Padding(NonZeroUsize),
}
pub type StructLayout = ThinVec<StructMemoryLayoutCell>;
```

This explicit layout representation is used by the LLVM codegen to generate the correct LLVM struct type with potentially anonymous padding members.

### Enum Types (Type::Enum)

```rust
Enum { def: EnumDefId }
```

Enums are tagged unions: they store a discriminant (tag) that identifies which variant is active, plus space for the variant's data. The `EnumDef`:

```rust
pub struct EnumDef {
    pub visibility: Visibility,
    pub name: NString,
    pub attributes: BTreeSet<EnumAttribute>,
    pub generics: Option<BTreeMap<NString, Option<TypeId>>>,
    pub variants: ThinVec<EnumVariant>,
}

pub struct EnumVariant {
    pub attributes: BTreeSet<EnumVariantAttribute>,
    pub name: NString,
    pub ty: TypeId,                    // The data carried by this variant
    pub default_value: Option<ValueId>,
}
```

Enum size computation is particularly interesting because it must account for both the data payload (the maximum size across all variants) and the discriminant (the tag identifying which variant is active):

```rust
Type::Enum { def } => {
    let EnumDef { variants, .. } = &*def.borrow();

    // Find the maximum data size across all variants
    let mut size = 0_u64;
    for variant in variants {
        let variant_size = get_size_of(&variant.ty, ctx)?;
        size = max(size, variant_size);
    }

    // Compute discriminant size based on variant count
    let (discrim_size, discrim_align) = match variants.len() as u64 {
        0..=1   => (0, 1),         // Single variant: no discriminant needed
        2..=256 => (1, 1),         // Up to 256 variants: 1 byte
        257..=65536 => (2, 2),     // Up to 65536 variants: 2 bytes
        65537..=4294967296 => (4, 4), // Up to 2^32 variants: 4 bytes
        4294967297.. => (8, 8),    // More: 8 bytes
    };

    // Add discriminant after data (with alignment padding)
    size = size.next_multiple_of(discrim_align);
    size += discrim_size;
    Ok(size)
}
```

The discriminant is placed at the end of the enum (after the data), which differs from some C ABIs that place the tag first. This layout choice allows the data field to maintain its natural alignment without being affected by the discriminant size.

### Type Alias Types (Type::TypeAlias)

```rust
TypeAlias { def: TypeAliasDefId }
```

Type aliases provide alternative names for existing types:

```rust
pub struct TypeAliasDef {
    pub visibility: Visibility,
    pub name: NString,
    pub generics: Option<BTreeMap<NString, Option<TypeId>>>,
    pub type_id: TypeId,  // The underlying type
}
```

The `Substitution::apply` method resolves aliases transparently during type substitution:

```rust
Type::TypeAlias { def } => {
    let type_alias = def.borrow();
    self.apply(&type_alias.type_id)  // Resolve through the alias
}
```

This means that during type checking, aliases are transparent — `MyInt = i32` and `i32` are equivalent types. However, the alias is preserved in the `Type::TypeAlias` representation for debug information, so that variable declarations using `MyInt` are correctly annotated in debug metadata.

## Reference and Pointer Types

Nitrate's reference and pointer type system provides fine-grained control over memory access semantics through four type families, each with a `Lifetime` specification and `exclusive`/`mutable` flags.

### The Lifetime System

```rust
pub enum Lifetime {
    Static,        // 'static — valid for entire program
    Gc,            // Managed by garbage collector
    ThreadLocal,   // Valid only within one thread
    TaskLocal,     // Valid within an async task
    Inferred,      // To be determined by solver
}
```

The `Lifetime::Inferred` variant is a placeholder that the solver must resolve. In the current implementation, lifetimes are recorded in the type system infrastructure but full lifetime inference is a future enhancement. The codegen currently uses `Inferred` for all borrows and relies on the programmer to write correct code (unsafe Rust-like approach rather than Rust's strict borrow checking).

### Reference Types (Type::Reference)

```rust
Reference { lifetime: Lifetime, exclusive: bool, mutable: bool, to: TypeId }
```

References are safe, guaranteed-aligned, non-null pointers. The combination of `exclusive` and `mutable` flags defines the access semantics:

| exclusive | mutable | Syntax            | Semantics                                             |
| --------- | ------- | ----------------- | ----------------------------------------------------- |
| false     | false   | `&T`              | Shared read-only: many concurrent readers allowed     |
| false     | true    | `&mut T` (shared) | Shared mutable: requires external synchronization     |
| true      | false   | `&uniq T`         | Exclusive read-only: no other references to same data |
| true      | true    | `&mut T`          | Exclusive mutable: typical mutable borrow             |

In memory, references are pointer-sized (4 bytes on 32-bit, 8 bytes on 64-bit). They are generated in the HIR via `Value::Borrow`:

```rust
Value::Borrow { exclusive: bool, mutable: bool, place: ValueId }
```

The `HirGetType` implementation constructs the reference type:

```rust
Value::Borrow { mutable, exclusive, place } => {
    let place_type = place.borrow().determine_type(ctx)?;
    Ok(Type::Reference {
        lifetime: Lifetime::Inferred,
        exclusive: *exclusive,
        mutable: *mutable,
        to: place_type.into(),
    })
}
```

### Slice Reference Types (Type::SliceRef)

```rust
SliceRef { lifetime: Lifetime, exclusive: bool, mutable: bool, element_type: TypeId }
```

Slice references represent dynamically-sized views into contiguous sequences. They are "fat pointers" at runtime, consisting of a data pointer and a length field. The total size is `ptr_size * 2` — two pointer-sized values. Slice references enable safe array slicing operations without copying data.

### Pointer Types (Type::Pointer)

```rust
Pointer { lifetime: Lifetime, exclusive: bool, mutable: bool, to: TypeId }
```

Raw pointers are the unsafe counterpart to references. They can be null, dangling, or misaligned, and their lifetime is not tracked. Pointers are only usable in `unsafe` blocks. In memory, they are a single pointer-sized value.

### Slice Pointer Types (Type::SlicePtr)

```rust
SlicePtr { lifetime: Lifetime, exclusive: bool, mutable: bool, element_type: TypeId }
```

Raw slice pointers are unsafe fat pointers (pointer + length), analogous to `*const [T]` in Rust. They provide the same dynamic-length view as slice references but without safety guarantees.

## Function Types

```rust
Type::Function {
    function_type: Box<FunctionType>,
}

pub struct FunctionType {
    pub attributes: BTreeSet<FunctionAttribute>,
    pub params: ThinVec<(NString, TypeId)>,
    pub return_type: TypeId,
}
```

Function types represent callable values — both named functions and function pointers. They are **structural types**: two function types with the same parameter types and return type are equivalent, regardless of the function names. This structural equivalence is essential for function pointers, callbacks, and FFI.

The `FunctionAttribute` enum specifies calling convention details:

```rust
pub enum FunctionAttribute {
    CVariadic,              // C-style varargs (e.g., printf-like)
    NoMangle,               // Preserve original name (no mangling)
    ExternAbi(ExternAbi),   // Foreign function interface calling convention
}

pub struct ExternAbi {
    pub name: NString,      // e.g., "C", "stdcall", "fastcall", "win64"
}
```

The LLVM codegen maps `ExternAbi` names to LLVM calling convention IDs through the `get_abi_call_conv()` function in `src/translation/src/llvm_from_hir/src/symbol.rs`. This function handles over 30 calling conventions including standard conventions (C, system), x86-specific (stdcall, fastcall, thiscall), ARM (AAPCS, APCS), GPU (PTX, AMDGPU), and special-purpose (fast, cold, swift).

## Trait Object Types

```rust
Type::TraitObject {
    bounds: Vec<TypeBound>,
}

pub enum TypeBound {
    Trait(TraitId),
    Lifetime(Lifetime),
}
```

Trait objects enable dynamic dispatch through vtable-based method resolution. When a value has a trait object type (e.g., `dyn Clone`), method calls on that value are dispatched through a vtable pointer rather than being statically resolved. The `bounds` field records both the trait requirements and the minimum lifetime required for the object.

## Refinement Types

```rust
Type::Refine { base: TypeId, min: LiteralId, max: LiteralId }
```

Refinement types are one of Nitrate's distinctive features. They constrain an integer base type to a specific value range that is verified at compile time (where possible) and at runtime. The `min` and `max` are `LiteralId` handles pointing to `Lit` values in the literal store that represent the inclusive bounds of the valid range.

The solver provides extensive refinement type checking through multiple mechanisms:

1. **Bounds Extraction**: The `get_effective_bounds()` method determines the value range of any expression by examining its concrete value (for literals), its declared type (for variables and parameters), or its constraints:

```rust
fn get_effective_bounds(&self, id: &ValueId) -> Option<Bounds> {
    let own_bounds = {
        let value = id.borrow();
        match &*value {
            Value::I8(_) => Some((-128, 127)),
            Value::U8(_) => Some((0, 255)),
            Value::InferredInteger(v) => Some((**v as i128, **v as i128)),
            Value::LocalVariableSymbol { id } => Self::extract_bounds_from_type(id.borrow().ty.deref()),
            Value::ParameterSymbol { id } => Self::extract_bounds_from_type(id.borrow().ty.deref()),
            _ => None,
        }
    };
    // Intersect with constraint-derived bounds
    if let Some(constraints) = self.constraints.get(id) {
        let mut effective = own_bounds;
        for constraint in constraints {
            let TypeConstraint::Equal(ty) = constraint;
            if let Some(bounds) = Self::extract_bounds_from_type(ty) {
                effective = match effective {
                    Some((cur_min, cur_max)) => Some((cur_min.max(bounds.0), cur_max.min(bounds.1))),
                    None => Some(bounds),
                };
            }
        }
        return effective;
    }
    own_bounds
}
```

2. **Bounds Propagation Through Operations**: The solver computes how arithmetic operations affect value ranges:

```rust
fn compute_binary_bounds(op: &BinaryOp, left: Bounds, right: Bounds) -> Option<Bounds> {
    match op {
        BinaryOp::Add => Some((l_min.saturating_add(r_min), l_max.saturating_add(r_max))),
        BinaryOp::Sub => Some((l_min.saturating_sub(r_max), l_max.saturating_sub(r_min))),
        BinaryOp::Mul => {
            let products = [l_min*r_min, l_min*r_max, l_max*r_min, l_max*r_max];
            Some((*products.iter().min().unwrap(), *products.iter().max().unwrap()))
        }
        BinaryOp::Div => { /* handles division by zero case */ }
        // ... other operations
    }
}
```

3. **Bounds Checking Against Refinements**: When the solver assigns a value to a refinement type, it checks that the computed bounds fit:

```rust
fn check_bounds_against_constraint(&mut self, computed_bounds: Bounds, constraint_ty: &Type) -> bool {
    if let Some((target_min, target_max)) = Self::extract_bounds_from_type(constraint_ty) {
        let (comp_min, comp_max) = computed_bounds;
        if (comp_min < target_min || comp_max > target_max) {
            self.errors.insert(TypeErr::OperationResultOutOfRefinementBounds {
                refinement_type: TypeId::from(constraint_ty.clone()),
                computed_min: comp_min.max(0) as u128,
                computed_max: comp_max.max(0) as u128,
            });
            return false;
        }
    }
    true
}
```

## Generic-Related Types

### GenericParam (Declaration-Site)

```rust
GenericParam { index: u32, name: NString }
```

`GenericParam` represents a generic type parameter at its declaration site. When a user writes `fn foo<T>(x: T) -> T`, the type of `x` and the return type both contain `GenericParam { index: 0, name: "T" }`. The `index` identifies the parameter's position in the generic parameter list (0-based). The `name` is the user-visible name used in error messages.

Generic parameters persist through HIR lowering and are only resolved during monomorphization. The solver detects generic calls and creates `Substitution` maps that replace `GenericParam` instances with concrete types.

### Parameterized (Use-Site)

```rust
Parameterized { base: TypeId, args: Arguments<TypeId> }
```

`Parameterized` represents a generic type applied to specific type arguments, such as `Option<i32>` or `HashMap<String, Vec<u8>>`. The `base` is a `TypeId` pointing to the generic type definition (which contains `GenericParam` variants), and `args` is the list of concrete type arguments. During monomorphization, the `Substitution::apply` method resolves `Parameterized` by applying the substitution to the base type.

### Inferred Types (Local Type Variables)

```rust
Inferred { id: NonZeroU32, name: Option<NString> }
InferredFloat
InferredInteger
```

These three variants represent type variables that must be resolved by the Hindley-Milner inference engine:

- `Inferred`: A general type variable created for `let` bindings without explicit type annotations. Each `Inferred` variable has a unique `id` and an optional `name` for debugging.
- `InferredInteger`: An integer literal that hasn't been assigned a concrete integer type. The literal `42` starts as `InferredInteger(42)` and becomes `Value::I32(42)` or `Value::U64(42)` based on the constraints.
- `InferredFloat`: A float literal that hasn't been assigned a concrete float type. The literal `3.14` starts as `InferredFloat(3.14)` and becomes `Value::F32(3.14)` or `Value::F64(3.14)` based on constraints.

The separation between `Inferred`, `InferredInteger`, and `InferredFloat` is important for type inference. If a value is constrained to `Type::I32`, only `InferredInteger` should match (not `Inferred` which could become any type). The solver's `determine_action` method checks the specific variant:

```rust
fn determine_action(&mut self, value: &Value, id: &ValueId) -> NodeAction {
    match value {
        Value::InferredInteger(integer) => self.solve_inferred_integer(id, **integer),
        Value::InferredFloat(float) => self.solve_inferred_float(id, *float),
        _ => NodeAction::NoChange,
    }
}
```

## Type Size, Alignment, and Layout

The size and alignment computation system is defined across three files in `nitrate_hir`:

- `ty_size.rs`: Computes the memory size of each type in bytes
- `ty_alignment.rs`: Computes the memory alignment requirement of each type
- `ty_stride.rs`: Computes the stride (total size including end-of-type padding) for array element access

These computations are performed by `LayoutCtx`:

```rust
pub struct LayoutCtx<'a> {
    pub tab: &'a SymbolTab,
    pub ptr_size: PtrSize,
}
```

The `ptr_size` field is essential because pointer-sized types (`USize`, references, pointers, function pointers) have different sizes on 32-bit vs 64-bit architectures.

### Type Sizes Table

| Type                   | Size (bytes)                       | Notes                                       |
| ---------------------- | ---------------------------------- | ------------------------------------------- |
| `Never`, `Unit`        | 0                                  | Zero-sized types                            |
| `Bool`                 | 1                                  | Stored as `i1` in registers, `i8` in memory |
| `U8`, `I8`             | 1                                  |                                             |
| `U16`, `I16`           | 2                                  |                                             |
| `U32`, `I32`, `F32`    | 4                                  |                                             |
| `U64`, `I64`, `F64`    | 8                                  |                                             |
| `U128`, `I128`         | 16                                 |                                             |
| `USize`                | 4 or 8                             | Pointer-size dependent                      |
| `Array(T, N)`          | `stride(T) * N`                    |                                             |
| `Tuple(T1..Tn)`        | Aligned sum of element sizes       |                                             |
| `Struct`               | Field sizes with alignment padding | Packed variant has no padding               |
| `Enum`                 | Max variant size + discriminant    | Discriminant size based on variant count    |
| `Reference`, `Pointer` | 4 or 8                             | Pointer-size dependent                      |
| `SliceRef`, `SlicePtr` | 8 or 16                            | Pointer + length (2 \* ptr_size)            |
| `Function`             | 4 or 8                             | Function pointer                            |
| `TraitObject`          | 4 or 8                             | Object pointer (vtable + data)              |

### Type Alignments Table

| Type                    | Alignment                                  | Notes                  |
| ----------------------- | ------------------------------------------ | ---------------------- |
| `Never`, `Unit`, `Bool` | 1                                          |                        |
| `U8`, `I8`              | 1                                          |                        |
| `U16`, `I16`            | 2                                          |                        |
| `U32`, `I32`, `F32`     | 4                                          |                        |
| `U64`, `I64`, `F64`     | 8                                          |                        |
| `U128`, `I128`          | 16                                         |                        |
| `USize`                 | 4 or 8                                     | Pointer-size dependent |
| `Array(T, 0)`           | 1                                          | Zero-length arrays     |
| `Array(T, N)`           | align(T)                                   | Non-zero length        |
| `Tuple`                 | Max alignment of elements                  |                        |
| `Struct`                | Max alignment of fields                    | 1 if `Packed`          |
| `Enum`                  | Max of max variant align and discrim align |                        |
| Reference/Pointer types | 4 or 8                                     | Pointer-size dependent |
| `SliceRef`, `SlicePtr`  | 4 or 8                                     | Pointer alignment      |

## Type Interning and the TypeStore

The `TypeStore` is a critical performance infrastructure. It ensures that each semantically distinct `Type` value has exactly one `TypeId` handle:

```rust
impl_dedup_store!(TypeId, Type, TypeStore);
```

This macro expands to:

- A `TypeId` handle struct wrapping `NonZeroU32`
- A `TypeStore` struct with a `RwLock<BiMap<Arc<Type>, TypeId>>` and `AppendOnlyVec<Arc<Type>>`
- A `store()` method that checks the BiMap first, then inserts if not found
- A `Deref` implementation that retrieves the `Type` from the `AppendOnlyVec` via TLS

The BiMap (bidirectional map) provides two lookups:

- Left-to-right: `Arc<Type>` → `TypeId` (for deduplication when storing)
- Right-to-left: `TypeId` → `Arc<Type>` (for retrieval when dereferencing)

The `Arc` wrapping enables zero-cost cloning of type handles without duplicating the type data.

## Type Determination via HirGetType

The `nitrate_hir_get_type` crate provides the `HirGetType` trait:

```rust
pub trait HirGetType {
    fn determine_type(&self, ctx: &SymbolTab) -> Result<Type, TypeInferenceError>;
}
```

This trait is implemented for `Lit`, `Block`, and `Value`. The `Value` implementation is the most complex, handling all 30+ variants through a large `match` expression in `get_type.rs`. Key behaviors:

- `StringLit(s)` → `Type::Array { element_type: U8, len: s.len() }` — strings are arrays of bytes
- `BStringLit(v)` → `Type::Array { element_type: U8, len: v.len() }`
- `Binary { op: Lt/Gt/Lte/Gte/Eq/Ne }` → `Type::Bool` — comparisons always produce bool
- `Binary { op: Add/Sub/Mul/... }` → left operand's type — arithmetic preserves type
- `IndexAccess` → tries Array/Slice element type first, then falls back to trait method `index`
- `Deref` → unwraps the inner type from Reference/Pointer
- `Borrow` → wraps the inner type in a Reference

The `TypeInferenceError` enum captures failure modes:

```rust
pub enum TypeInferenceError {
    EnumVariantNotPresent,
    FieldAccessOnNonStruct,
    StructMissingField,
    CalleeIsNotFunctionType,
    MethodNotFound,
    CannotDeref,
    ClosureHasNoType,
}
```

## Type Checking Rules Summary

### Assignment

The assigned value must have the same type as the target. Integer and float literals are flexible — they adapt to the target type through the `InferredInteger`/`InferredFloat` resolution mechanism.

### Function Calls

- Argument types must match parameter types positionally
- Named arguments can appear in any order (matched by name)
- Return type is determined by the function's declared return type
- Generic calls trigger monomorphization during type inference

### Binary Operations

- Arithmetic: both operands must have the same numeric type; result has that type
- Comparison: both operands must have the same type; result is `Bool`
- Bitwise: both operands must have the same integer type
- Logical: both operands must be `Bool`

### Unary Operations

- `+`, `-`: operand must be numeric; result is the same type
- `!`: operand must be `Bool` (logical not) or integer (bitwise not/complement)

### Control Flow

- `if`/`while`/`for`: condition must be `Bool`
- `match`: scrutinee type must match pattern types
- `return`: value type must equal the enclosing function's declared return type
- `break`/`continue`: type is `Never` (diverging)

### Borrowing

- `&T` requires a place expression of type `T`
- `&mut T` requires a mutable place expression of type `T`
- The borrow expression's type is `Reference { to: T }`

## Design Rationale

### Nominal vs Structural Typing for Named Types

Structs and enums use nominal typing (type identity by name) because this provides intentional distinction and type safety. Two structs with identical fields but different names represent different concepts — the programmer explicitly chose to name them differently. This prevents accidental mixing of structurally compatible but semantically distinct types.

### Structural Typing for Function Types

Function types use structural typing because function equivalence is defined by signature, not by name. Two functions with the same parameter types and return type are interchangeable at the call site. This enables function pointers, callbacks, and FFI compatibility.

### Separation of Inferred vs GenericParam

The separation between inference variables and generic parameters is fundamental to the architecture. `Inferred` variables are resolved locally within a single function through constraint propagation. `GenericParam` variables persist across function boundaries until a caller provides concrete types. Using one mechanism for both would make the inference engine more complex and would prevent clean separation of local inference from monomorphization.

### Refinement Types at the Type Level

Embedding refinement types in the type system (rather than as runtime checks) enables compile-time verification of value ranges. The solver tracks bounds through arithmetic operations, and violations are reported as compile-time errors rather than runtime panics.
