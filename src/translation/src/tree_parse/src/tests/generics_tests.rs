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

// ============================================================================
// Exhaustive tests for EVERY SyntaxErr format() branch in diagnosis.rs
// ============================================================================

// ---------- GENERIC ERRORS ----------

// SyntaxErr::GenericMissingParameterName (variant 0)
#[test]
fn test_gen_missing_param() {
    let (_, log) = parse_source_no_assert("struct Foo<T, = i32> { x: T }");
    assert!(log.error_bit());
}

// SyntaxErr::GenericParameterLimit (variant 1) - needs >65536 params
#[test]
fn test_gen_param_limit() {
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

// SyntaxErr::GenericParameterExpectedEnd (variant 2)
#[test]
fn test_gen_expected_end() {
    let (_, log) = parse_source_no_assert("struct Foo<T U> { x: T }");
    assert!(log.error_bit());
}

// ========== GENERICS EDGE CASES ==========

#[test]
fn test_generic_multi_params() {
    let s = single_struct(parse_source("struct Foo<T, U, V> { a: T, b: U, c: V }"));
    assert_eq!(s.generics.unwrap().params.len(), 3);
}
