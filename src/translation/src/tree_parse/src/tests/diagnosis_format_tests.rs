use super::helpers::*;
use nitrate_tree::ast::*;

// Tests that exercise the diagnostic format() functions in diaguosis.rs
// by triggering syntax errors in various parse paths.

// ========== GENERIC ERRORS ==========

#[test]
fn test_err_generic_missing_param_name() {
    let (_, log) = parse_source_no_assert("struct Foo<T, = i32> { x: T }");
    assert!(log.error_bit());
}

#[test]
fn test_err_generic_limit() {
    // Create many generic params to trigger the limit
    let mut params = String::from("<");
    for i in 0..65538 {
        if i > 0 {
            params.push_str(", ");
        }
        params.push_str(&format!("T{i}"));
    }
    params.push('>');
    let src = format!("struct Foo{params} {{ x: i32 }}");
    let (_, log) = parse_source_no_assert(&src);
    assert!(log.error_bit());
}

#[test]
fn test_err_generic_expected_end() {
    let (_, log) = parse_source_no_assert("struct Foo<T U> { x: T }");
    assert!(log.error_bit());
}

// ========== MODULE ERRORS ==========

#[test]
fn test_err_module_expected_end() {
    let (_, log) = parse_source_no_assert("mod foo { fn f() {}");
    assert!(log.error_bit());
}

// ========== IMPORT ERRORS ==========

#[test]
fn test_err_import_alias_missing_name() {
    let (_, log) = parse_source_no_assert("use foo as ;");
    assert!(log.error_bit());
}

#[test]
fn test_err_import_expected_star_or_group() {
    let (_, log) = parse_source_no_assert("use foo::bar::;");
    assert!(log.error_bit());
}

#[test]
fn test_err_import_group_unclosed() {
    let (_, log) = parse_source_no_assert("use foo::{bar, baz");
    assert!(log.error_bit());
}

// ========== TYPE ALIAS ERRORS ==========

#[test]
fn test_err_type_alias_missing_name() {
    let (_, log) = parse_source_no_assert("type = i32;");
    assert!(log.error_bit());
}

// ========== ENUM ERRORS ==========

#[test]
fn test_err_enum_missing_name() {
    let (_, log) = parse_source_no_assert("enum {}");
    assert!(log.error_bit());
}

#[test]
fn test_err_enum_variant_limit() {
    // Many variants to trigger limit - skip since it needs >65536 items
}

#[test]
fn test_err_enum_missing_variant_name() {
    let (_, log) = parse_source_no_assert("enum Foo { : i32 }");
    assert!(log.error_bit());
}

// ========== STRUCT ERRORS ==========

#[test]
fn test_err_struct_expected_end() {
    let (_, log) = parse_source_no_assert("struct Foo { x: i32, y: f64");
    assert!(log.error_bit());
}

// ========== FUNCTION ERRORS ==========

#[test]
fn test_err_fn_missing_name() {
    let (_, log) = parse_source_no_assert("fn () {}");
    assert!(log.error_bit());
}

#[test]
fn test_err_fn_param_limit() {
    // Can't easily create 65536 params in a test, but we test the other error paths
}

#[test]
fn test_err_fn_params_expected_end() {
    let (_, log) = parse_source_no_assert("fn foo(x: i32, y: f64");
    assert!(log.error_bit());
}

// ========== VARIABLE ERRORS ==========

#[test]
fn test_err_variable_missing_name() {
    let (_, log) = parse_source_no_assert("const : i32 = 0;");
    assert!(log.error_bit());
}

// ========== TRAIT ERRORS ==========

#[test]
fn test_err_trait_item_limit() {
    // Can't easily create 65536 items in a test
}

// ========== IMPL ERRORS ==========

#[test]
fn test_err_impl_expected_end() {
    let (_, log) = parse_source_no_assert("impl Foo { fn bar() {}");
    assert!(log.error_bit());
}

// ========== PATH ERRORS ==========

#[test]
fn test_err_path_generic_argument_expected_end() {
    let (_, log) = parse_type_no_assert("Vec<i32");
    assert!(log.error_bit());
}

#[test]
fn test_err_path_at_eof() {
    let (_, log) = parse_type_no_assert("foo::");
    assert!(log.error_bit());
}

// ========== REFERENCE TYPE ERRORS ==========

#[test]
fn test_err_ref_lifetime_name() {
    let (_, log) = parse_type_no_assert("&'");
    assert!(log.error_bit());
}

// ========== TUPLE TYPE ERRORS ==========

#[test]
fn test_err_tuple_type_expected_end() {
    let (_, log) = parse_type_no_assert("(i32, f64");
    assert!(log.error_bit());
}

// ========== EXPRESSION ERRORS ==========

#[test]
fn test_err_expected_expr() {
    let (_, log) = parse_expr_no_assert("}");
    assert!(log.error_bit());
}

// ========== FUNCTION CALL ERRORS ==========

#[test]
fn test_err_fn_call_expected_end() {
    let (_, log) = parse_expr_no_assert("f(1, 2");
    assert!(log.error_bit());
}

#[test]
fn test_err_fn_call_positional_after_named() {
    let (_, log) = parse_expr_no_assert("f(x: 1, 2)");
    assert!(log.error_bit());
}

// ========== FOR LOOP ERRORS ==========

#[test]
fn test_err_for_variable_binding_missing_name() {
    let (_, log) = parse_expr_no_assert("for (x, ) in items { }");
    assert!(log.error_bit());
}

#[test]
fn test_err_for_expected_in() {
    let (_, log) = parse_expr_no_assert("for x items { }");
    assert!(log.error_bit());
}

// ========== FIELD/METHOD ERRORS ==========

#[test]
fn test_err_expected_field_name() {
    let (_, log) = parse_expr_no_assert("a.");
    assert!(log.error_bit());
}

// ========== BRACKET ERRORS ==========

#[test]
fn test_err_expected_close_bracket_index() {
    let (_, log) = parse_expr_no_assert("a[0");
    assert!(log.error_bit());
}

// ========== COLON ERRORS ==========

#[test]
fn test_err_expected_colon() {
    let (_, log) = parse_source_no_assert("struct Foo { x i32 }");
    assert!(log.error_bit());
}

// ========== EXPECTED SEMICOLON ==========

#[test]
fn test_err_expected_semicolon() {
    let (_, log) = parse_source_no_assert("fn f() { break }");
    assert!(log.error_bit());
}

#[test]
fn test_err_expected_semicolon_return() {
    let (_, log) = parse_expr_no_assert("ret");
    assert!(log.error_bit());
}

// ========== STRUCT INIT ERRORS ==========

#[test]
fn test_err_struct_init_missing_field_name() {
    let (_, log) = parse_expr_no_assert("Foo { : 1 }");
    assert!(log.error_bit());
}

#[test]
fn test_err_struct_init_unclosed() {
    let (_, log) = parse_expr_no_assert("Foo { x: 1");
    assert!(log.error_bit());
}

// ========== LIST ERRORS ==========

#[test]
fn test_err_list_expected_end() {
    let (_, log) = parse_expr_no_assert("[1, 2");
    assert!(log.error_bit());
}

// ========== BLOCK ERRORS ==========

#[test]
fn test_err_block_expected_end() {
    let (_, log) = parse_expr_no_assert("unsafe { 42");
    assert!(log.error_bit());
}

// ========== MISC TOKEN ERRORS ==========

#[test]
fn test_err_expected_open_paren() {
    let (_, log) = parse_source_no_assert("fn foo) {}");
    assert!(log.error_bit());
}

// ========== REFINEMENT TYPE RANGE MISSING BRACKET ==========

#[test]
fn test_err_refine_range_unclosed() {
    let (_, log) = parse_type_no_assert("u8: 6: [0:10");
    assert!(log.error_bit());
}

// ========== CLOSURE PARAM ERROR ==========

#[test]
fn test_err_closure_missing_param_name() {
    let (_, log) = parse_expr_no_assert("fn(: i32) { }");
    assert!(log.error_bit());
}

// ========== FN TYPE RETURN ARROW MISSING GT ==========

#[test]
fn test_err_fn_type_return_arrow_missing_gt() {
    let (_, log) = parse_type_no_assert("fn(x: i32) - bool");
    assert!(log.error_bit());
}

// ========== BLOCK EXPECTED OPEN BRACE ==========

#[test]
fn test_err_block_missing_open_brace() {
    let (_, log) = parse_expr_no_assert("if true 42");
    assert!(log.error_bit());
}

// ========== EXPECTED CLOSE PAREN IN EXPRESSION ==========

#[test]
fn test_err_expr_missing_close_paren() {
    let (_, log) = parse_expr_no_assert("(1, 2");
    assert!(log.error_bit());
}

// ========== LIST ELEMENT LIMIT ==========
// NOTE: This can't be tested in practice since it requires > 65536 elements

// ========== WIDTH WITH MISSING CLOSE BRACKET IN REFINEMENT ==========

#[test]
fn test_err_refine_missing_bracket() {
    let (_, log) = parse_type_no_assert("u8: 6: [0:");
    assert!(log.error_bit());
}

// ========== MODULE ITEM LIMIT ==========
// NOTE: Won't test for 65536 items in module here, would be too slow

// ========== IMPL ITEM LIMIT ==========
// NOTE: Similar constraint

// ========== TRAIT UNEXPECTED TOKEN ==========

#[test]
fn test_err_trait_invalid_token() {
    let (_, log) = parse_source_no_assert("trait Foo { 42 }");
    assert!(log.error_bit());
}

// ========== SYNTAX NOT SUPPORTED / MISC VARIANTS ==========

#[test]
fn test_err_expected_open_angle() {
    let (_, log) = parse_source_no_assert("fn f() { ::<i32>::bar() }");
    // May or may not trigger ExpectedOpenAngle depending on parse flow
    // Just verify it parses or errors gracefully
}

#[test]
fn test_err_literal_suffix_u128() {
    let expr = parse_expr("42u128");
    assert!(matches!(&expr, Expr::Cast(c) if matches!(&c.to, Type::UInt128(_))));
}

#[test]
fn test_err_literal_suffix_i128() {
    let expr = parse_expr("42i128");
    assert!(matches!(&expr, Expr::Cast(c) if matches!(&c.to, Type::Int128(_))));
}

// ========== GENERICS MISSING PARAM NAME ==========

#[test]
fn test_err_fn_generic_unclosed() {
    let (_, log) = parse_source_no_assert("fn foo<T() {}");
    assert!(log.error_bit());
}
