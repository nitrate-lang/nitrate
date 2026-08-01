use nitrate_hir::{Type, TypeId};
use std::matches;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub(crate) enum TypeConstraint {
    Equal(TypeId),
}

impl TypeConstraint {
    pub fn type_id(&self) -> TypeId {
        match self {
            TypeConstraint::Equal(ty) => *ty,
        }
    }

    pub fn eq_type(ty: Type) -> Self {
        TypeConstraint::Equal(TypeId::from(ty))
    }
}

#[derive(Debug)]
pub(crate) enum NodeAction {
    NoChange,
    Replace(nitrate_hir::Value),
}

pub(crate) fn propagate_to_children(
    parent_constraints: &std::collections::HashSet<TypeConstraint>,
) -> Vec<TypeConstraint> {
    parent_constraints
        .iter()
        .map(|c| {
            let ty = c.type_id();
            TypeConstraint::Equal(ty)
        })
        .collect()
}

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
