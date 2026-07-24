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



// ---------- VARIABLE ERRORS ----------

// SyntaxErr::VariableMissingName (variant 140)
#[test]
fn test_var_missing_name() {
    let (_, log) = parse_source_no_assert("const : i32 = 0;");
    assert!(log.error_bit());
}


// ========== STATIC TY ==========
#[test]
fn test_static_with_type() {
    let v = single_variable(parse_source("static x: i32 = 0;"));
    assert!(v.ty.is_some());
}


// ========== VISIBILITY EDGE CASES ==========

#[test]
fn test_visibility_sec_enum() {
    assert!(matches!(
        single_enum(parse_source("sec enum E { A }")).visibility,
        Some(Visibility::Private)
    ));
}


#[test]
fn test_visibility_pro_struct() {
    assert!(matches!(
        single_struct(parse_source("pro struct S { x: i32 }")).visibility,
        Some(Visibility::Protected)
    ));
}


#[test]
fn test_visibility_pub_type_alias() {
    assert!(matches!(
        single_type_alias(parse_source("pub type MyInt = i32;")).visibility,
        Some(Visibility::Public)
    ));
}


#[test]
fn test_visibility_sec_fn() {
    assert!(matches!(
        single_function(parse_source("sec fn f() {}")).visibility,
        Some(Visibility::Private)
    ));
}


#[test]
fn test_visibility_pro_fn() {
    assert!(matches!(
        single_function(parse_source("pro fn f() {}")).visibility,
        Some(Visibility::Protected)
    ));
}


// ========== GLOBAL VARIABLE EDGE CASES ==========

#[test]
fn test_static_visibility() {
    assert!(matches!(
        single_variable(parse_source("pub static x: i32 = 0;")).visibility,
        Some(Visibility::Public)
    ));
}


#[test]
fn test_const_pub() {
    assert!(matches!(
        single_variable(parse_source("pub const X: i32 = 0;")).visibility,
        Some(Visibility::Public)
    ));
}


#[test]
fn test_const_no_type_no_init() {
    let (_, log) = parse_source_no_assert("const X;");
    assert!(!log.error_bit());
}


// ========== LOCAL VARIABLE EDGE CASES ==========

#[test]
fn test_let_no_type_no_init() {
    let f = single_function(parse_source("fn f() { let x; }"));
    assert!(f.definition.is_some());
}


#[test]
fn test_let_with_type() {
    let f = single_function(parse_source("fn f() { let x: i32 = 42; }"));
    assert!(f.definition.is_some());
}


#[test]
fn test_let_mut() {
    let f = single_function(parse_source("fn f() { let mut x: i32 = 42; }"));
    assert!(f.definition.is_some());
}


#[test]
fn test_let_const_mutability() {
    let f = single_function(parse_source("fn f() { let const x: i32 = 42; }"));
    assert!(f.definition.is_some());
}


#[test]
fn test_var_local() {
    let f = single_function(parse_source("fn f() { var x: i32 = 42; }"));
    assert!(f.definition.is_some());
}


#[test]
fn test_let_missing_semicolon() {
    let (_, log) = parse_source_no_assert("fn f() { let x: i32 = 42 }");
    assert!(log.error_bit());
}


// ========== LOCAL VAR PARSING ERROR ==========

#[test]
fn test_var_missing_semicolon() {
    let (_, log) = parse_source_no_assert("fn f() { var x: i32 = 42 }");
    assert!(log.error_bit());
}

