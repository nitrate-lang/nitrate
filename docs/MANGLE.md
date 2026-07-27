# Name Mangling Subsystem

## Overview and Purpose

Name mangling translates high-level symbol names — function names, variable names, type names — into unique, deterministic, low-level strings suitable for use as LLVM linkage names and linker symbols. This translation is necessary for three fundamental reasons:

First, **overloaded functions and generic instantiations**: Nitrate supports function overloading and generic functions. Without mangling, two different functions named `foo` with different parameter types would produce the same linker symbol, causing a symbol collision. The mangler encodes the type signature into the name, ensuring that each unique function signature gets a unique linker symbol.

Second, **module qualification**: Functions defined in different modules or packages can share the same name. The mangler encodes the package name and module path, ensuring that `bar::foo` and `baz::foo` produce distinct linker symbols that the linker can distinguish.

Third, **monomorphized generics**: When a generic function is instantiated with concrete type arguments (e.g., `identity::<i32>` and `identity::<String>`), each instantiation must have a unique linker symbol. The mangler encodes the concrete type arguments, ensuring that different generic instantiations produce distinct symbols that the linker can resolve independently.

## Architecture

**Crate**: `nitrate_hir_mangle`  
**Key files**: `lib.rs` (public API), `mangle.rs` (core algorithm), `string.rs` (encoding helpers), `ty.rs` (type encoding)

## The Mangling Algorithm

The core function `mangle_name` takes three inputs and produces one output:

```
mangle_name(package_name: &str, name: &str, type_: &Type) -> NString
```

The output is an `NString` (interned string) containing the mangled name, stored in `Function::mangled_name` and `GlobalVariable::mangled_name` during HIR lowering.

## Mangled Name Format

Mangled names follow a compact encoding:

```
_<package>_<name>_<type_hash>
```

For generic instantiations, a monomorphization suffix is appended:

```
_<package>_<name>_<type_hash>_mono_<N>
```

Where `<N>` is the monomorphization counter assigned by the Solver.

## Type Encoding Scheme

Each HIR type maps to a compact string encoding: `Unit`→`v`, `Bool`→`b`, `U8`→`u8`, `I32`→`i32`, `F64`→`f64`, `Array(T,N)`→`A{<T>}N`, `Tuple(T1,T2)`→`T{<T1>}{<T2>}`, `Struct(S)`→`S{<name>}`, `Function(params→ret)`→`F{<params>}{<ret>}`, and so on. The encoding is recursive — compound types encode their inner types using the same scheme.

## Integration

The mangler is invoked during HIR lowering for each function and global variable. During monomorphization, new mangled names are generated for each instantiation using the `mono-N` counter suffix. The LLVM codegen uses the `mangled_name` field when creating LLVM functions. Functions with the `NoMangle` attribute bypass mangling entirely, preserving the original name for FFI compatibility.

## Design Rationale

Type information is encoded in mangled names to prevent symbol collisions (two `add` functions with different parameter types get different names) and to enable generic instantiation (each unique type argument combination produces a distinct name). The monomorphization counter ensures uniqueness even when type encoding produces identical strings for different types.
