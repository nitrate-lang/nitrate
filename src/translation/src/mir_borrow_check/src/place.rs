//! # Place Semantics and Utilities
//!
//! A `Place` in MIR describes a path to a memory location: a root (`Local` or
//! `Static`) followed by a chain of projections (`Deref`, `Field`, `Index`,
//! `Downcast`). The borrow checker needs precise answers to three families of
//! questions about places:
//!
//! 1. **Memory-identity questions**: do two places refer to the same (or
//!    overlapping) memory? These drive conflict detection between accesses and
//!    active borrows. The crucial subtlety is that `Deref` starts a *new*
//!    memory region: `x` and `*x` do **not** overlap, but `*x` and `(*x).f` do.
//!
//! 2. **Initialization questions**: which places must be initialized (or not
//!    moved-from) before a read or borrow? The `init_chain` of a place is the
//!    set of ancestor places we must verify — walking through `Field`/`Index`/
//!    `Downcast` projections but stopping at (and including) a `Deref` base,
//!    since the pointee's initialization is guaranteed by the reference's
//!    contract once the reference itself is initialized.
//!
//! 3. **Type questions**: what is the type of a place, and is it mutable?
//!    These drive the `&mut` legality check and borrow-value propagation
//!    (copying a reference-typed value carries its borrows with it).

use crate::check::FunctionData;
use nitrate_mir::MirModule;
use nitrate_mir::MirType;
use nitrate_mir::MirTypeId;
use nitrate_mir::Place;
use nitrate_nstring::NString;
use std::format;
use std::matches;
use std::ops::Deref;

/// Return the root place (the `Local`/`Static` leaf with all projections
/// stripped). For `*p.f`, the root is the local holding the reference `p`.
///
/// This is the identity used to determine which local's declaration (mutability,
/// type) governs a place's *storage*.
#[must_use]
pub fn root_place(place: &Place) -> &Place {
    match place {
        Place::Deref(base) | Place::Field { base, .. } | Place::Index { base, .. } | Place::Downcast { base, .. } => {
            root_place(base)
        }
        Place::Local(_) | Place::Static(_) => place,
    }
}

/// Returns `true` if the first projection of `place` is a `Deref`.
///
/// This matters for the "borrow of a local escapes the function" check: a
/// borrow whose first projection is `Deref` is a *reborrow* of memory owned
/// elsewhere and may safely be returned; a borrow of a local (or a field of a
/// local) dangles when the frame is popped.
#[must_use]
pub fn first_projection_is_deref(place: &Place) -> bool {
    matches!(place, Place::Deref(_))
}

/// Returns `true` if `prefix` is a prefix of `other` (or equal), i.e. `other`
/// can be reached by projecting `prefix` further.
///
/// `Deref` is a hard boundary: `x` is **not** a prefix of `*x`, because
/// dereferencing moves to an entirely different memory region. However `*x`
/// *is* a prefix of `(*x).f`, and a `Deref` place can only extend into another
/// `Deref` place with a compatible base.
#[must_use]
pub fn is_prefix(prefix: &Place, other: &Place) -> bool {
    if prefix == other {
        return true;
    }
    match other {
        // Projections that stay within the same memory region: a prefix of the
        // base is also a prefix of the projected place.
        Place::Field { base, .. } | Place::Index { base, .. } | Place::Downcast { base, .. } => is_prefix(prefix, base),
        // Deref starts a new memory region. `prefix` can only be a prefix of
        // `*base` if `prefix` is itself a deref whose own base is a prefix of
        // `base` (i.e. the two places agree on the entire deref spine).
        Place::Deref(base) => match prefix {
            Place::Deref(pbase) => is_prefix(pbase, base),
            _ => false,
        },
        Place::Local(_) | Place::Static(_) => false,
    }
}

/// Returns `true` if the two places refer to overlapping memory, i.e. one is a
/// prefix of the other. Overlap is the test used for borrow conflicts: writing
/// to a place that overlaps an active borrow's source is forbidden.
#[must_use]
pub fn overlaps(a: &Place, b: &Place) -> bool {
    is_prefix(a, b) || is_prefix(b, a)
}

/// The set of ancestor places whose initialization must be verified before
/// `place` can be safely read, written, or borrowed.
///
/// Walks through `Field`/`Index`/`Downcast` projections. When a `Deref` is
/// reached, the deref base (the reference itself) is included and the walk
/// stops: the pointee's validity is the reference's contract.
#[must_use]
pub fn init_chain(place: &Place) -> Vec<Place> {
    let mut chain = Vec::new();
    let mut current = place;
    loop {
        chain.push(current.clone());
        match current {
            Place::Field { base, .. } | Place::Index { base, .. } | Place::Downcast { base, .. } => {
                current = base;
            }
            Place::Deref(base) => {
                chain.push((**base).clone());
                break;
            }
            Place::Local(_) | Place::Static(_) => break,
        }
    }
    chain
}

/// Returns `true` if a borrow of `place` may legally escape the function, i.e.
/// the place is a static or a deref of a local (a reborrow of memory owned
/// elsewhere). Borrows of plain locals (or their fields) dangle when the frame
/// is popped.
#[must_use]
pub fn is_escapable(place: &Place) -> bool {
    if first_projection_is_deref(place) {
        // A reborrow of memory owned elsewhere — always escapable.
        return true;
    }
    matches!(root_place(place), Place::Static(_))
}

/// Returns the `MirTypeId` of the value stored at `place`, if it can be
/// determined. `None` for unknown statics or malformed projections.
#[must_use]
pub fn type_of(place: &Place, function: &FunctionData, module: &MirModule) -> Option<MirTypeId> {
    match place {
        Place::Local(id) => function.local_ty(id),
        Place::Static(name) => static_type(name, module),
        Place::Deref(base) => {
            let base_ty = type_of(base, function, module)?;
            deref_pointee(base_ty)
        }
        Place::Field { base, field_name } => {
            let base_ty = type_of(base, function, module)?;
            field_type(base_ty, field_name)
        }
        Place::Index { base, .. } => {
            let base_ty = type_of(base, function, module)?;
            element_type(base_ty)
        }
        Place::Downcast { base, variant_name } => {
            let base_ty = type_of(base, function, module)?;
            variant_payload_type(base_ty, variant_name)
        }
    }
}

/// Returns `true` if the place's type is a reference-like type (a reference,
/// pointer, or slice reference/pointer). Copying such a value copies a borrow
/// (and therefore extends its region to the copy's liveness).
#[must_use]
pub fn is_reference_typed(place: &Place, function: &FunctionData, module: &MirModule) -> bool {
    type_of(place, function, module).is_some_and(|ty| is_reference_ty(&ty))
}

/// Returns `true` if the type is a reference-like type.
#[must_use]
pub fn is_reference_ty(ty: &MirTypeId) -> bool {
    matches!(
        ty.deref(),
        MirType::Reference { .. }
            | MirType::SliceRef { .. }
            | MirType::Pointer { .. }
            | MirType::SlicePtr { .. }
            | MirType::Str
    )
}

/// Returns whether a reference-like type is mutable, or `None` if the type is
/// not a reference-like type.
#[must_use]
pub fn reference_mutability(ty: &MirTypeId) -> Option<bool> {
    match ty.deref() {
        MirType::Reference { mutable, .. }
        | MirType::SliceRef { mutable, .. }
        | MirType::Pointer { mutable, .. }
        | MirType::SlicePtr { mutable, .. } => Some(*mutable),
        _ => None,
    }
}

/// Returns `true` if `place` can be mutably accessed (borrowed `&mut`).
///
/// Rules:
/// * The root `Local` must be declared mutable.
/// * A `Deref` requires the dereferenced reference/pointer to be mutable;
///   once a mutable deref is reached, the pointee and its projections are all
///   mutable.
/// * `Static` string literals are never mutable. (The MIR module does not yet
///   carry mutability for general globals — `mir_from_hir` emits none — so
///   this is conservative.)
#[must_use]
pub fn is_place_mutable(place: &Place, function: &FunctionData, module: &MirModule) -> bool {
    let mut current = place;
    loop {
        match current {
            Place::Deref(base) => {
                let base_ty = type_of(base, function, module);
                match base_ty.and_then(|ty| reference_mutability(&ty)) {
                    Some(true) => return true,
                    _ => return false,
                }
            }
            Place::Field { base, .. } | Place::Index { base, .. } | Place::Downcast { base, .. } => {
                current = base;
            }
            Place::Local(id) => return function.local_is_mutable(id),
            Place::Static(_) => return false,
        }
    }
}

/// Human-readable rendering of a place for diagnostics. MIR locals carry no
/// names, so they render as `_N` (matching the builder's positional numbering).
#[must_use]
pub fn place_to_string(place: &Place, function: &FunctionData) -> String {
    match place {
        Place::Local(id) => function.local_display_name(id),
        Place::Static(name) => format!("`{}`", &**name),
        Place::Deref(base) => format!("*{}", place_to_string(base, function)),
        Place::Field { base, field_name } => {
            format!("{}.{}", place_to_string(base, function), &**field_name)
        }
        Place::Index { base, index } => {
            format!(
                "{}[{}]",
                place_to_string(base, function),
                place_to_string(index, function)
            )
        }
        Place::Downcast { base, variant_name } => {
            format!("{} as {}", place_to_string(base, function), &**variant_name)
        }
    }
}

// ─────────────────────────────────────────────────────────────
// Internal type helpers
// ─────────────────────────────────────────────────────────────

fn static_type(name: &NString, module: &MirModule) -> Option<MirTypeId> {
    for global in &module.globals {
        if &global.name == name {
            return Some(global.ty.clone());
        }
    }
    // String literal globals are immutable string references.
    for (global_name, _) in &module.string_globals {
        if global_name == name {
            let u8_ty: MirTypeId = MirType::U8.into();
            return Some(
                MirType::SliceRef {
                    exclusive: false,
                    mutable: false,
                    element_type: u8_ty,
                }
                .into(),
            );
        }
    }
    None
}

fn deref_pointee(ty: MirTypeId) -> Option<MirTypeId> {
    match ty.deref() {
        MirType::Reference { to, .. } | MirType::Pointer { to, .. } => Some(to.clone()),
        MirType::SliceRef { element_type, .. } | MirType::SlicePtr { element_type, .. } => Some(element_type.clone()),
        MirType::Str => Some(MirType::U8.into()),
        _ => None,
    }
}

fn field_type(ty: MirTypeId, field_name: &NString) -> Option<MirTypeId> {
    match ty.deref() {
        MirType::Struct { fields, .. } => fields
            .iter()
            .find(|(name, _)| name == field_name)
            .map(|(_, ty)| ty.clone()),
        MirType::Tuple { element_types } => field_name
            .parse::<usize>()
            .ok()
            .and_then(|i| element_types.get(i).cloned()),
        _ => None,
    }
}

fn element_type(ty: MirTypeId) -> Option<MirTypeId> {
    match ty.deref() {
        MirType::Array { element_type, .. }
        | MirType::SliceRef { element_type, .. }
        | MirType::SlicePtr { element_type, .. } => Some(element_type.clone()),
        _ => None,
    }
}

fn variant_payload_type(ty: MirTypeId, variant_name: &NString) -> Option<MirTypeId> {
    match ty.deref() {
        MirType::Enum { variants, .. } => variants
            .iter()
            .find(|v| &v.name == variant_name)
            .and_then(|v| v.payload.clone()),
        _ => None,
    }
}
