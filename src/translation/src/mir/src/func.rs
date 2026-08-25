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

/// Function body data — only present for function definitions (not extern declarations).
///
/// When a function has a body (i.e., it is not an extern/FFI declaration), this
/// struct contains all locals, the basic block CFG, and the entry point.
/// For extern functions, `MirFunction.body` is `None`.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct MirFunctionBody {
    /// All local variable declarations (parameters + temporaries + user variables).
    pub locals: ThinVec<LocalDecl>,

    /// LocalIds for each local, in 1:1 correspondence with `locals`.
    /// These are the global TLS store handles needed for codegen lookups.
    pub local_ids: ThinVec<LocalId>,

    /// The entry basic block (where execution starts).
    pub entry_block: BasicBlockId,

    /// All basic blocks in the function body.
    pub blocks: ThinVec<BasicBlockId>,

    /// Number of "arguments" (parameter locals). The first `arg_count`
    /// entries in `locals` are parameters.
    pub arg_count: u32,
}

/// The complete MIR representation of a function.
///
/// Contains the function signature (name, parameters, return type, variadic flag)
/// and optionally a body (`MirFunctionBody`). For extern/FFI declarations, `body`
/// is `None`. For function definitions, `body` is `Some(...)`.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct MirFunction {
    /// Function name (mangled, for codegen)
    pub name: NString,

    /// Parameter declarations.
    /// Parameters are stored as locals — the first N locals are params.
    pub params: ThinVec<LocalId>,

    /// Return type of the function.
    pub return_ty: MirTypeId,

    /// Whether this function is C-variadic (has `...` in its parameter list).
    pub is_c_variadic: bool,

    /// The function body, if this is a definition (not just a declaration).
    /// `None` for extern functions, FFI imports, and forward declarations.
    pub body: Option<MirFunctionBody>,
}

impl MirFunction {
    #[must_use]
    pub fn new(
        name: NString,
        params: ThinVec<LocalId>,
        return_ty: MirTypeId,
        is_c_variadic: bool,
        body: Option<MirFunctionBody>,
    ) -> Self {
        Self {
            name,
            params,
            return_ty,
            is_c_variadic,
            body,
        }
    }

    /// Iterate over all basic blocks in the function body.
    /// Returns an empty iterator for extern functions.
    #[must_use]
    pub fn iter_blocks(&self) -> impl Iterator<Item = &BasicBlockId> {
        self.body.as_ref().map(|b| b.blocks.iter()).into_iter().flatten()
    }

    /// Returns true if the function has no body (external/extern function).
    #[must_use]
    pub fn is_extern(&self) -> bool {
        self.body.is_none()
    }

    /// Access the function body, panicking if it's an extern function.
    /// Use this only in contexts where a body is guaranteed to exist
    /// (e.g., inside `gen_function` after checking `!is_extern()`).
    #[must_use]
    pub fn body_or_panic(&self) -> &MirFunctionBody {
        self.body
            .as_ref()
            .expect("body_or_panic called on extern function with no body")
    }

    /// Returns the arg_count from the body, or 0 if extern.
    #[must_use]
    pub fn arg_count(&self) -> u32 {
        self.body.as_ref().map_or(0, |b| b.arg_count)
    }

    /// Returns the locals from the body, or an empty slice if extern.
    #[must_use]
    pub fn locals(&self) -> &[LocalDecl] {
        self.body.as_ref().map_or(&[], |b| b.locals.as_slice())
    }

    /// Returns the local_ids from the body, or an empty slice if extern.
    #[must_use]
    pub fn local_ids(&self) -> &[LocalId] {
        self.body.as_ref().map_or(&[], |b| b.local_ids.as_slice())
    }

    /// Returns the entry block from the body, panicking if extern.
    #[must_use]
    pub fn entry_block(&self) -> &BasicBlockId {
        &self.body_or_panic().entry_block
    }

    /// Returns the blocks from the body, or an empty slice if extern.
    #[must_use]
    pub fn blocks(&self) -> &[BasicBlockId] {
        self.body.as_ref().map_or(&[], |b| b.blocks.as_slice())
    }
}

// ─────────────────────────────────────────────────────────────
// MIR Global Variables
// ─────────────────────────────────────────────────────────────

/// Global variable body data — only present for globals with an initializer.
///
/// For declarations-only globals (extern imports), `MirGlobal.body` is `None`.
/// For defined globals, the body provides initialization data.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct MirGlobalBody {
    /// Initializer data as raw bytes, if this global has a constant initializer.
    /// `None` means zero-initialized (default for mutable statics).
    pub initializer_data: Option<ThinVec<u8>>,
}

/// A global variable in the MIR representation.
///
/// Contains the global's signature (name, type) and optionally a body with
/// initialization data. For extern/imported globals, `body` is `None`.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct MirGlobal {
    /// Global variable name (mangled).
    pub name: NString,

    /// Type of the global variable.
    pub ty: MirTypeId,

    /// The global's body (initializer), if this is a definition.
    /// `None` for extern/imported global declarations.
    pub body: Option<MirGlobalBody>,
}

impl MirGlobal {
    #[must_use]
    pub fn new(name: NString, ty: MirTypeId, body: Option<MirGlobalBody>) -> Self {
        Self { name, ty, body }
    }

    /// Returns true if this is an extern declaration (no body/initializer).
    #[must_use]
    pub fn is_extern(&self) -> bool {
        self.body.is_none()
    }
}

// ─────────────────────────────────────────────────────────────
// Module-level MIR
// ─────────────────────────────────────────────────────────────

/// A complete MIR module, containing all functions for a compilation unit.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct MirModule {
    /// Functions defined in this module.
    pub functions: ThinVec<MirFunctionId>,
    /// Global variable declarations and definitions for this module.
    pub globals: ThinVec<MirGlobal>,
    /// String literal data to be emitted as global constants.
    /// Each entry is (global_name, byte_data). The name is used
    /// to create a `Place::Static(name)` for borrowing string literals.
    pub string_globals: ThinVec<(NString, ThinVec<u8>)>,
    /// Target pointer size
    pub ptr_size: PtrSize,
}

impl MirModule {
    #[must_use]
    pub fn new(
        functions: ThinVec<MirFunctionId>,
        globals: ThinVec<MirGlobal>,
        string_globals: ThinVec<(NString, ThinVec<u8>)>,
        ptr_size: PtrSize,
    ) -> Self {
        Self {
            functions,
            globals,
            string_globals,
            ptr_size,
        }
    }
}
