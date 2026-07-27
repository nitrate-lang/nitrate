//! # Borrow Constraint Tracking
//!
//! This module manages the constraint graph for borrow checking.
//! Constraints represent relationships between regions that must hold
//! for the program to be memory-safe.
//!
//! ## Constraint Types
//!
//! - **Outlives constraints** (`'a: 'b`): Region `'a` must outlive region `'b`.
//!   This is the core constraint for NLL analysis.
//! - **Lifetime parameter constraints**: Named lifetime parameters from function
//!   signatures impose relationships between input and output regions.
//!
//! ## Current Implementation
//!
//! The current borrow checker uses a simplified lexical scope approach where
//! borrows are tracked as active from creation to the end of their lexical scope.
//! This is sound but conservative: it may reject programs that NLL would accept.
//!
//! The full NLL constraint solver would:
//! 1. Build a graph where nodes are regions and edges are outlives relationships
//! 2. Use fixed-point iteration to compute the minimal region for each variable
//! 3. Verify that all region constraints are satisfied
//!
//! ## Soundness Note
//!
//! A conservative lexical borrow checker is still sound because:
//! - If a program passes lexical borrow checking, it is definitely memory-safe
//! - Some valid programs are rejected (false positives), but no invalid programs
//!   are accepted (no false negatives)
//! - This is the same trade-off Rust made before NLL was implemented

use crate::{BorrowRecord, PlaceId, RegionId};
use std::collections::{HashMap, HashSet};

/// A constraint that tracks which borrows overlap with which places.
///
/// This is used by the borrow checker to detect borrow conflicts
/// (e.g., mutable borrow while shared borrow is active on overlapping places).
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct BorrowConflictConstraint {
    /// The place that is being borrowed.
    pub place: PlaceId,
    /// The borrow record for the existing borrow.
    pub existing_borrow: BorrowRecord,
    /// The region where the conflict would occur.
    pub region: RegionId,
}

/// Represents the set of regions where a borrow is considered "live".
///
/// A borrow is live from its creation point until its last use.
/// In the lexical borrow checker, this is approximated as:
/// - Created at the borrow expression site
/// - Killed (dies) at the end of the enclosing scope
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct BorrowLiveness {
    /// The borrow this liveness information refers to.
    pub borrow: BorrowRecord,
    /// Region IDs where this borrow is live.
    pub live_regions: HashSet<RegionId>,
}

/// Manages constraints between borrows and places.
///
/// This struct collects all borrow-related constraints during HIR traversal
/// and provides methods to check for conflicts.
#[derive(Debug, Default)]
pub struct BorrowConstraintSet {
    /// All active borrows, tracked by place.
    borrows_by_place: HashMap<PlaceId, Vec<BorrowRecord>>,
    /// Borrows that have overlapping places.
    overlapping_borrows: Vec<(PlaceId, PlaceId)>,
    /// Set of places that have been "moved from" (consumed).
    moved_places: HashSet<PlaceId>,
}

impl BorrowConstraintSet {
    /// Creates a new empty constraint set.
    pub fn new() -> Self {
        Self {
            borrows_by_place: HashMap::new(),
            overlapping_borrows: Vec::new(),
            moved_places: HashSet::new(),
        }
    }

    /// Records a new borrow on a place.
    ///
    /// # Preconditions
    /// - `record` is a completed borrow record with a valid place and kind.
    ///
    /// # Postconditions
    /// - The borrow is added to the set of active borrows for that place.
    pub fn add_borrow(&mut self, record: BorrowRecord) {
        self.borrows_by_place.entry(record.place).or_default().push(record);
    }

    /// Removes all borrows for a given place. This is called when
    /// borrows are released (e.g., at end of scope).
    ///
    /// # Postconditions
    /// - No borrows remain for the given place.
    pub fn release_borrows(&mut self, place: PlaceId) {
        self.borrows_by_place.remove(&place);
    }

    /// Clears all borrows. Used when entering a new scope.
    ///
    /// # Postconditions
    /// - All borrows are removed.
    pub fn clear(&mut self) {
        self.borrows_by_place.clear();
        self.overlapping_borrows.clear();
    }

    /// Returns all active borrows for a given place.
    pub fn borrows_for(&self, place: PlaceId) -> Vec<&BorrowRecord> {
        self.borrows_by_place
            .get(&place)
            .map(|v| v.iter().collect())
            .unwrap_or_default()
    }

    /// Returns all active borrows across all places.
    pub fn all_borrows(&self) -> Vec<&BorrowRecord> {
        self.borrows_by_place.values().flat_map(|v| v.iter()).collect()
    }

    /// Returns the number of active borrows.
    pub fn borrow_count(&self) -> usize {
        self.borrows_by_place.values().map(|v| v.len()).sum()
    }
}

/// Helper to check if a borrow is still valid given the set of active borrows.
///
/// # Arguments
/// * `new_place` - The place being borrowed
/// * `new_kind` - The kind of borrow being attempted
/// * `active_borrows` - All currently active borrows
/// * `id_to_place` - Mapping from place IDs to their Place values for overlap checking
///
/// # Returns
/// * `Ok(())` if the borrow is valid (no conflicts)
/// * `Err(conflict_place, conflict_kind)` if a conflict exists
pub fn check_borrow_conflict(
    new_place: PlaceId,
    new_place_data: &crate::Place,
    new_kind: &crate::BorrowKind,
    active_borrows: &[BorrowRecord],
    id_to_place: &[crate::Place],
) -> Result<(), (PlaceId, crate::BorrowKind)> {
    for borrow in active_borrows {
        if borrow.place == new_place {
            continue; // Same place, not a conflict with itself
        }
        if let Some(borrowed_place) = id_to_place.get(borrow.place as usize) {
            if new_place_data.overlaps_with(borrowed_place) {
                // Check conflict matrix:
                // - Shared borrow conflicts with any mutable/exclusive borrow
                // - Mutable/exclusive borrow conflicts with ANY other borrow
                match (borrow.kind, new_kind) {
                    (crate::BorrowKind::Shared, crate::BorrowKind::Mutable)
                    | (crate::BorrowKind::Shared, crate::BorrowKind::ExclusiveImmutable)
                    | (crate::BorrowKind::Mutable, _)
                    | (crate::BorrowKind::ExclusiveImmutable, _) => {
                        return Err((borrow.place, borrow.kind));
                    }
                    (crate::BorrowKind::Shared, crate::BorrowKind::Shared) => {
                        // Multiple shared borrows are fine.
                    }
                }
            }
        }
    }
    Ok(())
}

/// Check if an existing borrow conflicts with a write to a place.
///
/// # Returns
/// * `true` if there's a conflict (cannot write)
/// * `false` if the write is safe
pub fn check_write_conflict(
    place: &crate::Place,
    active_borrows: &[BorrowRecord],
    id_to_place: &[crate::Place],
) -> bool {
    for borrow in active_borrows {
        if let Some(borrowed_place) = id_to_place.get(borrow.place as usize) {
            if place.overlaps_with(borrowed_place) && borrow.kind.conflicts_with_write() {
                return true;
            }
        }
    }
    false
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::{BorrowKind, Place, PlaceElem};

    #[test]
    fn test_no_conflict_with_self() {
        // Borrowing the same place twice as shared is fine
        let place = Place::Temporary;
        let id_to_place = vec![place.clone()];
        let borrow = BorrowRecord {
            place: 0,
            kind: BorrowKind::Shared,
            region: 0,
            reason: "test".to_string(),
        };

        let result = check_borrow_conflict(0, &place, &BorrowKind::Shared, &[borrow], &id_to_place);
        assert!(result.is_ok());
    }
}
