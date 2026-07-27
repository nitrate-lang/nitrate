# Interned String System (NString)

## Overview and Motivation

The `NString` type is Nitrate's interned string representation, used throughout the compiler for identifiers, names, and other frequently repeated strings. String interning is a memory optimization technique where each unique string value is stored exactly once in a global pool. Subsequent references to the same string content reuse the existing entry rather than allocating new storage. This provides three critical benefits for a compiler:

**Memory efficiency**: Compilers deal with a vast number of symbolic names — function names, variable names, type names, module paths, parameter names, and field names. Many of these names are repeated many times throughout the compilation process. Without interning, each occurrence would allocate its own `String` on the heap, leading to significant memory waste. With interning, each unique string is stored once and all references share that single allocation.

**Comparison speed**: String comparison is normally O(n) where n is the length of the strings being compared. In a compiler, strings are compared millions of times during name resolution, type checking, and symbol table lookups. With interning, comparison reduces to O(1) integer handle comparison — we compare the interned handles rather than the string contents.

**Hash table performance**: Interned strings provide stable, precomputed hash values. When used as keys in `HashMap`s and `BTreeMap`s in symbol tables and the Store, interned strings avoid recomputing the hash on every lookup and provide fast equality checks.

## Architecture

**Crate**: `nitrate_nstring`  
**Key type**: `NString`  
**Key files**: `lib.rs`, `nstring.rs`

## Design and Implementation

The `NString` type wraps an interned string handle. It implements `Deref<Target = str>`, `Clone`, `Eq`, `Ord`, `Hash`, and `Serialize`/`Deserialize`, making it a drop-in replacement for `String` in most compiler data structures. When `NString::from(s)` is called, the pool checks for an existing entry matching the string content; if found, the existing handle is returned; otherwise, a new handle is allocated and the string is stored.

## Properties

`NString` provides immutability (cannot be modified after creation, essential for safe sharing), deduplication (identical strings always produce the same handle), O(1) comparison and hashing, low memory usage (each unique string stored once), and thread-safe access through synchronization.

## Compiler Integration

`NString` is used pervasively throughout every subsystem. In type definitions, it appears in `GenericParam` names, `Inferred` variable names, `FunctionType` parameter names, and `ExternAbi` names. In item definitions, it appears in `Function` names and mangled names, `StructDef` names and `StructField` names, `EnumDef` names and `EnumVariant` names, `Trait` names and associated type/constant names, `Module` names, `TypeAliasDef` names, and all variable and parameter names. In expressions, it appears in `FieldAccess` field names, `MethodCall` method names, `EnumVariant` variant names, named arguments, and loop labels.
