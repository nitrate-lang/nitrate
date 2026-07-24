use super::helpers::*;
use nitrate_tree::ast::*;

// Push coverage for expr.rs and ty.rs uncovered paths

// ========== EXPR: PREFIX WITH NESTED PARENS ==========
#[test]
fn test_nested_prefix_parens() {
    let expr = parse_expr("(&x)");
    assert!(matches!(&expr, Expr::Parentheses(_)));
}

// ========== EXPR: DELETE_PRECEDENCE_EDGE ==========
#[test]
fn test_expr_precedence_rewind() {
    // This tests the rewind path when binop precedence is lower than min
    let expr = parse_expr("1 + 2 + 3");
    // Left associative, so (1+2)+3
    assert!(matches!(&expr, Expr::BinExpr(b) if matches!(b.operator, BinExprOp::Add)));
}

// Note: literal suffix with bool keyword after boolean literal - depends on lexer spacing
// Skipped because "true bool" parses as boolean expression followed by path

// ========== EXPR: TYPE INFO WITH COMPLEX TYPE ==========
#[test]
fn test_type_info_complex() {
    let expr = parse_expr("type (&i32)");
    assert!(matches!(&expr, Expr::TypeInfo(_)));
}

// ========== EXPR: CLOSURE RETURN ARROW ==========
#[test]
fn test_closure_with_return_arrow_missing_gt() {
    let (_, log) = parse_expr_no_assert("fn(x: i32) - i32 { x }");
    assert!(log.error_bit());
}

// ========== EXPR: CLOSURE PARAM DEFAULT ==========
#[test]
fn test_closure_param_default() {
    let expr = parse_expr("fn(x: i32 = 42) { }");
    assert!(matches!(&expr, Expr::Closure(_)));
}

// ========== TYPE: MORE COMPLEX POINTER ==========
#[test]
fn test_type_ptr_poly_const() {
    let ty = parse_type("*poly const i32");
    assert!(matches!(&ty, Type::PointerType(p) if matches!(p.exclusivity, Some(Exclusivity::Poly))));
}

// ========== TYPE: REF WITH CONST MUT ==========
#[test]
fn test_type_ref_const() {
    let ty = parse_type("&const i32");
    assert!(matches!(&ty, Type::ReferenceType(r) if matches!(r.mutability, Some(Mutability::Const))));
}

// ========== TYPE: REF WITH ISO ==========
#[test]
fn test_type_ref_iso() {
    let ty = parse_type("&iso i32");
    assert!(matches!(&ty, Type::ReferenceType(r) if matches!(r.exclusivity, Some(Exclusivity::Iso))));
}

// ========== TYPE: REF WITH LIFETIME ==========
#[test]
fn test_type_ref_lifetime() {
    let ty = parse_type("&'a i32");
    assert!(matches!(&ty, Type::ReferenceType(r) if r.lifetime.is_some()));
}

// ========== TYPE: NESTED PATH ==========
#[test]
fn test_type_nested_path_with_generics() {
    let ty = parse_type("std::collections::HashMap<K, V>");
    assert!(matches!(&ty, Type::TypePath(p) if p.segments.len() >= 2));
}

// ========== TYPE: TUPLE WITH ONE ELEMENT ==========
#[test]
fn test_type_tuple_one_elem() {
    let ty = parse_type("(i32,)");
    assert!(matches!(&ty, Type::TupleType(t) if t.element_types.len() == 1));
}

// ========== TYPE: REFINEMENT WITH ALL FIELDS ==========
#[test]
fn test_type_refine_full() {
    let ty = parse_type("u8: 6: [0:10]");
    assert!(matches!(&ty, Type::RefinementType(_)));
}

// ========== TYPE: LATENT ==========
#[test]
fn test_type_latent_expression() {
    let ty = parse_type("{ 42 }");
    assert!(matches!(&ty, Type::LatentType(_)));
}

// ========== TYPE: ARRAY WITH COMPLEX LENGTH ==========
#[test]
fn test_type_array_complex_len() {
    let ty = parse_type("[i32; 2 + 2]");
    assert!(matches!(&ty, Type::ArrayType(_)));
}

// ========== TYPE: POINTER WITH POLY ==========
#[test]
fn test_type_ptr_poly() {
    let ty = parse_type("*poly i32");
    assert!(matches!(&ty, Type::PointerType(p) if matches!(p.exclusivity, Some(Exclusivity::Poly))));
}

// ========== TYPE: FN WITH MULTIPLE PARAMS ==========
#[test]
fn test_type_fn_multi_params() {
    let ty = parse_type("fn(x: i32, y: f64, z: bool)");
    assert!(matches!(&ty, Type::FunctionType(_)));
}

// ========== TYPE: FN RETURN TYPE ==========
#[test]
fn test_type_fn_return() {
    let ty = parse_type("fn(x: i32) -> f64");
    assert!(matches!(&ty, Type::FunctionType(f) if f.return_type.is_some()));
}

// ========== TYPE: PARENTHESIZED TUPLE ==========
#[test]
fn test_type_paren_in_tuple() {
    let ty = parse_type("((i32, f64))");
    assert!(matches!(&ty, Type::Parentheses(_)));
}

// ========== TYPE: NAMED GENERIC ==========
#[test]
fn test_type_named_generic2() {
    let ty = parse_type("Foo<Bar: i32>");
    assert!(matches!(&ty, Type::TypePath(_)));
}

// ========== TYPE: MISSING SEMICOLON IN ARRAY ==========
#[test]
fn test_type_array_no_semi() {
    let (_, log) = parse_type_no_assert("[i32 10]");
    assert!(log.error_bit());
}
