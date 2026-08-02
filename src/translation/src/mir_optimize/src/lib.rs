use nitrate_diagnosis::CompilerLog;
use nitrate_mir::prelude as mir;

/// Per-function MIR optimization pass.
///
/// Each pass receives mutable access to a single MIR function and can rewrite
/// its basic blocks, statements, terminators, and locals. Passes run on the
/// control-flow-graph representation, enabling dataflow analyses, dead code
/// elimination, constant folding, and other standard compiler optimizations.
pub trait MirOptimization {
    /// Apply this optimization to a single MIR function.
    ///
    /// The pass may mutate the function's basic blocks, statements, terminators,
    /// and locals. It should not modify the function's signature (name,
    /// parameters, return type) unless it is an IPA (inter-procedural analysis)
    /// pass, which is handled via `MirModuleOptimization`.
    fn optimize(&mut self, function: &mut mir::MirFunction, log: &CompilerLog);
}

/// Module-level MIR optimization pass.
///
/// These passes operate on the entire MIR module, enabling cross-function
/// optimizations such as inlining, dead function elimination, global value
/// numbering across functions, or inter-procedural constant propagation.
pub trait MirModuleOptimization {
    /// Apply this optimization to the entire MIR module.
    ///
    /// The pass has mutable access to all functions and globals in the module,
    /// allowing it to add, remove, or rewrite top-level definitions.
    fn optimize(&mut self, module: &mut mir::MirModule, log: &CompilerLog);
}
