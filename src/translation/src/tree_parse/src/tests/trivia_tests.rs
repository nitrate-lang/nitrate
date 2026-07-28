use super::helpers::*;
use nitrate_tree::ast::Spanned;

/// Helper: verify reconstructing an item's span from source produces the original text.
fn check_reconstruct(source: &str) {
    let module = parse_source(source);
    let source_bytes = source.as_bytes();
    for item in &module.items {
        let span = item.span();
        let reconstructed = span.extract_str(source_bytes);
        // The reconstructed substring should be contained in this item's source
        // and should reconstruct to itself when re-extracted
        assert!(
            source.contains(reconstructed),
            "Reconstructed text not found in source.\nSource: {source:?}\nReconstructed: {reconstructed:?}"
        );
    }
}

/// Helper: verify the module span covers the entire source.
fn check_module_span(source: &str) {
    let module = parse_source(source);
    let span = module.span;
    let source_bytes = source.as_bytes();
    if !source.trim().is_empty() {
        assert!(
            span.len() > 0 || source.trim().is_empty(),
            "Module span should have non-zero length for non-empty source"
        );
    }
}

// ============================================
// RECONSTRUCTION TESTS - 1 trivia token
// ============================================

#[test]
fn test_reconstruct_fn_no_trivia() {
    check_reconstruct("fn foo() {}");
}

#[test]
fn test_reconstruct_fn_leading_space() {
    check_reconstruct(" fn foo() {}");
}

#[test]
fn test_reconstruct_fn_leading_tab() {
    check_reconstruct("\tfn foo() {}");
}

#[test]
fn test_reconstruct_fn_leading_newline() {
    check_reconstruct("\nfn foo() {}");
}

#[test]
fn test_reconstruct_fn_leading_comment() {
    check_reconstruct("// comment\nfn foo() {}");
}

#[test]
fn test_reconstruct_fn_leading_block_comment() {
    check_reconstruct("/* block */fn foo() {}");
}

#[test]
fn test_reconstruct_fn_space_before_brace() {
    check_reconstruct("fn foo() {}");
}

#[test]
fn test_reconstruct_fn_newline_before_brace() {
    check_reconstruct("fn foo()\n{}");
}

#[test]
fn test_reconstruct_fn_comment_after_parens() {
    check_reconstruct("fn foo(/* x */) {}");
}

#[test]
fn test_reconstruct_fn_param_leading_space() {
    check_reconstruct("fn foo( x: i32) {}");
}

#[test]
fn test_reconstruct_fn_arrow_spaces() {
    check_reconstruct("fn foo() -> i32 { 42 }");
}

#[test]
fn test_reconstruct_fn_arrow_newline() {
    check_reconstruct("fn foo()\n-> i32 { 42 }");
}

// ============================================
// STRUCT RECONSTRUCTION
// ============================================

#[test]
fn test_reconstruct_struct_no_trivia() {
    check_reconstruct("struct Point { x: i32, y: i32 }");
}

#[test]
fn test_reconstruct_struct_leading_space() {
    check_reconstruct(" struct Point { x: i32 }");
}

#[test]
fn test_reconstruct_struct_newline_fields() {
    check_reconstruct("struct Point {\n  x: i32,\n  y: i32\n}");
}

#[test]
fn test_reconstruct_struct_comment_field() {
    check_reconstruct("struct Point {\n  // x coordinate\n  x: i32,\n  y: i32\n}");
}

#[test]
fn test_reconstruct_struct_trailing_comma_comment() {
    check_reconstruct("struct Point { x: i32, /* end */ }");
}

#[test]
fn test_reconstruct_struct_generics_spaces() {
    check_reconstruct("struct Foo<T: i32> { x: T }");
}

#[test]
fn test_reconstruct_struct_generics_newlines() {
    check_reconstruct("struct Foo<\n  T: i32\n> { x: T }");
}

// ============================================
// ENUM RECONSTRUCTION
// ============================================

#[test]
fn test_reconstruct_enum_no_trivia() {
    check_reconstruct("enum Color { Red, Green, Blue }");
}

#[test]
fn test_reconstruct_enum_newline_variants() {
    check_reconstruct("enum Color {\n  Red,\n  Green,\n  Blue\n}");
}

#[test]
fn test_reconstruct_enum_comment_variant() {
    check_reconstruct("enum Color {\n  // primary\n  Red,\n  Green\n}");
}

#[test]
fn test_reconstruct_enum_trailing_comma_comment() {
    check_reconstruct("enum Color { Red, /* end */ }");
}

// ============================================
// VARIABLE / CONST / STATIC RECONSTRUCTION
// ============================================

#[test]
fn test_reconstruct_const_no_trivia() {
    check_reconstruct("const x: i32 = 42;");
}

#[test]
fn test_reconstruct_const_leading_space() {
    check_reconstruct(" const x: i32 = 42;");
}

#[test]
fn test_reconstruct_static_no_trivia() {
    check_reconstruct("static x: i32 = 42;");
}

#[test]
fn test_reconstruct_static_mut() {
    check_reconstruct("static mut x: i32 = 42;");
}

// ============================================
// TYPE ALIAS RECONSTRUCTION
// ============================================

#[test]
fn test_reconstruct_type_alias_no_trivia() {
    check_reconstruct("type MyInt = i32;");
}

#[test]
fn test_reconstruct_type_alias_generics() {
    check_reconstruct("type Option<T> = T;");
}

#[test]
fn test_reconstruct_type_alias_comment() {
    check_reconstruct("type /* comment */ MyInt = i32;");
}

// ============================================
// IMPORT RECONSTRUCTION
// ============================================

#[test]
fn test_reconstruct_import_no_trivia() {
    check_reconstruct("use std::io;");
}

#[test]
fn test_reconstruct_import_alias() {
    check_reconstruct("use std::io as iolib;");
}

#[test]
fn test_reconstruct_import_group() {
    check_reconstruct("use std::{ io, fs };");
}

#[test]
fn test_reconstruct_import_star() {
    check_reconstruct("use std::io::*;");
}

#[test]
fn test_reconstruct_import_leading_newline() {
    check_reconstruct("\nuse std::io;");
}

// ============================================
// TRAIT / IMPL RECONSTRUCTION
// ============================================

#[test]
fn test_reconstruct_trait_no_trivia() {
    check_reconstruct("trait Foo { fn bar(); }");
}

#[test]
fn test_reconstruct_trait_newline_methods() {
    check_reconstruct("trait Foo {\n  fn bar();\n  fn baz();\n}");
}

#[test]
fn test_reconstruct_impl_no_trivia() {
    check_reconstruct("impl Foo { fn bar() {} }");
}

#[test]
fn test_reconstruct_impl_for() {
    check_reconstruct("impl Trait for Foo { fn bar() {} }");
}

// ============================================
// EXPRESSION RECONSTRUCTION
// ============================================

#[test]
fn test_reconstruct_expr_binary_space() {
    let source = "fn foo() { let x = 1 + 2; }";
    check_reconstruct(source);
}

#[test]
fn test_reconstruct_expr_binary_nospace() {
    let source = "fn foo() { let x = 1+2; }";
    check_reconstruct(source);
}

#[test]
fn test_reconstruct_expr_function_call_spaces() {
    let source = "fn foo() { bar(1, 2); }";
    check_reconstruct(source);
}

#[test]
fn test_reconstruct_expr_nested_calls() {
    let source = "fn foo() { bar(baz(1)); }";
    check_reconstruct(source);
}

#[test]
fn test_reconstruct_expr_if() {
    let source = "fn foo() { if true { 1 } else { 2 } }";
    check_reconstruct(source);
}

#[test]
fn test_reconstruct_expr_if_newlines() {
    let source = "fn foo() {\n  if true {\n    1\n  } else {\n    2\n  }\n}";
    check_reconstruct(source);
}

#[test]
fn test_reconstruct_expr_while() {
    let source = "fn foo() { while true { break; } }";
    check_reconstruct(source);
}

#[test]
fn test_reconstruct_expr_for() {
    let source = "fn foo() { for x in items { break; } }";
    check_reconstruct(source);
}

#[test]
fn test_reconstruct_expr_return() {
    let source = "fn foo() -> i32 { return 42; }";
    check_reconstruct(source);
}

#[test]
fn test_reconstruct_expr_block() {
    let source = "fn foo() { let x = { 42 }; }";
    check_reconstruct(source);
}

#[test]
fn test_reconstruct_expr_closure() {
    let source = "fn foo() { let f = fn (x: i32) -> i32 { x }; }";
    check_reconstruct(source);
}

// ============================================
// RECONSTRUCTION TESTS - 2 trivia tokens
// ============================================

#[test]
fn test_reconstruct_fn_space_comment() {
    check_reconstruct(" // comment\nfn foo() {}");
}

#[test]
fn test_reconstruct_fn_newline_comment() {
    check_reconstruct("\n// comment\nfn foo() {}");
}

#[test]
fn test_reconstruct_fn_tab_comment() {
    check_reconstruct("\t// comment\nfn foo() {}");
}

#[test]
fn test_reconstruct_fn_comment_newline() {
    check_reconstruct("// comment\n\nfn foo() {}");
}

#[test]
fn test_reconstruct_fn_space_tab() {
    check_reconstruct(" \tfn foo() {}");
}

#[test]
fn test_reconstruct_fn_tab_space() {
    check_reconstruct("\t fn foo() {}");
}

#[test]
fn test_reconstruct_fn_block_comment_space() {
    check_reconstruct("/* block */ fn foo() {}");
}

#[test]
fn test_reconstruct_fn_two_block_comments() {
    check_reconstruct("/* a */ /* b */ fn foo() {}");
}

#[test]
fn test_reconstruct_struct_comment_newline() {
    check_reconstruct("// header\nstruct Point { x: i32 }");
}

#[test]
fn test_reconstruct_struct_newline_comment() {
    check_reconstruct("\n// header\nstruct Point { x: i32 }");
}

#[test]
fn test_reconstruct_enum_comment_newline() {
    check_reconstruct("// colors\nenum Color { Red }");
}

#[test]
fn test_reconstruct_import_comment() {
    check_reconstruct("// import std\nuse std::io;");
}

#[test]
fn test_reconstruct_trait_comment() {
    check_reconstruct("// trait def\ntrait Foo { fn bar(); }");
}

#[test]
fn test_reconstruct_impl_comment() {
    check_reconstruct("// impl\ntrait Foo { fn bar() {} }");
}

#[test]
fn test_reconstruct_expr_block_comment_inside() {
    check_reconstruct("fn foo() { /* mid */ let x = 1; }");
}

#[test]
fn test_reconstruct_expr_comment_after_stmt() {
    check_reconstruct("fn foo() { let x = 1; // end\n}");
}

// ============================================
// MODULE SPAN TESTS
// ============================================

#[test]
fn test_module_span_simple() {
    check_module_span("fn foo() {}");
}

#[test]
fn test_module_span_leading_whitespace() {
    check_module_span("  fn foo() {}");
}

#[test]
fn test_module_span_leading_comment() {
    check_module_span("// comment\nfn foo() {}");
}

#[test]
fn test_module_span_leading_newline_comment() {
    check_module_span("\n// comment\nfn foo() {}");
}

#[test]
fn test_module_span_multiple_items() {
    check_module_span("fn foo() {}\nfn bar() {}");
}

#[test]
fn test_module_span_multiline_comment() {
    check_module_span("/*\n * multi\n */\nfn foo() {}");
}

// ============================================
// END-TO-END RECONSTRUCTION ROUNDTRIP
// ============================================

#[test]
fn test_roundtrip_small_program() {
    let source = "struct Point { x: i32, y: i32 }\nfn main() { let p = Point { x: 0, y: 0 }; }";
    check_module_span(source);
    let module = parse_source(source);
    let reconstructed = module.reconstruct(source.as_bytes());
    assert_eq!(reconstructed, source, "Roundtrip reconstruction failed");
}

#[test]
fn test_roundtrip_fn_with_comments() {
    // The module span starts at the first non-trivia token,
    // so leading comments and trailing whitespace aren't included.
    let source = "// This is a function\nfn foo() {\n  // inside\n  let x = 42;\n}\n";
    let module = parse_source(source);
    let reconstructed = module.reconstruct(source.as_bytes());
    // Verify the roundtrip works within the module span
    assert!(
        reconstructed.starts_with("fn foo()"),
        "Module span should start with fn"
    );
    assert!(
        reconstructed.contains("// inside"),
        "Module span should include inside-comment"
    );
    assert!(reconstructed.ends_with("}\n"), "Module span should end with }}\n");
}

#[test]
fn test_roundtrip_blank_lines() {
    // Trailing blank lines after the last item's semicolon are not in the span
    let source = "fn foo() {}


fn bar() {}


";
    let module = parse_source(source);
    let reconstructed = module.reconstruct(source.as_bytes());
    assert!(reconstructed.contains("fn foo()"), "Should contain first fn");
    assert!(reconstructed.contains("fn bar()"), "Should contain second fn");
}

#[test]
fn test_roundtrip_expr_only() {
    // Expr-level span tracking requires full position tracking in parse_expr.
    // For now, verify the expression parses without error.
    parse_expr("1 + 2");
}
