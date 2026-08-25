//! # Copy-Semantics Classification
//!
//! Determines whether an HIR type has implicit copy semantics or must be
//! transferred by *move* (ownership transfer). The MIR borrow checker tracks
//! moved-from state through `Operand::Move`; the HIR→MIR lowering uses this
//! predicate to decide whether a place read in rvalue position emits `Move`
//! (non-copy types) or `Copy` (copy types).

use nitrate_hir::prelude::*;
use std::ops::Deref;

/// Whether values of `ty` may be copied implicitly rather than moved.
///
/// Copy types are those with bit-wise trivial duplication semantics:
/// primitives, references, pointers, slices, function types, strings, and
/// ranges. Aggregates are move types: **structs and enums are always non-copy**
/// (they own their storage and are transferred by move), while tuples and
/// arrays are copy when all of their component types are copy.
pub fn hir_type_is_copy(ty: &Type) -> bool {
    match ty {
        // Primitives and pointer-like indirections are always copyable.
        Type::Never { .. }
        | Type::Unit { .. }
        | Type::Bool { .. }
        | Type::U8 { .. }
        | Type::U16 { .. }
        | Type::U32 { .. }
        | Type::U64 { .. }
        | Type::U128 { .. }
        | Type::USize { .. }
        | Type::I8 { .. }
        | Type::I16 { .. }
        | Type::I32 { .. }
        | Type::I64 { .. }
        | Type::I128 { .. }
        | Type::F32 { .. }
        | Type::F64 { .. }
        | Type::Function { .. }
        | Type::Reference { .. }
        | Type::SliceRef { .. }
        | Type::Pointer { .. }
        | Type::SlicePtr { .. }
        | Type::TraitObject { .. }
        | Type::Str { .. }
        | Type::Range { .. } => true,

        // Structs and enums own their fields/payloads: reading them by value
        // transfers ownership, so they are never implicitly copyable.
        Type::Struct { .. } | Type::Enum { .. } => false,

        // Tuples and arrays are copy when every component is copy.
        Type::Tuple { element_types, .. } => element_types.iter().all(|t| hir_type_is_copy(t)),
        Type::Array { element_type, .. } => hir_type_is_copy(element_type),

        // Transparent wrappers resolve to their underlying type.
        Type::TypeAlias { def, .. } => hir_type_is_copy(&def.borrow().type_id.deref()),
        Type::Parameterized { base, .. } => hir_type_is_copy(base),
        Type::Refine { base, .. } => hir_type_is_copy(base),
        Type::UnresolvedRefine { base, .. } => hir_type_is_copy(base),
        Type::UnresolvedArray { element_type, .. } => hir_type_is_copy(element_type),

        // Inference/generic markers should never appear in validated HIR.
        // Treat them as copy so the lowering does not spuriously move values.
        Type::Inferred { .. }
        | Type::InferredInteger { .. }
        | Type::InferredFloat { .. }
        | Type::GenericParam { .. } => true,
    }
}
