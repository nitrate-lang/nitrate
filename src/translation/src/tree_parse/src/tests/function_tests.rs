use super::helpers::*;
use nitrate_tree::ast::*;

#[test]
fn test_fn_empty() {
    let f = single_function(parse_source("fn foo() {}"));
    assert_eq!(&*f.name, "foo");
    assert!(f.definition.is_some());
}

#[test]
fn test_fn_params() {
    let f = single_function(parse_source("fn add(x: i32, y: i32) {}"));
    assert_eq!(f.parameters.params.len(), 2);
}

#[test]
fn test_fn_return() {
    let f = single_function(parse_source("fn add(x: i32, y: i32) -> i32 { 42 }"));
    assert!(f.return_type.is_some());
}

#[test]
fn test_fn_decl() {
    assert!(single_function(parse_source("fn foo();")).definition.is_none());
}

#[test]
fn test_fn_pub() {
    assert!(matches!(
        single_function(parse_source("pub fn f() {}")).visibility,
        Some(Visibility::Public)
    ));
}

#[test]
fn test_fn_sec() {
    assert!(matches!(
        single_function(parse_source("sec fn f() {}")).visibility,
        Some(Visibility::Private)
    ));
}

#[test]
fn test_fn_pro() {
    assert!(matches!(
        single_function(parse_source("pro fn f() {}")).visibility,
        Some(Visibility::Protected)
    ));
}

#[test]
fn test_fn_variadic() {
    assert!(
        single_function(parse_source("fn f(x: i32, ...) {}"))
            .parameters
            .variadic
    );
}

#[test]
fn test_fn_mut_param() {
    assert!(matches!(
        single_function(parse_source("fn f(mut x: i32) {}")).parameters.params[0].mutability,
        Some(Mutability::Mut)
    ));
}

#[test]
fn test_fn_const_param() {
    assert!(matches!(
        single_function(parse_source("fn f(const x: i32) {}")).parameters.params[0].mutability,
        Some(Mutability::Const)
    ));
}

#[test]
fn test_fn_attr() {
    assert!(
        single_function(parse_source("fn [inline] foo() {}"))
            .attributes
            .is_some()
    );
}

#[test]
fn test_fn_default() {
    assert!(
        single_function(parse_source("fn f(x: i32 = 42) {}")).parameters.params[0]
            .default_value
            .is_some()
    );
}

#[test]
fn test_unicode_name() {
    assert_eq!(&*single_function(parse_source("fn 日本語() {}")).name, "日本語");
}

#[test]
fn test_underscore_name() {
    assert_eq!(&*single_function(parse_source("fn _() {}")).name, "_");
}

// ========== FUNCTION ERROR PATHS ==========

#[test]
fn test_fn_missing_name() {
    let (_, log) = parse_source_no_assert("fn () {}");
    assert!(log.error_bit());
}

#[test]
fn test_fn_missing_params() {
    let (_, log) = parse_source_no_assert("fn foo");
    assert!(log.error_bit());
}

#[test]
fn test_fn_param_missing_name() {
    let (_, log) = parse_source_no_assert("fn foo(: i32) {}");
    assert!(log.error_bit());
}

#[test]
fn test_fn_param_missing_type() {
    let (_, log) = parse_source_no_assert("fn foo(x) {}");
    assert!(log.error_bit());
}

#[test]
fn test_fn_return_arrow_no_type() {
    let (_, log) = parse_source_no_assert("fn foo() -> {}");
    assert!(log.error_bit());
}

#[test]
fn test_fn_generics_unclosed() {
    let (_, log) = parse_source_no_assert("fn foo<T() {}");
    assert!(log.error_bit());
}

// ========== FUNCTION PARAMETER VARIADIC EXPECTED ==========
#[test]
fn test_err_fn_variadic_incomplete() {
    // Single dot instead of triple dot should trigger variadic expected
    let (_, log) = parse_source_no_assert("fn f(x: i32, .) {}");
    assert!(log.error_bit());
}

// ========== STRUCTURE FIELD LIMIT ==========
// NOTE: Would need >65536 fields, impractical

// ========== ENUM VARIANT LIMIT ==========
// NOTE: Would need >65536 variants, impractical

// ========== CLOSURE PARAMETER LIMIT ==========
// SyntaxErr::ClosureParameterLimit - needs >65536 closure params
#[test]
fn test_closure_param_limit() {
    let mut params = String::new();
    for i in 0..65538 {
        if i > 0 {
            params.push_str(", ");
        }
        params.push_str(&format!("x{i}: i32"));
    }
    let src = format!("fn({params}) {{ 42 }}");
    let (_, log) = parse_expr_no_assert(&src);
    assert!(log.error_bit());
}

// ========== FUNCTION TYPE PARAM LIMIT ==========
// SyntaxErr::FunctionTypeParamLimit - needs >65536 function type params
#[test]
fn test_fn_type_param_limit() {
    let mut params = String::new();
    for i in 0..65538 {
        if i > 0 {
            params.push_str(", ");
        }
        params.push_str(&format!("x{i}: i32"));
    }
    let src = format!("fn({params}) -> bool");
    let (_, log) = parse_type_no_assert(&src);
    assert!(log.error_bit());
}

// ========== VARIADIC FUNCTION PARAMETER MISSING SEMICOLON ==========
#[test]
fn test_err_fn_variadic_close_paren_missing() {
    let (_, log) = parse_source_no_assert("fn f(x: i32, ...");
    assert!(log.error_bit());
}

// SyntaxErr::FunctionParameterLimit (variant 122) - needs >65536 params
#[test]
fn test_fn_param_limit() {
    let mut params = String::new();
    for i in 0..65538 {
        if i > 0 {
            params.push_str(", ");
        }
        params.push_str(&format!("x{i}: i32"));
    }
    let src = format!("fn f({params}) {{}}");
    let (_, log) = parse_source_no_assert(&src);
    assert!(log.error_bit());
}

// SyntaxErr::FunctionParametersExpectedEnd (variant 124)
#[test]
fn test_fn_params_expected_end() {
    let (_, log) = parse_source_no_assert("fn f(x: i32");
    assert!(log.error_bit());
}

// SyntaxErr::FunctionParameterVariadicExpected (variant 126)

// ---------- FUNCTION CALL ERRORS ----------

// SyntaxErr::FunctionCallExpectedEnd (variant 400)
#[test]
fn test_fn_call_expected_end() {
    let (_, log) = parse_expr_no_assert("f(1, 2");
    assert!(log.error_bit());
}

// SyntaxErr::FunctionCallArgumentLimit (variant 401) - needs >65536 args
#[test]
fn test_fn_call_arg_limit() {
    let mut args = String::new();
    for _ in 0..65538 {
        args.push_str("0, ");
    }
    args.push_str("0");
    let src = format!("f({args})");
    let (_, log) = parse_expr_no_assert(&src);
    assert!(log.error_bit());
}

// SyntaxErr::FunctionCallPositionFollowsNamed (variant 402)
#[test]
fn test_fn_call_positional_after_named() {
    let (_, log) = parse_expr_no_assert("f(x: 1, 2)");
    assert!(log.error_bit());
}

// ========== EMPTY FUNCTION PARAMS ==========
#[test]
fn test_fn_empty_params() {
    let f = single_function(parse_source("fn f() {}"));
    assert!(f.parameters.params.is_empty());
}

// ========== FUNCTION PARAMETER EDGE CASES ==========

#[test]
fn test_fn_missing_open_paren() {
    let (_, log) = parse_source_no_assert("fn foo x: i32) {}");
    assert!(log.error_bit());
}

#[test]
fn test_fn_param_variadic_no_dots() {
    let (_, log) = parse_source_no_assert("fn foo(x: i32, .) {}");
    assert!(log.error_bit());
}

#[test]
fn test_fn_param_variadic_missing_close_paren() {
    let (_, log) = parse_source_no_assert("fn foo(x: i32, ...");
    assert!(log.error_bit());
}

// ========== VARIADIC FUNCTION PARAMETER EDGE CASES ==========

#[test]
fn test_fn_variadic_no_other_params() {
    let f = single_function(parse_source("fn f(...) {}"));
    assert!(f.parameters.variadic);
    assert!(f.parameters.params.is_empty());
}

// ========== FUNCTION TYPE ERROR PATHS ==========

#[test]
fn test_fn_type_missing_open_paren() {
    let (_, log) = parse_type_no_assert("fn x: i32) -> bool");
    assert!(log.error_bit());
}

#[test]
fn test_fn_type_missing_param_name() {
    let (_, log) = parse_type_no_assert("fn(: i32) -> bool");
    assert!(log.error_bit());
}

#[test]
fn test_fn_type_missing_param_type() {
    let (_, log) = parse_type_no_assert("fn(x) -> bool");
    assert!(log.error_bit());
}
