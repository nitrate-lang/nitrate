use std::unimplemented;

use nitrate_hir::prelude as hir;
use nitrate_mir::prelude as mir;

/// Lower a validated HIR module into a MIR module.
///
/// This function converts the high-level, AST-like HIR representation
/// into a control-flow-graph-based MIR representation with basic blocks,
/// flat statements, and explicit terminators.
///
/// # Design
///
/// The lowering process performs the following transformations:
/// 1. Converts HIR `Value` trees into flat MIR `Statement` + `Rvalue` sequences
/// 2. Transforms HIR control flow (`If`, `While`, `Loop`, `Break`, `Continue`,
///    `Return`) into basic blocks with `Terminator` instructions
/// 3. Monomorphizes all types into fully concrete `MirType` values
/// 4. Converts HIR locals/variables into MIR `LocalDecl` entries with SSA form
/// 5. Translates HIR expressions (that are non-branching) into DAG-like Rvalues
///    within basic blocks
///
/// After this lowering, the HIR `Store` and its associated memory can be
/// dropped (freeing all HIR data) since all relevant information has been
/// transferred to the MIR representation.
///
/// # Arguments
///
/// * `hir_module` - The validated HIR module (from `nitrate_hir_validate`)
/// * `mir_store` - The MIR storage to populate with lowered data
/// * `symbol_tab` - The HIR symbol table for name resolution
///
/// # Returns
///
/// A fully constructed `MirModule` containing all lowered functions.
///
/// # TODO
///
/// This function is a placeholder. The actual HIR → MIR lowering
/// will be implemented in a future task.
#[must_use]
pub fn lower_hir_to_mir(
    _hir_module: &hir::Module,
    _mir_store: &mir::MirStore,
    _symbol_tab: &hir::SymbolTab,
) -> mir::MirModule {
    unimplemented!("HIR → MIR lowering is not yet implemented.")
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    #[should_panic(expected = "not yet implemented")]
    fn test_lower_hir_to_mir_placeholder() {
        let store = mir::MirStore::new();
        let tab = hir::SymbolTab::new(hir::PtrSize::U64);
        // We can't easily create a real Module without the full HIR pipeline,
        // so this test just verifies the placeholder panics as expected.
        // The real implementation will replace this.
        let _result = lower_hir_to_mir(
            &hir::Module {
                span: Default::default(),
                visibility: hir::Visibility::Pub,
                name: "test".into(),
                attributes: Default::default(),
                items: vec![],
            },
            &store,
            &tab,
        );
    }
}
