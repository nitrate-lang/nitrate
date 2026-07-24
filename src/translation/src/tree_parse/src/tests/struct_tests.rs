use super::helpers::*;
use nitrate_tree::ast::*;

#[test]
fn test_empty_struct() {
    let s = single_struct(parse_source("struct Foo {}"));
    assert_eq!(&*s.name, "Foo");
    assert!(s.generics.is_none());
    assert!(s.fields.is_empty());
}

#[test]
fn test_struct_single_field() {
    let s = single_struct(parse_source("struct Foo { x: i32 }"));
    assert_eq!(s.fields.len(), 1);
    assert_eq!(&*s.fields[0].name, "x");
}

#[test]
fn test_struct_multiple_fields() {
    let s = single_struct(parse_source("struct Foo { x: i32, y: f64, z: bool }"));
    assert_eq!(s.fields.len(), 3);
}

#[test]
fn test_struct_trailing_comma() {
    assert_eq!(single_struct(parse_source("struct Foo { x: i32, }")).fields.len(), 1);
}

#[test]
fn test_struct_generics() {
    assert!(single_struct(parse_source("struct Foo<T> { x: T }")).generics.is_some());
}

#[test]
fn test_struct_visibility() {
    let s = single_struct(parse_source("pub struct Foo { x: i32 }"));
    assert!(matches!(s.visibility, Some(Visibility::Public)));
}

#[test]
fn test_struct_sec() {
    assert!(matches!(
        single_struct(parse_source("sec struct Foo { x: i32 }")).visibility,
        Some(Visibility::Private)
    ));
}

#[test]
fn test_struct_pro() {
    assert!(matches!(
        single_struct(parse_source("pro struct Foo { x: i32 }")).visibility,
        Some(Visibility::Protected)
    ));
}

#[test]
fn test_struct_attr() {
    let s = single_struct(parse_source("struct [derive(Debug)] Foo { x: i32 }"));
    assert!(s.attributes.is_some());
}

#[test]
fn test_struct_default() {
    assert!(
        single_struct(parse_source("struct Foo { x: i32 = 42 }")).fields[0]
            .default_value
            .is_some()
    );
}

#[test]
fn test_struct_ref_field() {
    assert!(matches!(
        &single_struct(parse_source("struct F { r: &i32 }")).fields[0].ty,
        Type::ReferenceType(_)
    ));
}

#[test]
fn test_struct_ptr_field() {
    assert!(matches!(
        &single_struct(parse_source("struct F { p: *i32 }")).fields[0].ty,
        Type::PointerType(_)
    ));
}

#[test]
fn test_struct_arr_field() {
    assert!(matches!(
        &single_struct(parse_source("struct F { a: [i32; 10] }")).fields[0].ty,
        Type::ArrayType(_)
    ));
}

#[test]
fn test_struct_slice_field() {
    assert!(matches!(
        &single_struct(parse_source("struct F { s: [i32] }")).fields[0].ty,
        Type::SliceType(_)
    ));
}

#[test]
fn test_struct_tuple_field() {
    assert!(matches!(
        &single_struct(parse_source("struct F { t: (i32, f64) }")).fields[0].ty,
        Type::TupleType(_)
    ));
}

#[test]
fn test_struct_fn_field() {
    assert!(matches!(
        &single_struct(parse_source("struct F { f: fn(x: i32) -> bool }")).fields[0].ty,
        Type::FunctionType(_)
    ));
}

// ========== STRUCT ERROR PATHS ==========

#[test]
fn test_struct_missing_name() {
    let (_, log) = parse_source_no_assert("struct {}");
    assert!(log.error_bit());
}

#[test]
fn test_struct_missing_field_name() {
    let (_, log) = parse_source_no_assert("struct Foo { : i32 }");
    assert!(log.error_bit());
}

#[test]
fn test_struct_field_missing_colon() {
    let (_, log) = parse_source_no_assert("struct Foo { x i32 }");
    assert!(log.error_bit());
}

#[test]
fn test_struct_field_missing_brace() {
    let (_, log) = parse_source_no_assert("struct Foo { x: i32");
    assert!(log.error_bit());
}



// SyntaxErr::StructureFieldLimit (variant 101) - needs >65536 fields
#[test]
fn test_struct_field_limit() {
    let mut fields = String::new();
    for i in 0..65538 {
        if i > 0 {
            fields.push_str(", ");
        }
        fields.push_str(&format!("x{i}: i32"));
    }
    let src = format!("struct Foo {{ {fields} }}");
    let (_, log) = parse_source_no_assert(&src);
    assert!(log.error_bit());
}


// SyntaxErr::StructureExpectedEnd (variant 103)


// ---------- STRUCT INIT ERRORS ----------

// SyntaxErr::StructExpectedFieldOrEnd (variant 260)
#[test]
fn test_struct_init_field_or_end() {
    let (_, log) = parse_expr_no_assert("Foo { x: 1");
    assert!(log.error_bit());
}


// SyntaxErr::StructExpectedColon (variant 262)
#[test]
fn test_struct_init_missing_colon() {
    let (_, log) = parse_expr_no_assert("Foo { x 1 }");
    assert!(log.error_bit());
}


// ========== STRUCT EDGE CASES ==========


#[test]
fn test_struct_missing_comma() {
    let (_, log) = parse_source_no_assert("struct Foo { x: i32 y: f64 }");
    assert!(log.error_bit());
}


#[test]
fn test_struct_expected_close_brace() {
    let (_, log) = parse_source_no_assert("struct Foo { x: i32, ");
    assert!(log.error_bit());
}


#[test]
fn test_struct_field_visibility() {
    let s = single_struct(parse_source("struct Foo { pub x: i32 }"));
    assert!(matches!(s.fields[0].visibility, Some(Visibility::Public)));
}


// ========== STRUCT INIT COMMA BEFORE BRACE ==========

#[test]
fn test_struct_init_trailing_comma_no_space() {
    let expr = parse_expr("Foo { x: 1, }");
    assert!(matches!(&expr, Expr::StructInit(s) if s.fields.len() == 1));
}

