use super::helpers::*;

#[test]
fn test_generic_struct() {
    assert_eq!(
        &*single_struct(parse_source("struct F<T> { x: T }"))
            .generics
            .unwrap()
            .params[0]
            .name,
        "T"
    );
}

#[test]
fn test_generic_default() {
    assert!(
        single_struct(parse_source("struct F<T = i32> { x: T }"))
            .generics
            .unwrap()
            .params[0]
            .default_value
            .is_some()
    );
}

// ========== GENERICS ERROR PATHS ==========

#[test]
fn test_generics_unclosed() {
    let (_, log) = parse_source_no_assert("struct Foo<T { x: T }");
    assert!(log.error_bit());
}
