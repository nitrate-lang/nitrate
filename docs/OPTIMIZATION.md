# Optimization Subsystem

## Overview and Purpose

The optimization subsystem wraps LLVM's optimization passes and provides a configurable optimization pipeline for Nitrate-compiled code. While the vast majority of optimizations are delegated to LLVM's mature and battle-tested pass infrastructure — refined over two decades of production use in Clang/LLVM — the optimization crate provides the integration layer and infrastructure for future Nitrate-specific optimization passes.

The purpose of optimization is to transform generated LLVM IR into more efficient machine code without changing the program's observable behavior. Optimizations can reduce code size, improve execution speed, reduce memory usage, and canonicalize the IR for further optimization opportunities.

## Architecture

**Crate**: `nitrate_optimization`  
**Key types**: `ModuleOptimizer`, `OptimizationPass` trait  
**Key files**: `lib.rs` (module definitions), `traits.rs` (optimization pass traits)

## Optimization Levels

| Level | LLVM Equivalent | Use Case                                    |
| ----- | --------------- | ------------------------------------------- |
| 0     | `-O0`           | Debug builds, fastest compilation           |
| 1     | `-O1`           | Minimal optimization, quick feedback        |
| 2     | `-O2`           | Standard optimization, balanced performance |
| 3     | `-O3`           | Maximum optimization, release builds        |

## LLVM Optimization Pipeline

At **Level 1**, the optimizer runs Mem2Reg (promotes stack allocations to SSA registers), instruction combining (simplifies patterns), dead instruction elimination, and CFG simplification.

At **Level 2**, all Level 1 passes run plus Global Value Numbering (eliminates redundant computations), Sparse Conditional Constant Propagation, loop invariant code motion, basic inlining, and expression reassociation.

At **Level 3**, all Level 2 passes run plus aggressive function inlining, loop unrolling, loop and SLP vectorization, and interprocedural optimization including argument promotion and constant propagation through call chains.

## Nitrate-Specific Optimization Infrastructure

The `OptimizationPass` trait provides a standard interface for future HIR-level optimizations:

```rust
pub trait OptimizationPass<T> {
    fn optimize(&mut self, input: T) -> T;
}
```

Planned future passes include: dead generic instantiation elimination (removing unused monomorphized copies), specialization of common generic patterns, constant propagation for refinement types (eliminating runtime bounds checks), trait method devirtualization, and custom calling convention optimization.

## Integration

Optimization runs after LLVM IR code generation and before object file emission:

```
Codegen → [LLVM Module] → [ModuleOptimizer] → [Optimized Module] → [Object/Assembly Output]
```

The optimization level is read from `TranslationOptions::optimization_level`. The `ModuleOptimizer` is created with the module and optimization level, and `optimize()` runs the LLVM pass pipeline.
