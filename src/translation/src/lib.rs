pub mod pipeline;

pub use nitrate_hir as hir;
pub use nitrate_hir_dump as hir_dump;
pub use nitrate_hir_evaluate as hir_evaluate;
pub use nitrate_hir_from_tree as hir_from_tree;
pub use nitrate_hir_mangle as hir_mangle;
pub use nitrate_hir_optimize as hir_optimize;
pub use nitrate_hir_solve as hir_solve;
pub use nitrate_hir_type as hir_type;
pub use nitrate_hir_validate as hir_validate;
pub use nitrate_llvm as llvm;
pub use nitrate_llvm_from_mir as llvm_from_mir;
pub use nitrate_mir as mir;
pub use nitrate_mir_borrow_check as mir_borrow_check;
pub use nitrate_mir_from_hir as mir_from_hir;
pub use nitrate_mir_optimize as mir_optimize;
pub use nitrate_nstring as nstring;
pub use nitrate_token as token;
pub use nitrate_token_lexer as token_lexer;
pub use nitrate_tree as parsetree;
pub use nitrate_tree_parse as parse;
pub use nitrate_tree_resolve as tree_resolve;

pub use pipeline::{
    Emitted, HirLowered, HirMangled, HirOptimized, HirValidated, LlvmGenerated, LlvmOptimized, MirLowered,
    MirOptimized, Parsed, Pipeline, PipelineConfig, PipelineError, Source, Tokenized,
};

use nitrate_diagnosis::DiagnosticExplanation;
use std::sync::OnceLock;

/// Every diagnostic error-code explanation registered by all compilation
/// stages.
///
/// This is the single source of truth for `no3 --explain <CODE>`: the driver
/// builds its lookup table from this slice, and each stage contributes its
/// error definitions through its `explanations()` function (kept in the
/// stage's `diagnosis` module, next to the error types they document).
#[must_use]
pub fn diagnostic_explanations() -> &'static [DiagnosticExplanation] {
    static ALL: OnceLock<Vec<DiagnosticExplanation>> = OnceLock::new();
    ALL.get_or_init(|| {
        let mut all = Vec::new();
        all.extend_from_slice(parse::explanations());
        all.extend_from_slice(tree_resolve::explanations());
        all.extend_from_slice(hir_from_tree::explanations());
        all.extend_from_slice(hir_solve::explanations());
        all.extend_from_slice(hir_validate::explanations());
        all.extend_from_slice(mir_borrow_check::explanations());
        all.extend_from_slice(hir_evaluate::explanations());
        all
    })
}
