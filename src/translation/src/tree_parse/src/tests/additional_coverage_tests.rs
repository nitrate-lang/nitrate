use super::helpers::*;
use nitrate_tree::ast::*;

// Additional coverage tests targeting specific remaining uncovered format() arms

// ========== EXPECTED CLOSE BRACE ==========
#[test]
fn test_err_stmt_after_module() {
    // Trigger ExpectedCloseBrace by having statement directly at module level
    let (_, log) = parse_source_no_assert("mod foo { 42 }");
    // Should error because 42 appears in a module context as a statement
    assert!(log.error_bit());
}

// ========== EXPECTED OPEN BRACKET ==========
#[test]
fn test_err_expected_open_bracket() {
    // Trigger by trying to parse something after attribute syntax
    let (_, log) = parse_expr_no_assert("a[");
    assert!(log.error_bit());
}

// ========== EXPECTED CLOSE ANGLE ==========
#[test]
fn test_err_expected_close_angle() {
    // Generic argument issue
    let (_, log) = parse_type_no_assert("Vec<i32");
    assert!(log.error_bit());
}

// ========== FUNCTION PARAMETER VARIADIC EXPECTED ==========
#[test]
fn test_err_fn_variadic_incomplete() {
    // Single dot instead of triple dot should trigger variadic expected
    let (_, log) = parse_source_no_assert("fn f(x: i32, .) {}");
    assert!(log.error_bit());
}

// ========== PATH EXPECTED NAME OR SEPARATOR ==========
#[test]
fn test_err_path_expected_name_or_separator() {
    // Triple colon in expression path triggers ExpectedColon then path recovery
    let (_, log) = parse_source_no_assert("fn f() { foo:::bar; }");
    assert!(log.error_bit());
}

// ========== PATH SEGMENT LIMIT ==========
// NOTE: Would need >65536 segments, impractical

// ========== FUNCTION PARAMETER MISSING NAME ==========
#[test]
fn test_err_fn_param_missing_name() {
    let (_, log) = parse_source_no_assert("fn f(: i32) {}");
    assert!(log.error_bit());
}

// ========== STRUCTURE FIELD LIMIT ==========
// NOTE: Would need >65536 fields, impractical

// ========== ENUM VARIANT LIMIT ==========
// NOTE: Would need >65536 variants, impractical

// ========== CLOSURE PARAMETER LIMIT ==========
// NOTE: Would need >65536 params, impractical

// ========== FUNCTION TYPE PARAM LIMIT ==========
// NOTE: Would need >65536 params, impractical

// ========== VARIADIC FUNCTION PARAMETER MISSING SEMICOLON ==========
#[test]
fn test_err_fn_variadic_close_paren_missing() {
    let (_, log) = parse_source_no_assert("fn f(x: i32, ...");
    assert!(log.error_bit());
}

// ========== EXPECTED OPEN BRACKET IN EXPR ==========
#[test]
fn test_err_expr_index_missing_expr() {
    let (_, log) = parse_expr_no_assert("a[]");
    // Parse `a[]` - `a` is a path, then `[]` is index with missing expression
    assert!(log.error_bit());
}

// ========== EMPTY TUPLE ==========
#[test]
fn test_empty_tuple_expr() {
    let expr = parse_expr("()");
    assert!(matches!(&expr, Expr::Tuple(t) if t.elements.is_empty()));
}

// ========== NAMED GENERIC ARGUMENT ==========
#[test]
fn test_type_named_generic_argument() {
    let ty = parse_type("Map<Key: i32, Value: f64>");
    assert!(matches!(&ty, Type::TypePath(p) if p.segments[0].type_arguments.is_some()));
}

// ========== FOR LOOP VARIADIC BINDING EXPECTED END ==========
#[test]
fn test_err_for_binding_expected_end() {
    let (_, log) = parse_source_no_assert("fn f() { for (x, y in items { } }");
    assert!(log.error_bit());
}

// ========== STRUCT INIT FIELD NAME BEFORE COLON ==========
#[test]
fn test_struct_init_field_name() {
    let expr = parse_expr("Foo { x: 42 }");
    assert!(matches!(&expr, Expr::StructInit(s) if s.fields.len() == 1));
}

// ========== TYPE PATH WITH EMPTY GENERICS ==========
#[test]
fn test_type_path_empty_generics() {
    let ty = parse_type("Foo<>");
    assert!(matches!(&ty, Type::TypePath(_)));
}

// ========== TYPE PATH WITH TRAILING COLON ==========
#[test]
fn test_type_path_trailing_colon() {
    let (_, log) = parse_type_no_assert("Foo::");
    assert!(log.error_bit());
}

// ========== EXPRESSION IN BLOCK ITEM ==========
#[test]
fn test_block_unsafe_expr() {
    let (_, log) = parse_source_no_assert("fn f() { unsafe { 42 } }");
    assert!(!log.error_bit());
}

// ========== BOOLEAN LITERAL ==========
#[test]
fn test_bool_literal() {
    let expr = parse_expr("true");
    assert!(matches!(&expr, Expr::Boolean(b) if b.value));
    let expr = parse_expr("false");
    assert!(matches!(&expr, Expr::Boolean(b) if !b.value));
}

// ========== LONG STRING LITERAL ==========
#[test]
fn test_string_literal() {
    let expr = parse_expr("\"hello world\"");
    assert!(matches!(&expr, Expr::String(_)));
}

// ========== FLOAT LITERAL ==========
#[test]
fn test_float_literal() {
    let expr = parse_expr("3.14");
    assert!(matches!(&expr, Expr::Float(_)));
}

// ========== INTEGER LITERAL ==========
#[test]
fn test_integer_literal() {
    let expr = parse_expr("42");
    assert!(matches!(&expr, Expr::Integer(_)));
}

// ========== TYPE PATH WITH MULTIPLE SEGMENTS ==========
#[test]
fn test_type_path_multi_segment() {
    let ty = parse_type("std::collections::HashMap<K, V>");
    assert!(matches!(&ty, Type::TypePath(p) if p.segments.len() == 3));
}

// ========== REFERENCE WITH LIFETIME + ISO + MUT ==========
#[test]
fn test_type_ref_complex() {
    let ty = parse_type("&'a iso mut i32");
    assert!(matches!(&ty, Type::ReferenceType(_)));
}

// ========== GLOBAL PATH ==========
#[test]
fn test_global_type_path() {
    let ty = parse_type("::std::mem");
    assert!(matches!(&ty, Type::TypePath(p) if p.segments[0].name == ""));
}

// ========== TYPE AS EXPR PREFIX ==========
#[test]
fn test_type_info_parse() {
    let expr = parse_expr("type u8");
    assert!(matches!(&expr, Expr::TypeInfo(_)));
}
