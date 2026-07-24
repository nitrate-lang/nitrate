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
