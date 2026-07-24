use super::helpers::*;
use nitrate_tree::ast::*;

#[test]
fn test_int_dec() {
    assert!(matches!(&parse_expr("42"), Expr::Integer(i) if i.value == 42));
}

#[test]
fn test_int_hex() {
    assert!(matches!(&parse_expr("0xFF"), Expr::Integer(i) if i.value == 255));
}

#[test]
fn test_int_bin() {
    assert!(matches!(&parse_expr("0b1010"), Expr::Integer(i) if i.value == 10));
}

#[test]
fn test_int_oct() {
    assert!(matches!(&parse_expr("0o77"), Expr::Integer(i) if i.value == 63));
}

#[test]
fn test_int_underscore() {
    assert!(matches!(&parse_expr("1_000"), Expr::Integer(i) if i.value == 1000));
}

#[test]
fn test_int_zero() {
    assert!(matches!(&parse_expr("0"), Expr::Integer(i) if i.value == 0));
}

#[test]
fn test_float() {
    assert!(matches!(&parse_expr("3.14"), Expr::Float(_)));
}

#[test]
fn test_float_zero() {
    assert!(matches!(&parse_expr("0.5"), Expr::Float(_)));
}

#[test]
fn test_str() {
    assert!(matches!(&parse_expr("\"hello\""), Expr::String(s) if s.value == "hello"));
}

#[test]
fn test_str_empty() {
    assert!(matches!(&parse_expr("\"\""), Expr::String(_)));
}

#[test]
fn test_true() {
    assert!(matches!(&parse_expr("true"), Expr::Boolean(b) if b.value));
}

#[test]
fn test_false() {
    assert!(matches!(&parse_expr("false"), Expr::Boolean(b) if !b.value));
}

#[test]
fn test_suffix_u8() {
    assert!(matches!(&parse_expr("42u8"), Expr::Cast(c) if matches!(&c.to, Type::UInt8(_))));
}

#[test]
fn test_suffix_i32() {
    assert!(matches!(&parse_expr("42i32"), Expr::Cast(c) if matches!(&c.to, Type::Int32(_))));
}

#[test]
fn test_suffix_f64() {
    assert!(matches!(&parse_expr("42.0f64"), Expr::Cast(c) if matches!(&c.to, Type::Float64(_))));
}
