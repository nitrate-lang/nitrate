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
