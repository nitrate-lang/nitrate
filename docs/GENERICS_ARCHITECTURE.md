# Generics and Monomorphization Architecture

## Overview

Nitrate implements generics via **monomorphization** — the same fundamental approach as C++ templates and Rust generics. A generic declaration — whether a function, struct, enum, or type alias — serves as a template. When used with concrete type arguments at call sites or construction sites, the compiler creates a specialized copy of the declaration with all generic parameters substituted with the provided concrete types. This approach produces zero runtime overhead because each monomorphized copy is ordinary concrete code that the optimizer can analyze and optimize fully.

### Key Distinction: Two Kinds of Type Variable

The HIR has two type-variable mechanisms with fundamentally different lifetimes and resolution strategies:

| Mechanism            | Purpose                               | Created By                            | Resolved By                                     |
| -------------------- | ------------------------------------- | ------------------------------------- | ----------------------------------------------- |
| `Type::Inferred`     | Local type inference (`let x = ...`)  | HIR lowering for unannotated bindings | Constraint propagation within the same function |
| `Type::GenericParam` | User-declared generic (`fn foo<T>()`) | HIR lowering based on source generics | Monomorphization substitution at each call site |

`Inferred` variables die when their containing function is solved — they are purely local to the inference process. `GenericParam` variables persist until a caller provides concrete types, potentially across compilation unit boundaries.

## Pipeline Overview

The generics pipeline flows through the compilation stages as follows:

```
Source Code
    │
    ▼ [AST Parser]
Produces Generics { params: [TypeParam] } — captures the syntactic generic declarations
    │
    ▼ [HIR Lowering]
Creates Type::GenericParam for each declared parameter
Creates Type::Parameterized for each type-with-arguments usage
    │
    ▼ [Hindley-Milner Solver + Monomorphization]
The core pass that performs both inference and instantiation:
  • Walks each function body
  • When it finds a Call to a generic function:
    1. Infers concrete type args from argument types via unification
    2. Clones the function body, substituting GenericParam → concrete types
    3. Registers the concrete copy in the symbol table
    4. Redirects the call site to the copy
    │
    ▼ [LLVM Codegen]
Skips generic (uninstantiated) functions — they have no concrete code to emit
Compiles only the concrete monomorphized copies
```

## The Type System for Generics

### Two Key Variants

**`Type::GenericParam { index, name }`** marks the declaration site of a generic parameter. Appears in parameter types, field types, return types, and local variable types wherever the user wrote the parameter name. The `index` identifies the parameter's position in the generic parameter list (0-based), and `name` is the user-visible name used in error messages.

**`Type::Parameterized { base, args }`** marks a use site where a generic type is applied to concrete arguments (e.g., `Option<i32>`). Stores the base type ID and the argument list (both positional and named).

### Where Generics Live

Four item types carry a `generics` field:

- `Function` — generic function parameters
- `StructDef` — generic struct parameters
- `EnumDef` — generic enum parameters
- `TypeAliasDef` — generic type-alias parameters

When `None`, the item has no generics. When `Some(map)`, each key is a parameter name and each value is its optional default type.

## Monomorphization Mechanics

### Function Monomorphization

When `monomorphize_function` is called, the solver:

1. **Clones** the generic `Function` struct including all params, body elements, and types
2. **Substitutes**: Walks every type in the clone, replacing `GenericParam { index, .. }` with the concrete type from the substitution map
3. **Names**: Generates a unique name (`original::<mono-N>`) using a monotonically increasing counter
4. **Stores**: Stores the new function in the Store and registers in the symbol table
5. **Caches**: Inserts into `mono_cache` to deduplicate future instantiations of the same generic with the same type arguments

### Struct Monomorphization

Generic struct instantiation follows a similar pattern but requires special handling:

1. **Detection**: When visiting a `StructObject` value, check if the `struct_def` has generics
2. **Inference**: For each field, match the field value's concrete type against the field's declared type (which may contain `GenericParam`)
3. **Substitution**: Create a substitution map from the inferred type arguments
4. **Rebuild layout**: Compute the memory layout for the monomorphized struct (fields may change size with different type arguments)
5. **Storage**: Register the monomorphized struct in the symbol table

### The Monomorphization Cache

```rust
mono_cache: HashMap<(usize, Vec<(u32, TypeId)>), FunctionId>
```

Key = `(original_function_store_index, [(param_index, concrete_type), ...])`

Before monomorphizing, the solver checks the cache. If an identical instantiation was already created, the existing copy is reused. This prevents duplicate function definitions (code bloat), recursive infinite monomorphization, and redundant compilation work.

## Integration Requirements

For monomorphization to work end-to-end, three integration points must be maintained:

1. **HIR Lowering**: The lowerers must not reject generic constructs. They should accept syntax like `Foo<i32>` and `impl<T> ...` even if the type arguments aren't immediately processed — monomorphization handles them later.

2. **Symbol Table Registration**: Monomorphized functions must be added to the symbol table via `add_function()`. Otherwise, LLVM codegen won't iterate them and the call site will reference an undefined symbol.

3. **LLVM Codegen**: The codegen must skip any function whose `generics` field is non-empty. Only concrete monomorphized copies should produce LLVM IR.

## Design Rationale

**Why monomorphization at the HIR level instead of LLVM?** LLVM has no template system. Generating LLVM IR with generic placeholders would require runtime dispatch or JIT compilation, both of which defeat monomorphization's purpose of zero runtime overhead.

**Why separate `GenericParam` from `Inferred`?** They have different lifetimes and resolution strategies. `Inferred` is solved by constraint propagation within a single function. `GenericParam` persists until a caller instantiates the generic. Using one variant for both caused confusion and prevented clean separation of the two resolution strategies.

**Why the monomorphization cache?** Without deduplication, each call site of `identity::<i32>(x)` would produce a separate monomorphized copy. The cache ensures all call sites with identical type arguments share a single copy, reducing code size and compilation time.

**Why fixed-point iteration for generics?** Nested generics require multiple passes. For example, `map(list, fn(x) -> identity(x))` where both `map` and `identity` are generic: the solver first monomorphizes `identity<i32>`, then monomorphizes `map<List<i32>, fn(i32) -> i32>`. Without fixed-point iteration, the inner generic would not yet exist when the outer generic is processed.
