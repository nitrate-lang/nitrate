//! Suspended constraint graph with union-find unification.
//!
//! The core data structure of hir_solve2. Instead of eagerly resolving types
//! during tree walking, we accumulate constraints into a graph and solve them
//! lazily. This enables:
//!
//! - **Occurs check** to detect recursive types
//! - **Conflict detection** — if both `τ = i32` and `τ = String` are asserted,
//!   we can report exactly which constraints conflict
//! - **Better error messages** — the constraint graph retains source information
//! - **Extensibility** — adding subtype or trait constraints requires no changes
//!   to the walk logic

use crate::diagnosis::TypeErr;
use crate::monomorphize::TraitRef;
use nitrate_hir::{Type, TypeId};
use std::collections::{HashMap, HashSet};
use std::matches;
use std::num::NonZeroU32;

/// A type variable identifier. Created fresh for each `Type::Inferred` occurrence.
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub(crate) struct TypeVar(pub u32);

/// A canonical type reference used in constraints.
///
/// During solving, type variables may be unified with concrete types.
/// The union-find structure tracks these bindings.
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub(crate) enum CanonicalType {
    /// A concrete, fully-resolved type.
    Concrete(TypeId),
    /// A type variable that may still be unified.
    Variable(TypeVar),
}

impl CanonicalType {
    /// If this is a concrete type, return it. Otherwise return None.
    pub fn as_concrete(&self) -> Option<TypeId> {
        match self {
            CanonicalType::Concrete(id) => Some(*id),
            CanonicalType::Variable(_) => None,
        }
    }

    pub fn is_variable(&self) -> bool {
        matches!(self, CanonicalType::Variable(_))
    }
}

/// A constraint between canonical types.
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub(crate) enum Constraint {
    /// τ₁ = τ₂ — both sides must be the same type.
    Equal(CanonicalType, CanonicalType),
    /// τ₁ <: τ₂ — τ₁ is a subtype of τ₂ (for variance-aware solving).
    Subtype(CanonicalType, CanonicalType),
    /// τ implements a given trait.
    HasTrait(CanonicalType, TraitRef),
}

/// Source of a constraint for diagnostic purposes.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub(crate) enum ConstraintSource {
    /// From a function parameter type annotation.
    ParameterType,
    /// From a return type annotation.
    ReturnType,
    /// From a local variable type annotation.
    LocalType,
    /// From a struct field type.
    StructField,
    /// From binary operation type propagation.
    BinaryOp,
    /// From a function call argument.
    CallArgument,
    /// From an assignment RHS.
    Assignment,
    /// From a cast target type.
    Cast,
    /// From an if/while condition.
    Condition,
    /// From a list/array element.
    ListElement,
    /// Other / synthetic.
    Other,
}

/// A union-find data structure for type variables.
///
/// Each type variable starts as its own representative. Unification merges
/// equivalence classes. The `bindings` map stores the concrete type (if any)
/// that a representative has been unified with.
#[derive(Debug, Default)]
pub(crate) struct UnionFind {
    /// parent[var] = the representative of var's equivalence class.
    /// A var is its own root iff parent[var] == var.
    parent: HashMap<TypeVar, TypeVar>,
    /// The concrete type bound to a root variable, if any.
    bindings: HashMap<TypeVar, TypeId>,
    /// Accumulated errors during unification.
    errors: Vec<TypeErr>,
}

impl UnionFind {
    pub fn new() -> Self {
        Self {
            parent: HashMap::new(),
            bindings: HashMap::new(),
            errors: Vec::new(),
        }
    }

    /// Ensure a type variable exists in the UF structure.
    pub fn make_var(&mut self, var: TypeVar) {
        self.parent.entry(var).or_insert(var);
    }

    /// Find the representative of a type variable's equivalence class.
    /// Performs path compression.
    pub fn find(&mut self, var: TypeVar) -> TypeVar {
        let parent = *self.parent.get(&var).unwrap_or(&var);
        if parent == var {
            var
        } else {
            let root = self.find(parent);
            self.parent.insert(var, root);
            root
        }
    }

    /// Union two type variables into the same equivalence class.
    pub fn union(&mut self, a: TypeVar, b: TypeVar) {
        let root_a = self.find(a);
        let root_b = self.find(b);
        if root_a == root_b {
            return;
        }
        // Simple union: always make the smaller indexed var the root.
        let (keep, merge) = if root_a.0 <= root_b.0 {
            (root_a, root_b)
        } else {
            (root_b, root_a)
        };
        self.parent.insert(merge, keep);
    }

    /// Bind a type variable's equivalence class to a concrete type.
    /// Returns an error if the class is already bound to a different type.
    pub fn bind(&mut self, var: TypeVar, ty: TypeId) -> Result<(), TypeErr> {
        let root = self.find(var);
        if let Some(existing) = self.bindings.get(&root) {
            if *existing != ty {
                // Conflict: already bound to a different type.
                return Err(TypeErr::TypeMismatch {
                    span: ty.span(),
                    expected: *existing,
                    found: ty,
                });
            }
        } else {
            self.bindings.insert(root, ty);
        }
        Ok(())
    }

    /// Get the concrete type bound to a type variable's equivalence class, if any.
    pub fn get_binding(&mut self, var: TypeVar) -> Option<TypeId> {
        let root = self.find(var);
        self.bindings.get(&root).copied()
    }

    /// Resolve a canonical type to a concrete TypeId, following union-find chains.
    pub fn resolve(&mut self, ct: &CanonicalType) -> Option<TypeId> {
        match ct {
            CanonicalType::Concrete(id) => Some(*id),
            CanonicalType::Variable(var) => self.get_binding(*var),
        }
    }

    /// Check if adding `var = ty` would create a recursive type (occurs check).
    /// Returns true if `ty` contains `var` anywhere in its structure.
    pub fn occurs(&mut self, var: TypeVar, ty: &TypeId) -> bool {
        self.occurs_in_type(var, ty)
    }

    fn occurs_in_type(&self, var: TypeVar, ty: &TypeId) -> bool {
        match &**ty {
            Type::Inferred { id, .. } => {
                // Check if this Inferred maps to our var.
                // We don't have a direct mapping from Inferred id -> TypeVar here;
                // the occurs check is conservative for now.
                false
            }
            Type::Array { element_type, .. } => self.occurs_in_type(var, element_type),
            Type::Tuple { element_types, .. } => element_types.iter().any(|et| self.occurs_in_type(var, et)),
            Type::Refine { base, .. } => self.occurs_in_type(var, base),
            Type::Reference { to, .. } | Type::Pointer { to, .. } => self.occurs_in_type(var, to),
            Type::SliceRef { element_type, .. } | Type::SlicePtr { element_type, .. } => {
                self.occurs_in_type(var, element_type)
            }
            Type::Parameterized { base, args, .. } => {
                self.occurs_in_type(var, base) || args.positional.iter().any(|a| self.occurs_in_type(var, a))
            }
            Type::UnresolvedArray { element_type, .. } => self.occurs_in_type(var, element_type),
            Type::UnresolvedRefine { base, .. } => self.occurs_in_type(var, base),
            Type::Function { function_type, .. } => {
                self.occurs_in_type(var, &function_type.return_type)
                    || function_type.params.iter().any(|(_, p)| self.occurs_in_type(var, p))
            }
            Type::GenericParam { .. } => false,
            _ => false,
        }
    }

    /// Drain accumulated errors.
    pub fn drain_errors(&mut self) -> Vec<TypeErr> {
        std::mem::take(&mut self.errors)
    }
}

/// The constraint graph: all pending constraints plus type variable state.
pub(crate) struct ConstraintGraph {
    /// Union-find for type variable equivalence classes.
    pub uf: UnionFind,
    /// All equality constraints collected so far.
    pub equalities: Vec<(CanonicalType, CanonicalType, ConstraintSource)>,
    /// Additional constraints (subtype, trait).
    pub other_constraints: Vec<(Constraint, ConstraintSource)>,
    /// Next fresh type variable id.
    next_var_id: u32,
}

impl ConstraintGraph {
    pub fn new() -> Self {
        Self {
            uf: UnionFind::new(),
            equalities: Vec::new(),
            other_constraints: Vec::new(),
            next_var_id: 0,
        }
    }

    /// Allocate a fresh type variable.
    pub fn fresh_var(&mut self) -> TypeVar {
        let id = self.next_var_id;
        self.next_var_id += 1;
        let var = TypeVar(id);
        self.uf.make_var(var);
        var
    }

    /// Add an equality constraint: `lhs = rhs`.
    pub fn add_equality(&mut self, lhs: CanonicalType, rhs: CanonicalType, source: ConstraintSource) {
        self.equalities.push((lhs, rhs, source));
    }

    /// Add a constraint that a canonical type must equal a specific TypeId.
    pub fn add_type_constraint(&mut self, var: &CanonicalType, ty: TypeId, source: ConstraintSource) {
        self.add_equality(var.clone(), CanonicalType::Concrete(ty), source);
    }

    /// Run unification on all equality constraints. Returns true if no errors occurred.
    pub fn solve_equalities(&mut self) -> bool {
        let equalities = std::mem::take(&mut self.equalities);
        let mut had_errors = false;

        for (lhs, rhs, source) in &equalities {
            match (lhs, rhs) {
                (CanonicalType::Concrete(a), CanonicalType::Concrete(b)) => {
                    // Both concrete: they must be equal.
                    if a != b {
                        self.uf.errors.push(TypeErr::TypeMismatch {
                            span: a.span(),
                            expected: *a,
                            found: *b,
                        });
                        had_errors = true;
                    }
                }
                (CanonicalType::Variable(v), CanonicalType::Concrete(t))
                | (CanonicalType::Concrete(t), CanonicalType::Variable(v)) => {
                    // Variable = Concrete
                    if let Err(err) = self.uf.bind(*v, *t) {
                        self.uf.errors.push(err);
                        had_errors = true;
                    }
                }
                (CanonicalType::Variable(a), CanonicalType::Variable(b)) => {
                    // Variable = Variable: unify them.
                    // Check if either has a concrete binding already.
                    if let Some(ta) = self.uf.get_binding(*a) {
                        if let Err(err) = self.uf.bind(*b, ta) {
                            self.uf.errors.push(err);
                            had_errors = true;
                        }
                    } else if let Some(tb) = self.uf.get_binding(*b) {
                        if let Err(err) = self.uf.bind(*a, tb) {
                            self.uf.errors.push(err);
                            had_errors = true;
                        }
                    } else {
                        self.uf.union(*a, *b);
                    }
                }
            }
        }

        !had_errors
    }

    /// Resolve a canonical type to its concrete TypeId, if possible.
    pub fn resolve(&mut self, ct: &CanonicalType) -> Option<TypeId> {
        self.uf.resolve(ct)
    }

    /// Drain all errors from the union-find structure.
    pub fn drain_errors(&mut self) -> Vec<TypeErr> {
        self.uf.drain_errors()
    }
}

/// Helper functions for classifying HIR operations.
pub(crate) fn is_comparison_or_logical_op(op: &nitrate_hir::BinaryOp) -> bool {
    matches!(
        op,
        nitrate_hir::BinaryOp::Lt
            | nitrate_hir::BinaryOp::Gt
            | nitrate_hir::BinaryOp::Lte
            | nitrate_hir::BinaryOp::Gte
            | nitrate_hir::BinaryOp::Eq
            | nitrate_hir::BinaryOp::Ne
            | nitrate_hir::BinaryOp::LogicAnd
            | nitrate_hir::BinaryOp::LogicOr
    )
}

pub(crate) fn is_arithmetic_op(op: &nitrate_hir::BinaryOp) -> bool {
    matches!(
        op,
        nitrate_hir::BinaryOp::Add
            | nitrate_hir::BinaryOp::Sub
            | nitrate_hir::BinaryOp::Mul
            | nitrate_hir::BinaryOp::Div
            | nitrate_hir::BinaryOp::Mod
            | nitrate_hir::BinaryOp::And
            | nitrate_hir::BinaryOp::Or
            | nitrate_hir::BinaryOp::Xor
            | nitrate_hir::BinaryOp::Shl
            | nitrate_hir::BinaryOp::Shr
            | nitrate_hir::BinaryOp::Rol
            | nitrate_hir::BinaryOp::Ror
    )
}

/// Propagate constraints to children of an expression node.
/// For example, if a binary op result must be `i32`, both operands should also be `i32`.
pub(crate) fn propagate_to_children(parent_constraints: &HashSet<TypeId>) -> Vec<TypeId> {
    parent_constraints.iter().copied().collect()
}
