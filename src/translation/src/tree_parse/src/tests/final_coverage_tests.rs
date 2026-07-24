use super::helpers::*;
use nitrate_tree::ast::*;

// Tests to cover remaining reachable format() arms by triggering edge cases

// ========== EXPECTED TYPE ERROR ==========
#[test]
fn test_err_expected_type() {
    let (_, log) = parse_type_no_assert("+");
    assert!(log.error_bit());
}

// ========== TUPLE TYPE IN EXPRESSION ==========
#[test]
fn test_tuple_type_expr() {
    let expr = parse_expr("(1, 2, 3)");
    assert!(matches!(&expr, Expr::Tuple(t) if t.elements.len() == 3));
}

// ========== PAREN EXPRESSION ==========
#[test]
fn test_paren_expression() {
    let expr = parse_expr("(42,)");
    assert!(matches!(&expr, Expr::Tuple(t) if t.elements.len() == 1));
}

// ========== GLOBAL PATH IN TYPE ==========
#[test]
fn test_global_type_path_segments() {
    let ty = parse_type("::std::vec::Vec<i32>");
    assert!(matches!(&ty, Type::TypePath(p) if p.segments[0].name == "" && p.segments.len() >= 3));
}

// ========== EMPTY TYPE PATH ==========
#[test]
fn test_type_path_empty_global() {
    let (_, log) = parse_type_no_assert("::");
    assert!(log.error_bit());
}

// ========== TUPLE TYPE WITH SINGLE ELEMENT ==========
#[test]
fn test_type_paren() {
    let ty = parse_type("(i32)");
    assert!(matches!(&ty, Type::Parentheses(_)));
}

// ========== LONG TYPE PATH ==========
#[test]
fn test_type_path_long() {
    let ty = parse_type("a::b::c::d::e");
    assert!(matches!(&ty, Type::TypePath(p) if p.segments.len() == 5));
}

// ========== EMPTY FUNCTION PARAMS ==========
#[test]
fn test_fn_empty_params() {
    let f = single_function(parse_source("fn f() {}"));
    assert!(f.parameters.params.is_empty());
}

// ========== STATIC TY ==========
#[test]
fn test_static_with_type() {
    let v = single_variable(parse_source("static x: i32 = 0;"));
    assert!(v.ty.is_some());
}

// ========== MISSING SEMICOLON ON TYPE ==========
#[test]
fn test_err_type_alias_no_semi() {
    let (_, log) = parse_source_no_assert("type Foo = i32");
    assert!(log.error_bit());
}

// ========== ENUM WITH DEFAULT VALUE ==========
#[test]
fn test_enum_defaulted_variant() {
    let e = single_enum(parse_source("enum Foo { A = 1, B = 2 }"));
    assert!(e.variants[0].default_value.is_some());
    assert!(e.variants[1].default_value.is_some());
}

// ========== PARENTHESIZED TYPE IN EXPRESSION ==========
#[test]
fn test_paren_type_path_expr() {
    let expr = parse_expr("(foo)");
    // With no comma, this should be Parentheses, not Tuple
    assert!(matches!(&expr, Expr::Parentheses(_)));
}

// ========== CLOSURE WITH DEFAULT PARAM ==========
#[test]
fn test_closure_default_param() {
    let expr = parse_expr("fn(x: i32 = 42) { x }");
    assert!(matches!(&expr, Expr::Closure(_)));
}

// ========== GENERIC ARGUMENT EXPECTED END ==========
#[test]
fn test_err_generic_arg_expected_close() {
    let (_, log) = parse_type_no_assert("Vec<i32,");
    assert!(log.error_bit());
}

// ========== TYPE PATH EXPECTED NAME ==========
#[test]
fn test_err_type_path_expected_name() {
    let (_, log) = parse_source_no_assert("fn f() { let x: 42 = 0; }");
    // 42 as a type should fail
    assert!(log.error_bit());
}

// ========== FUNCTION TYPE EXPECTED OPEN PAREN ==========
#[test]
fn test_fn_type_missing_open_paren2() {
    let (_, log) = parse_type_no_assert("fn i32) -> bool");
    assert!(log.error_bit());
}
