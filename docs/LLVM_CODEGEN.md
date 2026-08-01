# LLVM IR Code Generation

## Theoretical Foundation

Code generation is the final major phase of the Nitrate compiler pipeline. It translates the validated, type-resolved High-Level Intermediate Representation (HIR) into LLVM Intermediate Representation (LLVM IR), which is then optimized and compiled to native machine code by the LLVM backend — one of the most mature and widely used code generation frameworks in existence, powering Clang, Rust (rustc), Swift, Julia, and many other language implementations.

LLVM IR is a low-level, SSA-based (Static Single Assignment) representation with several fundamental properties that make it ideal for code generation. Every value in LLVM IR is defined before use — values cannot be referenced before their definition, simplifying analysis and optimization. Values are held in an infinite set of virtual registers, eliminating the need for register allocation at the IR level (that is handled later by LLVM's backend). Phi (φ) nodes merge values at control flow joins, providing a mechanism to represent values that come from different predecessor blocks. Every value has a concrete LLVM type, making the IR self-verifying and enabling type-based optimizations. The SSA form makes data dependencies explicit, which in turn enables powerful optimizations like Global Value Numbering, constant propagation, and dead code elimination.

## Architecture

**Crate**: `nitrate_llvm_from_hir` (the HIR-to-LLVM translator — the core codegen logic)  
**Crate**: `nitrate_llvm` (the LLVM context wrapper — provides type factories, optimization, and output)  
**Dependencies**: `inkwell` — safe Rust bindings for the LLVM C API (version 0.4 targeting LLVM 18)  
**Key types**: `SymbolGenCtx` (top-level codegen context), `CodegenCtx` (per-function value codegen), `TypegenCtx` (type translation)

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
