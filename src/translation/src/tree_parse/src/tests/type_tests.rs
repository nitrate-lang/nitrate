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
    assert!(matches!(&parse_type("::std::collections"), Type::TypePath(p) if p.segments[0].name.is_empty()));
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



// ========== EXPECTED CLOSE ANGLE ==========
#[test]
fn test_err_expected_close_angle() {
    // Generic argument issue
    let (_, log) = parse_type_no_assert("Vec<i32");
    assert!(log.error_bit());
}


// ========== PATH EXPECTED NAME OR SEPARATOR ==========
#[test]
fn test_err_path_expected_name_or_separator() {
    // Triple colon in expression path triggers ExpectedColon then path recovery
    let (_, log) = parse_source_no_assert("fn f() { foo:::bar; }");
    assert!(log.error_bit());
}


// ========== PATH SEGMENT LIMIT ==========
// NOTE: Would need >65536 segments, impractical

// ========== FUNCTION PARAMETER MISSING NAME ==========
#[test]
fn test_err_fn_param_missing_name() {
    let (_, log) = parse_source_no_assert("fn f(: i32) {}");
    assert!(log.error_bit());
}


// ========== NAMED GENERIC ARGUMENT ==========


// ========== TYPE PATH WITH EMPTY GENERICS ==========
#[test]
fn test_type_path_empty_generics() {
    let ty = parse_type("Foo<>");
    assert!(matches!(&ty, Type::TypePath(_)));
}


// ========== TYPE PATH WITH TRAILING COLON ==========
#[test]
fn test_type_path_trailing_colon() {
    let (_, log) = parse_type_no_assert("Foo::");
    assert!(log.error_bit());
}


// ========== TYPE PATH WITH MULTIPLE SEGMENTS ==========
#[test]
fn test_type_path_multi_segment() {
    let ty = parse_type("std::collections::HashMap<K, V>");
    assert!(matches!(&ty, Type::TypePath(p) if p.segments.len() == 3));
}


// ========== REFERENCE WITH LIFETIME + ISO + MUT ==========
#[test]
fn test_type_ref_complex() {
    let ty = parse_type("&'a iso mut i32");
    assert!(matches!(&ty, Type::ReferenceType(_)));
}


// ========== GLOBAL PATH ==========
#[test]
fn test_global_type_path() {
    let ty = parse_type("::std::mem");
    assert!(matches!(&ty, Type::TypePath(p) if p.segments[0].name.is_empty()));
}


// ---------- PATH ERRORS ----------

// SyntaxErr::PathGenericArgumentExpectedEnd (variant 222)
#[test]
fn test_path_generic_arg_expected_end() {
    let (_, log) = parse_type_no_assert("Vec<i32,");
    assert!(log.error_bit());
}


// SyntaxErr::PathGenericArgumentLimit (variant 223) - needs >65536 args
#[test]
fn test_path_generic_arg_limit() {
    let mut args = String::new();
    for i in 0..65538 {
        if i > 0 {
            args.push_str(", ");
        }
        args.push_str("i32");
    }
    let src = format!("Foo<{args}>");
    let (_, log) = parse_type_no_assert(&src);
    assert!(log.error_bit());
}


// SyntaxErr::PathExpectedNameOrSeparator (variant 224)
#[test]
fn test_path_expected_name_or_separator() {
    let (_, log) = parse_source_no_assert("fn f() { ::; }");
    assert!(log.error_bit());
}


// SyntaxErr::PathSegmentLimit (variant 225) - needs >65536 segments
#[test]
fn test_path_segment_limit() {
    let mut path = String::from("a");
    for _ in 0..65538 {
        path.push_str("::b");
    }
    let (_, log) = parse_type_no_assert(&path);
    assert!(log.error_bit());
}


// SyntaxErr::PathExpectedName (variant 226)


// ---------- REFERENCE TYPE ERRORS ----------

// SyntaxErr::ReferenceTypeExpectedLifetimeName (variant 240)
#[test]
fn test_ref_lifetime_missing() {
    let (_, log) = parse_type_no_assert("&'");
    assert!(log.error_bit());
}


// ---------- TUPLE TYPE ERRORS ----------

// SyntaxErr::TupleTypeExpectedEnd (variant 280)


// SyntaxErr::TupleTypeElementLimit (variant 281) - needs >65536 elements
#[test]
fn test_tuple_type_element_limit() {
    let mut elems = String::from("i32");
    for _ in 0..65538 {
        elems.push_str(", i32");
    }
    let src = format!("({elems})");
    let (_, log) = parse_type_no_assert(&src);
    assert!(log.error_bit());
}


// ---------- ATTRIBUTES ERRORS ----------

// SyntaxErr::AttributesExpectedEnd (variant 320)
#[test]
fn test_attrs_expected_end() {
    let (_, log) = parse_source_no_assert("fn [a,");
    assert!(log.error_bit());
}


// SyntaxErr::AttributesElementLimit (variant 321) - needs >65536 elements
#[test]
fn test_attrs_element_limit() {
    let mut attrs = String::new();
    for i in 0..65538 {
        if i > 0 {
            attrs.push_str(", ");
        }
        attrs.push_str(&format!("x{i}"));
    }
    let src = format!("fn [{attrs}] foo() {{}}");
    let (_, log) = parse_source_no_assert(&src);
    assert!(log.error_bit());
}


// ---------- EXPECTED TOKEN ERRORS (1000-1010) ----------

// SyntaxErr::ExpectedOpenParen (variant 1000)
#[test]
fn test_exp_open_paren() {
    let (_, log) = parse_source_no_assert("fn foo) {}");
    assert!(log.error_bit());
}


// SyntaxErr::ExpectedCloseParen (variant 1001)
#[test]
fn test_exp_close_paren() {
    let (_, log) = parse_expr_no_assert("(1, 2");
    assert!(log.error_bit());
}


// SyntaxErr::ExpectedOpenBrace (variant 1002)
#[test]
fn test_exp_open_brace() {
    let (_, log) = parse_expr_no_assert("if true 42");
    assert!(log.error_bit());
}


// SyntaxErr::ExpectedCloseBrace (variant 1003)
#[test]
fn test_exp_close_brace() {
    let (_, log) = parse_source_no_assert("mod foo { fn f() {} ");
    assert!(log.error_bit());
}


// SyntaxErr::ExpectedOpenBracket (variant 1004)
#[test]
fn test_exp_open_bracket() {
    let (_, log) = parse_expr_no_assert("a[");
    assert!(log.error_bit());
}


// SyntaxErr::ExpectedCloseBracket (variant 1005)
#[test]
fn test_exp_close_bracket() {
    let (_, log) = parse_expr_no_assert("a[0");
    assert!(log.error_bit());
}


// SyntaxErr::ExpectedOpenAngle (variant 1006)
#[test]
fn test_exp_open_angle() {
    // Trigger by having Foo< where there's no matching >
    let (_, log) = parse_type_no_assert("Foo<i32");
    assert!(log.error_bit());
}

// SyntaxErr::ExpectedCloseAngle (variant 1007)

// SyntaxErr::ExpectedSemicolon (variant 1008)
#[test]
fn test_exp_semicolon() {
    let (_, log) = parse_source_no_assert("fn f() { break }");
    assert!(log.error_bit());
}

// SyntaxErr::ExpectedColon (variant 1009)
#[test]
fn test_exp_colon() {
    let (_, log) = parse_source_no_assert("struct Foo { x i32 }");
    assert!(log.error_bit());
}

// SyntaxErr::ExpectedArrow (variant 1010)

// ---------- GENERAL EXPECTED ERRORS (2000-2020) ----------

// SyntaxErr::ExpectedItem (variant 2000)
#[test]
fn test_exp_item() {
    let (_, log) = parse_source_no_assert("@");
    assert!(log.error_bit());
}

// SyntaxErr::ExpectedType (variant 2001)

// SyntaxErr::ExpectedExpr (variant 2002)

// SyntaxErr::SyntaxNotSupported (variant 2020)
#[test]
fn test_syntax_not_supported() {
    // This is #[allow(dead_code)] but we can still try to trigger it
    // It's not used in production code, but test format() by creating a SyntaxErr directly
    use nitrate_token::SourcePosition;
    let err = crate::diagnosis::SyntaxErr::SyntaxNotSupported(SourcePosition {
        offset: 0,
        line: 0,
        column: 0,
        fileid: None,
    });
    let info = nitrate_diagnosis::FormattableDiagnosticGroup::format(&err);
    assert_eq!(info.message, "this syntax is not supported");
}


// Tests to cover remaining reachable format() arms by triggering edge cases

// ========== EXPECTED TYPE ERROR ==========
#[test]
fn test_err_expected_type() {
    let (_, log) = parse_type_no_assert("+");
    assert!(log.error_bit());
}


// ========== GLOBAL PATH IN TYPE ==========
#[test]
fn test_global_type_path_segments() {
    let ty = parse_type("::std::vec::Vec<i32>");
    assert!(matches!(&ty, Type::TypePath(p) if p.segments[0].name.is_empty() && p.segments.len() >= 3));
}


// ========== EMPTY TYPE PATH ==========
#[test]
fn test_type_path_empty_global() {
    let (_, log) = parse_type_no_assert("::");
    assert!(log.error_bit());
}


// ========== TUPLE TYPE WITH SINGLE ELEMENT ==========


// ========== LONG TYPE PATH ==========
#[test]
fn test_type_path_long() {
    let ty = parse_type("a::b::c::d::e");
    assert!(matches!(&ty, Type::TypePath(p) if p.segments.len() == 5));
}


// ========== MISSING SEMICOLON ON TYPE ==========


// ========== GENERIC ARGUMENT EXPECTED END ==========


// ========== TYPE PATH EXPECTED NAME ==========
#[test]
fn test_err_type_path_expected_name() {
    let (_, log) = parse_source_no_assert("fn f() { let x: 42 = 0; }");
    // 42 as a type should fail
    assert!(log.error_bit());
}


// ========== FUNCTION TYPE EXPECTED OPEN PAREN ==========
#[test]
fn test_fn_type_missing_open_paren2() {
    let (_, log) = parse_type_no_assert("fn i32) -> bool");
    assert!(log.error_bit());
}


// ========== TYPE: MORE COMPLEX POINTER ==========
#[test]
fn test_type_ptr_poly_const() {
    let ty = parse_type("*poly const i32");
    assert!(matches!(&ty, Type::PointerType(p) if matches!(p.exclusivity, Some(Exclusivity::Poly))));
}


// ========== TYPE: NESTED PATH ==========


// ========== TYPE: TUPLE WITH ONE ELEMENT ==========
#[test]
fn test_type_tuple_one_elem() {
    let ty = parse_type("(i32,)");
    assert!(matches!(&ty, Type::TupleType(t) if t.element_types.len() == 1));
}


// ========== TYPE: REFINEMENT WITH ALL FIELDS ==========


// ========== TYPE: LATENT ==========


// ========== TYPE: ARRAY WITH COMPLEX LENGTH ==========
#[test]
fn test_type_array_complex_len() {
    let ty = parse_type("[i32; 2 + 2]");
    assert!(matches!(&ty, Type::ArrayType(_)));
}


// ========== TYPE: FN WITH MULTIPLE PARAMS ==========
#[test]
fn test_type_fn_multi_params() {
    let ty = parse_type("fn(x: i32, y: f64, z: bool)");
    assert!(matches!(&ty, Type::FunctionType(_)));
}


// ========== TYPE: FN RETURN TYPE ==========
#[test]
fn test_type_fn_return() {
    let ty = parse_type("fn(x: i32) -> f64");
    assert!(matches!(&ty, Type::FunctionType(f) if f.return_type.is_some()));
}


// ========== TYPE: PARENTHESIZED TUPLE ==========
#[test]
fn test_type_paren_in_tuple() {
    let ty = parse_type("((i32, f64))");
    assert!(matches!(&ty, Type::Parentheses(_)));
}


// ========== TYPE: NAMED GENERIC ==========
#[test]
fn test_type_named_generic2() {
    let ty = parse_type("Foo<Bar: i32>");
    assert!(matches!(&ty, Type::TypePath(_)));
}


// ========== TYPE: MISSING SEMICOLON IN ARRAY ==========


// ========== BLOCK EDGE CASES ==========

#[test]
fn test_empty_block() {
    let expr = parse_expr("{ }");
    assert!(matches!(&expr, Expr::Closure(_)));
}


// ========== TYPES ==========


#[test]
fn test_function_type_no_return() {
    let ty = parse_type("fn(x: i32)");
    assert!(matches!(&ty, Type::FunctionType(f) if f.return_type.is_none()));
}


// ========== TYPE PARSE EDGE CASES ==========

#[test]
fn test_type_f8() {
    let ty = parse_type("f8");
    assert!(matches!(&ty, Type::TypePath(_)));
}


#[test]
fn test_type_f16() {
    let ty = parse_type("f16");
    assert!(matches!(&ty, Type::TypePath(_)));
}


#[test]
fn test_type_f128() {
    let ty = parse_type("f128");
    assert!(matches!(&ty, Type::TypePath(_)));
}


#[test]
fn test_type_ref_with_lifetime_and_iso() {
    let ty = parse_type("&'a iso i32");
    assert!(
        matches!(&ty, Type::ReferenceType(r) if r.lifetime.is_some() && matches!(r.exclusivity, Some(Exclusivity::Iso)))
    );
}


#[test]
fn test_type_ref_with_lifetime_and_mut() {
    let ty = parse_type("&'a mut i32");
    assert!(
        matches!(&ty, Type::ReferenceType(r) if r.lifetime.is_some() && matches!(r.mutability, Some(Mutability::Mut)))
    );
}


#[test]
fn test_type_pointer_iso_mut() {
    let ty = parse_type("*iso mut i32");
    assert!(matches!(&ty, Type::PointerType(_)));
}


// ========== REFINEMENT TYPE EDGE CASES ==========


#[test]
fn test_type_refine_range_no_min() {
    let ty = parse_type("u8: [:10]");
    assert!(matches!(&ty, Type::RefinementType(r) if r.minimum.is_none() && r.maximum.is_some()));
}


#[test]
fn test_type_refine_range_no_max() {
    let ty = parse_type("u8: [0:]");
    assert!(matches!(&ty, Type::RefinementType(r) if r.minimum.is_some() && r.maximum.is_none()));
}


#[test]
fn test_type_refine_width_range_missing_bracket() {
    let (_, log) = parse_type_no_assert("u8: 6: [0:10");
    assert!(log.error_bit());
}


#[test]
fn test_type_refine_width_then_no_bracket() {
    let (_, log) = parse_type_no_assert("u8: 6: foo");
    assert!(log.error_bit());
}


// ========== PATH EDGE CASES ==========


#[test]
fn test_type_path_global_with_generics() {
    let ty = parse_type("::std::Vec<i32>");
    assert!(matches!(&ty, Type::TypePath(p) if p.segments.len() >= 2));
}


// ========== TYPE ALIAS EDGE CASES ==========

#[test]
fn test_type_alias_no_value() {
    let ta = single_type_alias(parse_source("type Foo;"));
    assert_eq!(&*ta.name, "Foo");
    assert!(ta.alias_type.is_none());
}


#[test]
fn test_type_alias_with_generics() {
    let ta = single_type_alias(parse_source("type MyVec<T> = Vec<T>;"));
    assert!(ta.generics.is_some());
}


// ========== ARRAY TYPE ERROR PATHS ==========


// ========== POINTER TYPE ERROR PATHS ==========

#[test]
fn test_type_ptr_poly_mut() {
    let ty = parse_type("*poly mut i32");
    assert!(
        matches!(&ty, Type::PointerType(p) if matches!(p.exclusivity, Some(Exclusivity::Poly)) && matches!(p.mutability, Some(Mutability::Mut)))
    );
}


// ========== PATH ERROR PATHS ==========


// ========== TYPE RECURSION ==========

#[test]
fn test_type_double_parens() {
    let ty = parse_type("((i32))");
    assert!(matches!(&ty, Type::Parentheses(p) if matches!(&p.inner, Type::Parentheses(_))));
}


#[test]
fn test_type_unexpected_after_paren() {
    let (_, log) = parse_type_no_assert("(i32");
    assert!(log.error_bit());
}


// ========== INLINE TYPE PATHS ==========

#[test]
fn test_type_path_with_triple_nested() {
    let ty = parse_type("a::b::c");
    assert!(matches!(&ty, Type::TypePath(p) if p.segments.len() == 3));
}


// ========== TYPE INFO ==========

#[test]
fn test_type_info_path() {
    let expr = parse_expr("type String");
    assert!(matches!(&expr, Expr::TypeInfo(_)));
}


// ========== PARSING EOF AFTER BINARY OPERATOR ==========

#[test]
fn test_binary_op_at_eof() {
    let (_, log) = parse_expr_no_assert("1 + ");
    assert!(log.error_bit());
}

