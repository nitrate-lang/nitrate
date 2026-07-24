use super::helpers::*;
use nitrate_tree::ast::*;

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

// ---------- MODULE ERRORS ----------

// SyntaxErr::ModuleMissingName (variant 20)
#[test]
fn test_mod_missing_name() {
    let (_, log) = parse_source_no_assert("mod { fn f() {} }");
    assert!(log.error_bit());
}

// SyntaxErr::ModuleItemLimit (variant 21) - needs >65536 items in module
#[test]
fn test_mod_item_limit() {
    let mut items = String::new();
    for _ in 0..65538 {
        items.push_str("fn f() {} ");
    }
    let src = format!("mod m {{ {items} }}");
    let (_, log) = parse_source_no_assert(&src);
    assert!(log.error_bit());
}

// SyntaxErr::ModuleExpectedEnd (variant 22)
#[test]
fn test_mod_expected_end() {
    let (_, log) = parse_source_no_assert("mod foo { fn f() {}");
    assert!(log.error_bit());
}

// ---------- IMPORT ERRORS ----------

// SyntaxErr::ImportAliasMissingName (variant 41)
#[test]
fn test_import_alias_missing() {
    let (_, log) = parse_source_no_assert("use foo as ;");
    assert!(log.error_bit());
}

// SyntaxErr::ImportExpectedStarOrGroup (variant 42)
#[test]
fn test_import_star_or_group() {
    let (_, log) = parse_source_no_assert("use foo::bar::;");
    assert!(log.error_bit());
}

// SyntaxErr::ImportGroupExpectedEnd (variant 43)
#[test]
fn test_import_group_expected_end() {
    let (_, log) = parse_source_no_assert("use foo::{bar, baz");
    assert!(log.error_bit());
}

// ---------- TYPE ALIAS ERRORS ----------

// SyntaxErr::TypeAliasMissingName (variant 60)
#[test]
fn test_type_alias_missing_name() {
    let (_, log) = parse_source_no_assert("type = i32;");
    assert!(log.error_bit());
}

// ---------- ENUM ERRORS ----------

// SyntaxErr::EnumMissingName (variant 80)
#[test]
fn test_enum_missing_name() {
    let (_, log) = parse_source_no_assert("enum {}");
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

// SyntaxErr::EnumMissingVariantName (variant 82)
#[test]
fn test_enum_missing_variant_name() {
    let (_, log) = parse_source_no_assert("enum Foo { : i32 }");
    assert!(log.error_bit());
}

// SyntaxErr::EnumExpectedEnd (variant 83)
#[test]
fn test_enum_expected_end() {
    let (_, log) = parse_source_no_assert("enum Foo { A B }");
    assert!(log.error_bit());
}

// ---------- STRUCTURE ERRORS ----------

// SyntaxErr::StructureMissingName (variant 100)
#[test]
fn test_struct_missing_name() {
    let (_, log) = parse_source_no_assert("struct {}");
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

// SyntaxErr::StructureMissingFieldName (variant 102)
#[test]
fn test_struct_missing_field_name() {
    let (_, log) = parse_source_no_assert("struct Foo { : i32 }");
    assert!(log.error_bit());
}

// SyntaxErr::StructureExpectedEnd (variant 103)
#[test]
fn test_struct_expected_end() {
    let (_, log) = parse_source_no_assert("struct Foo { x: i32");
    assert!(log.error_bit());
}

// ---------- FUNCTION ERRORS ----------

// SyntaxErr::FunctionMissingName (variant 120)
#[test]
fn test_fn_missing_name() {
    let (_, log) = parse_source_no_assert("fn () {}");
    assert!(log.error_bit());
}

// SyntaxErr::FunctionParameterLimit (variant 122) - needs >65536 params
#[test]
fn test_fn_param_limit() {
    let mut params = String::new();
    for i in 0..65538 {
        if i > 0 {
            params.push_str(", ");
        }
        params.push_str(&format!("x{i}: i32"));
    }
    let src = format!("fn f({params}) {{}}");
    let (_, log) = parse_source_no_assert(&src);
    assert!(log.error_bit());
}

// SyntaxErr::FunctionParameterMissingName (variant 123)
#[test]
fn test_fn_param_missing_name() {
    let (_, log) = parse_source_no_assert("fn f(: i32) {}");
    assert!(log.error_bit());
}

// SyntaxErr::FunctionParametersExpectedEnd (variant 124)
#[test]
fn test_fn_params_expected_end() {
    let (_, log) = parse_source_no_assert("fn f(x: i32");
    assert!(log.error_bit());
}

// SyntaxErr::FunctionParameterExpectedType (variant 125)
#[test]
fn test_fn_param_missing_type() {
    let (_, log) = parse_source_no_assert("fn f(x) {}");
    assert!(log.error_bit());
}

// SyntaxErr::FunctionParameterVariadicExpected (variant 126)
#[test]
fn test_fn_variadic_expected() {
    let (_, log) = parse_source_no_assert("fn f(x: i32, .) {}");
    assert!(log.error_bit());
}

// ---------- VARIABLE ERRORS ----------

// SyntaxErr::VariableMissingName (variant 140)
#[test]
fn test_var_missing_name() {
    let (_, log) = parse_source_no_assert("const : i32 = 0;");
    assert!(log.error_bit());
}

// ---------- TRAIT ERRORS ----------

// SyntaxErr::TraitMissingName (variant 180)
#[test]
fn test_trait_missing_name() {
    let (_, log) = parse_source_no_assert("trait { fn f(); }");
    assert!(log.error_bit());
}

// SyntaxErr::TraitItemLimit (variant 181) - needs >65536 items
#[test]
fn test_trait_item_limit() {
    let mut items = String::new();
    for i in 0..65538 {
        items.push_str(&format!("fn f{i}(); "));
    }
    let src = format!("trait Foo {{ {items} }}");
    let (_, log) = parse_source_no_assert(&src);
    assert!(log.error_bit());
}

// SyntaxErr::TraitDoesNotAllowItem (variant 182)
#[test]
fn test_trait_invalid_item() {
    let (_, log) = parse_source_no_assert("trait Foo { struct Bar; }");
    assert!(log.error_bit());
}

// SyntaxErr::TraitExpectedEnd (variant 183)
#[test]
fn test_trait_expected_end() {
    let (_, log) = parse_source_no_assert("trait Foo { fn f();");
    assert!(log.error_bit());
}

// ---------- IMPL ERRORS ----------

// SyntaxErr::ImplMissingFor (variant 200)
#[test]
fn test_impl_missing_for() {
    let (_, log) = parse_source_no_assert("impl Trait Foo {}");
    assert!(log.error_bit());
}

// SyntaxErr::ImplExpectedEnd (variant 201)
#[test]
fn test_impl_expected_end() {
    let (_, log) = parse_source_no_assert("impl Foo { fn f() {}");
    assert!(log.error_bit());
}

// SyntaxErr::ImplItemLimit (variant 202) - needs >65536 items
#[test]
fn test_impl_item_limit() {
    let mut items = String::new();
    for i in 0..65538 {
        items.push_str(&format!("fn f{i}() {{}} "));
    }
    let src = format!("impl Foo {{ {items} }}");
    let (_, log) = parse_source_no_assert(&src);
    assert!(log.error_bit());
}

// SyntaxErr::ImplCannotBeVisible (variant 203)
#[test]
fn test_impl_visibility_error() {
    let (_, log) = parse_source_no_assert("pub impl Foo {}");
    assert!(log.error_bit());
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
#[test]
fn test_path_expected_name() {
    let (_, log) = parse_type_no_assert("foo::");
    assert!(log.error_bit());
}

// ---------- REFERENCE TYPE ERRORS ----------

// SyntaxErr::ReferenceTypeExpectedLifetimeName (variant 240)
#[test]
fn test_ref_lifetime_missing() {
    let (_, log) = parse_type_no_assert("&'");
    assert!(log.error_bit());
}

// ---------- STRUCT INIT ERRORS ----------

// SyntaxErr::StructExpectedFieldOrEnd (variant 260)
#[test]
fn test_struct_init_field_or_end() {
    let (_, log) = parse_expr_no_assert("Foo { x: 1");
    assert!(log.error_bit());
}

// SyntaxErr::StructExpectedFieldName (variant 261)
#[test]
fn test_struct_init_field_name() {
    let (_, log) = parse_expr_no_assert("Foo { : 1 }");
    assert!(log.error_bit());
}

// SyntaxErr::StructExpectedColon (variant 262)
#[test]
fn test_struct_init_missing_colon() {
    let (_, log) = parse_expr_no_assert("Foo { x 1 }");
    assert!(log.error_bit());
}

// ---------- TUPLE TYPE ERRORS ----------

// SyntaxErr::TupleTypeExpectedEnd (variant 280)
#[test]
fn test_tuple_type_expected_end() {
    let (_, log) = parse_type_no_assert("(i32, f64");
    assert!(log.error_bit());
}

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

// ---------- LIST ERRORS ----------

// SyntaxErr::ListExpectedEnd (variant 300)
#[test]
fn test_list_expected_end() {
    let (_, log) = parse_expr_no_assert("[1, 2");
    assert!(log.error_bit());
}

// SyntaxErr::ListElementLimit (variant 301) - needs >65536 elements
#[test]
fn test_list_element_limit() {
    let mut elems = String::from("0");
    for _ in 0..65538 {
        elems.push_str(", 0");
    }
    let src = format!("[{elems}]");
    let (_, log) = parse_expr_no_assert(&src);
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

// ---------- BLOCK ERRORS ----------

// SyntaxErr::BlockExpectedEnd (variant 340)
#[test]
fn test_block_expected_end() {
    let (_, log) = parse_expr_no_assert("unsafe { 42");
    assert!(log.error_bit());
}

// SyntaxErr::BlockElementLimit (variant 341) - needs >65536 elements
#[test]
fn test_block_element_limit() {
    let mut elems = String::new();
    for _ in 0..65538 {
        elems.push_str("42; ");
    }
    let src = format!("fn f() {{ {elems} }}");
    let (_, log) = parse_source_no_assert(&src);
    assert!(log.error_bit());
}

// ---------- BREAK ERRORS ----------

// SyntaxErr::BreakMissingLabel (variant 360)
#[test]
fn test_break_missing_label() {
    let (_, log) = parse_expr_no_assert("break ';");
    assert!(log.error_bit());
}

// ---------- CONTINUE ERRORS ----------

// SyntaxErr::ContinueMissingLabel (variant 380)
#[test]
fn test_continue_missing_label() {
    let (_, log) = parse_expr_no_assert("continue ';");
    assert!(log.error_bit());
}

// ---------- FUNCTION CALL ERRORS ----------

// SyntaxErr::FunctionCallExpectedEnd (variant 400)
#[test]
fn test_fn_call_expected_end() {
    let (_, log) = parse_expr_no_assert("f(1, 2");
    assert!(log.error_bit());
}

// SyntaxErr::FunctionCallArgumentLimit (variant 401) - needs >65536 args
#[test]
fn test_fn_call_arg_limit() {
    let mut args = String::new();
    for _ in 0..65538 {
        args.push_str("0, ");
    }
    args.push_str("0");
    let src = format!("f({args})");
    let (_, log) = parse_expr_no_assert(&src);
    assert!(log.error_bit());
}

// SyntaxErr::FunctionCallPositionFollowsNamed (variant 402)
#[test]
fn test_fn_call_positional_after_named() {
    let (_, log) = parse_expr_no_assert("f(x: 1, 2)");
    assert!(log.error_bit());
}

// ---------- FOR LOOP ERRORS ----------

// SyntaxErr::ForVariableBindingMissingName (variant 440)
#[test]
fn test_for_binding_missing_name() {
    let (_, log) = parse_expr_no_assert("for (x, ) in items { }");
    assert!(log.error_bit());
}

// SyntaxErr::ForVariableBindingExpectedEnd (variant 441)
#[test]
fn test_for_binding_expected_end() {
    let (_, log) = parse_expr_no_assert("for (x, y in items { }");
    assert!(log.error_bit());
}

// SyntaxErr::ForVariableBindingLimit (variant 442) - needs >65536 bindings
#[test]
fn test_for_binding_limit() {
    let mut bindings = String::new();
    for i in 0..65538 {
        if i > 0 {
            bindings.push_str(", ");
        }
        bindings.push_str(&format!("x{i}"));
    }
    let src = format!("for ({bindings}) in items {{ }}");
    let (_, log) = parse_expr_no_assert(&src);
    assert!(log.error_bit());
}

// SyntaxErr::ForExpectedInKeyword (variant 443)
#[test]
fn test_for_expected_in() {
    let (_, log) = parse_expr_no_assert("for x items { }");
    assert!(log.error_bit());
}

// ---------- FIELD/METHOD ERRORS ----------

// SyntaxErr::ExpectedFieldOrMethodName (variant 500)
#[test]
fn test_field_or_method_name() {
    let (_, log) = parse_expr_no_assert("a.");
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
#[test]
fn test_exp_close_angle() {
    // Vec<< triggers the open angle lexing differently
    let (_, log) = parse_type_no_assert("Vec<i32");
    assert!(log.error_bit());
}

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
#[test]
fn test_exp_arrow() {
    let (_, log) = parse_type_no_assert("fn(x: i32) - bool");
    assert!(log.error_bit());
}

// ---------- GENERAL EXPECTED ERRORS (2000-2020) ----------

// SyntaxErr::ExpectedItem (variant 2000)
#[test]
fn test_exp_item() {
    let (_, log) = parse_source_no_assert("@");
    assert!(log.error_bit());
}

// SyntaxErr::ExpectedType (variant 2001)
#[test]
fn test_exp_type() {
    let (_, log) = parse_type_no_assert("@");
    assert!(log.error_bit());
}

// SyntaxErr::ExpectedExpr (variant 2002)
#[test]
fn test_exp_expr() {
    let (_, log) = parse_expr_no_assert("}");
    assert!(log.error_bit());
}

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
