# Name Mangling Subsystem

## Overview and Purpose

Name mangling translates high-level symbol names — function names, global variable names — into unique, deterministic, low-level strings suitable for use as LLVM linkage names and linker symbols. This translation is necessary for three fundamental reasons:

First, **overloaded functions and generic instantiations**: Nitrate supports function overloading and generic functions. Without mangling, two different functions named `foo` with different parameter types would produce the same linker symbol, causing a symbol collision. The mangler encodes the type signature into the name, ensuring that each unique function signature gets a unique linker symbol.

Second, **module qualification**: Functions defined in different modules or packages can share the same name. The mangler encodes the package name and module path, ensuring that `bar::foo` and `baz::foo` produce distinct linker symbols that the linker can distinguish.

Third, **monomorphized generics**: When a generic function is instantiated with concrete type arguments (e.g., `identity::<i32>` and `identity::<String>`), each instantiation must have a unique linker symbol. The mangler encodes the concrete type arguments, ensuring that different generic instantiations produce distinct symbols that the linker can resolve independently.

## Architecture

**Crate**: `nitrate_hir_mangle`  
**Key files**: `lib.rs` (public API), `mangle.rs` (name-level mangling/demangling), `string.rs` (string encoding with compression), `ty.rs` (type encoding), `pass.rs` (symbol-table pass)

## Mangled Name Format

Mangled symbol names follow the format:

```
_N<package><name><type>
```

Where:

- `_N` is the fixed Nitrate mangling prefix, used to distinguish Nitrate-mangled symbols from externally-defined (C ABI) symbols.
- `<package>` is the package name, encoded as a mangled string.
- `<name>` is the symbol's name (with the package qualifier stripped), encoded as a mangled string.
- `<type>` is the type encoding (for functions, this includes the full function signature).

All mangled output consists **only** of characters from the C99 identifier charset `[A-Za-z0-9_]` (no dollar signs, no dots, no hyphens). This guarantees the mangled names are valid identifiers on every C99-compliant linker and assembler.

Example: a function `add(a: i32, b: i32) -> i32` in package `test-mangle` might mangle to:

```
_N1_x22_746573742d6d616e676c651_3addW2_LLL
```

### String Encoding

Each string (package name, symbol name, path segments) is encoded as:

```
<segment_count>_<segment_1><segment_2>...<segment_n>
```

Where the string is split on `::` separators and each segment is encoded independently:

| Segment kind         | Encoding format                  |
| -------------------- | -------------------------------- |
| Valid C99 identifier | `<len><segment>`                 |
| Other UTF-8 text     | `x<hex_len>_<hex_bytes>`         |
| Long segment         | `z<base63_len>_<base63_payload>` |

- **Valid C99 identifiers** are length-prefixed directly: `4main` encodes `main`.
- **Non-C99 text** (any UTF-8, including `λ`, `foo-bar`, `foo.bar`) is hex-encoded with a length prefix: `x4_cebb` encodes `λ`.
- **Long segments** (encoding exceeds 64 bytes) are DEFLATE-compressed, then the compressed bytes are encoded with a base-63 encoding using the C99 alphabet (`[A-Za-z0-9_]`). The base-63 payload is length-prefixed: `z<base63_len>_<base63_payload>`. Compression is only used if it actually reduces the encoded size.

The string encoding is **self-delimiting**: given a cursor into a byte stream, `demangle_string` can determine exactly how many bytes belong to the string, allowing strings to be safely concatenated with type encodings in a full symbol name.

### Type Encoding

Each HIR type variant maps to a compact, C99-safe, **self-delimiting** string encoding:

| Type            | Encoding                          |
| --------------- | --------------------------------- |
| `Never`         | `A`                               |
| `Unit`          | `B`                               |
| `Bool`          | `C`                               |
| `U8`            | `D`                               |
| `U16`           | `E`                               |
| `U32`           | `F`                               |
| `U64`           | `G`                               |
| `U128`          | `H`                               |
| `USize`         | `I`                               |
| `I8`            | `J`                               |
| `I16`           | `K`                               |
| `I32`           | `L`                               |
| `I64`           | `M`                               |
| `I128`          | `N`                               |
| `F32`           | `O`                               |
| `F64`           | `P`                               |
| `Array`         | `Q<element><len>_`                |
| `Tuple`         | `R<count>_<elements...>`          |
| `Struct`        | `S<name>`                         |
| `Enum`          | `T<name>`                         |
| `TypeAlias`     | `U<name>`                         |
| `Refine`        | `V<base>`                         |
| `Function`      | `W<param_count>_<params...><ret>` |
| `Reference`     | `X<flags><to>`                    |
| `SliceRef`      | `Y<flags><element>`               |
| `Pointer`       | `Z<flags><to>`                    |
| `SlicePtr`      | `a<flags><element>`               |
| `TraitObject`   | `b`                               |
| `Parameterized` | `c<base><arg_count>_<args...>`    |
| `GenericParam`  | `d<index>_`                       |
| `Inferred`      | `e<id>_`                          |
| `InferredFloat` | `f`                               |
| `InferredInt`   | `g`                               |

Where:

- `<name>` is a mangled string (self-delimiting).
- `<len>`, `<count>`, `<index>`, `<id>` are decimal numbers followed by `_`.
- `<flags>` is a single character encoding reference/pointer mutability and exclusivity: `0`=shared-immutable, `1`=shared-mutable, `2`=exclusive-immutable, `3`=exclusive-mutable.

The type encoding is **fully self-delimiting**: `demangle_type` can parse a type from a byte cursor and know exactly where it ends, allowing types to be safely nested (e.g. `Array` of `Tuple` of primitives) and concatenated.

Function parameter names and generic parameter names are deliberately **not** encoded in the mangled type signature — only types and indices matter for symbol identity.

## Demangling

The crate provides complete demangling support:

- `demangle_name(mangled: &str) -> Result<(String, String, Type), ()>` — returns `(package_name, name, type)`.
- `demangle_module(mangled: &str) -> Result<String, ()>` — decodes a module name.
- `demangle_string(input: &mut &[u8]) -> Result<String, ()>` — decodes a mangled string (consuming exactly the encoded bytes).
- `demangle_type(input: &mut &[u8]) -> Result<Type, ()>` — decodes a type encoding (consuming exactly the encoded bytes).

Demangling is lossless for the encoded fields (package, name, type structure). Note that parameter names and generic parameter names are not recoverable from the mangled form — they are intentionally excluded for compactness.

## Symbol Table Pass

The `mangle_symbols(package_name, &mut SymbolTab)` pass runs after HIR solving and validation, before LLVM codegen. It populates `Function::mangled_name` and `GlobalVariable::mangled_name` for every symbol in the table:

- **Functions**: The function type signature is mangled with the package name and name. The `name` field is preserved as the internal symbol lookup key.
- **Global variables**: The global's type is mangled with the package name and name.
- **`NoMangle` attribute**: Symbols with `NoMangle` use the bare (unqualified) final path segment as the mangled name, so the symbol appears in the object file exactly as the user wrote it (e.g. `main` for an entry point, or `printf` for an extern C function).

The mangler is invoked as a distinct pass rather than during HIR lowering, so it runs after the solver has resolved all types and monomorphized generic instantiations. The LLVM codegen uses the `mangled_name` field when creating LLVM functions and globals.

## LLVM Codegen Integration

LLVM codegen (`nitrate_llvm_from_hir`) uses `Function::mangled_name` and `GlobalVariable::mangled_name` directly when creating LLVM functions and globals. The synthetic global-constructor function (used to initialize globals with non-constant initializers) is named by calling `mangle_name(package_name, "<global>_ctor", <void() type>)` internally, producing a unique mangled symbol for the constructor.

## Design Rationale

- **Single-character type discriminants** keep mangled names compact while remaining unambiguous. Uppercase letters encode primitives and compound types; lowercase letters encode the remaining variants.
- **Self-delimiting encodings** mean no terminator characters are needed between concatenated fields, and no escaping is needed for embedded data.
- **Decimal length prefixes** (`N_`) are unambiguous because the length is always followed by exactly that many bytes of a known encoding.
- **DEFLATE + base-63 compression** handles very long names (e.g. deeply nested generics) by compressing the encoded segment before encoding it into the C99 alphabet. Compression is only used when it reduces size.
- **C99 identifier charset** guarantees compatibility with every assembler, linker, and object-file format.
- **Parameter names excluded** from type encoding keeps symbol names compact. Two functions with identical types but different parameter names produce the same mangled name, which is correct because they cannot coexist in the same scope.

## Breaking Compatibility

The project has not released yet, so the mangling scheme may change incompatibly. The `_N` prefix is reserved for Nitrate-mangled symbols; external C symbols using `NoMangle` never receive the prefix.
