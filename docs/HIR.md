# High-Level Intermediate Representation (HIR)

## Theoretical Foundation and Role

An Intermediate Representation (IR) sits between the source-level Abstract Syntax Tree (AST) and the backend-specific code representation (LLVM IR). The HIR is the central data structure of the Nitrate compiler — all semantic analysis, type inference, validation, and monomorphization operate on the HIR. Understanding the HIR is the key to understanding the entire compiler; every major transformation in the pipeline either produces HIR, consumes HIR, or transforms HIR into a more resolved form.

The HIR serves five critical functions within the compilation pipeline:

### 1. Normalization

Complex syntactic constructs — syntactic sugar, desugared control flow, implicit conversions — are transformed into simpler, uniform representations when lowered from the AST to the HIR. For example, `for x in iter { body }` might be lowered to a `while` loop operating on an iterator, and all type annotations are resolved to their canonical `Type` representations. This normalization ensures that subsequent passes operate on a consistent, predictable representation regardless of the syntactic variations in the source code. The parser preserves every detail of the source syntax; the HIR normalizes away the inessential details, leaving only the semantic core.

### 2. Type Annotation

Every expression and binding in the HIR carries resolved type information. During lowering, types are associated with all value nodes, either directly (from explicit annotations in the source code) or through `Type::Inferred` variables that the solver will later resolve through constraint propagation. The result is a fully typed representation where code generation can determine the memory layout, instruction selection, and optimization strategy for every operation without needing to recompute type information from the AST.

### 3. Analysis Target

Type checking, constraint solving, refinement type verification, and semantic validation all operate on the HIR. The HIR's structure is designed to make these analyses efficient: types are interned for O(1) comparison, values are stored in a traversable graph where every expression references its sub-expressions through `ValueId` handles, and control flow is explicitly represented through block structures with well-defined entry points.

### 4. Monomorphization Boundary

Generic code is instantiated at the HIR level through a process of cloning and type substitution. The solver detects generic function calls during its fixed-point traversal of the expression graph, creates monomorphized copies of the function body with concrete types substituted for generic parameters, and registers these copies in the symbol table for code generation. The HIR provides an ideal level for monomorphization because it preserves the structure of the original code (making cloning straightforward) while carrying enough type information to make the substitution meaningful and verifiable.

### 5. Codegen Input

The validated, solved HIR drives LLVM IR generation. The codegen traverses the HIR expression graph, translating each `Value` node into the corresponding LLVM instructions. The HIR's type information guides the selection of LLVM types and instruction variants, ensuring correct and efficient code generation.

## Architecture Overview

**Crate**: `nitrate_hir` (also re-exported as `nitrate::hir` via `nitrate_translation`) — the central crate defining all HIR types, the storage infrastructure, and the pass framework.

**Dependencies**: HIR types carry source location information via `nitrate_tree::ByteSpan`, providing byte-offset ranges into source files that allow error messages, LSP features, and diagnostics to reference precise source locations.

**Sub-crates**: Each major HIR operation lives in its own sub-crate for modularity and independent testing:

| Sub-crate               | Function                                                                                                                             |
| ----------------------- | ------------------------------------------------------------------------------------------------------------------------------------ |
| `nitrate_hir_from_tree` | AST→HIR lowering: transforms the resolved parse tree into HIR items, types, values, and blocks within the TLS Store                  |
| `nitrate_hir_solve`     | Type inference and monomorphization: the Hindley-Milner constraint solver that resolves all type variables and instantiates generics |
| `nitrate_hir_validate`  | Validation: semantic correctness checks producing the `ValidHir` wrapper that guarantees the HIR is ready for code generation        |
| `nitrate_hir_get_type`  | Type determination: the `HirGetType` trait for computing the type of any expression at any point during compilation                  |
| `nitrate_hir_mangle`    | Name mangling: deterministic LLVM linkage name generation encoding package, function, and type information                           |
| `nitrate_hir_evaluate`  | Constant evaluation: compile-time computation of constant expressions for global initializers and default values                     |
| `nitrate_hir_dump`      | Pretty-printing: HIR visualization for debugging and diagnostics                                                                     |

**Key types**: `Store` (the central data repository), type handles (`TypeId`, `ValueId`, `FunctionId`, `StructDefId`, `EnumDefId`, `TraitId`, `ModuleId`, `BlockId`, `LocalVariableId`, `ParameterId`, `GlobalVariableId`, `TypeAliasDefId`), the `Type` enum, the `Value` enum, item definition structs (`Function`, `StructDef`, `EnumDef`, `Trait`, `Module`, `TypeAliasDef`, `GlobalVariable`, `LocalVariable`, `Parameter`, `Block`).

**Key files**: `store.rs` (storage architecture and TLS pattern), `ty.rs` (Type enum with all 37+ type variants — each variant carries a `ByteSpan`), `expr.rs` (Value enum with all 30+ expression variants — each carries a `ByteSpan`), `item.rs` (item definition structs with `ByteSpan` fields), `table.rs` (symbol table integration).

### Source Location Tracking

All HIR types now carry source location information from the parser. Every `Type` variant, `Value` variant, and item struct (`Function`, `StructDef`, `EnumDef`, `Trait`, `Module`, `TypeAliasDef`, `GlobalVariable`, `LocalVariable`, `Parameter`, `StructField`, `EnumVariant`) has a `span: ByteSpan` field that records the byte range of the source text that produced it.

The `ByteSpan` type (from `nitrate_tree`) stores start and end byte offsets as `u32` values, providing compact 8-byte source ranges that support:

- **Error reporting**: Diagnostics can reference exact source locations for meaningful error messages
- **LSP integration**: IDE features like hover, go-to-definition, and references can map HIR nodes back to source
- **Debugging**: The HIR dump can annotate output with source positions

For the `Type` enum specifically, `ByteSpan` is excluded from `Hash`, `Eq`, and `Ord` implementations via custom trait implementations. This preserves the critical deduplication property: `Type::U8 { span: (0, 1) }` and `Type::U8 { span: (5, 6) }` produce the same `TypeId`, and `Type::Bool` in one expression is structurally identical to `Type::Bool` in another. The custom `Hash` uses discriminant-based hashing while custom `PartialEq` compares only structural fields, ignoring span. This deduplication extends to all compound types: `Type::Array`, `Type::Tuple`, `Type::Reference`, etc. all compare structurally without considering their `span` fields.

For `Value`, `Block`, and item types, span is included in `Hash`/`Eq` since these types use `impl_store_mut!` (append-only vector storage) where each handle is unique and structural comparison is not required for deduplication.

## Storage Architecture in Detail

### The Store

The `Store` is the central repository for all HIR data within a single compilation session. It is organized as a collection of typed sub-stores, each optimized for the access patterns of its particular data type:

```rust
pub struct Store {
    // Immutable, deduplicated stores (BiMap + AppendOnlyVec):
    types: TypeStore,                        // Interned Type values
    literals: ExprLiteralStore,              // Interned Lit values

    // Mutable, append-only stores (AppendOnlyVec<RefCell<T>>):
    global_variables: GlobalVariableStore,   // Global variable declarations
    local_variables: LocalVariableStore,     // Local variable declarations
    parameters: ParameterStore,              // Function parameters
    functions: FunctionStore,                // Function definitions
    traits: TraitStore,                      // Trait definitions
    modules: ModuleStore,                    // Module definitions
    type_aliases: TypeAliasStore,            // Type alias definitions
    struct_defs: StructDefStore,             // Struct definitions
    enum_defs: EnumDefStore,                 // Enum definitions
    values: ExprValueStore,                  // Expression value nodes (the IR graph)
    blocks: ExprBlockStore,                  // Code blocks (sequences of elements)
}
```

Each sub-store uses one of two distinct storage strategies, selected at macro invocation time based on the data's requirements.

### Strategy 1: Immutable Deduplicated Store (TypeStore, ExprLiteralStore)

Used for `Type` and `Lit` — these are immutable values that benefit from structural sharing. The deduplication strategy ensures that structurally identical values always produce the same handle, enabling O(1) equality comparison.

Implementation (from the `impl_dedup_store!` macro expansion):

```rust
pub struct TypeStore {
    bimap: RwLock<BiMap<Arc<Type>, TypeId>>,        // Bidirectional map for O(1) dedup lookup
    quick_vec: AppendOnlyVec<Arc<Type>>,             // O(1) handle-to-value access
}

impl TypeStore {
    pub fn store(&self, item: Type) -> TypeId {
        // Double-check pattern: read lock first, then write lock if needed
        if let Some(id) = self.bimap.read().unwrap().get_by_left(&item) {
            return *id;
        }
        let mut bimap = self.bimap.write().unwrap();
        if let Some(id) = bimap.get_by_left(&item) {
            return *id;  // Another thread inserted between read and write
        }
        let arc_item = Arc::new(item);
        self.quick_vec.push(arc_item.clone());
        let id = TypeId(NonZeroU32::new(self.quick_vec.len() as u32).unwrap());
        bimap.insert(arc_item, id);
        id
    }
}
```

Key properties:

- **Type deduplication**: Two identical types always produce the same `TypeId` — if `T1` and `T2` are structurally equivalent, `store(T1) == store(T2)`. This means `Array(U8, 4)` always produces the same handle regardless of how many times it's created.
- **Handle comparison = structural equality**: `type_id_a == type_id_b` is equivalent to comparing the full type structures, but is O(1) instead of O(n). This is the single most important optimization in the compiler because type comparison occurs millions of times during inference.
- **Read-optimized**: The `RwLock` allows concurrent reads; the write lock is only acquired during deduplication checks
- **Arc sharing**: Types are stored behind `Arc` for zero-cost cloning without mutex contention

### Strategy 2: Mutable Append-Only Store (All Other Sub-Stores)

Used for `Function`, `StructDef`, `EnumDef`, `TypeAliasDef`, `Module`, `Trait`, `GlobalVariable`, `LocalVariable`, `Parameter`, `Value`, and `Block`. These items are created during lowering and potentially modified during solving.

```rust
pub struct FunctionStore {
    vec: AppendOnlyVec<RefCell<Function>>,
}

impl FunctionStore {
    pub fn store(&self, item: Function) -> FunctionId {
        self.vec.push(RefCell::new(item));
        let id = NonZeroU32::new(self.vec.len() as u32).unwrap();
        FunctionId(id)
    }
}
```

Key properties:

- **Append-only growth**: New items are always appended to the end; existing items are never moved or invalidated, so handles remain valid for the entire compilation session. This is critical because handles are stored in HashMaps throughout the compiler.
- **Interior mutability**: `RefCell<T>` enables modification of stored items through shared references (`&self`), which is essential for the solver pattern where a pass holds references to some items while creating new ones
- **No deduplication**: Each `store()` call creates a new unique handle — `Function` identity is by creation order, not by structural equality

### Thread-Local Access Pattern

The TLS pattern is the mechanism through which handles access their underlying data. It avoids both global mutable state and passing the Store through every function call:

```rust
thread_local! {
    static TLS_STORE: Cell<Option<*const Store>> = const { Cell::new(None) };
}

pub fn using_storage<R>(store: &Store, f: impl FnOnce() -> R) -> R {
    TLS_STORE.with(|tls| {
        let old = tls.take();
        tls.set(Some(store));
        let result = f();
        tls.set(old);  // Restore previous store (supports nesting)
        result
    })
}

pub fn get_storage<R>(f: impl FnOnce(&Store) -> R) -> R {
    TLS_STORE.with(|tls| {
        let store_ptr = tls.get()
            .expect("No Store found in TLS. Did you forget to call using_storage?");
        let store = unsafe { &*store_ptr };
        f(store)
    })
}
```

The data flow:

1. **Setup**: The translation pipeline calls `using_storage(&store, || { compile() })`, saving a raw pointer to the store in thread-local storage
2. **Access**: Any `Deref` implementation on a handle calls `get_storage()` to retrieve the store pointer, then indexes into the appropriate sub-store using the handle's inner `NonZeroU32`
3. **Teardown**: When `using_storage` returns, the previous TLS value is restored, supporting nested store usage

For example, `FunctionId`'s `Deref` implementation:

```rust
impl Deref for FunctionId {
    type Target = RefCell<Function>;
    fn deref(&self) -> &Self::Target {
        TLS_STORE.with(|tls| {
            let store = unsafe { &*tls.get().unwrap() };
            &store.functions[self]  // Indexes into the FunctionStore
        })
    }
}
```

### Handle Usage Pattern

```rust
// Creating and immediately storing via Into trait
let func_id: FunctionId = Function { ... }.into();  // Calls FunctionId::from()

// Deref to access contents
let func: &RefCell<Function> = &*func_id;
func.borrow().name         // Access fields through RefCell::borrow()
func.borrow_mut().body = Some(new_body);  // Mutate through RefCell::borrow_mut()

// Type interning — identical types produce identical handles
let type_id: TypeId = Type::I32.into();
let same_type_id: TypeId = Type::I32.into();
assert_eq!(type_id, same_type_id);  // Handle equality = structural equality

// Compound types recursively intern their components
let array_type: TypeId = Type::Array {
    element_type: type_id,
    len: 10,
}.into();
```

## Type Representation

The `Type` enum represents all type expressions. The complete enum has 37+ variants organized into logical categories that mirror the type system's structure.

### Primitive Types (20 variants)

```rust
Never        // Bottom type for diverging functions (return, break, continue, panic)
Unit         // () — zero-size type for void computations
Bool         // Boolean truth values
U8, U16, U32, U64, U128, USize   // Unsigned integers
I8, I16, I32, I64, I128           // Signed integers (two's complement)
F32, F64                           // IEEE 754 floating point
```

`Never` is the bottom type — it represents computations that never produce a value. `Never` can be coerced to any type because the value never exists at runtime. The solver uses `Never` when unifying branches of an `if` expression where one branch diverges (e.g., returns or panics).

`Unit` is zero bytes in memory, mapped to `void` in LLVM. It is the implicit return type of functions without a return value and blocks that end with a statement rather than an expression.

`Bool` is 1 byte, stored as LLVM `i1` in registers (zero-extended to `i8` for memory). The solver constrains all control flow conditions and comparison results to `Bool`.

`USize` is architecture-dependent: 32 bits on 32-bit architectures and 64 bits on 64-bit architectures. This is queried from the LLVM target data layout through `PtrSize`.

### Compound Types (5 variants)

```rust
Array { element_type: TypeId, len: u32 }   // Fixed-size homogeneous array
Tuple { element_types: ThinVec<TypeId> }   // Heterogeneous tuple
Struct { def: StructDefId }                 // Named struct reference
Enum { def: EnumDefId }                     // Named enum reference
TypeAlias { def: TypeAliasDefId }           // Type alias reference
```

The `Array`, `Struct`, `Enum`, and `TypeAlias` variants all use handle indirection — they store `*DefId` handles that dereference to their definition structs. This indirection enables late binding (definitions can be modified after type creation), name resolution via the definition's name field, and field type access during type determination and codegen.

### Refinement Type

```rust
Refine { base: TypeId, min: LiteralId, max: LiteralId }
```

Constrains a base integer type to a compile-time verified value range. The solver tracks bounds through arithmetic operations (e.g., `Add` produces bounds `[a_min + b_min, a_max + b_max]`) and checks results against declared refinement ranges. Out-of-bounds values produce compile-time errors rather than runtime panics.

### Function Type

```rust
Function { function_type: Box<FunctionType> }

pub struct FunctionType {
    pub attributes: BTreeSet<FunctionAttribute>,  // CVariadic, NoMangle, ExternAbi
    pub params: ThinVec<(NString, TypeId)>,       // Named, typed parameters
    pub return_type: TypeId,                       // Return type
}
```

Function types are structural — two function types with the same parameter types and return type are equivalent regardless of the function names.

### Reference and Pointer Types (4 variants)

```rust
Reference { lifetime: Lifetime, exclusive: bool, mutable: bool, to: TypeId }
SliceRef   { lifetime: Lifetime, exclusive: bool, mutable: bool, element_type: TypeId }
Pointer    { lifetime: Lifetime, exclusive: bool, mutable: bool, to: TypeId }
SlicePtr   { lifetime: Lifetime, exclusive: bool, mutable: bool, element_type: TypeId }
```

The combination of `exclusive` and `mutable` defines four access semantics: `&T` (shared read-only), `&mut T` shared (requires synchronization), `&uniq T` (exclusive read-only), and `&mut T` exclusive (standard mutable borrow).

### Value Representation

The `Value` enum represents all expression nodes with 30+ variants: literal values (19 variants), compound values (struct/enum/list/tuple construction), operator values (binary/unary operations), access values (field/index/dereference), control flow values (if/while/loop/break/continue/return/block), call values (function calls/method calls), symbol values (function/global/local/parameter references), and special values (assignment/cast/borrow).

### Design Rationale

**Why Thread-Local Store instead of global state?** Testability (each test creates a fresh Store), deterministic RAII teardown, no global locks, and explicit visibility through the call stack.

**Why both dedup and non-dedup stores?** Dedup stores optimize the most frequent operation (type comparison at O(1)). Non-dedup stores support the mutation patterns required by the solver during monomorphization.

**Why RefCell instead of &mut Store?** The solver iterates existing items while creating new ones during monomorphization. `RefCell` provides interior mutability without requiring exclusive `&mut` access to the entire store.
