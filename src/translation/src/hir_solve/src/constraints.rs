use nitrate_hir::{Type, TypeId};
use std::matches;

/// A constraint on a value's type.
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub(crate) enum TypeConstraint {
    /// The value must have exactly this type.
    Equal(TypeId),
    /// The value must be a subtype of this type (for safe variance).
    /// Currently reserved for future use with refinement types and variance.
    SubtypeOf(TypeId),
}

impl TypeConstraint {
    /// Return the TypeId if this is an equality or subtype constraint.
    pub fn type_id(&self) -> TypeId {
        match self {
            TypeConstraint::Equal(ty) => *ty,
            TypeConstraint::SubtypeOf(ty) => *ty,
        }
    }

    /// Create an equality constraint from a Type.
    pub fn eq_type(ty: Type) -> Self {
        TypeConstraint::Equal(TypeId::from(ty))
    }
}

/// The action to take after visiting a value node.
#[derive(Debug)]
pub(crate) enum NodeAction {
    /// No change needed; continue visiting children.
    NoChange,
    /// Replace this value node with a new one.
    Replace(nitrate_hir::Value),
}

/// Helper to propagate parent constraints to child expressions.
///
/// Given a set of parent constraints, this derives appropriate constraints
/// for child nodes. For example, if a binary expression must be `i32`,
/// then both operands should also be `i32`.
///
/// When the parent constraint is a refinement type like `u32::<1..100>`,
/// the full refinement type is propagated to children so that range
/// information flows through the expression tree (e.g., for bounds checking).
pub(crate) fn propagate_to_children(
    parent_constraints: &std::collections::HashSet<TypeConstraint>,
) -> Vec<TypeConstraint> {
    parent_constraints
        .iter()
        .map(|c| {
            let ty = c.type_id();
            // Preserve the full type including refinements so that
            // range information flows to child expressions.
            TypeConstraint::Equal(ty)
        })
        .collect()
}

/// Determine if a binary operation yields a boolean result (comparisons, logical ops).
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

/// Determine if a binary operation is arithmetic (produces same type as operands).
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
