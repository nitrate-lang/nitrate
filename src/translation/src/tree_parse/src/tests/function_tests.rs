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
