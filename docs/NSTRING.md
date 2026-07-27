# Interned String System (NString)

## Overview and Motivation

The `NString` type is Nitrate's interned string representation, used throughout the compiler for identifiers, names, and other frequently repeated strings. String interning is a memory optimization technique where each unique string value is stored exactly once in a global pool. Subsequent references to the same string content reuse the existing entry rather than allocating new storage. This provides three critical benefits for a compiler:

First, **memory efficiency**: Compilers deal with a vast number of symbolic names — function names, variable names, type names, module paths, parameter names, field names, and more. Many of these names are repeated many times throughout the compilation process (every reference to a function uses its name string). Without interning, each occurrence would allocate its own `String` on the heap, leading to significant memory waste. With interning, each unique string is stored once and all references share that single allocation.

Second, **comparison speed**: String comparison is normally O(n) where n is the length of the strings being compared. In a compiler, strings are compared millions of times during name resolution, type checking, and symbol table lookups. With interning, comparison reduces to O(1) integer handle comparison — we compare the interned handles rather than the string contents.

Third, **hash table performance**: Interned strings provide stable, precomputed hash values. When used as keys in `HashMap`s and `BTreeMap`s (which happens extensively in symbol tables and the Store), interned strings avoid recomputing the hash on every lookup and provide fast equality checks.

## Architecture

**Crate**: `nitrate_nstring`  
**Key type**: `NString`  
**Key files**: `src/translation/src/nstring/src/lib.rs`, `src/translation/src/nstring/src/nstring.rs`

## Design and Implementation

The `NString` type wraps an interned string handle. It implements `Deref<Target = str>`, `Clone`, `Eq`, `Ord`, `Hash`, and `Serialize`/`Deserialize`, making it a drop-in replacement for `String` in most compiler data structures.

```rust
// NString wraps an interned string handle
pub struct NString(/* internal handle */);
```

The intern pool is typically backed by a global or thread-local `HashMap<String, Handle>` that maps string content to compact integer handles. When `NString::from(s)` is called:

1. The pool is checked for an existing entry matching the string content `s`
2. If an entry exists, the existing handle is returned (no new allocation)
3. If no entry exists, a new handle is allocated, the string is stored in the pool, and the new handle is returned

The handle is then used for all subsequent operations. Handles are typically small integers (u32-sized), enabling compact storage in `HashMap` keys, `ThinVec` elements, and `BTreeSet` entries.

## Properties

- **Immutability**: Once created, an `NString` cannot be modified. This is essential for safe sharing — if the string content could change, all references would be invalidated.
- **Deduplication**: Identical strings always produce the same `NString` value with the same handle. This enables pointer-equality comparison.
- **Efficient comparison**: `nstring == other` is O(1) — it compares the underlying handles rather than the string bytes.
- **Efficient hashing**: `nstring.hash()` is O(1) — it hashes the handle value rather than the string bytes.
- **Low memory**: Each unique string is stored once regardless of how many times it appears in the compiler's data structures.
- **Thread-safe**: The pool uses synchronization (typically a `RwLock` or thread-local storage) to ensure safe concurrent access.

## Operations

```rust
// Creation — interning a string
let s: NString = "hello".into();
let s2 = NString::from("hello");

// Accessing the string content
let content: &str = &*s;           // Via Deref
let content = s.as_str();          // Explicit method

// Comparison (O(1))
assert_eq!(s, s2);                 // Handle comparison
assert_eq!(&*s, "hello");          // Content comparison

// Hashing (O(1))
let hash = s.hash();               // Based on handle value

// Storage in collections
let mut map: HashMap<NString, TypeId> = HashMap::new();
map.insert(s, some_type_id);

// Cloning (cheap — handle copy)
let s3 = s.clone();
```

## Compiler Integration

`NString` is used pervasively throughout every subsystem of the compiler. Here is a comprehensive catalog of its usage locations across the HIR types:

### In Type Definitions (ty.rs)

```rust
// Generic parameter names — the user-visible name of each generic param
GenericParam { index: u32, name: NString }

// Inferred type variable names — optional debug name
Inferred { id: NonZeroU32, name: Option<NString> }

// Function type parameters — (parameter_name, parameter_type) pairs
FunctionType { params: ThinVec<(NString, TypeId)>, ... }

// Extern ABI names — the ABI specification string
ExternAbi { name: NString }
```

### In Item Definitions (item.rs)

```rust
// Function names and mangled names
Function { name: NString, mangled_name: NString, ... }

// Struct definitions and field names
StructDef { name: NString, fields: BTreeMap<NString, StructField>, ... }
StructField { name: NString, ... }
StructMemoryLayoutCell::Field { field_name: NString }

// Enum definitions and variant names
EnumDef { name: NString, variants: ThinVec<EnumVariant>, ... }
EnumVariant { name: NString, ... }

// Trait definitions
Trait { name: NString, associated_types: Vec<NString>, associated_constants: Vec<NString>, ... }

// Module names
Module { name: NString, ... }

// Type alias names
TypeAliasDef { name: NString, ... }

// Variable and parameter names
GlobalVariable { name: NString, mangled_name: NString, ... }
LocalVariable { name: NString, ... }
Parameter { name: NString, ... }
```

### In Expression Definitions (expr.rs)

```rust
// Field access and method call targets
FieldAccess { expr: ValueId, field_name: NString }
MethodCall { object: ValueId, method_name: NString, args: ... }

// Enum variant construction
EnumVariant { enum_def: EnumDefId, variant: NString, value: ValueId }

// Named arguments to function calls
Arguments<T> { positional: ThinVec<T>, named: ThinVec<(NString, T)> }

// Loop labels for break/continue
Break { label: Option<NString> }
Continue { label: Option<NString> }
```

### In the Symbol Table

```rust
pub struct SymbolTab {
    functions: HashMap<NString, FunctionId>,
    named_types: HashMap<NString, TypeOrDef>,
    globals: HashMap<NString, GlobalVariableId>,
    methods: HashMap<TypeId, HashMap<NString, FunctionId>>,
    ...
}
```

## Performance Considerations

String interning provides significant performance benefits for the compiler:

- **Symbol table lookups**: Each name resolution requires checking the symbol table for the given name. With `NString` keys, these lookups use O(1) hash and O(1) equality, making name resolution fast even for large codebases with thousands of symbols.
- **Type comparison**: Generic parameter names are `NString`s, enabling fast comparison during type unification and substitution.
- **Method dispatch**: Method lookups in the symbol table use `NString` keys for method names, enabling fast dispatch to the correct method implementation.
- **Serialization**: `NString` implements `Serialize`/`Deserialize` from serde, enabling the compiler state to be serialized for incremental compilation and persistence.

## Comparison to Alternatives

Compared to using Rust's standard `String` type directly, `NString` provides:

- **O(1) comparison** vs O(n) for `String`
- **O(1) hashing** vs O(n) for `String`
- **Memory sharing** (one allocation per unique string) vs separate allocation per occurrence
- **Cheap cloning** (handle copy) vs full string copy and allocation

The tradeoff is a small overhead for the intern pool lookups during `NString::from()` creation. However, since most strings are created once during HIR lowering and then referenced many times afterward, this one-time cost is negligible compared to the ongoing savings in comparison and memory.
