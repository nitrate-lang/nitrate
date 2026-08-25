# MIR: Mid-Level Intermediate Representation

## Theoretical Foundation

MIR (Mid-level Intermediate Representation) is a control-flow-graph-based representation that sits between the high-level, AST-like HIR (High-Level Intermediate Representation) and the low-level LLVM IR. Where HIR represents expressions as nested trees mirroring the syntactic structure of source code, MIR flattens those trees into sequences of statements organized within basic blocks connected by explicit control-flow edges. This flattening exposes the actual execution order of operations, makes temporary values and their lifetimes explicit, and enables a class of dataflow analyses (dead code elimination, constant propagation, liveness analysis) that are impractical on tree-structured IRs.

The MIR design draws from established compiler IRs — particularly Rust's MIR, which pioneered the approach of lowering a rich expression language into a minimal statement-based form as a prerequisite for borrow checking and optimization. Like Rust's MIR, Nitrate's MIR uses flat statements, explicit terminators, and places/operands/rvalues as its core value taxonomy. Unlike Rust's MIR, Nitrate's MIR employs **block arguments** (inspired by MLIR and Cranelift) instead of phi nodes to carry values across control flow edges, simplifying SSA construction and making data flow explicit in the CFG.

The MIR is **monomorphized and fully concrete**: there is no type inference, no generic parameters, and no unresolved types. Every local variable, temporary, and operand has a known `MirType`. This property makes MIR an ideal target for optimization passes and simplifies code generation, since every type is immediately translatable to an LLVM type without further resolution.

## Architecture

**Crate**: `nitrate_mir` (type definitions, storage, and builder API)  
**Crate**: `nitrate_mir_from_hir` (HIR → MIR lowering)  
**Crate**: `nitrate_mir_optimize` (optimization pass traits)  
**Crate**: `nitrate_llvm_from_mir` (MIR → LLVM IR code generation)  
**Key types**: `MirModule`, `MirFunction`, `BasicBlock`, `Statement`, `Terminator`, `Place`, `Operand`, `Rvalue`, `MirType`, `MirStore`

## The Place / Operand / Rvalue Trichotomy

MIR divides all value-level computation into three distinct categories, each with a specific role in the IR. This separation is the single most important design decision in MIR: it eliminates ambiguity about what is a memory location versus what is a computed value, and it forces the lowering pass to make storage decisions explicit.

### Place: Memory Locations

A `Place` represents a location in memory — where data is stored, not the data itself. Places appear as the left-hand side of assignments (where values are written) and as the source of `Copy`/`Move` operands (where values are read). Places form a tree that mirrors memory layout:

```
Place ::= Local(LocalId)                  -- a local variable or temporary
        | Static(NString)                 -- a global/static variable
        | Deref(Box<Place>)              -- dereference of a pointer: *place
        | Field { base, field_name }     -- field access: place.field
        | Index { base, index }          -- index into array/slice: place[idx]
        | Downcast { base, variant_name } -- enum variant projection
```

The key invariant is that a `Place` is always the **address** of some storage — never a value loaded from that storage. This is directly analogous to the place-expression model in Rust and HIR: `gen_place()` returns a pointer to storage, and loading from that pointer is a separate operation.

### Operand: Values Read from Places or Constants

An `Operand` is a value consumed by an `Rvalue` or passed as a block argument in a `Terminator`. It exists in two flavors:

- **`Copy(Place)`** — reads the value from the place, leaving the place initialized and usable. This is the MIR equivalent of HIR's implicit copy semantics for `Copy` types.
- **`Move(Place)`** — reads the value from the place, deinitializing it. The place cannot be used afterward. This represents ownership transfer for non-`Copy` types.
- **`Constant(MirLiteral)`** — a compile-time constant value (integer, float, boolean, string, unit). Constants require no storage and are embedded directly in the IR.

The copy/move distinction in operands provides the foundation for ownership tracking and borrow checking. The MIR borrow checker (`nitrate_mir_borrow_check`) consumes `Copy`/`Move` operands to track moved-from state, and the IR's `StorageLive`/`StorageDead` markers bound storage lifetimes. The HIR→MIR lowering decides which flavor to emit via the copy-classification predicate in `nitrate_hir_type` (`hir_type_is_copy`): place reads of copy types (primitives, references, pointers, tuples/arrays of copy types) emit `Copy`, while reads of non-copy types (structs, enums) emit `Move` so the borrow checker can enforce single-ownership. Global/static reads always emit `Copy` (shared value semantics).

### Rvalue: Computed Values

An `Rvalue` produces a value — it is the right-hand side of `Assign` statements. Rvalues are **flat**: unlike HIR expressions which form nested trees, MIR rvalues have all their operands already evaluated into `Operand` values (which are either constants or references to places). This flatness is essential for optimization — every temporary value has an explicit storage location (a `Local`), and every computation is a single operation.

```
Rvalue ::= Use(Operand)                                    -- identity: read operand
         | Ref { region: BorrowKind, place }               -- create reference &place
         | Len(Place)                                      -- slice length
         | Cast { value, target_ty }                       -- type cast
         | BinaryOp { op, lhs, rhs }                       -- binary operation
         | CheckedBinaryOp { op, lhs, rhs }                -- checked binary with overflow
         | UnaryOp { op, operand }                         -- unary operation
         | NullaryOp(NullaryOp, MirTypeId)                 -- size_of / align_of
         | Aggregate(AggregateKind, ThinVec<Operand>)      -- construct struct/tuple/array/enum
```

`BorrowKind` distinguishes shared (`&T`) from mutable (`&mut T`) borrows. `NullaryOp` provides `SizeOf` and `AlignOf` — metadata queries that depend on the target type but take no value operands. `AggregateKind` covers `Tuple` (construct a tuple), `Array(MirTypeId)` (construct an array with repeated element), `Struct(NString, ThinVec<NString>)` (construct a struct literal), and `Enum(NString, NString)` (construct an enum variant).

## Basic Blocks and Control Flow

### Basic Block Structure

A `BasicBlock` is a straight-line sequence of statements terminated by exactly one `Terminator`. Every basic block must have a terminator — the `Unreachable` variant serves as a placeholder for blocks that should never terminate normally (dead code, diverging paths). Block arguments (see below) are an optional list of `MirTypeId` values declaring the formal parameters of the block.

```rust
pub struct BasicBlock {
    pub statements: ThinVec<Statement>,
    pub terminator: Terminator,
    pub args: ThinVec<MirTypeId>,
}
```

### Statements

Statements execute sequentially within a basic block. MIR has four statement kinds:

- **`Assign(Place, Rvalue)`** — the primary computational statement. Evaluates the rvalue and stores the result into the place. In SSA form, each `Local` is assigned exactly once.
- **`SetDiscriminant { place, variant_index }`** — initializes the discriminant field of an enum value before writing the variant payload. This separates discriminant setup from payload assignment, enabling the codegen to compute the correct GEP for the variant's storage.
- **`StorageLive(LocalId)`** — marks the start of a local's storage lifetime. Used by borrow checking and optimization passes to determine when memory is allocated.
- **`StorageDead(LocalId)`** — marks the end of a local's storage lifetime. Indicates that the local is no longer needed and its storage can be reused.

### Terminators

Terminators transfer control to successor basic blocks. Every basic block ends with exactly one terminator:

- **`Goto { target, args }`** — unconditional branch, passing block arguments to the target.
- **`If { condition, true_target, true_args, false_target, false_args }`** — conditional branch on a boolean operand, with block arguments for each path.
- **`SwitchInt { discr, targets, otherwise, otherwise_args }`** — multi-way branch on an integer discriminant. Each arm maps an integer value to a target block with block arguments. The `otherwise` arm handles unmatched values.
- **`Return { value }`** — return from the function with an optional return value.
- **`Unreachable`** — marks a code path that must never execute. Used for diverging code (e.g., after a call to a function that never returns).
- **`Call { callee, args, destination, target, target_args }`** — function call with two forms: returning (both `destination` and `target` are `Some`) and diverging (both `None`). Returning calls store the return value into `destination` and branch to `target` with `target_args`.

### Block Arguments: Replacing Phi Nodes

Traditional SSA form uses phi (φ) instructions at block entry points to merge values from different predecessor edges. MIR instead uses **block arguments** — formal parameters on basic blocks — following the MLIR/Cranelift design. When a terminator branches to a target block, it provides operand values for each of that block's arguments.

Consider a simple if-else expression:

```
// Traditional phi approach:
//   bb0: cond = ...
//        br cond, bb1, bb2
//   bb1: x = 42
//        br bb3
//   bb2: x = 99
//        br bb3
//   bb3: result = φ(x from bb1, x from bb2)

// Block argument approach:
//   bb0: cond = ...
//        If cond, true→bb1(args=[]), false→bb2(args=[])
//   bb1: x = 42
//        Goto bb3(args=[Copy(x)])
//   bb2: x = 99
//        Goto bb3(args=[Copy(x)])
//   bb3(result_local):  -- result_local is the block argument
//        ...
```

The block argument approach has several advantages: data flow is explicit in the CFG — you can see what values are passed on each edge without consulting a separate phi instruction, SSA construction is simpler because values flow forward along edges rather than being assembled at merge points, and code generation maps naturally to LLVM's branch instruction (block arguments become PHI nodes during lowering or are resolved through direct edge forwarding).

### Successor Iteration

Each basic block provides a `successors()` method that returns the set of successor `BasicBlockId` values based on its terminator. `Goto` yields one successor, `If` yields two (true and false), `SwitchInt` yields one per arm plus the otherwise target, `Call` with a target yields one successor, and `Return`/`Unreachable` yield none. This method supports CFG traversal for analysis passes.

## MIR Type System

### Fully Concrete Types

`MirType` is a parallel type system to HIR's `Type`, with two critical differences: there are no inference variables (`Inferred`, `InferredInteger`, `InferredFloat`), and there are no generic parameters (`GenericParam`). Every type is fully resolved at MIR construction time. The type system supports:

| Category   | Variants                                                                                         |
| ---------- | ------------------------------------------------------------------------------------------------ |
| Primitives | `Never`, `Unit`, `Bool`, `U8`..`U128`, `USize`, `I8`..`I128`, `F32`, `F64`, `Range`, `Str`       |
| Aggregates | `Array { element_type, len }`, `Tuple { element_types }`, `Struct { name, fields, layout }`      |
| Enums      | `Enum { name, variants }` where each variant has a name and optional payload type                |
| Pointers   | `Reference { exclusive, mutable, to }`, `Pointer { exclusive, mutable, to }`                     |
| Slices     | `SliceRef { exclusive, mutable, element_type }`, `SlicePtr { exclusive, mutable, element_type }` |
| Functions  | `Function { params, return_type, is_c_variadic }`                                                |

The `MirStructLayout` type tracks the physical memory layout of struct fields using a sequence of `MirStructLayoutCell` values, each of which is either `Field { field_name }` (a named field) or `Padding(NonZeroU32)` (inter-field padding bytes for alignment). This enables the LLVM codegen to emit GEP instructions with correct byte offsets.

### Type Classification Helpers

`MirType` provides a rich set of classification predicates: `is_diverging()`, `is_bool()`, `is_unsigned_primitive()`, `is_signed_primitive()`, `is_integer_primitive()`, `is_float_primitive()`, `is_reference()`, `is_pointer()`, `is_slice_ref()`, `is_slice_ptr()`, `is_aggregate()`, `is_array()`, `is_tuple()`, `is_struct()`, `is_enum()`, `is_function()`, and `is_zst()` (zero-sized type). These are used pervasively in the lowering and codegen to dispatch type-specific logic.

### Pointer Size

`PtrSize` is an enum with two variants: `U32` (4 bytes) and `U64` (8 bytes), corresponding to 32-bit and 64-bit target architectures. The pointer size is carried in `MirModule` and used to resolve `USize` literals (which are pointer-width) and compute struct layouts.

## TLS Storage Infrastructure

MIR uses the same Thread-Local Storage (TLS) pattern as HIR, with a dedicated `MirStore` that holds all MIR data for a compilation session. The store provides four sub-stores:

| Sub-store         | Handle type     | Item type     | Deduplication?                     |
| ----------------- | --------------- | ------------- | ---------------------------------- |
| `MirTypeStore`    | `MirTypeId`     | `MirType`     | Yes (BiMap-backed)                 |
| `LocalStore`      | `LocalId`       | `LocalDecl`   | No (append-only)                   |
| `BasicBlockStore` | `BasicBlockId`  | `BasicBlock`  | No (append-only, `RefCell`-backed) |
| `FunctionStore`   | `MirFunctionId` | `MirFunction` | No (append-only, `RefCell`-backed) |

### Type Deduplication

`MirTypeStore` uses a `RwLock<BiMap<Arc<MirType>, MirTypeId>>` backed by an `AppendOnlyVec<Arc<MirType>>` for O(1) deduplication. When a type is stored, the BiMap is checked under a read lock; if the type already exists, the existing `MirTypeId` is returned. Otherwise, under a write lock, a second check prevents races, and the new type is appended. This ensures that structural type equality is equivalent to handle equality — critical for the frequent type comparisons performed during optimization.

### Handle Access

Handle types (`MirTypeId`, `LocalId`, `BasicBlockId`, `MirFunctionId`) implement `Deref` to provide transparent read access through TLS. `MirTypeId` dereferences directly to `&MirType` (since types are immutable after interning). The mutable handles (`LocalId`, `BasicBlockId`, `MirFunctionId`) dereference to `&RefCell<T>`, providing interior mutability for passes that need to modify items through shared references to the store.

### RAII Store Scoping

`using_storage(store, || { ... })` sets the store pointer in TLS for the duration of the closure, saving and restoring the previous pointer. This supports nested store usage (e.g., if a codegen pass internally creates a temporary MIR store for testing). The `From` trait implementations for handle types invoke `get_storage()` to access the TLS store, enabling ergonomic type interning:

```rust
let ty_id: MirTypeId = MirType::I32.into();  // implicitly uses get_storage()
```

## MirFunction and MirModule

### MirFunction and MirFunctionBody

A `MirFunction` represents a single function in the MIR. The signature fields (name, parameters, return type, variadic flag) are always present. The function body — containing locals, basic blocks, and the CFG — is split into a separate `MirFunctionBody` struct wrapped in `Option<>`:

```rust
pub struct MirFunctionBody {
    pub locals: ThinVec<LocalDecl>,       // all locals (params + temporaries + user variables)
    pub local_ids: ThinVec<LocalId>,      // handles for each local (1:1 with locals)
    pub entry_block: BasicBlockId,        // where execution starts
    pub blocks: ThinVec<BasicBlockId>,    // all blocks in the function
    pub arg_count: u32,                   // number of parameters
}

pub struct MirFunction {
    pub name: NString,
    pub params: ThinVec<LocalId>,        // parameter locals
    pub return_ty: MirTypeId,
    pub is_c_variadic: bool,              // C variadic (...)
    pub body: Option<MirFunctionBody>,    // None for extern declarations
}
```

For function definitions (bodies with code), `body` is `Some(MirFunctionBody { ... })`. For extern/FFI declarations, `body` is `None`. The `is_extern()` method returns `true` when `body.is_none()`, providing a clean check for bodyless functions. Accessor methods (`locals()`, `blocks()`, `local_ids()`, `entry_block()`, `arg_count()`) safely handle both cases, returning empty slices or panicking as appropriate.

Parameters are stored as the first `arg_count` entries in `locals` — the same vector holds parameters, user-declared variables, and compiler-generated temporaries. This uniform treatment simplifies iteration and avoids separate parameter-specific code paths in analysis passes.

### MirGlobal and MirGlobalBody

Global variables follow the same pattern as functions: a `MirGlobal` contains the signature (name, type), while initialization data is separated into an optional `MirGlobalBody`:

```rust
pub struct MirGlobalBody {
    pub initializer_data: Option<ThinVec<u8>>,
}

pub struct MirGlobal {
    pub name: NString,
    pub ty: MirTypeId,
    pub body: Option<MirGlobalBody>,
}
```

For extern/imported globals, `body` is `None`. For defined globals, `body` is `Some(MirGlobalBody { ... })`. The `initializer_data` is `None` for zero-initialized globals (the default for mutable statics) and `Some(...)` for globals with constant initializer data.

### MirModule

A `MirModule` bundles all functions and global variables for a compilation unit:

```rust
pub struct MirModule {
    pub functions: ThinVec<MirFunctionId>,
    pub globals: ThinVec<MirGlobal>,
    pub string_globals: ThinVec<(NString, ThinVec<u8>)>,
    pub ptr_size: PtrSize,
}
```

The module is the unit of code generation — the LLVM codegen iterates over `functions` and `globals` to produce an LLVM module. String literals are stored as byte data in `string_globals` and emitted as private global constant arrays during codegen. The `ptr_size` field determines pointer width (32 or 64 bits) for `USize` resolution and struct layout computation.

### MIR CFG Graphviz Export

`MirFunction` and `MirModule` both provide `emit_dot()` methods that produce Graphviz DOT-format strings suitable for rendering with `dot` or `xdot`. Each function's basic blocks become nodes; terminators become directed edges with color-coded styling (green for `true` branches, red for `false`, blue for `switch`, purple for call returns). Block arguments are shown as edge labels, and entry blocks are distinguished with a comment annotation.

For module-level export, each function is rendered inside its own subgraph cluster, making it easy to visualize the entire compilation unit's control flow structure.

```rust
use nitrate_mir::prelude::*;

// Export a single function
let dot = mir_func.emit_dot();
std::fs::write("function.dot", dot)?;

// Export the entire module
let dot = mir_module.emit_dot();
std::fs::write("module.dot", dot)?;
```

## Builder API

### MirBuilder

`MirBuilder` is the top-level builder for constructing MIR modules. It accumulates `MirFunctionId` values and provides `start_function()` to create a per-function builder. The `build_module(ptr_size)` method finalizes the module:

```rust
let mut builder = MirBuilder::new();
// ... build functions ...
let module = builder.build_module(PtrSize::U64);
```

### MirFunctionBuilder

`MirFunctionBuilder` constructs a single MIR function body. The builder holds all data locally during construction and commits it to the `MirStore` only when `finish_function()` is called, returning a `MirFunctionId`. This local buffering avoids polluting the global store with partially-constructed functions.

**Locals and Parameters**: `add_param(name, ty, mutable)` registers a parameter local and returns its `LocalId`. Parameters must be added before any blocks are created. `new_temp(ty, mutable)` creates a compiler-generated temporary for intermediate computation results.

**Block Management**: `create_block()` creates a new basic block (without arguments) and makes it the current block. `create_block_with_args(arg_types)` creates a block with formal parameters, returning a `NewBlock` containing the block ID and the `arg_locals` that will receive values from predecessor edges. `reserve_block()` and `reserve_block_with_args()` create blocks without switching to them — useful when the block needs to be referenced in terminators before its body is constructed. `switch_to_block(bb_id)` makes a previously reserved block the current block.

The first block created or reserved becomes the entry block.

**Statements**: `push_stmt(stmt)` appends a statement to the current block. Shorthand methods include `push_assign(place, rvalue)`, `push_storage_live(local)`, `push_storage_dead(local)`, and `push_set_discriminant(place, variant_index)`.

**Terminators**: Each terminator method (`goto`, `if_br`, `ret`, `call`, `call_return`, `unreachable`) sets the current block's terminator and finalizes the block (the current block is cleared). Each branch-like terminator has a `_with_args` variant that supplies block arguments to the target.

**Convenience Constructors**: Static methods `rv_use`, `rv_ref`, `rv_binary`, `rv_aggregate`, etc. provide ergonomic rvalue construction. `place_local`, `place_field`, `op_copy`, `op_const`, etc. do the same for places and operands.

**Finish**: `finish_function()` commits the function to the global store. For functions with a body, it constructs the `MirFunction` from the accumulated locals, blocks, and metadata. For extern functions, it creates a `MirFunction` with `body=None`. The builder is then reset for the next function.

## HIR to MIR Lowering

**Crate**: `nitrate_mir_from_hir`

The lowering pass (`lower_hir_to_mir`) converts a validated, monomorphized HIR module into a MIR module. It iterates over all functions in the symbol table and top-level module items, lowering each function independently.

### Type Lowering

HIR types are lowered to MIR types via `lower_type()`. The translation is largely structural: primitives map directly, structs lose their generic parameter information, enums retain their variant structure, references/pointers preserve their exclusivity and mutability flags, and function types capture their lowered parameter and return types. The key simplification is that all `GenericParam` and `Inferred` variants must have been eliminated by the solver before lowering — encountering one is a logic error.

### Expression Lowering

The expression lowerer (`expr::lower_value`) converts HIR `Value` trees into MIR statements. For simple operations (literals, locals, binary operations, unary operations), the HIR value is lowered into an rvalue and assigned to a fresh temporary, and an `Operand::Copy` of that temporary is returned. For control flow (if, while, loop, match), the lowerer creates basic blocks and terminators, using block arguments to merge values from different branches.

**If expressions** lower to a diamond CFG: a condition evaluation block, a true block, a false block, and a merge block with a block argument for the result. The merge block's argument receives the value from whichever branch executes.

**While loops** lower to a header block (condition evaluation), a body block, and an exit block. `break` emits a `Goto` to the exit block; `continue` emits a `Goto` to the header block. Loop nesting is tracked via a `loop_stack` in the lowering context.

**Match expressions** lower to a switch-like structure: the scrutinee is evaluated, then a `SwitchInt` terminator dispatches to per-pattern blocks. Each pattern block binds variables and executes the match arm body, then branches to a merge block with the result as a block argument.

**Short-circuit logical operators** (`&&`, `||`) lower to conditional branches: `a && b` becomes an `If` that evaluates `b` only when `a` is true, with a merge block receiving the result.

### Lowering Context

`LoweringCtx` carries shared state during lowering: the HIR symbol table (for type lookups), a `local_map` from HIR variable names to MIR `LocalId` values, a `loop_stack` for break/continue resolution, and a `value_operand_map` from HIR `ValueId` to lowered `Operand` for propagating block results.

### Block Lowering

`lower_block_elements` processes a sequence of HIR `BlockElement`s (expressions and local declarations). Local declarations create a new MIR local, emit `StorageLive`, lower the initializer, and emit an `Assign`. The last expression in a block becomes the block's result and is stored in the context for the caller.

## MIR Optimization Infrastructure

**Crate**: `nitrate_mir_optimize`

The optimization crate defines two traits for MIR passes, following the visitor/rewriter pattern familiar from LLVM and other compiler frameworks:

### MirOptimization (Per-Function)

```rust
pub trait MirOptimization {
    fn optimize(&mut self, function: &mut mir::MirFunction, log: &CompilerLog);
}
```

Per-function passes operate on a single `MirFunction` and can rewrite its basic blocks, statements, terminators, and locals. They should not modify the function's signature. Examples include constant folding, dead code elimination, copy propagation, and strength reduction.

### MirModuleOptimization (Module-Level)

```rust
pub trait MirModuleOptimization {
    fn optimize(&mut self, module: &mut mir::MirModule, log: &CompilerLog);
}
```

Module-level passes have mutable access to all functions and globals, enabling cross-function optimizations: inlining, dead function elimination, global value numbering, and inter-procedural constant propagation. These passes can add or remove top-level definitions from the module.

The `CompilerLog` parameter allows passes to emit diagnostics (warnings about suspicious code patterns, for example) without aborting compilation.

## MIR to LLVM Code Generation

**Crate**: `nitrate_llvm_from_mir`

The MIR code generator translates `MirModule` into LLVM IR. Because MIR basic blocks and terminators map closely to LLVM basic blocks and branch instructions, this translation is more direct than generating LLVM IR from HIR's tree-structured expressions.

### Two-Pass Function Generation

Functions are generated in two passes: **declarations first**, then **definitions**. The declaration pass creates LLVM function prototypes for all MIR functions, establishing their signatures before any body references them. This handles mutual recursion and forward references correctly. The definition pass compiles each function's body: basic blocks become LLVM basic blocks, `Assign` statements become LLVM store/load instructions (or are eliminated by mem2reg), and terminators become LLVM branch/return/call instructions.

### Type Translation

MIR types map to LLVM types straightforwardly: `Unit`/`Never`→`void`, integers→LLVM integer types, floats→LLVM float/double, `Array`→LLVM array, `Tuple`/`Struct`→LLVM struct, `Reference`/`Pointer`→LLVM pointer, `SliceRef`→fat pointer struct `{T*, i64}`. The `MirStructLayout` provides the byte-level layout information needed for GEP offset computation.

### Block Argument Lowering

Block arguments are lowered to LLVM phi nodes at the target block. For each block argument, a phi instruction is created at the start of the LLVM basic block. Each predecessor edge adds an incoming value to the phi corresponding to the operand passed in the terminator's argument list. This lowers the MLIR-style block argument model into standard SSA phi form for LLVM consumption.

### Operand and Place Translation

`Operand::Copy(place)` emits a load from the place's address. `Operand::Move(place)` also emits a load (the move semantics are not enforced at the LLVM level). `Operand::Constant(lit)` emits the corresponding LLVM constant. Place translation (`gen_place`) follows the HIR codegen's invariant: it always returns a pointer to the place's storage, never a loaded value. Field access on places becomes GEP on the struct pointer. Index access becomes GEP on the array/slice data pointer.

### Module Verification

After code generation, the LLVM module is verified using `module.verify()`. Verification failures trigger a panic with the full module IR printed for debugging. This catches type mismatches, invalid control flow, and malformed instructions early, before they can produce incorrect machine code.

## Removing HIR After MIR Lowering

A key architectural benefit of MIR is that after lowering (`nitrate_mir_from_hir`), the HIR `Store` and parse tree can be dropped entirely. All semantic information necessary for optimization and code generation has been transferred to the MIR: types are lowered to `MirType`, control flow is explicit in the CFG, and all expressions are flattened into statements. This memory savings is significant for large compilation units, as the HIR (with its deeply nested expression DAG, source spans, and type inference metadata) is substantially larger than the MIR.

## Design Decisions

**Why a separate MIR instead of generating LLVM IR directly from HIR?** The HIR is a tree-structured representation with nested expressions, implicit control flow, and type inference artifacts. Generating correct LLVM IR from HIR requires constant vigilance about expression evaluation order, temporary lifetimes, and control flow lowering — all of which the HIR leaves implicit. The MIR makes these aspects explicit: every temporary has a named local, every control flow decision is a basic block terminator, and every operation is a single flat rvalue. This separation of concerns means the HIR→MIR lowering handles the semantic decisions once, and the MIR→LLVM codegen is a straightforward mechanical translation. It also enables optimization passes that operate on a representation designed for analysis rather than code generation.

**Why block arguments instead of phi nodes?** Block arguments make data flow explicit on control flow edges. When reading MIR, you can see exactly what values travel from one block to another without consulting a separate phi instruction in the target block. This is especially valuable during debugging and for optimization passes that rewrite the CFG — when you redirect an edge, you update the terminator's argument list, not a remote phi node. The phi-node approach (used in LLVM IR and traditional SSA) separates the data flow from the control flow in a way that makes CFG transformations more error-prone.

**Why `SetDiscriminant` as a separate statement?** Enum initialization requires setting the discriminant before writing the variant payload, because the discriminant determines which variant's storage is active and where the payload's fields are located. Separating `SetDiscriminant` from payload assignment makes the ordering explicit and enables the codegen to compute the correct GEP for the variant's storage after the discriminant is known.

**Why the builder pattern instead of direct MIR construction?** The builder (`MirFunctionBuilder`) buffers all data locally and only commits to the global store on `finish_function()`. This avoids partially constructed functions polluting the store, enables the builder to enforce invariants (every block has a terminator, blocks are created before they are referenced), and simplifies error handling — if lowering fails mid-function, the builder is simply dropped without affecting the store.

**Why are locals and basic blocks `RefCell`-backed in the store?** MIR optimization passes that rewrite statement sequences, replace terminators, or modify local types need mutable access to individual items while the store is shared across all passes. `RefCell` provides interior mutability without requiring `&mut Store`, which would prevent concurrent analysis of other items. The append-only guarantee ensures that adding new locals or blocks never invalidates existing handles.
