# Name Mangling Subsystem

## Overview and Purpose

Name mangling is the process of translating high-level symbol names (function names, variable names, type names) into unique, deterministic, low-level strings suitable for use as LLVM linkage names and linker symbols. This translation is necessary because:

First, **overloaded functions and generic instantiations**: Nitrate supports function overloading and generic functions. Without mangling, two different functions named `foo` but with different parameter types would produce the same linker symbol, causing a symbol collision. The mangler encodes the type signature into the name, ensuring each unique function signature gets a unique linker symbol.

Second, **module qualification**: Functions defined in different modules or packages can have the same name. The mangler encodes the package name and module path, ensuring that `bar::foo` and `baz::foo` produce distinct linker symbols.

Third, **monomorphized generics**: When a generic function is instantiated with concrete type arguments (e.g., `identity::<i32>` and `identity::<String>`), each instantiation must have a unique linker symbol. The mangler encodes the concrete type arguments, ensuring that different generic instantiations produce distinct symbols.

## Architecture

**Crate**: `nitrate_hir_mangle`  
**Key types**: Mangling functions  
**Key files**: `src/translation/src/hir_mangle/src/lib.rs` (public API), `src/translation/src/hir_mangle/src/mangle.rs` (core algorithm), `src/translation/src/hir_mangle/src/string.rs` (string encoding helpers), `src/translation/src/hir_mangle/src/ty.rs` (type encoding)

## The Mangling Algorithm

The core mangling function `mangle_name` takes three inputs and produces one output:

```
mangle_name(package_name: &str, name: &str, type_: &Type) -> NString
```

### Input Parameters

1. **`package_name`**: The name of the package containing the symbol. This ensures that symbols from different packages have distinct linkage names even if they have the same source-level name.
2. **`name`**: The user-visible name of the symbol. This is the name as declared in source code (e.g., `compute`, `Point`, `identity`).
3. **`type_`**: The `Type` of the symbol. For functions, this is a `Type::Function` containing the parameter types and return type. For other symbols, this is the symbol's declared type.

### Output

The output is an `NString` (interned string) containing the mangled name, which is stored in `Function::mangled_name` and `GlobalVariable::mangled_name` during HIR lowering. The LLVM codegen uses these names when creating LLVM functions and global variables.

## Mangled Name Format

Mangled names follow a compact encoding designed to be both human-readable and collision-free:

```
_<package>_<name>_<type_hash>
```

For generic instantiations, a monomorphization suffix is appended:

```
_<package>_<name>_<type_hash>_mono_<N>
```

Where `<N>` is the monomorphization counter (a monotonically increasing integer assigned by the Solver).

### Example Mangled Names

| Source Declaration                                     | Mangled Name                             |
| ------------------------------------------------------ | ---------------------------------------- |
| `fn compute(x: i32) -> i32` in package `myapp`         | `_myapp_compute_F_i32_i32`               |
| `struct Point` in package `geometry`                   | `_geometry_Point_S`                      |
| `fn identity<T>(x: T) -> T` instantiated with `i32`    | `_myapp_identity_F_i32_i32_mono_1`       |
| `fn identity<T>(x: T) -> T` instantiated with `String` | `_myapp_identity_F_string_string_mono_2` |

## Type Encoding Scheme

Each HIR type is encoded as a compact string representation:

| HIR Type                  | Mangled Encoding     | Notes                   |
| ------------------------- | -------------------- | ----------------------- |
| `Unit`                    | `v`                  | Void/unit type          |
| `Bool`                    | `b`                  | Boolean                 |
| `U8`                      | `u8`                 |                         |
| `U16`                     | `u16`                |                         |
| `U32`                     | `u32`                |                         |
| `U64`                     | `u64`                |                         |
| `U128`                    | `u128`               |                         |
| `I8`                      | `i8`                 |                         |
| `I16`                     | `i16`                |                         |
| `I32`                     | `i32`                |                         |
| `I64`                     | `i64`                |                         |
| `I128`                    | `i128`               |                         |
| `F32`                     | `f32`                |                         |
| `F64`                     | `f64`                |                         |
| `USize`                   | `u` or `us`          | Pointer-width dependent |
| `Array(T, N)`             | `A{<T>}N`            | Array encoding          |
| `Tuple(T1, T2)`           | `T{<T1>}{<T2>}`      | Tuple encoding          |
| `Struct(S)`               | `S{<name>}`          | Struct by name          |
| `Enum(E)`                 | `E{<name>}`          | Enum by name            |
| `Function(params -> ret)` | `F{<params>}{<ret>}` | Function signature      |
| `Reference(T)`            | `R{<T>}`             | Reference               |
| `Pointer(T)`              | `P{<T>}`             | Pointer                 |
| `SliceRef(T)`             | `RS{<T>}`            | Slice reference         |
| `SlicePtr(T)`             | `PS{<T>}`            | Slice pointer           |

The encoding is recursive: compound types encode their inner types using the same scheme. For example, `&[i32]` (a reference to a slice of i32) would be encoded as `RS{i32}`.

## Integration into the Compiler Pipeline

1. **During HIR Lowering**: The `nitrate_hir_from_tree` crate calls `mangle_name()` for each function and global variable it lowers, storing the result in the `mangled_name` field of the `Function` or `GlobalVariable` struct.

2. **During Monomorphization**: When the solver creates a monomorphized copy of a generic function, it generates a new mangled name using the `mono-<N>` counter suffix. This ensures each unique instantiation has a distinct linkage name.

3. **During LLVM Codegen**: The `generate_llvmir()` function in `symbol.rs` uses the `mangled_name` field when creating LLVM functions:

```rust
let llvm_function = ctx.module.add_function(
    &hir_function.mangled_name,  // Use mangled name as LLVM symbol name
    llvm_fn_type,
    None,
);
```

4. **NoMangle Attribute**: Functions with the `NoMangle` function attribute bypass mangling entirely. The original user-provided name is used as the LLVM linkage name. This is essential for FFI functions that must have specific symbol names (e.g., `main`, `malloc`, exported C API functions).

## Design Rationale

### Why Encode Type Information in Mangled Names?

Encoding type information in the mangled name serves two purposes:

1. **Preventing symbol collisions**: Two functions named `add` that take different parameter types (e.g., `add(i32, i32) -> i32` vs `add(f64, f64) -> f64`) would produce the same object file symbol without mangling. The type encoding makes them distinct (`add_F_i32_i32_i32` vs `add_F_f64_f64_f64`).

2. **Enabling generic instantiation**: Monomorphized copies of generic functions must have distinct names. The mangler encodes the concrete type arguments into the name, ensuring that `identity::<i32>` and `identity::<String>` get different linkage names.

### Why the Monomorphization Counter?

The monomorphization counter (`mono_N`) ensures uniqueness even when the type encoding produces the same string for different types (due to type aliases or structural equivalence of different nominal types). The counter provides a deterministic, incrementally increasing identifier that guarantees each monomorphized copy has a unique name.
