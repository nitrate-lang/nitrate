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

// ========== CAST SUFFIX ==========

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

#[test]
fn test_cast_suffix_name() {
    assert!(matches!(&parse_expr("42mytype"), Expr::Cast(c) if matches!(&c.to, Type::TypePath(_))));
}

#[test]
fn test_cast_suffix_f32() {
    assert!(matches!(&parse_expr("42f32"), Expr::Cast(c) if matches!(&c.to, Type::Float32(_))));
}

#[test]
fn test_cast_suffix_f8() {
    assert!(matches!(&parse_expr("42f8"), Expr::Cast(c) if matches!(&c.to, Type::TypePath(_))));
}

#[test]
fn test_cast_suffix_f16() {
    assert!(matches!(&parse_expr("42f16"), Expr::Cast(c) if matches!(&c.to, Type::TypePath(_))));
}

#[test]
fn test_cast_suffix_f128() {
    assert!(matches!(&parse_expr("42f128"), Expr::Cast(c) if matches!(&c.to, Type::TypePath(_))));
}

// ========== INTEGER SUFFIXES ==========

#[test]
fn test_integer_suffix_u8() {
    let expr = parse_expr("42u8");
    assert!(matches!(&expr, Expr::Cast(c) if matches!(&c.to, Type::UInt8(_))));
}

#[test]
fn test_integer_suffix_u16() {
    let expr = parse_expr("42u16");
    assert!(matches!(&expr, Expr::Cast(c) if matches!(&c.to, Type::UInt16(_))));
}

#[test]
fn test_integer_suffix_u64() {
    let expr = parse_expr("42u64");
    assert!(matches!(&expr, Expr::Cast(c) if matches!(&c.to, Type::UInt64(_))));
}

#[test]
fn test_integer_suffix_u128() {
    let expr = parse_expr("42u128");
    assert!(matches!(&expr, Expr::Cast(c) if matches!(&c.to, Type::UInt128(_))));
}

#[test]
fn test_integer_suffix_usize() {
    let expr = parse_expr("42usize");
    assert!(matches!(&expr, Expr::Cast(c) if matches!(&c.to, Type::USize(_))));
}

#[test]
fn test_integer_suffix_i8() {
    let expr = parse_expr("42i8");
    assert!(matches!(&expr, Expr::Cast(c) if matches!(&c.to, Type::Int8(_))));
}

#[test]
fn test_integer_suffix_i16() {
    let expr = parse_expr("42i16");
    assert!(matches!(&expr, Expr::Cast(c) if matches!(&c.to, Type::Int16(_))));
}

#[test]
fn test_integer_suffix_i64() {
    let expr = parse_expr("42i64");
    assert!(matches!(&expr, Expr::Cast(c) if matches!(&c.to, Type::Int64(_))));
}

#[test]
fn test_integer_suffix_i128() {
    let expr = parse_expr("42i128");
    assert!(matches!(&expr, Expr::Cast(c) if matches!(&c.to, Type::Int128(_))));
}

#[test]
fn test_string_as_bstring() {
    let expr = parse_expr(r#""\xff\xfe""#);
    assert!(matches!(&expr, Expr::BString(_)));
}
