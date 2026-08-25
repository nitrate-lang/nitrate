#![forbid(unsafe_code)]

use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude as hir;

/// Per-function HIR optimization pass.
///
/// Each pass receives mutable access to a single function and can rewrite
/// its body, remove dead code, inline calls, etc.
pub trait HirOptimization {
    /// Apply this optimization to a single HIR function.
    ///
    /// The pass may mutate the function body and metadata. It should not
    /// modify the function's signature (name, parameters, return type).
    fn optimize(&mut self, function: &mut hir::Function, log: &CompilerLog);
}

/// Module-level HIR optimization pass.
///
/// These passes operate on the entire HIR module and symbol table, enabling
/// cross-function optimizations such as dead function elimination, global
/// constant propagation, or struct layout optimization.
pub trait HirModuleOptimization {
    /// Apply this optimization to the entire HIR module.
    ///
    /// The pass has mutable access to the module items and the symbol table,
    /// allowing it to add, remove, or rewrite top-level definitions.
    fn optimize(&mut self, module: &mut hir::Module, tab: &mut hir::SymbolTab, log: &CompilerLog);
}
