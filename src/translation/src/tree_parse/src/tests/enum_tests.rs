use super::helpers::*;
use nitrate_tree::ast::*;

#[test]
fn test_enum_empty() {
    assert!(single_enum(parse_source("enum Foo {}")).variants.is_empty());
}

#[test]
fn test_enum_single() {
    assert_eq!(single_enum(parse_source("enum Foo { Bar }")).variants.len(), 1);
}

#[test]
fn test_enum_multi() {
    assert_eq!(single_enum(parse_source("enum Foo { A, B, C }")).variants.len(), 3);
}

#[test]
fn test_enum_type() {
    assert!(
        single_enum(parse_source("enum Foo { A(i32) }")).variants[0]
            .ty
            .is_some()
    );
}

#[test]
fn test_enum_default() {
    assert!(
        single_enum(parse_source("enum Foo { A = 42 }")).variants[0]
            .default_value
            .is_some()
    );
}

#[test]
fn test_enum_generic() {
    assert!(
        single_enum(parse_source("enum O<T> { Some(T), None }"))
            .generics
            .is_some()
    );
}

#[test]
fn test_enum_pub() {
    assert!(matches!(
        single_enum(parse_source("pub enum F { A }")).visibility,
        Some(Visibility::Public)
    ));
}

#[test]
fn test_enum_attr() {
    assert!(
        single_enum(parse_source("enum [repr(C)] Foo { Bar }"))
            .attributes
            .is_some()
    );
}

// ========== ENUM ERROR PATHS ==========

#[test]
fn test_enum_missing_name() {
    let (_, log) = parse_source_no_assert("enum {}");
    assert!(log.error_bit());
}

#[test]
fn test_enum_missing_variant_name() {
    let (_, log) = parse_source_no_assert("enum Foo { : i32 }");
    assert!(log.error_bit());
}

#[test]
fn test_enum_missing_brace() {
    let (_, log) = parse_source_no_assert("enum Foo");
    assert!(log.error_bit());
}



// SyntaxErr::EnumVariantLimit (variant 81) - needs >65536 variants
#[test]
fn test_enum_variant_limit() {
    let mut variants = String::new();
    for i in 0..65538 {
        if i > 0 {
            variants.push_str(", ");
        }
        variants.push_str(&format!("V{i}"));
    }
    let src = format!("enum Foo {{ {variants} }}");
    let (_, log) = parse_source_no_assert(&src);
    assert!(log.error_bit());
}


// SyntaxErr::EnumExpectedEnd (variant 83)
#[test]
fn test_enum_expected_end() {
    let (_, log) = parse_source_no_assert("enum Foo { A B }");
    assert!(log.error_bit());
}


// ========== ENUM WITH DEFAULT VALUE ==========
#[test]
fn test_enum_defaulted_variant() {
    let e = single_enum(parse_source("enum Foo { A = 1, B = 2 }"));
    assert!(e.variants[0].default_value.is_some());
    assert!(e.variants[1].default_value.is_some());
}


// ========== ENUM EDGE CASES ==========

#[test]
fn test_enum_variant_with_type_unclosed_paren() {
    let (_, log) = parse_source_no_assert("enum Foo { Bar(i32 }");
    assert!(log.error_bit());
}


#[test]
fn test_enum_variant_missing_comma() {
    let (_, log) = parse_source_no_assert("enum Foo { A B }");
    assert!(log.error_bit());
}


#[test]
fn test_enum_variant_expected_close_brace() {
    let (_, log) = parse_source_no_assert("enum Foo { A, B, ");
    assert!(log.error_bit());
}


#[test]
fn test_enum_with_default_value() {
    let e = single_enum(parse_source("enum Foo { A = 42, B }"));
    assert!(e.variants[0].default_value.is_some());
    assert!(e.variants[1].default_value.is_none());
}


#[test]
fn test_enum_variant_trailing_comma() {
    let e = single_enum(parse_source("enum Foo { A, B, }"));
    assert_eq!(e.variants.len(), 2);
}

