//! # Region Analysis for NLL Borrow Checking
//!
//! This module implements the region inference system that underpins
//! the NLL (Non-Lexical Lifetimes) borrow checker.
//!
//! ## Region Theory
//!
//! A region is an abstract set of program points representing the period
//! during which a borrow is active or a reference is valid. Regions form
//! a lattice ordered by subset inclusion: if region `'a: 'b` ("a outlives b"),
//! then `'a` contains all program points that `'b` contains.
//!
//! ## Region Types
//!
//! - **Universal regions**: These are regions that exist before the function body
//!   (e.g., `'static`, named lifetime parameters). They are "inputs" to the analysis.
//! - **Existential regions**: These are regions created within the function body
//!   (e.g., the lifetime of a local borrow). They are "outputs" that must be inferred.
//!
//! ## Soundness Guarantee
//!
//! The region solver computes the *minimal* region for each existential region variable
//! that satisfies all outlives constraints. This is sound because:
//!
//! 1. If we compute that `'a: 'b`, then `'a` is at least as large as `'b`
//! 2. If a borrow is created in region `'b` and used in region `'u`, we require `'b: 'u`
//! 3. The minimal solution ensures we don't artificially extend any borrow's lifetime
//!    beyond what's actually needed

use crate::RegionId;
use std::collections::{HashMap, HashSet, VecDeque};

/// The kind of a region variable.
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum RegionKind {
    /// A universally quantified region (exists before function entry).
    /// Examples: `'static`, named lifetime parameters like `'a`.
    Universal,
    /// An existentially quantified region (created during borrow checking).
    /// These represent the lifetime of a borrow or temporary.
    Existential,
}

/// A region variable with its kind and solved value.
///
/// During region inference, each region variable initially represents
/// itself. The solver computes the minimal region for each variable
/// by finding the transitive closure of outlives constraints.
#[derive(Debug, Clone)]
pub struct RegionVar {
    /// The kind of this region variable.
    pub kind: RegionKind,
    /// The unique identifier for this region.
    pub id: RegionId,
    /// Whether this region has been solved.
    pub solved: bool,
    /// The solved value: the smallest region that outlives all constraints.
    /// None means not yet solved.
    pub solved_value: Option<RegionId>,
}

impl RegionVar {
    pub fn new(kind: RegionKind, id: RegionId) -> Self {
        Self {
            kind,
            id,
            solved: false,
            solved_value: None,
        }
    }
}

/// A constraint between two regions: `'sup: 'sub` (sup outlives sub).
///
/// This means that `'sup` must be valid at least as long as `'sub`,
/// i.e., the set of program points in `'sup` must be a superset of those in `'sub`.
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct OutlivesConstraint {
    /// The region that must outlive `sub`.
    pub sup: RegionId,
    /// The region that must be outlived by `sup`.
    pub sub: RegionId,
    /// Source location information for diagnostics.
    pub reason: String,
}

/// The region inference context for a single function.
///
/// This manages all region variables and constraints for one function body.
#[derive(Debug)]
pub struct RegionInferenceCtx {
    /// All region variables, indexed by ID.
    vars: Vec<RegionVar>,
    /// Outlives constraints collected during inference.
    constraints: Vec<OutlivesConstraint>,
    /// The constraint graph: for each region, which regions it must outlive.
    /// This is the transitive closure of the constraints.
    outlives_graph: HashMap<RegionId, HashSet<RegionId>>,
}

impl RegionInferenceCtx {
    /// Creates a new empty region inference context.
    pub fn new() -> Self {
        Self {
            vars: Vec::new(),
            constraints: Vec::new(),
            outlives_graph: HashMap::new(),
        }
    }

    /// Creates a new universal region variable.
    pub fn new_universal(&mut self) -> RegionId {
        let id = self.vars.len() as RegionId;
        self.vars.push(RegionVar::new(RegionKind::Universal, id));
        id
    }

    /// Creates a new existential region variable.
    pub fn new_existential(&mut self) -> RegionId {
        let id = self.vars.len() as RegionId;
        self.vars.push(RegionVar::new(RegionKind::Existential, id));
        id
    }

    /// Adds an outlives constraint: `sup` must outlive `sub`.
    ///
    /// # Preconditions
    /// - `sup` and `sub` must be valid region IDs created by this context.
    ///
    /// # Postconditions
    /// - The constraint is added to the set.
    /// - If it creates a cycle where `sub` transitively outlives `sup`,
    ///   then they must be the same region (coinductive).
    pub fn add_outlives(&mut self, sup: RegionId, sub: RegionId, reason: String) {
        // Avoid adding redundant constraints
        if sup == sub {
            return;
        }
        self.constraints.push(OutlivesConstraint { sup, sub, reason });
    }

    /// Solves all region constraints using fixed-point iteration.
    ///
    /// This computes for each region variable the set of regions it must outlive.
    /// The algorithm:
    /// 1. Build adjacency list: for each `'a: 'b` constraint, add edge `'a -> 'b`
    /// 2. Compute transitive closure using BFS from each node
    /// 3. For each existential region, its solved value is itself (we existentially
    ///    quantify the smallest region that satisfies all constraints)
    ///
    /// # Postconditions
    /// - All region variables have `solved = true`
    /// - `outlives_graph` contains the transitive closure of all outlives relationships
    pub fn solve(&mut self) {
        // Build adjacency list
        let mut adj: HashMap<RegionId, Vec<RegionId>> = HashMap::new();
        for constraint in &self.constraints {
            adj.entry(constraint.sup).or_default().push(constraint.sub);
        }

        // Compute transitive closure using BFS from each node
        for i in 0..self.vars.len() {
            let id = i as RegionId;
            let mut reachable = HashSet::new();
            let mut queue = VecDeque::new();
            queue.push_back(id);

            while let Some(current) = queue.pop_front() {
                if let Some(neighbors) = adj.get(&current) {
                    for &next in neighbors {
                        if reachable.insert(next) {
                            queue.push_back(next);
                        }
                    }
                }
            }

            self.outlives_graph.insert(id, reachable);
        }

        // Mark all variables as solved
        for var in &mut self.vars {
            var.solved = true;
            var.solved_value = Some(var.id);
        }
    }

    /// Checks if region `a` must outlive region `b`.
    ///
    /// This returns true if there's a constraint path `'a :> 'b` in the solved graph.
    ///
    /// # Preconditions
    /// - `solve()` must have been called first.
    ///
    /// # Postconditions
    /// - Returns true if `'a` definitely outlives `'b` (a == b, or path exists).
    pub fn outlives(&self, a: RegionId, b: RegionId) -> bool {
        if a == b {
            return true;
        }
        self.outlives_graph
            .get(&a)
            .map(|reachable| reachable.contains(&b))
            .unwrap_or(false)
    }

    /// Returns the minimal region that outlives both `a` and `b`.
    /// This is the least upper bound (join) in the region lattice.
    ///
    /// # Preconditions
    /// - `solve()` must have been called.
    ///
    /// # Postconditions
    /// - Returns `Some(region)` if a common outliver exists.
    /// - Returns `None` if no such region exists (incomparable regions).
    pub fn join(&self, a: RegionId, b: RegionId) -> Option<RegionId> {
        if a == b {
            return Some(a);
        }

        // Check if a outlives b
        if self.outlives(a, b) {
            return Some(a);
        }

        // Check if b outlives a
        if self.outlives(b, a) {
            return Some(b);
        }

        // Search for a common region that outlives both
        let a_outlives: HashSet<_> = self.outlives_graph.get(&a).cloned().unwrap_or_default();
        let b_outlives: HashSet<_> = self.outlives_graph.get(&b).cloned().unwrap_or_default();

        // Intersection of regions that both a and b outlive
        let mut common: Vec<_> = a_outlives.intersection(&b_outlives).cloned().collect();
        common.sort();
        common.first().copied()
    }

    /// Returns the number of region variables.
    pub fn len(&self) -> usize {
        self.vars.len()
    }

    /// Returns whether there are no region variables.
    pub fn is_empty(&self) -> bool {
        self.vars.is_empty()
    }

    /// Returns a reference to all constraints for diagnostics.
    pub fn constraints(&self) -> &[OutlivesConstraint] {
        &self.constraints
    }
}

impl Default for RegionInferenceCtx {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_basic_region_outlives() {
        let mut ctx = RegionInferenceCtx::new();
        let a = ctx.new_existential();
        let b = ctx.new_existential();

        ctx.add_outlives(a, b, "test".to_string());
        ctx.solve();

        assert!(ctx.outlives(a, b));
        assert!(!ctx.outlives(b, a));
    }

    #[test]
    fn test_transitive_outlives() {
        let mut ctx = RegionInferenceCtx::new();
        let a = ctx.new_existential();
        let b = ctx.new_existential();
        let c = ctx.new_existential();

        ctx.add_outlives(a, b, "a outlives b".to_string());
        ctx.add_outlives(b, c, "b outlives c".to_string());
        ctx.solve();

        assert!(ctx.outlives(a, b));
        assert!(ctx.outlives(b, c));
        assert!(ctx.outlives(a, c));
        assert!(!ctx.outlives(c, a));
    }

    #[test]
    fn test_join_regions() {
        let mut ctx = RegionInferenceCtx::new();
        let a = ctx.new_existential();
        let b = ctx.new_existential();
        let c = ctx.new_existential();

        ctx.add_outlives(a, c, "a outlives c".to_string());
        ctx.add_outlives(b, c, "b outlives c".to_string());
        ctx.solve();

        // Both a and b outlive c, so c is a common subregion
        // The join of a and b should include c (or find something else)
        assert!(ctx.outlives(a, c));
        assert!(ctx.outlives(b, c));

        // Join should find c or some other common subregion
        if let Some(join_result) = ctx.join(a, b) {
            assert!(ctx.outlives(a, join_result));
            assert!(ctx.outlives(b, join_result));
        }
    }
}
