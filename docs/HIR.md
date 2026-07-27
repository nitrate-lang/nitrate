# High-Level Intermediate Representation (HIR)

## Theoretical Foundation

An Intermediate Representation (IR) sits between the source-level Abstract Syntax Tree (AST) and the backend-specific code representation (LLVM IR). The HIR is the central data structure of the Nitrate compiler — all semantic analysis, type inference, and validation operate on the HIR.

The HIR serves these critical functions:

1. **Normalization**: Complex syntactic constructs are desugared into simpler, uniform representations
2. **Type Annotation**: Every expression and binding carries resolved type information
3. **Analysis Target**: Type checking, inference, and validation operate on the HIR
4. **Monomorphization Boundary**: Generic code is instantiated at the HIR level
5. **Codegen Input**: The validated HIR drives LLVM IR generation

## Architecture

**Crate**: `nitrate_hir` (also re-exported as `nitrate::hir` via `nitrate_translation`)  
**Sub-crates**: `nitrate_hir_from_tree` (AST→HIR lowering), `nitrate_hir_solve` (type inference + monomorphization), `nitrate_hir_validate` (validation), `nitrate_hir_get_type` (type determination via `HirGetType` trait), `nitrate_hir_mangle` (name mangling), `nitrate_hir_evaluate` (constant evaluation), `nitrate_hir_dump` (pretty-printing)  
**Key types**: `Store`, type handles (`TypeId`, `ValueId`, `FunctionId`, `StructDefId`, `EnumDefId`, etc.), `Type`, `Value`, item definition structs  
**Key files**: `src/translation/src/hir/src/store.rs`, `ty.rs`, `expr.rs`, `item.rs`, `pass.rs`, `table.rs`

## Storage Architecture

### The Store

The `Store` is the central repository for all HIR data. It is organized as a collection of typed sub-stores, each backed by an `AppendOnlyVec`:

```rust
pub struct Store {
    types: TypeStore,                        // Interned, deduplicated immutable types
    global_variables: GlobalVariableStore,   // Mutable RefCell-backed globals
    local_variables: LocalVariableStore,     // Mutable RefCell-backed locals
    parameters: ParameterStore,              // Mutable RefCell-backed params
    functions: FunctionStore,                // Mutable RefCell-backed functions
    traits: TraitStore,                      // Mutable RefCell-backed traits
    modules: ModuleStore,                    // Mutable RefCell-backed modules
    type_aliases: TypeAliasStore,            // Mutable RefCell-backed type aliases
    struct_defs: StructDefStore,             // Mutable RefCell-backed structs
    enum_defs: EnumDefStore,                 // Mutable RefCell-backed enums
    values: ExprValueStore,                  // Mutable RefCell-backed expressions
    literals: ExprLiteralStore,              // Interned, deduplicated immutable literals
    blocks: ExprBlockStore,                  // Mutable RefCell-backed code blocks
}
```

Each sub-store uses one of **two distinct storage strategies**, defined by macros in `store.rs`:

### Strategy 1: Immutable Deduplicated Store (`impl_dedup_store!`)

Used for `Type` and `Lit` — these are immutable values that benefit from structural sharing.

Implementation (from the macro expansion):

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

- **Type deduplication**: Two identical types always produce the same `TypeId`
- **Handle comparison = structural equality**: `type_id_a == type_id_b` is equivalent to comparing the full type structures
- **Read-optimized**: The `RwLock` allows concurrent reads
- **Arc sharing**: Types are stored behind `Arc` for zero-cost cloning

### Strategy 2: Mutable Store (`impl_store_mut!`)

Used for `Function`, `StructDef`, `EnumDef`, `TypeAliasDef`, `Module`, `Trait`, `GlobalVariable`, `LocalVariable`, `Parameter`, `Value`, `Block`.

Implementation:

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

- **Append-only growth**: New items never invalidate existing handles
- **Interior mutability**: `RefCell` enables modification without `&mut Store`
- **No deduplication**: Each `store()` call creates a new unique handle

### Thread-Local Access Pattern

The TLS pattern is defined in `store.rs`:

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

The flow is:

1. **Setup**: The translation pipeline calls `using_storage(&store, || { compile() })`, saving the store pointer in TLS
2. **Access**: Any `Deref` implementation on a handle calls `get_storage()` to retrieve the store and then indexes into the appropriate sub-store
3. **Teardown**: On `using_storage` return, the previous TLS value is restored

For example, `FunctionId`'s `Deref`:

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
// Creating and immediately storing:
let func_id: FunctionId = Function { ... }.into();  // Calls FunctionId::from()

// Deref to access contents:
let func: &RefCell<Function> = &*func_id;
func.borrow().name  // Access fields
func.borrow_mut().body = Some(new_body);  // Mutate

// Type interning (deduplicated):
let type_id: TypeId = Type::I32.into();
let array_type: TypeId = Type::Array {
    element_type: type_id,
    len: 10,
}.into();
```

## Type Representation

The `Type` enum (defined in `ty.rs`) represents all type expressions in the compiler:

### Primitive Types (20 variants)

```rust
Never        // Bottom type for diverging functions
Unit         // () — zero-size type
Bool         // Boolean
U8, U16, U32, U64, U128, USize   // Unsigned integers
I8, I16, I32, I64, I128           // Signed integers
F32, F64                           // Floating point
```

### Compound Types (3 variants)

```rust
Array { element_type: TypeId, len: u32 }   // Fixed-size homogeneous array
Tuple { element_types: ThinVec<TypeId> }   // Heterogeneous tuple
Struct { def: StructDefId }                 // Named struct reference
Enum { def: EnumDefId }                     // Named enum reference
TypeAlias { def: TypeAliasDefId }           // Type alias reference
```

The `Array` variant stores:

- `element_type`: A `TypeId` pointing to the interned element type
- `len`: A `u32` length (compile-time known)

The `Struct`, `Enum`, and `TypeAlias` variants store `*DefId` handles that dereference to their respective definition structs. This indirection allows:

- **Late binding**: The definition can be modified after the type is created
- **Name resolution**: The definition contains the name and generic parameters
- **Field type access**: Type determination can look up field types via the definition

### Refinement Type

```rust
Refine { base: TypeId, min: LiteralId, max: LiteralId }
```

Constrains a base type to a specific value range. The `min` and `max` are `LiteralId` handles pointing to `Lit` values in the literal store.

### Function Type

```rust
Function { function_type: Box<FunctionType> }

pub struct FunctionType {
    pub attributes: BTreeSet<FunctionAttribute>,  // CVariadic, NoMangle, ExternAbi
    pub params: ThinVec<(NString, TypeId)>,       // Named, typed parameters
    pub return_type: TypeId,                       // Return type
}
```

The `FunctionAttribute` enum:

```rust
pub enum FunctionAttribute {
    CVariadic,           // C-style variadic parameters (printf-like)
    NoMangle,            // Preserve original name (no mangling)
    ExternAbi(ExternAbi), // Foreign calling convention
}
```

### Reference and Pointer Types (4 variants)

```rust
Reference { lifetime: Lifetime, exclusive: bool, mutable: bool, to: TypeId }
SliceRef   { lifetime: Lifetime, exclusive: bool, mutable: bool, element_type: TypeId }
Pointer    { lifetime: Lifetime, exclusive: bool, mutable: bool, to: TypeId }
SlicePtr   { lifetime: Lifetime, exclusive: bool, mutable: bool, element_type: TypeId }
```

The `Lifetime` enum:

```rust
pub enum Lifetime {
    Static,        // 'static — entire program duration
    Gc,            // Garbage-collected
    ThreadLocal,   // Per-thread
    TaskLocal,     // Per-async-task
    Inferred,      // To be inferred by the solver
}
```

### Trait Object

```rust
TraitObject { bounds: Vec<TypeBound> }
```

Where `TypeBound`:

```rust
pub enum TypeBound {
    Trait(TraitId),      // Requires implementation of a trait
    Lifetime(Lifetime),  // Requires at least a minimum lifetime
}
```

### Generic-Related Types (5 variants)

```rust
Parameterized { base: TypeId, args: Arguments<TypeId> }  // Type applied to arguments
GenericParam { index: u32, name: NString }                // Declaration-site generic
Inferred { id: NonZeroU32, name: Option<NString> }        // Inference variable
InferredFloat                                              // Unresolved float literal
InferredInteger                                            // Unresolved integer literal
```

The `Parameterized` variant uses `Arguments<TypeId>`:

```rust
pub struct Arguments<T> {
    pub positional: ThinVec<T>,
    pub named: ThinVec<(NString, T)>,
}
```

This supports both positional (e.g., `HashMap<K, V>`) and named type arguments.

## Value Representation

The `Value` enum (defined in `expr.rs`) represents all expression nodes. It has 30+ variants:

### Literal Values (19 variants)

```rust
Unit, Bool(bool)
I8(i8), I16(i16), I32(i32), I64(i64), I128(Box<i128>)
U8(u8), U16(u16), U32(u32), U64(u64), U128(Box<u128>)
F32(OrderedFloat<f32>), F64(OrderedFloat<f64>)
USize(u8, u64)           // (bit_width, value)
StringLit(ThinStr)       // UTF-8 string
BStringLit(ThinVec<u8>)  // Byte string
InferredInteger(Box<u128>)   // Unresolved integer literal
InferredFloat(OrderedFloat<f64>)  // Unresolved float literal
```

Note the distinction:

- `I32(42)` is a fully resolved `i32` literal
- `InferredInteger(42)` is a literal awaiting type inference (could become `u8`, `i64`, etc.)

### Compound Values

```rust
StructObject { struct_def: StructDefId, fields: ThinVec<(NString, ValueId)> }
EnumVariant { enum_def: EnumDefId, variant: NString, value: ValueId }
List { elements: ThinVec<ValueId> }     // Array/list literal
Tuple { elements: ThinVec<ValueId> }
```

### Operator Values

```rust
Binary { left: ValueId, op: BinaryOp, right: ValueId }
Unary { op: UnaryOp, operand: ValueId }
```

The `BinaryOp` enum has 16 variants:

```rust
pub enum BinaryOp {
    Add, Sub, Mul, Div, Mod,           // Arithmetic
    And, Or, Xor,                       // Bitwise
    Shl, Shr, Rol, Ror,                  // Shift and rotate
    LogicAnd, LogicOr,                   // Logical
    Lt, Gt, Lte, Gte, Eq, Ne,           // Comparison
}
```

The `UnaryOp` enum has 3 variants: `Add` (identity/plus), `Sub` (negation), `Not` (bitwise/logical negation).

### Access Values

```rust
IndexAccess { collection: ValueId, index: ValueId }
FieldAccess { expr: ValueId, field_name: NString }
Deref { place: ValueId }
```

### Control Flow Values

```rust
If { condition: ValueId, true_branch: BlockId, false_branch: Option<BlockId> }
While { condition: ValueId, body: BlockId }
Loop { body: BlockId }
Break { label: Option<NString> }
Continue { label: Option<NString> }
Return { value: ValueId }
Block { block: BlockId }
```

The `Block` type:

```rust
pub struct Block {
    pub safety: BlockSafety,  // Safe or Unsafe
    pub elements: Vec<BlockElement>,
}

pub enum BlockElement {
    Expr(ValueId),
    Local(LocalVariableId),
}
```

### Call Values

```rust
Call { callee: ValueId, args: Arguments<ValueId> }
MethodCall { object: ValueId, method_name: NString, args: Arguments<ValueId> }
```

### Symbol Values

```rust
FunctionSymbol { id: FunctionId }
GlobalVariableSymbol { id: GlobalVariableId }
LocalVariableSymbol { id: LocalVariableId }
ParameterSymbol { id: ParameterId }
```

### Special Values

```rust
Assign { place: ValueId, value: ValueId }
Cast { value: ValueId, target_type: TypeId }
Borrow { exclusive: bool, mutable: bool, place: ValueId }
```

## Item Definitions

### Function

Defined in `item.rs`:

```rust
pub struct Function {
    pub visibility: Visibility,                   // Pub, Pro, Sec
    pub attributes: BTreeSet<FunctionAttribute>,  // NoMangle, CVariadic, ExternAbi
    pub name: NString,                            // User-visible name
    pub mangled_name: NString,                    // LLVM linkage name
    pub generics: Option<BTreeMap<NString, Option<TypeId>>>,  // Generic params with optional defaults
    pub params: Vec<ParameterId>,                 // Parameters (by handle)
    pub return_type: TypeId,                      // Return type
    pub body: Option<Vec<BlockElement>>,          // None = declaration (extern/trait method)
}
```

The `get_type()` method constructs the `FunctionType` from params:

```rust
pub fn get_type(&self) -> FunctionType {
    let params: Vec<(NString, TypeId)> = self.params.iter()
        .map(|param_id| { let p = param_id.borrow(); (p.name.clone(), p.ty) })
        .collect();
    FunctionType {
        attributes: self.attributes.clone(),
        params: params.into(),
        return_type: self.return_type,
    }
}
```

### StructDef

```rust
pub struct StructDef {
    pub visibility: Visibility,
    pub name: NString,
    pub attributes: BTreeSet<StructAttribute>,  // Packed
    pub fields: BTreeMap<NString, StructField>,
    pub generics: Option<BTreeMap<NString, Option<TypeId>>>,
    pub layout: StructLayout,  // ThinVec<StructMemoryLayoutCell>
}
```

The `StructAttribute` enum has one variant: `Packed` (no padding between fields).

The layout is a sequence of cells:

```rust
pub enum StructMemoryLayoutCell {
    Field { field_name: NString },
    Padding(NonZeroUsize),  // Explicit padding bytes
}

pub type StructLayout = ThinVec<StructMemoryLayoutCell>;
```

### EnumDef

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

### Trait

```rust
pub struct Trait {
    pub visibility: Visibility,
    pub name: NString,
    pub generics: Option<BTreeMap<NString, Option<TypeId>>>,
    pub supertraits: Vec<TraitId>,
    pub where_clause: Option<Vec<WhereClause>>,
    pub methods: Vec<FunctionId>,          // Method declarations (no bodies)
    pub associated_types: Vec<NString>,    // Associated type names
    pub associated_constants: Vec<NString>, // Associated constant names
}
```

Where clauses:

```rust
pub struct WhereClause {
    pub type_id: TypeId,
    pub bounds: Vec<TypeBound>,
}
```

### Module

```rust
pub struct Module {
    pub visibility: Visibility,
    pub name: NString,
    pub attributes: BTreeSet<ModuleAttribute>,
    pub items: Vec<Item>,
}

pub enum Item {
    Module(ModuleId),
    GlobalVariable(GlobalVariableId),
    Function(FunctionId),
    TypeAliasDef(TypeAliasDefId),
    StructDef(StructDefId),
    EnumDef(EnumDefId),
    Trait(TraitId),
}
```

### LocalVariable

```rust
pub struct LocalVariable {
    pub kind: LocalKind,  // Let, Var, Static
    pub attributes: BTreeSet<LocalVariableAttribute>,
    pub is_mutable: bool,
    pub name: NString,
    pub ty: TypeId,
    pub initializer: ValueId,
}

pub enum LocalKind {
    Let,     // Immutable binding
    Var,     // Mutable binding
    Static,  // Static local
}
```

### Parameter

```rust
pub struct Parameter {
    pub attributes: BTreeSet<ParameterAttribute>,
    pub is_mutable: bool,
    pub name: NString,
    pub ty: TypeId,
    pub default_value: Option<ValueId>,
}
```

### GlobalVariable

```rust
pub struct GlobalVariable {
    pub visibility: Visibility,
    pub attributes: BTreeSet<GlobalVariableAttribute>,  // NoMangle
    pub is_mutable: bool,
    pub name: NString,
    pub mangled_name: NString,
    pub ty: TypeId,
    pub initializer: ValueId,
}
```

## HIR Pass Infrastructure

Defined in `pass.rs`:

```rust
pub trait Pass<T> {
    fn run(&mut self, input: T) -> T;
}

pub struct PassManager<T> {
    passes: Vec<Box<dyn Pass<T>>>,
}

impl<T> PassManager<T> {
    pub fn new() -> Self { Self { passes: Vec::new() } }
    pub fn add_pass(&mut self, pass: Box<dyn Pass<T>>) { self.passes.push(pass); }
    pub fn run(&mut self, input: T) -> T {
        let mut data = input;
        for pass in &mut self.passes {
            data = pass.run(data);
        }
        data
    }
}
```

## Lowering: AST to HIR

**Crate**: `nitrate_hir_from_tree`

The lowerer transforms the resolved parse tree into HIR. Key files:

- **`context.rs`**: `LoweringContext` — tracks scope chains, current function, type environment
- **`item.rs`**: `LowerItem` trait — lowers `Function`, `StructDef`, `EnumDef`, `Trait`, `TypeAliasDef`, `Module`, `GlobalVariable`
- **`expr.rs`**: `LowerExpr` trait — lowers expression AST nodes to `Value` nodes
- **`ty.rs`**: Lower type expressions to interned `Type` nodes
- **`lower.rs`**: Orchestration — organizes the pass sequence
- **`diagnosis.rs`**: Lowering-specific error types

During lowering:

1. **Types are interned immediately**: `Type::from(parsed_type)` calls `store.store_type(ty)` which deduplicates
2. **Items are stored immediately**: Each item declaration becomes a `FunctionId`, `StructDefId`, etc.
3. **Generics are preserved**: `Type::GenericParam` is created for each declared generic parameter
4. **Inferred variables are created**: `let x = ...` without a type annotation creates `Type::Inferred`

## Type Determination: HirGetType

**Crate**: `nitrate_hir_get_type`

The `HirGetType` trait defined in `get_type.rs`:

```rust
pub trait HirGetType {
    fn determine_type(&self, ctx: &SymbolTab) -> Result<Type, TypeInferenceError>;
}
```

Implemented for `Lit`, `Block`, and `Value`. The `Value` implementation handles all 30+ variants:

- **Literals** → their concrete type
- **`InferredInteger`/`InferredFloat`** → returns the inferred variant (solver resolves later)
- **`StringLit(s)`** → `Type::Array { element_type: U8, len: s.len() }`
- **`BStringLit(v)`** → `Type::Array { element_type: U8, len: v.len() }`
- **`StructObject`** → `Type::Struct { def: struct_def }`
- **`EnumVariant`** → the variant's declared type (looked up from the enum definition)
- **`Binary`** → left operand's type for arithmetic, `Bool` for comparisons/logical
- **`Unary`** → operand's type
- **`IndexAccess`** → element type from Array/SliceRef/SlicePtr; falls back to trait method `index`
- **`FieldAccess`** → field's declared type from struct definition
- **`Deref`** → the pointed-to type
- **`Borrow`** → `Type::Reference { to: place_type }`
- **`List`** → `Type::Array { element_type, len }`
- **`Tuple`** → `Type::Tuple { element_types }`
- **`If`** → true branch type (or `Unit` if no false branch)
- **`While`/`Loop`** → `Unit`
- **`Break`/`Continue`/`Return`** → `Never`
- **`Call`** → callee's return type
- **`MethodCall`** → method's return type (looked up from symbol table)
- **`FunctionSymbol`** → `Type::Function { function_type }`
- **`GlobalVariableSymbol`/`LocalVariableSymbol`/`ParameterSymbol`** → declared type

Type inference errors:

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

## HIR Dump

**Crate**: `nitrate_hir_dump`

Provides pretty-printing for debugging:

- `dump_item.rs`: Item formatting
- `dump_expr.rs`: Expression formatting
- `dump_ty.rs`: Type formatting
- `dump.rs`: Top-level dispatch

## Design Rationale

### Why Thread-Local Store Instead of Global State?

1. **Testability**: Each test creates a fresh Store — no shared state between tests
2. **Deterministic teardown**: `using_storage` uses RAII — the store is available exactly when needed
3. **No global locks**: Single-thread access eliminates synchronization overhead
4. **Explicit visibility**: The store is set at the pipeline entry and visible in the call stack

### Why Both Dedup and Non-Dedup Stores?

- **Dedup stores** (`TypeStore`, `ExprLiteralStore`): Types are compared structurally millions of times during compilation. Deduplication makes `TypeId::eq` equivalent to pointer equality — O(1) instead of O(n) structural comparison.
- **Non-dedup stores** (`FunctionStore`, `ValueStore`): These items are mutated by the solver. A BiMap would be invalidated by mutations. Append-only + RefCell provides the mutation flexibility needed.

### Why `RefCell` Instead of `&mut Store`?

The solver pattern requires **iterating over existing items while creating new ones** (during monomorphization):

- Solver visits all block elements
- If a generic call is found, the solver clones the function and stores it
- Storing inserts into the FunctionStore's AppendOnlyVec (requires `&self`, not `&mut self`)
- Meanwhile, the solver is still holding references to other items

`RefCell` provides interior mutability without requiring exclusive `&mut` access to the entire store.

### Why Separate Stores Instead of One Big `Vec<Box<dyn Any>>`?

Each sub-store has specialized access patterns:

- `TypeStore` uses BiMap + RwLock (read-optimized, concurrent)
- `FunctionStore` uses AppendOnlyVec (append-only, never invalidates)
- `ExprValueStore` is append-only like functions

Separating them also enables future parallelism: different passes can access different sub-stores concurrently.
