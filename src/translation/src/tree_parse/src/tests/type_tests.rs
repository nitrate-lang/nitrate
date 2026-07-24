use super::helpers::*;
use nitrate_tree::ast::*;

#[test]
fn test_type_bool() {
    assert!(matches!(parse_type("bool"), Type::Bool(_)));
}

#[test]
fn test_type_u8() {
    assert!(matches!(parse_type("u8"), Type::UInt8(_)));
}

#[test]
fn test_type_u16() {
    assert!(matches!(parse_type("u16"), Type::UInt16(_)));
}

#[test]
fn test_type_u32() {
    assert!(matches!(parse_type("u32"), Type::UInt32(_)));
}

#[test]
fn test_type_u64() {
    assert!(matches!(parse_type("u64"), Type::UInt64(_)));
}

#[test]
fn test_type_u128() {
    assert!(matches!(parse_type("u128"), Type::UInt128(_)));
}

#[test]
fn test_type_usize() {
    assert!(matches!(parse_type("usize"), Type::USize(_)));
}

#[test]
fn test_type_i8() {
    assert!(matches!(parse_type("i8"), Type::Int8(_)));
}

#[test]
fn test_type_i16() {
    assert!(matches!(parse_type("i16"), Type::Int16(_)));
}

#[test]
fn test_type_i32() {
    assert!(matches!(parse_type("i32"), Type::Int32(_)));
}

#[test]
fn test_type_i64() {
    assert!(matches!(parse_type("i64"), Type::Int64(_)));
}

#[test]
fn test_type_i128() {
    assert!(matches!(parse_type("i128"), Type::Int128(_)));
}

#[test]
fn test_type_f32() {
    assert!(matches!(parse_type("f32"), Type::Float32(_)));
}

#[test]
fn test_type_f64() {
    assert!(matches!(parse_type("f64"), Type::Float64(_)));
}

#[test]
fn test_type_ref() {
    assert!(matches!(&parse_type("&i32"), Type::ReferenceType(_)));
}

#[test]
fn test_type_mut_ref() {
    assert!(matches!(&parse_type("&mut i32"), Type::ReferenceType(_)));
}

#[test]
fn test_type_ptr() {
    assert!(matches!(&parse_type("*i32"), Type::PointerType(_)));
}

#[test]
fn test_type_mut_ptr() {
    assert!(matches!(&parse_type("*mut i32"), Type::PointerType(_)));
}

#[test]
fn test_type_arr() {
    assert!(matches!(&parse_type("[i32; 10]"), Type::ArrayType(_)));
}

#[test]
fn test_type_slice() {
    assert!(matches!(&parse_type("[u8]"), Type::SliceType(_)));
}

#[test]
fn test_type_tuple() {
    assert!(matches!(&parse_type("(i32, f64)"), Type::TupleType(_)));
}

#[test]
fn test_type_empty_tuple() {
    assert!(matches!(&parse_type("()"), Type::TupleType(t) if t.element_types.is_empty()));
}

#[test]
fn test_type_parens() {
    assert!(matches!(&parse_type("(i32)"), Type::Parentheses(_)));
}

#[test]
fn test_type_fn() {
    assert!(matches!(&parse_type("fn(x: i32) -> bool"), Type::FunctionType(_)));
}

#[test]
fn test_type_fn_no_ret() {
    assert!(matches!(&parse_type("fn()"), Type::FunctionType(f) if f.return_type.is_none()));
}

#[test]
fn test_type_lifetime() {
    assert!(matches!(&parse_type("'a"), Type::Lifetime(_)));
}

#[test]
fn test_type_static_lifetime() {
    assert!(matches!(&parse_type("'static"), Type::Lifetime(_)));
}

#[test]
fn test_type_refine() {
    assert!(matches!(&parse_type("u8: 6"), Type::RefinementType(_)));
}

#[test]
fn test_type_latent() {
    assert!(matches!(&parse_type("{ 42 }"), Type::LatentType(_)));
}

#[test]
fn test_type_path() {
    assert!(matches!(&parse_type("String"), Type::TypePath(p) if p.segments[0].name == "String"));
}

#[test]
fn test_type_nested_path() {
    assert!(matches!(&parse_type("std::collections::HashMap"), Type::TypePath(p) if p.segments.len() == 3));
}

#[test]
fn test_type_global_path() {
    assert!(matches!(&parse_type("::std::collections"), Type::TypePath(p) if p.segments[0].name == ""));
}

#[test]
fn test_type_generic_path() {
    assert!(matches!(&parse_type("Vec<i32>"), Type::TypePath(p) if p.segments[0].type_arguments.is_some()));
}

// ========== TYPE ALIAS ERROR PATHS ==========

#[test]
fn test_type_alias_missing_name() {
    let (_, log) = parse_source_no_assert("type = i32;");
    assert!(log.error_bit());
}

#[test]
fn test_type_alias_no_semicolon() {
    let (_, log) = parse_source_no_assert("type Foo = i32");
    assert!(log.error_bit());
}

#[test]
fn test_type_alias_no_semicolon_no_value() {
    let (_, log) = parse_source_no_assert("type Foo");
    assert!(log.error_bit());
}

#[test]
fn test_type_alias_with_attributes() {
    assert!(
        single_type_alias(parse_source("type [some_attr] MyInt = i32;"))
            .attributes
            .is_some()
    );
}

// ========== TYPE REFINEMENTS ==========

#[test]
fn test_type_refine_width() {
    assert!(matches!(&parse_type("u8: 6"), Type::RefinementType(r) if matches!(&r.width, Some(Expr::Integer(_)))));
}

#[test]
fn test_type_refine_range() {
    assert!(matches!(&parse_type("u8: [0:10]"), Type::RefinementType(_)));
}

#[test]
fn test_type_refine_width_range() {
    assert!(matches!(&parse_type("u8: 6: [0:10]"), Type::RefinementType(_)));
}

#[test]
fn test_type_refine_width_missing_bracket() {
    let (_, log) = parse_type_no_assert("u8: 6:");
    assert!(log.error_bit());
}

// ========== REFERENCE TYPES ==========

#[test]
fn test_type_ref_lifetime() {
    assert!(matches!(&parse_type("&'a i32"), Type::ReferenceType(r) if r.lifetime.is_some()));
}

#[test]
fn test_type_ref_poly() {
    assert!(
        matches!(&parse_type("&poly i32"), Type::ReferenceType(r) if matches!(r.exclusivity, Some(Exclusivity::Poly)))
    );
}

#[test]
fn test_type_ref_iso() {
    assert!(
        matches!(&parse_type("&iso i32"), Type::ReferenceType(r) if matches!(r.exclusivity, Some(Exclusivity::Iso)))
    );
}

#[test]
fn test_type_ref_const() {
    assert!(
        matches!(&parse_type("&const i32"), Type::ReferenceType(r) if matches!(r.mutability, Some(Mutability::Const)))
    );
}

// ========== POINTER TYPES ==========

#[test]
fn test_type_ptr_poly() {
    assert!(
        matches!(&parse_type("*poly i32"), Type::PointerType(p) if matches!(p.exclusivity, Some(Exclusivity::Poly)))
    );
}

#[test]
fn test_type_ptr_iso() {
    assert!(matches!(&parse_type("*iso i32"), Type::PointerType(p) if matches!(p.exclusivity, Some(Exclusivity::Iso))));
}

#[test]
fn test_type_ptr_const() {
    assert!(
        matches!(&parse_type("*const i32"), Type::PointerType(p) if matches!(p.mutability, Some(Mutability::Const)))
    );
}

// ========== FUNCTION TYPE ==========

#[test]
fn test_type_fn_attr() {
    assert!(matches!(
        &parse_type("fn [inline](x: i32) -> bool"),
        Type::FunctionType(_)
    ));
}

#[test]
fn test_type_fn_no_params() {
    assert!(matches!(&parse_type("fn()"), Type::FunctionType(f) if f.parameters.is_empty()));
}

// ========== NAMED GENERIC ==========

#[test]
fn test_type_named_generic() {
    assert!(matches!(&parse_type("Map<Key: i32, Value: f64>"), Type::TypePath(_)));
}

#[test]
fn test_fn_type_return_arrow_missing() {
    let (_, log) = parse_type_no_assert("fn(x: i32) - bool");
    assert!(log.error_bit());
}

// ========== TYPE ERROR PATHS ==========

#[test]
fn test_type_expected_type() {
    let (_, log) = parse_type_no_assert("}");
    assert!(log.error_bit());
}

#[test]
fn test_type_tuple_unclosed() {
    let (_, log) = parse_type_no_assert("(i32, f64");
    assert!(log.error_bit());
}

#[test]
fn test_type_array_missing_semi() {
    let (_, log) = parse_type_no_assert("[i32 10]");
    assert!(log.error_bit());
}

#[test]
fn test_type_array_unclosed() {
    let (_, log) = parse_type_no_assert("[i32; 10");
    assert!(log.error_bit());
}

#[test]
fn test_type_path_expected_name() {
    let (_, log) = parse_type_no_assert("foo::");
    assert!(log.error_bit());
}
