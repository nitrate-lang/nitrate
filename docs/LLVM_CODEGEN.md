# LLVM IR Code Generation

## Theoretical Foundation

Code generation is the final major phase of the Nitrate compiler pipeline. It translates the validated, type-resolved High-Level Intermediate Representation (HIR) into LLVM Intermediate Representation (LLVM IR), which is then optimized and compiled to native machine code by the LLVM backend — one of the most mature and widely used code generation frameworks in existence, powering Clang, Rust (rustc), Swift, Julia, and many other language implementations.

LLVM IR is a low-level, SSA-based (Static Single Assignment) representation with several fundamental properties that make it ideal for code generation. Every value in LLVM IR is defined before use — values cannot be referenced before their definition, simplifying analysis and optimization. Values are held in an infinite set of virtual registers, eliminating the need for register allocation at the IR level (that is handled later by LLVM's backend). Phi (φ) nodes merge values at control flow joins, providing a mechanism to represent values that come from different predecessor blocks. Every value has a concrete LLVM type, making the IR self-verifying and enabling type-based optimizations. The SSA form makes data dependencies explicit, which in turn enables powerful optimizations like Global Value Numbering, constant propagation, and dead code elimination.

## Architecture

**Crate**: `nitrate_llvm_from_hir` (the HIR-to-LLVM translator — the core codegen logic)  
**Crate**: `nitrate_llvm` (the LLVM context wrapper — provides type factories, optimization, and output)  
**Dependencies**: `inkwell` — safe Rust bindings for the LLVM C API (version 0.4 targeting LLVM 18)  
**Key types**: `SymbolGenCtx` (top-level codegen context), `CodegenCtx` (per-function value codegen), `TypegenCtx` (type translation)

## Memory Model and Place Semantics

Nitrate codegen follows Rust's place-expression model. Every HIR `Value` that denotes a memory location (a _place_) compiles to a `PointerValue` — the address of that location in memory. Borrow expressions (`&expr`, `&mut expr`) produce pointers to places, dereference expressions (`*ptr`) consume pointers, and assignment (`place = value`) stores into the place's address.

### The Place Concept

The core invariant of the codegen is:

> **`gen_place(value)` returns the address of `value`'s storage — never a copy.**

This invariant is what makes borrows alias their targets. When a program writes `let r: &i32 = &arr[2]`, the codegen produces a GEP computing the address of element 2 within the array's storage, and stores that address into `r`. Subsequent reads through `*r` load from that exact address. Any deviation — such as loading the element into a temporary and storing the temporary's address — would break aliasing and produce incorrect programs.

### Place Generation Rules

`gen_place` delegates to per-value helpers:

| HIR Value                          | Place generation                                                                                               |
| ---------------------------------- | -------------------------------------------------------------------------------------------------------------- |
| Local / Parameter / Global symbols | The alloca/global address stored in the symbol table                                                           |
| `FieldAccess { expr, field }`      | GEP on the struct address; auto-derefs through one layer of reference/pointer                                  |
| `IndexAccess { col, idx }`         | For arrays: GEP on the collection address. For slices: load the fat pointer's data field, then GEP the element |
| `Deref { place }`                  | The pointer value itself — **no load, no copy**                                                                |
| Non-place rvalues (literals, etc.) | Materialized into a temporary `alloca` (rvalue materialization)                                                |
| Function / global symbols          | The function/global pointer                                                                                    |

### Dereference is Zero-Cost

`gen_place` for `Value::Deref` returns the pointer value of the operand directly. For `*p` where `p: &T` or `p: *const T`, the pointee's address **is** the pointer value — there is no intermediate storage.

Previously `gen_place_deref` loaded the pointee into a fresh alloca and returned the alloca's address. This produced a _copy_ rather than a reference into the original storage, silently breaking:

- `&arr[i]` — borrowed a temporary copy instead of the array element
- `*ptr.field = v` — wrote to a copy instead of the actual field
- Method calls with `&self` on dereferenced receivers

This has been corrected. The old behavior can be summarized as:

```
// Old (WRONG): copies pointee into a temp
%loaded = load T, ptr %ptr
%tmp = alloca T
store T %loaded, ptr %tmp
ret ptr %tmp          // ← dangling/copy!

// New (CORRECT): address is the pointer
ret ptr %ptr          // ← aliases original storage
```

### Auto-Deref in Access Expressions

Field and index access on reference/pointer-typed expressions automatically dereference **one** layer:

- `p_ref.x` where `p_ref: &Point` → GEP on the pointee of `p_ref` (no `Value::Deref` node needed)
- `arr_ref[i]` where `arr_ref: &[i32; N]` → GEP on the array pointed to by `arr_ref`

The type-`determine_type` of the expression is used to find the underlying aggregate. If the expression's type is already `Reference` or `Pointer`, the rvalue (the pointer) is used directly as the GEP base. Otherwise `gen_place` is used.

### Slice Indexing

Slices are fat pointers `{ data_ptr, len }`. `gen_place_index_access` for slices:

1. Obtains a pointer to the slice (`gen_place` for locals, or materializes the fat pointer value into a temp for references-to-slices)
2. Loads the `data_ptr` field out of the fat pointer (GEP `[0, 0]` + load)
3. GEPs the _element_ pointer by the index — a single-index GEP on the data pointer

This yields a pointer to the actual element within the slice's backing storage.

## Codegen Pipeline

The codegen proceeds in three coordinated passes over the HIR module. This three-pass architecture ensures that all symbols are declared before any references to them are generated, handling mutual recursion and forward references correctly.

### Pass 1: Global Variable Generation

Each HIR global variable creates an LLVM global with the appropriate type (derived from the HIR type) and linkage (External for `Pub` globals, Internal for `Pro`, Private for `Sec`). Complex initializers — those that cannot be expressed as LLVM constants, such as values requiring function calls or arithmetic — are handled through constructor functions registered via `llvm_appendToGlobalCtors`. Each constructor function is a synthesized symbol named via `mangle_name(package_name, "<global>_ctor", <void() type>)` with a `void()` signature and internal linkage. The constructor evaluates the HIR initializer expression, stores the result into the global, and returns void. The global constructors list ensures these initializers run before `main()` in priority order.

### Pass 2: Function Declarations

All non-generic functions are declared first, establishing their LLVM function signatures in the module before any function body references them. This forward declaration pass is essential for correct code generation because functions may reference each other (mutual recursion, higher-order callbacks). Each declaration creates an LLVM function with the correct parameter types, return type, linkage, and calling convention. Bodyless functions (extern declarations, trait method stubs) are finalized at this point.

### Pass 3: Function Definitions

Function bodies are compiled by generating LLVM instructions for each HIR `Value` node. The codegen creates an entry basic block, allocates stack slots for parameters and local variables (`alloca`), stores parameter values, then iterates over the function's `BlockElement`s generating instructions. Type translation maps HIR types to LLVM types: `Unit`→`void`, integers→LLVM integer types, `F32`→`float`, `F64`→`double`, arrays→LLVM array types, structs→LLVM named struct types, enums→tagged unions, references→pointers, slice references→fat pointers.

## Type Translation (HIR → LLVM)

The `gen_ty()` function handles the complete mapping:

| HIR Type              | LLVM Type                                 | Notes                            |
| --------------------- | ----------------------------------------- | -------------------------------- |
| Never, Unit           | `void`                                    | Zero-size types                  |
| Bool                  | `i1`                                      | Zero-extended to `i8` for memory |
| U8/I8 through U64/I64 | `i8`, `i16`, `i32`, `i64`                 | Direct integer mapping           |
| U128/I128             | `i128`                                    | LLVM integer type                |
| USize                 | `i32` or `i64`                            | Pointer-size dependent           |
| F32, F64              | `float`, `double`                         | IEEE 754                         |
| Array(T, N)           | `[T x N]`                                 | Fixed-size LLVM array            |
| Tuple(T1..Tn)         | `{T1, ..., Tn}`                           | LLVM struct type                 |
| Reference(T)          | `T*`                                      | Pointer to T                     |
| Pointer(T)            | `T*`                                      | Pointer to T                     |
| SliceRef(T)           | `{T*, i64}`                               | Fat pointer (data + length)      |
| Struct                | Named struct type                         | With computed body from layout   |
| Enum                  | Tagged union `{discrim_type, {variants}}` | Discriminant + union storage     |

## Rvalue Codegen

`gen_rval` compiles each HIR `Value` that produces a value (as opposed to a place) into an LLVM `BasicValueEnum`. The rvalue passes use `gen_place` wherever a place is needed:

- **Field access rvalue** (`Value::FieldAccess`): computes the field GEP via `gen_place` (handling auto-deref through references), then loads — never copies the struct.
- **Index access rvalue** (`Value::IndexAccess`): computes the element GEP via `gen_place`, then loads with the element type resolved through references.
- **Deref rvalue** (`Value::Deref`): evaluates the operand (a pointer), then loads the pointee type.
- **Borrow rvalue** (`Value::Borrow`): computes `gen_place` of the operand — the address IS the borrow's value. There is no copy.

### Method Call Receiver Handling

`gen_rval_method_call` inspects the method's first parameter type to decide how to pass the receiver:

- **`&self` / `&mut self` methods**: the receiver is passed by pointer.
  - If the receiver expression is _already_ a reference/pointer type (auto-deref on method calls), its rvalue (a pointer) is passed directly.
  - Otherwise, `gen_place` provides the object's address.
- **By-value `self` methods**: the receiver is evaluated as an rvalue and passed directly.

Previously, method calls on reference-typed receivers passed the _address of the reference slot_ rather than the pointer value itself, causing `&self` methods to receive a `&Box<T>`-style double pointer.

## Control Flow Codegen

The codegen translates HIR control flow constructs into LLVM branch and phi instructions:

**If expressions** generate a diamond control flow graph: the condition is evaluated and a conditional branch selects either the true block or false block. A phi node in the merge block selects the result value from the appropriate predecessor. If there is no false branch, the phi node selects a unit value for the missing branch.

**While loops** generate three blocks: a header block that evaluates the condition, a body block that executes the loop body, and an exit block that continues after the loop. The header conditionally branches to either the body or exit. The body branches back to the header after execution.

**Infinite loops** generate a single body block that branches back to itself, terminated by explicit `break` or `continue` instructions. The codegen tracks loop nesting levels to correctly resolve break/continue targets.

## Module Verification

After code generation, the LLVM module is verified using `module.verify()`. If verification fails, the full module IR is printed for debugging and compilation panics — invalid IR cannot produce correct machine code. The verification catches type mismatches, invalid control flow, malformed instructions, and mismatched function signatures.

## Design Decisions

**Why two-pass function codegen?** Functions can reference each other through mutual recursion and callback patterns. The declaration pass ensures all function symbols are available in the LLVM module before any body compilation begins, preventing "undefined function" errors during code generation. This two-pass approach is simpler than generating code in dependency order.

**Why constructor functions for globals?** LLVM global variables require constant initializers that can be evaluated at link time. Complex HIR initializers involving function calls, arithmetic, or memory allocation cannot be expressed as LLVM constants. The constructor function approach zero-initializes the global (a valid LLVM constant) and runs a separate initialization function before `main()` that computes the actual initializer value.

**Why skip generic functions during codegen?** Generic functions are templates that cannot produce concrete code until all type parameters are known. Only monomorphized copies — produced by the solver with concrete type substitutions — should be compiled. The codegen checks `func.generics.is_some()` and skips any function that still has unresolved generic parameters. A panic-guard in `gen_ty()` catches any `GenericParam` that slips through, ensuring early failure rather than silent miscompilation.

**Why does dereference return the pointer value?** In LLVM's memory model, the address of `*p` IS the value of `p`. Returning `p` directly preserves aliasing: writes through `*p` and reads via the original variable observe the same memory. Any intermediate alloca would break this aliasing, producing stale reads and lost writes. The zero-cost dereference also eliminates unnecessary loads and stores, enabling mem2reg and other optimizations to see through the borrow immediately.
