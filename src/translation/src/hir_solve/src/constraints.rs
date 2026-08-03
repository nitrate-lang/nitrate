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

// ── Tests ────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;
    use nitrate_hir::{BinaryOp, Store, Type, TypeId, Value, using_storage};
    use nitrate_tree::SrcPos;

    fn sp() -> SrcPos {
        SrcPos::default()
    }

    #[test]
    fn type_constraint_equal_holds_type_id() {
        let store = Store::new();
        using_storage(&store, || {
            let tid = TypeId::from(Type::I32 { span: sp() });
            let c = TypeConstraint::Equal(tid);
            assert_eq!(c.type_id(), tid);
        });
    }

    #[test]
    fn type_constraint_eq_type() {
        let store = Store::new();
        using_storage(&store, || {
            let ty = Type::Bool { span: sp() };
            let c = TypeConstraint::eq_type(ty.clone());
            assert_eq!(c, TypeConstraint::Equal(TypeId::from(ty)));
        });
    }

    #[test]
    fn type_constraint_clone_works() {
        let store = Store::new();
        using_storage(&store, || {
            let tid = TypeId::from(Type::I32 { span: sp() });
            let c1 = TypeConstraint::Equal(tid);
            let c2 = c1.clone();
            assert_eq!(c1, c2);
        });
    }

    #[test]
    fn node_action_no_change() {
        let action = NodeAction::NoChange;
        assert!(matches!(action, NodeAction::NoChange));
    }

    #[test]
    fn node_action_replace() {
        let store = Store::new();
        using_storage(&store, || {
            let val = Value::Unit { span: sp() };
            let action = NodeAction::Replace(val.clone());
            match action {
                NodeAction::Replace(v) => assert_eq!(v, val),
                _ => panic!("expected Replace"),
            }
        });
    }

    #[test]
    fn comparison_ops_are_comparison_or_logical() {
        assert!(is_comparison_or_logical_op(&BinaryOp::Lt));
        assert!(is_comparison_or_logical_op(&BinaryOp::Gt));
        assert!(is_comparison_or_logical_op(&BinaryOp::Lte));
        assert!(is_comparison_or_logical_op(&BinaryOp::Gte));
        assert!(is_comparison_or_logical_op(&BinaryOp::Eq));
        assert!(is_comparison_or_logical_op(&BinaryOp::Ne));
    }

    #[test]
    fn logical_ops_are_comparison_or_logical() {
        assert!(is_comparison_or_logical_op(&BinaryOp::LogicAnd));
        assert!(is_comparison_or_logical_op(&BinaryOp::LogicOr));
    }

    #[test]
    fn arithmetic_ops_are_not_comparison_or_logical() {
        assert!(!is_comparison_or_logical_op(&BinaryOp::Add));
        assert!(!is_comparison_or_logical_op(&BinaryOp::Mul));
        assert!(!is_comparison_or_logical_op(&BinaryOp::Div));
    }

    #[test]
    fn arithmetic_ops_are_arithmetic() {
        assert!(is_arithmetic_op(&BinaryOp::Add));
        assert!(is_arithmetic_op(&BinaryOp::Sub));
        assert!(is_arithmetic_op(&BinaryOp::Mul));
        assert!(is_arithmetic_op(&BinaryOp::Div));
        assert!(is_arithmetic_op(&BinaryOp::Mod));
        assert!(is_arithmetic_op(&BinaryOp::And));
        assert!(is_arithmetic_op(&BinaryOp::Or));
        assert!(is_arithmetic_op(&BinaryOp::Xor));
        assert!(is_arithmetic_op(&BinaryOp::Shl));
        assert!(is_arithmetic_op(&BinaryOp::Shr));
        assert!(is_arithmetic_op(&BinaryOp::Rol));
        assert!(is_arithmetic_op(&BinaryOp::Ror));
    }

    #[test]
    fn comparison_ops_are_not_arithmetic() {
        assert!(!is_arithmetic_op(&BinaryOp::Lt));
        assert!(!is_arithmetic_op(&BinaryOp::Gt));
        assert!(!is_arithmetic_op(&BinaryOp::Eq));
        assert!(!is_arithmetic_op(&BinaryOp::Ne));
        assert!(!is_arithmetic_op(&BinaryOp::LogicAnd));
        assert!(!is_arithmetic_op(&BinaryOp::LogicOr));
    }
}
