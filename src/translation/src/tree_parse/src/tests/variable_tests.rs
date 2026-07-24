use super::helpers::*;
use nitrate_tree::ast::*;

#[test]
fn test_static_var() {
    let v = single_variable(parse_source("static x: i32 = 42;"));
    assert_eq!(&*v.name, "x");
    assert!(v.ty.is_some());
    assert!(v.initializer.is_some());
}

#[test]
fn test_const_var() {
    let v = single_variable(parse_source("const X: i32 = 100;"));
    assert_eq!(&*v.name, "X");
}

#[test]
fn test_static_mut_var() {
    assert!(matches!(
        single_variable(parse_source("static mut x: i32 = 0;")).mutability,
        Some(Mutability::Mut)
    ));
}

#[test]
fn test_static_no_type() {
    assert!(single_variable(parse_source("static x = 42;")).ty.is_none());
}

#[test]
fn test_static_no_init() {
    assert!(single_variable(parse_source("static x: i32;")).initializer.is_none());
}

#[test]
fn test_var_pub() {
    assert!(matches!(
        single_variable(parse_source("pub static x: i32 = 0;")).visibility,
        Some(Visibility::Public)
    ));
}

#[test]
fn test_var_attr() {
    assert!(
        single_variable(parse_source("static [used] x: i32 = 0;"))
            .attributes
            .is_some()
    );
}

// ========== VARIABLE ERROR PATHS ==========

#[test]
fn test_static_missing_name() {
    let (_, log) = parse_source_no_assert("static : i32 = 0;");
    assert!(log.error_bit());
}

#[test]
fn test_static_missing_semicolon() {
    let (_, log) = parse_source_no_assert("static x: i32 = 0");
    assert!(log.error_bit());
}

// ========== CONST MUTABILITY ==========

#[test]
fn test_const_mutability_const() {
    assert!(matches!(
        single_variable(parse_source("const const X: i32 = 0;")).mutability,
        Some(Mutability::Const)
    ));
}

#[test]
fn test_static_mutability_const() {
    assert!(matches!(
        single_variable(parse_source("static const x: i32 = 0;")).mutability,
        Some(Mutability::Const)
    ));
}
