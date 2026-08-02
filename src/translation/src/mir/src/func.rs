use crate::store::*;
use crate::ty::PtrSize;
use nitrate_nstring::NString;
use serde::{Deserialize, Serialize};
use thin_vec::ThinVec;

// ─────────────────────────────────────────────────────────────
// Local declarations
// ─────────────────────────────────────────────────────────────

/// A local variable declaration. Each local has a known type and mutability.
/// In SSA form, each local is assigned exactly once within a function.
/// Multiple versions of the same source variable get different LocalIds.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct LocalDecl {
    pub ty: MirTypeId,
    pub mutable: bool,
}

// ─────────────────────────────────────────────────────────────
// MIR Function
// ─────────────────────────────────────────────────────────────

/// The complete MIR representation of a function body.
/// Contains local declarations, the control flow graph (as basic blocks),
/// and function metadata.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct MirFunction {
    /// Function name (mangled, for codegen)
    pub name: NString,

    /// Parameter declarations.
    /// Parameters are stored as locals — the first N locals are params.
    /// This is intentionally typed as Vec<LocalId> not Option.
    pub params: ThinVec<LocalId>,

    /// Return type of the function.
    pub return_ty: MirTypeId,

    /// All local variable declarations (parameters + temporaries + user variables).
    pub locals: ThinVec<LocalDecl>,

    /// The entry basic block (where execution starts).
    pub entry_block: BasicBlockId,

    /// All basic blocks in the function body.
    pub blocks: ThinVec<BasicBlockId>,

    /// Number of "arguments" (parameter locals). The first `arg_count`
    /// entries in `locals` are parameters.
    pub arg_count: u32,
}

impl MirFunction {
    #[must_use]
    pub fn new(
        name: NString,
        params: ThinVec<LocalId>,
        return_ty: MirTypeId,
        locals: ThinVec<LocalDecl>,
        entry_block: BasicBlockId,
        blocks: ThinVec<BasicBlockId>,
        arg_count: u32,
    ) -> Self {
        Self {
            name,
            params,
            return_ty,
            locals,
            entry_block,
            blocks,
            arg_count,
        }
    }

    /// Iterate over all basic blocks in the function.
    #[must_use]
    pub fn iter_blocks(&self) -> impl Iterator<Item = &BasicBlockId> {
        self.blocks.iter()
    }

    /// Returns true if the function has no body (external/extern function).
    #[must_use]
    pub fn is_extern(&self) -> bool {
        self.blocks.is_empty()
    }
}

// ─────────────────────────────────────────────────────────────
// Module-level MIR
// ─────────────────────────────────────────────────────────────

/// A complete MIR module, containing all functions for a compilation unit.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct MirModule {
    /// Functions defined in this module (mangled name → function).
    pub functions: ThinVec<MirFunctionId>,
    /// Global variable declarations referenced by this module.
    pub globals: ThinVec<(NString, MirTypeId)>,
    /// Target pointer size
    pub ptr_size: PtrSize,
}

impl MirModule {
    #[must_use]
    pub fn new(functions: ThinVec<MirFunctionId>, globals: ThinVec<(NString, MirTypeId)>, ptr_size: PtrSize) -> Self {
        Self {
            functions,
            globals,
            ptr_size,
        }
    }
}
