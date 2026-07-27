# LLVM IR Code Generation

## Theoretical Foundation

Code generation is the final major phase of the Nitrate compiler pipeline. It translates the validated, type-resolved High-Level Intermediate Representation (HIR) into LLVM Intermediate Representation (LLVM IR), which is then optimized and compiled to native machine code by the LLVM backend.

LLVM IR is a low-level, SSA-based (Static Single Assignment) representation that:

- Uses an infinite set of virtual registers
- Requires all values to be defined before use
- Uses phi (φ) nodes to merge values at control flow joins
- Is typed: every value has a concrete LLVM type
- Supports target-independent optimization

## Architecture

**Crate**: `nitrate_llvm_from_hir` (codegen), `nitrate_llvm` (LLVM context wrapper)  
**Dependencies**: `inkwell` (Rust bindings for LLVM C API), `nitrate_hir`, `nitrate_hir_mangle`, `nitrate_hir_validate`  
**Key types**: `SymbolGenCtx`, `CodegenCtx`, `TypegenCtx`

## Codegen Pipeline

```
Validated HIR (ValidHir<Module>)
    │
    ▼ [generate_llvmir]
Pass 1: Global Variable Generation
    │  For each global:
    │  - Create LLVM global variable
    │  - Generate constructor function
    │  - Register in llvm.global_ctors
    │
    ▼
Pass 2: Function Declarations
    │  For each non-generic function:
    │  - Create LLVM function declaration
    │  - Set linkage (External/Internal/Private)
    │  - Set calling convention
    │
    ▼
Pass 3: Function Definitions
    │  For each non-generic function with body:
    │  - Create entry block
    │  - Allocate and store parameters
    │  - Compile body elements
    │  - Generate return instruction
    │
    ▼
Module Verification
    │
    ▼
LLVM Module → [nitrate_llvm] → Optimization → Object/Assembly
```

## The LLVM Context

The `LLVMContext` type (in `nitrate_llvm`) wraps the inkwell `Context` and provides:

- Context creation and management
- Module creation (`create_module(name)`)
- Module creation from IR string (`create_module_from_ir(ir_string)`)
- Assembly output (`print_to_assembly_file(module, path)`)
- Object file output (`print_to_object_file(module, path)`)
- Module optimization via `ModuleOptimizer`
- Target data layout and pointer size queries
- Basic type creation (void, integer, float types)
- Builder creation (`create_builder()`)
- Basic block management (`append_basic_block`)
- Native target initialization (triple, CPU, features)

## Codegen Context Types

### SymbolGenCtx

The top-level codegen context, created once per module:

```rust
pub struct SymbolGenCtx<'ctx, 'tab, 'package_name, 'module> {
    pub llvm: &'ctx LLVMContext,
    pub tab: &'tab hir::SymbolTab,
    pub module: &'module Module<'ctx>,
    pub globals: HashMap<NString, (PointerValue<'ctx>, BasicTypeEnum<'ctx>)>,
    pub package_name: &'package_name str,
}
```

- `llvm`: Reference to the LLVM context
- `tab`: The HIR symbol table (for looking up functions and types)
- `module`: The LLVM module being built
- `globals`: Map from HIR global variable names to their LLVM values
- `package_name`: The package name for mangling

### TypegenCtx

Context for type translation:

```rust
pub struct TypegenCtx<'ctx, 'tab, 'module> {
    pub llvm: &'ctx LLVMContext,
    pub tab: &'tab hir::SymbolTab,
    pub module: &'module Module<'ctx>,
}
```

### CodegenCtx

Context for value code generation within a single function:

```rust
// Created per-function with:
// - Builder reference
// - Parameter mapping (name → alloca, type)
// - Local variable mapping (name → alloca, type)
```

## Type Translation (HIR → LLVM)

The `gen_ty()` function translates HIR types to LLVM types:

| HIR Type               | LLVM Type                                |
| ---------------------- | ---------------------------------------- |
| `Never` / `Unit`       | `void`                                   |
| `Bool`                 | `i1` (zero-extended to `i8` for storage) |
| `U8` / `I8`            | `i8`                                     |
| `U16` / `I16`          | `i16`                                    |
| `U32` / `I32`          | `i32`                                    |
| `U64` / `I64`          | `i64`                                    |
| `U128` / `I128`        | `i128`                                   |
| `USize`                | `i32` or `i64` (pointer-sized)           |
| `F32`                  | `float`                                  |
| `F64`                  | `double`                                 |
| `Array { element, N }` | `[element_type x N]`                     |
| `Tuple { types }`      | `{ type1, type2, ... }`                  |
| `Reference { to: T }`  | `T*` (pointer to T)                      |
| `Pointer { to: T }`    | `T*`                                     |
| `SliceRef { element }` | `{ T*, i64 }` (pointer + length)         |
| `Function { ... }`     | Function pointer type                    |
| `Struct { def }`       | Named struct type (with computed body)   |
| `Enum { def }`         | Tagged union (discriminant + value)      |

### Struct Codegen

Struct types are generated as LLVM named struct types. The layout is computed from `StructMemoryLayoutCell` entries:

- `Field { field_name }` → the field's LLVM type
- `Padding(n)` → an array of `i8` with size `n`

The `Packed` attribute controls whether the struct type uses LLVM's packed layout (no alignment padding).

### Enum Codegen

Enums are code-generated as tagged unions:

```llvm
%EnumName = type { iN, { ... } }
```

Where `iN` is a large enough integer to hold the discriminant (tag), and the second element is a struct containing all variant payloads (union-like storage).

## Global Variable Codegen

Each HIR global variable generates:

1. **An LLVM global variable**: With the appropriate type (from HIR type), initial value (zero-initialized), and linkage (based on visibility: Pub→External, Pro→Internal, Sec→Private)

2. **A constructor function**: Named `{mangled_name}_ctor`, which:
   - Is an LLVM function with `void()` signature and internal linkage
   - Evaluates the HIR initializer expression via `gen_rval()`
   - Stores the result into the global variable
   - Returns void

3. **Registration in `llvm.global_ctors`**: The constructor is appended to the global constructor list with priority 65535 (low priority, runs after most C++ constructors)

This two-phase approach (zero-init + constructor) ensures that global variables are properly initialized before `main()` runs, regardless of initialization complexity.

## Function Codegen

### Function Declaration

Each function generates an LLVM function with:

- **Name**: The HIR `mangled_name` (produced by `nitrate_hir_mangle`)
- **Signature**: Parameter types (from HIR parameters) and return type
- **Linkage**:
  - External functions without bodies → `Linkage::External`
  - Bodyless non-extern functions → skipped (trait methods, etc.)
  - Functions with bodies:
    - `Pub` → `Linkage::External`
    - `Pro` → `Linkage::Internal`
    - `Sec` → `Linkage::Private`
- **Calling convention**: Determined by the `ExternAbi` attribute (maps to LLVM calling convention IDs)

### Function Definition

For functions with bodies:

1. **Entry block**: Create the function's entry basic block
2. **Parameter setup**: For each parameter, `alloca` a stack slot, store the LLVM parameter value, and record in the codegen context
3. **Body codegen**: Iterate over `BlockElement`s:
   - `Expr(value)`: Compile the expression (side effects via `gen_rval`)
   - `Local(local)`: `alloca` for the local variable, compile initializer, store

### Value Codegen (gen_rval)

The `gen_rval()` function translates HIR `Value` nodes to LLVM IR instructions:

| HIR Value                              | LLVM IR                                          |
| -------------------------------------- | ------------------------------------------------ |
| `Value::Unit`                          | None (void)                                      |
| `Value::Bool(b)`                       | `i1 b`                                           |
| `Value::I8(v)`                         | `i8 v`                                           |
| ... integer types                      | Corresponding LLVM integer constant              |
| `Value::F32(v)`                        | `float v`                                        |
| `Value::F64(v)`                        | `double v`                                       |
| `Value::StringLit(s)`                  | Global string constant `[N x i8]`                |
| `Value::Binary { l, op, r }`           | LLVM binary instruction (add, sub, mul, etc.)    |
| `Value::Unary { op, v }`               | `fneg` (negate) or `not` (xor -1)                |
| `Value::Cast { v, t }`                 | LLVM cast instruction (inttoptr, ptrtoint, etc.) |
| `Value::Borrow { place }`              | `alloca` + store + pointer                       |
| `Value::Deref { place }`               | `load` instruction                               |
| `Value::FieldAccess { expr, field }`   | `extractvalue` (for structs) or GEP + load       |
| `Value::IndexAccess { col, idx }`      | GEP (getelementptr)                              |
| `Value::If { c, t, f }`                | Branch + phi nodes                               |
| `Value::While { c, b }`                | Loop: header block, body block, continue block   |
| `Value::Loop { body }`                 | Infinite loop with back edge                     |
| `Value::Break` / `Continue`            | Branch to appropriate block                      |
| `Value::Return { v }`                  | `ret` instruction                                |
| `Value::Block { b }`                   | Compile block elements, return last value        |
| `Value::Call { callee, args }`         | `call` instruction                               |
| `Value::FunctionSymbol { id }`         | Function pointer                                 |
| `Value::LocalVariableSymbol { id }`    | `load` from alloca                               |
| `Value::ParameterSymbol { id }`        | `load` from param alloca                         |
| `Value::GlobalVariableSymbol { id }`   | `load` from global                               |
| `Value::StructObject { def, fields }`  | `insertvalue` to build struct                    |
| `Value::EnumVariant { def, var, val }` | `insertvalue` for tag + payload                  |
| `Value::List { elements }`             | Array constant or `insertelement`                |
| `Value::Tuple { elements }`            | `insertvalue` chain                              |
| `Value::Assign { place, value }`       | `store` instruction                              |

### Control Flow Codegen

**If expressions**: Generate a diamond control flow:

```
entry:
    %cond = gen_rval(condition)
    br %cond, true_bb, false_bb

true_bb:
    %t_val = gen_rval(true_branch)
    br merge_bb

false_bb:
    %f_val = gen_rval(false_branch)  // or default value
    br merge_bb

merge_bb:
    %result = phi [%t_val, true_bb], [%f_val, false_bb]
```

**While loops**: Generate loop structure:

```
header:
    %cond = gen_rval(condition)
    br %cond, body_bb, exit_bb

body_bb:
    gen_rval(body)
    br header

exit_bb:
    // Continue after loop
```

**Loop (infinite)**: Simple back edge:

```
body_bb:
    gen_rval(body)
    br body_bb
```

## Calling Convention Support

The codegen supports a comprehensive set of calling conventions for FFI, mapping HIR `ExternAbi` names to LLVM calling convention IDs:

| ABI Name                       | LLVM Calling Convention |
| ------------------------------ | ----------------------- |
| `"C"`, `"cdecl"`, `"system"`   | `CC` (0)                |
| `"fastcall"`, `"x86-fastcall"` | `X86_FastCall` (65)     |
| `"stdcall"`, `"x86-stdcall"`   | `X86_StdCall` (64)      |
| `"thiscall"`, `"x86-thiscall"` | `X86_ThisCall` (70)     |
| `"win64"`, `"x86-64-win64"`    | `Win64` (79)            |
| `"sysv64"`, `"x86-64-sysv"`    | `X86_64_SysV` (78)      |
| `"aapcs"`, `"arm-aapcs"`       | `ARM_AAPCS` (67)        |
| `"ptx-kernel"`                 | `PTX_Kernel` (71)       |
| `"amdgpu-kernel"`              | `AMDGPU_Kernel` (91)    |
| `"fast"`                       | `Fast` (8)              |
| `"cold"`                       | `Cold` (9)              |
| `"swift"`                      | `Swift` (16)            |
| `"avr-intr"`                   | `AVR_INTR` (83)         |

Unrecognized ABIs trigger a panic with a helpful error message.

## Function Attribute Handling

Functions with the `CVariadic` attribute generate variadic LLVM function types (enabling C-style varargs like `printf`).

Functions with `NoMangle` preserve their original name (no mangling applied).

## Module Verification

After code generation, the LLVM module is verified:

```rust
if let Err(e) = ctx.module.verify() {
    eprintln!("LLVM Module Verification Error: {}", e.to_string());
    eprintln!("Generated LLVM Module:\n");
    eprintln!("{}", ctx.module.print_to_string().to_string());
    panic!("Generated LLVM module is invalid");
}
```

If verification fails, the full module IR is printed for debugging. This is a hard failure — invalid IR cannot proceed to optimization.

## The `nitrate_llvm` Crate

The `nitrate_llvm` crate provides the LLVM context wrapper and optimization:

### LLVMContext

- `new()`: Create a new LLVM context with target initialization
- `create_module(name)`: Create a new LLVM module
- `create_module_from_ir(ir)`: Parse LLVM IR string into a module
- `print_to_assembly_file(module, path)`: Write assembly to file
- `print_to_object_file(module, path)`: Write object file
- `target_data()`: Get target data layout
- `ptr_sized_int_type(data, name)`: Get pointer-sized integer type
- `void_type()`, `i8_type()`, `f64_type()`, etc.: Basic type factories
- `create_builder()`: Create an IR builder
- `append_basic_block(function, name)`: Create a basic block

### ModuleOptimizer

Runs LLVM's optimization passes:

- `new(module, opt_level)`: Create optimizer with optimization level (0-3)
- `optimize()`: Run the optimization pipeline

Optimization passes include:

- **Mem2Reg**: Promote memory to SSA registers
- **GVN**: Global Value Numbering
- **SCCP**: Sparse Conditional Constant Propagation
- **Inlining**: Function inlining
- **Loop optimizations**: Loop invariant code motion, unrolling, vectorization
- **Dead code elimination**: Remove unreachable/unused code
- **Instruction combining**: Simplify instruction patterns

## Design Decisions

### Why Two-Pass Function Codegen (Declare + Define)?

Functions can reference each other (mutual recursion, callback patterns). The declaration pass ensures that all function symbols are available in the LLVM module before any function body references them. This prevents "undefined function" errors during code generation.

### Why Constructor Functions for Globals?

LLVM global variables require constant initializers. Complex HIR initializers (function calls, arithmetic) cannot be represented as LLVM constants. The constructor function approach:

1. Zero-initializes the global (valid LLVM constant)
2. Runs a separate function before `main()` that evaluates the actual initializer
3. Is compatible with LLVM's `llvm.global_ctors` mechanism

### Why Skip Generic Functions During Codegen?

Generic functions are templates, not concrete code. Only monomorphized copies (produced by the solver) should be compiled. The codegen checks `func.generics.is_some()` and skips any function that still has generic parameters.

### Why Verify the Module?

LLVM module verification catches codegen bugs early:

- Type mismatches (expecting `i32` but got `i64*`)
- Invalid control flow (branch to nonexistent block)
- Malformed instructions
- Mismatched function signatures

## Performance Considerations

- **SSA construction**: LLVM's `mem2reg` pass promotes `alloca`/`store`/`load` sequences to SSA registers. The codegen generates explicit `alloca`s for all variables (simpler codegen) and relies on LLVM optimization for SSA promotion.
- **Type caching**: LLVM types are created on demand. Struct types are cached to prevent duplicate definitions.
- **Module size**: Large programs may produce large LLVM modules. The total module size affects LLVM's optimization time and memory usage.
