use super::helpers::*;
use nitrate_tree::ast::*;

#[test]
fn test_unary_neg() {
    assert!(matches!(&parse_expr("-42"), Expr::UnaryExpr(u) if u.operator == UnaryExprOp::Sub));
}

#[test]
fn test_unary_not() {
    assert!(matches!(&parse_expr("!true"), Expr::UnaryExpr(u) if u.operator == UnaryExprOp::Not));
}

#[test]
fn test_unary_borrow() {
    assert!(matches!(&parse_expr("&x"), Expr::UnaryExpr(u) if u.operator == UnaryExprOp::Borrow));
}

#[test]
fn test_unary_deref() {
    assert!(matches!(&parse_expr("*ptr"), Expr::UnaryExpr(u) if u.operator == UnaryExprOp::Deref));
}

#[test]
fn test_unary_plus() {
    assert!(matches!(&parse_expr("+42"), Expr::UnaryExpr(u) if u.operator == UnaryExprOp::Add));
}

#[test]
fn test_unary_typeof() {
    assert!(matches!(&parse_expr("typeof x"), Expr::UnaryExpr(u) if u.operator == UnaryExprOp::Typeof));
}
