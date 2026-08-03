use std::matches;

use nitrate_hir::{BinaryOp, Type, TypeId, Value};

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
    Replace(Value),
}

pub(crate) fn is_comparison_or_logical_op(op: &BinaryOp) -> bool {
    matches!(
        op,
        BinaryOp::Lt
            | BinaryOp::Gt
            | BinaryOp::Lte
            | BinaryOp::Gte
            | BinaryOp::Eq
            | BinaryOp::Ne
            | BinaryOp::LogicAnd
            | BinaryOp::LogicOr
    )
}

pub(crate) fn is_arithmetic_op(op: &BinaryOp) -> bool {
    matches!(
        op,
        BinaryOp::Add
            | BinaryOp::Sub
            | BinaryOp::Mul
            | BinaryOp::Div
            | BinaryOp::Mod
            | BinaryOp::And
            | BinaryOp::Or
            | BinaryOp::Xor
            | BinaryOp::Shl
            | BinaryOp::Shr
            | BinaryOp::Rol
            | BinaryOp::Ror
    )
}
