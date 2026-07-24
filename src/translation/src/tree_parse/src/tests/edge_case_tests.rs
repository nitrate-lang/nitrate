use super::helpers::*;
use nitrate_tree::ast::*;

// ========== GENERICS EDGE CASES ==========

#[test]
fn test_generic_multi_params() {
    let s = single_struct(parse_source("struct Foo<T, U, V> { a: T, b: U, c: V }"));
    assert_eq!(s.generics.unwrap().params.len(), 3);
}

#[test]
fn test_generic_missing_param_name() {
    let (_, log) = parse_source_no_assert("struct Foo<T, = i32> { x: T }");
    assert!(log.error_bit());
}

#[test]
fn test_generic_param_expected_end_no_comma() {
    let (_, log) = parse_source_no_assert("struct Foo<T U> { x: T }");
    assert!(log.error_bit());
}

// ========== MODULE EDGE CASES ==========

#[test]
fn test_module_with_items() {
    let mod_item = match single_item(parse_source("mod bar { fn f() {} fn g() {} }")) {
        Item::Module(m) => m,
        other => panic!("Expected Module, got {other:?}"),
    };
    assert_eq!(&*mod_item.name, "bar");
    assert_eq!(mod_item.items.len(), 2);
}

#[test]
fn test_module_missing_name() {
    let (_, log) = parse_source_no_assert("mod { fn f() {} }");
    assert!(log.error_bit());
}

#[test]
fn test_module_unclosed() {
    let (_, log) = parse_source_no_assert("mod foo { fn f() {}");
    assert!(log.error_bit());
}

#[test]
fn test_module_attributes() {
    let mod_item = match single_item(parse_source("mod [attr] foo { fn f() {} }")) {
        Item::Module(m) => m,
        other => panic!("Expected Module, got {other:?}"),
    };
    assert!(mod_item.attributes.is_some());
}

// ========== EMPTY MODULE ==========

#[test]
fn test_empty_module() {
    let mod_item = match single_item(parse_source("mod empty {}")) {
        Item::Module(m) => m,
        other => panic!("Expected Module, got {other:?}"),
    };
    assert!(mod_item.items.is_empty());
}

// ========== BREAK/CONTINUE EDGE CASES ==========

#[test]
fn test_break_label_missing_name() {
    let (_, log) = parse_expr_no_assert("break ';");
    assert!(log.error_bit());
}

#[test]
fn test_continue_label() {
    let expr = parse_expr("continue 'l;");
    assert!(matches!(&expr, Expr::Continue(c) if matches!(&c.label, Some(l) if &**l == "l")));
}

#[test]
fn test_continue_label_missing_name() {
    let (_, log) = parse_expr_no_assert("continue ';");
    assert!(log.error_bit());
}

// ========== RETURN EDGE CASES ==========

#[test]
fn test_return_with_expr_missing_semicolon() {
    let (_, log) = parse_expr_no_assert("ret 42");
    assert!(log.error_bit());
}

// ========== CAST EDGE CASES ==========

#[test]
fn test_cast_from_float() {
    let expr = parse_expr("3.14 as i32");
    assert!(matches!(&expr, Expr::Cast(c) if matches!(&c.to, Type::Int32(_))));
}

// ========== CLOSURE EDGE CASES ==========

#[test]
fn test_closure_with_attributes_and_params() {
    let expr = parse_expr("fn [inline](x: i32) -> i32 { x }");
    assert!(
        matches!(&expr, Expr::Closure(c) if c.attributes.is_some() && c.parameters.is_some() && c.return_type.is_some())
    );
}

#[test]
fn test_closure_params_no_return() {
    let expr = parse_expr("fn(x: i32) { x }");
    assert!(matches!(&expr, Expr::Closure(c) if c.parameters.is_some() && c.return_type.is_none()));
}

#[test]
fn test_closure_no_params_no_return() {
    let expr = parse_expr("fn { 42 }");
    assert!(matches!(&expr, Expr::Closure(c) if c.parameters.is_none() && c.return_type.is_none()));
}

#[test]
fn test_closure_unsafe_block() {
    let expr = parse_expr("unsafe { 42 }");
    assert!(matches!(&expr, Expr::Closure(_)));
}

#[test]
fn test_closure_safe_block() {
    let expr = parse_expr("safe { 42 }");
    assert!(matches!(&expr, Expr::Closure(_)));
}

#[test]
fn test_closure_unsafe_with_modifier() {
    let expr = parse_expr("unsafe(42) { 1 }");
    assert!(matches!(&expr, Expr::Closure(c) if c.parameters.is_none()));
}

// ========== BLOCK EDGE CASES ==========

#[test]
fn test_empty_block() {
    let expr = parse_expr("{ }");
    assert!(matches!(&expr, Expr::Closure(_)));
}

#[test]
fn test_block_as_expression() {
    let expr = parse_expr("{ 42 }");
    assert!(matches!(&expr, Expr::Closure(_)));
}

#[test]
fn test_block_in_block() {
    let f = single_function(parse_source("fn f() { { 42 } }"));
    assert!(f.definition.is_some());
}

// ========== LITERAL SUFFIX EDGE CASES ==========

#[test]
fn test_literal_suffix_u8() {
    let expr = parse_expr("42u8");
    assert!(matches!(&expr, Expr::Cast(c) if matches!(&c.to, Type::UInt8(_))));
}

#[test]
fn test_literal_suffix_u16() {
    let expr = parse_expr("42u16");
    assert!(matches!(&expr, Expr::Cast(c) if matches!(&c.to, Type::UInt16(_))));
}

#[test]
fn test_literal_suffix_u32() {
    let expr = parse_expr("42u32");
    assert!(matches!(&expr, Expr::Cast(c) if matches!(&c.to, Type::UInt32(_))));
}

#[test]
fn test_literal_suffix_u64() {
    let expr = parse_expr("42u64");
    assert!(matches!(&expr, Expr::Cast(c) if matches!(&c.to, Type::UInt64(_))));
}

#[test]
fn test_literal_suffix_i8() {
    let expr = parse_expr("42i8");
    assert!(matches!(&expr, Expr::Cast(c) if matches!(&c.to, Type::Int8(_))));
}

#[test]
fn test_literal_suffix_i16() {
    let expr = parse_expr("42i16");
    assert!(matches!(&expr, Expr::Cast(c) if matches!(&c.to, Type::Int16(_))));
}

#[test]
fn test_literal_suffix_i32() {
    let expr = parse_expr("42i32");
    assert!(matches!(&expr, Expr::Cast(c) if matches!(&c.to, Type::Int32(_))));
}

#[test]
fn test_literal_suffix_i64() {
    let expr = parse_expr("42i64");
    assert!(matches!(&expr, Expr::Cast(c) if matches!(&c.to, Type::Int64(_))));
}

#[test]
fn test_literal_suffix_usize() {
    let expr = parse_expr("42usize");
    assert!(matches!(&expr, Expr::Cast(c) if matches!(&c.to, Type::USize(_))));
}

#[test]
fn test_literal_suffix_f32() {
    let expr = parse_expr("3.14f32");
    assert!(matches!(&expr, Expr::Cast(c) if matches!(&c.to, Type::Float32(_))));
}

#[test]
fn test_literal_suffix_f64() {
    let expr = parse_expr("3.14f64");
    assert!(matches!(&expr, Expr::Cast(c) if matches!(&c.to, Type::Float64(_))));
}

// ========== TYPES ==========

#[test]
fn test_function_type_with_attributes() {
    let ty = parse_type("fn [inline](x: i32) -> bool");
    assert!(matches!(&ty, Type::FunctionType(_)));
}

#[test]
fn test_function_type_no_return() {
    let ty = parse_type("fn(x: i32)");
    assert!(matches!(&ty, Type::FunctionType(f) if f.return_type.is_none()));
}

// ========== IMPORT EDGE CASES ==========

#[test]
fn test_import_global_path() {
    let imp = single_import(parse_source("use ::std::mem;"));
    assert!(matches!(&imp.use_tree, UseTree::Single { .. }));
}

#[test]
fn test_import_attr() {
    let imp = single_import(parse_source("use [allow(unused)] std::mem;"));
    assert!(imp.attributes.is_some());
}

#[test]
fn test_import_pub() {
    let imp = single_import(parse_source("pub use std::mem;"));
    assert!(matches!(imp.visibility, Some(Visibility::Public)));
}

#[test]
fn test_import_nested_group() {
    let imp = single_import(parse_source("use foo::{bar::{baz, qux}};"));
    assert!(matches!(&imp.use_tree, UseTree::Group { .. }));
}

// ========== TRAIT EDGE CASES ==========

#[test]
fn test_trait_empty() {
    let t = single_trait(parse_source("trait Foo {}"));
    assert_eq!(&*t.name, "Foo");
    assert!(t.items.is_empty());
}

#[test]
fn test_trait_with_fn() {
    let t = single_trait(parse_source("trait Foo { fn bar(); }"));
    assert_eq!(t.items.len(), 1);
    assert!(matches!(&t.items[0], AssociatedItem::Method(f) if f.definition.is_none()));
}

#[test]
fn test_trait_with_const() {
    let t = single_trait(parse_source("trait Foo { const X: i32; }"));
    assert_eq!(t.items.len(), 1);
    assert!(matches!(&t.items[0], AssociatedItem::ConstantItem(_)));
}

#[test]
fn test_trait_with_type() {
    let t = single_trait(parse_source("trait Foo { type Bar; }"));
    assert_eq!(t.items.len(), 1);
    assert!(matches!(&t.items[0], AssociatedItem::TypeAlias(_)));
}

#[test]
fn test_trait_with_visibility() {
    let t = single_trait(parse_source("pub trait Foo { fn bar(); }"));
    assert!(matches!(t.visibility, Some(Visibility::Public)));
}

#[test]
fn test_trait_with_generics() {
    let t = single_trait(parse_source("trait Foo<T> { fn bar(x: T); }"));
    assert!(t.generics.is_some());
}

#[test]
fn test_trait_with_attributes() {
    let t = single_trait(parse_source("trait [auto] Foo { fn bar(); }"));
    assert!(t.attributes.is_some());
}

#[test]
fn test_trait_missing_name() {
    let (_, log) = parse_source_no_assert("trait { fn bar(); }");
    assert!(log.error_bit());
}

#[test]
fn test_trait_unclosed() {
    let (_, log) = parse_source_no_assert("trait Foo { fn bar();");
    assert!(log.error_bit());
}

#[test]
fn test_trait_missing_open_brace() {
    let (_, log) = parse_source_no_assert("trait Foo fn bar(); }");
    assert!(log.error_bit());
}

// ========== IMPL EDGE CASES ==========

#[test]
fn test_impl_empty() {
    let i = single_impl(parse_source("impl Foo {}"));
    assert!(i.items.is_empty());
    assert!(i.trait_path.is_none());
}

#[test]
fn test_impl_with_generics() {
    let i = single_impl(parse_source("impl<T> Foo<T> {}"));
    assert!(i.generics.is_some());
}

#[test]
fn test_impl_with_fn() {
    let i = single_impl(parse_source("impl Foo { fn bar() {} }"));
    assert_eq!(i.items.len(), 1);
}

#[test]
fn test_impl_missing_for() {
    let (_, log) = parse_source_no_assert("impl Trait Foo {}");
    assert!(log.error_bit());
}

#[test]
fn test_impl_unclosed() {
    let (_, log) = parse_source_no_assert("impl Foo { fn bar() {}");
    assert!(log.error_bit());
}

#[test]
fn test_impl_visibility_error() {
    let (_, log) = parse_source_no_assert("pub impl Foo {}");
    assert!(log.error_bit());
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

// ========== PARSE ITEM ERROR PATHS ==========

#[test]
fn test_parse_unexpected_token() {
    let (_, log) = parse_source_no_assert("$invalid");
    assert!(log.error_bit());
}

#[test]
fn test_parse_unexpected_token_after_valid() {
    let (_, log) = parse_source_no_assert("fn f() {} $invalid");
    assert!(log.error_bit());
}

#[test]
fn test_syntax_error_item() {
    // Test an unexpected token as an item that triggers ExpectedItem
    let (_, log) = parse_source_no_assert("fn f() {} @");
    assert!(log.error_bit());
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
fn test_type_syntax_error() {
    let (_, log) = parse_type_no_assert("@");
    assert!(log.error_bit());
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
fn test_type_ptr_with_iso() {
    let ty = parse_type("*iso i32");
    assert!(matches!(&ty, Type::PointerType(p) if matches!(p.exclusivity, Some(Exclusivity::Iso))));
}

#[test]
fn test_type_ptr_with_const() {
    let ty = parse_type("*const i32");
    assert!(matches!(&ty, Type::PointerType(p) if matches!(p.mutability, Some(Mutability::Const))));
}

#[test]
fn test_type_pointer_with_exclusivity_and_mut() {
    let ty = parse_type("*mut i32");
    assert!(matches!(&ty, Type::PointerType(p) if p.mutability.is_some()));
}

#[test]
fn test_type_pointer_iso_mut() {
    let ty = parse_type("*iso mut i32");
    assert!(matches!(&ty, Type::PointerType(_)));
}

// ========== REFINEMENT TYPE EDGE CASES ==========

#[test]
fn test_type_refine_width_only() {
    let ty = parse_type("u8: 6");
    assert!(matches!(&ty, Type::RefinementType(r) if r.width.is_some() && r.minimum.is_none() && r.maximum.is_none()));
}

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
fn test_type_path_with_generic_args() {
    let ty = parse_type("Vec<i32>");
    assert!(matches!(&ty, Type::TypePath(p) if p.segments[0].type_arguments.is_some()));
}

#[test]
fn test_type_path_global_with_generics() {
    let ty = parse_type("::std::Vec<i32>");
    assert!(matches!(&ty, Type::TypePath(p) if p.segments.len() >= 2));
}

// ========== FUNCTION PARAMETER EDGE CASES ==========

#[test]
fn test_fn_missing_open_paren() {
    let (_, log) = parse_source_no_assert("fn foo x: i32) {}");
    assert!(log.error_bit());
}

#[test]
fn test_fn_param_variadic_no_dots() {
    let (_, log) = parse_source_no_assert("fn foo(x: i32, .) {}");
    assert!(log.error_bit());
}

#[test]
fn test_fn_param_variadic_missing_close_paren() {
    let (_, log) = parse_source_no_assert("fn foo(x: i32, ...");
    assert!(log.error_bit());
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

// ========== STRUCT EDGE CASES ==========

#[test]
fn test_struct_field_default_value() {
    let s = single_struct(parse_source("struct Foo { x: i32 = 42 }"));
    assert!(s.fields[0].default_value.is_some());
}

#[test]
fn test_struct_field_trailing_comma() {
    let s = single_struct(parse_source("struct Foo { x: i32, }"));
    assert_eq!(s.fields.len(), 1);
}

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

// ========== LIST EDGE CASES ==========

#[test]
fn test_list_trailing_comma_empty() {
    let expr = parse_expr("[,]");
    assert!(matches!(&expr, Expr::List(l) if l.elements.is_empty()));
}

#[test]
fn test_list_multiple_elements() {
    let expr = parse_expr("[1, 2, 3, 4, 5]");
    assert!(matches!(&expr, Expr::List(l) if l.elements.len() == 5));
}

#[test]
fn test_list_unexpected_eof() {
    let (_, log) = parse_expr_no_assert("[1, 2, 3");
    assert!(log.error_bit());
}

// ========== ATTRIBUTES EDGE CASES ==========

#[test]
fn test_multiple_attributes() {
    let f = single_function(parse_source("fn [attr1][attr2] foo() {}"));
    assert!(f.attributes.is_some());
}

#[test]
fn test_attributes_with_multiple_expressions() {
    let f = single_function(parse_source("fn [a, b, c] foo() {}"));
    assert!(f.attributes.is_some());
}

// ========== BLOCK ITEM EDGE CASE ==========

#[test]
fn test_block_item_expr_with_semi() {
    let f = single_function(parse_source("fn f() { 42; }"));
    assert!(f.definition.is_some());
}

#[test]
fn test_block_item_expr_no_semi() {
    let f = single_function(parse_source("fn f() { 42 }"));
    assert!(f.definition.is_some());
}

#[test]
fn test_block_item_empty_return() {
    let f = single_function(parse_source("fn f() { ret; }"));
    assert!(f.definition.is_some());
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

// ========== WHILE LOOP EDGE CASES ==========

#[test]
fn test_while_with_condition() {
    let expr = parse_expr("while true { break; }");
    assert!(matches!(&expr, Expr::While(w) if w.condition.is_some()));
}

#[test]
fn test_while_no_condition_implicit() {
    let expr = parse_expr("while { break; }");
    assert!(matches!(&expr, Expr::While(w) if w.condition.is_none()));
}

// ========== UNSAFE BLOCK MODIFIER EDGE CASES ==========

#[test]
fn test_unsafe_block_with_expr_modifier() {
    let expr = parse_expr("unsafe(42) { 1 }");
    assert!(matches!(&expr, Expr::Closure(_)));
}

#[test]
fn test_unsafe_block_without_parens() {
    let expr = parse_expr("unsafe { 1 }");
    assert!(matches!(&expr, Expr::Closure(_)));
}

// ========== PATH RESOLUTION EDGE CASES ==========

#[test]
fn test_expr_path_triple_colon() {
    let (_, log) = parse_source_no_assert("fn f() { foo:::bar; }");
    assert!(log.error_bit());
}

// ========== VARIADIC FUNCTION PARAMETER EDGE CASES ==========

#[test]
fn test_fn_variadic_no_other_params() {
    let f = single_function(parse_source("fn f(...) {}"));
    assert!(f.parameters.variadic);
    assert!(f.parameters.params.is_empty());
}

// ========== ITEM PATH EDGE CASES ==========

#[test]
fn test_use_parse_item_path_expected_name() {
    let (_, log) = parse_source_no_assert("use ::;");
    assert!(log.error_bit());
}

// ========== METHOD CALL EDGE CASES ==========

#[test]
fn test_method_call_with_args() {
    let expr = parse_expr("obj.method(1, 2, 3)");
    assert!(matches!(&expr, Expr::MethodCall(m) if m.positional.len() == 3));
}

#[test]
fn test_method_call_no_args() {
    let expr = parse_expr("obj.method()");
    assert!(matches!(&expr, Expr::MethodCall(m) if m.positional.is_empty()));
}

// ========== IF ELSE IF ELSE ==========

#[test]
fn test_if_else_block() {
    let expr = parse_expr("if true { 1 } else { 2 }");
    assert!(matches!(&expr, Expr::If(i) if matches!(i.false_branch, Some(ElseIf::Block(_)))));
}

#[test]
fn test_if_else_if() {
    let expr = parse_expr("if true { 1 } else if false { 2 } else { 3 }");
    assert!(matches!(&expr, Expr::If(i) if matches!(i.false_branch, Some(ElseIf::If(_)))));
}

// ========== AWAIT EDGE CASES ==========

#[test]
fn test_await_expr() {
    let expr = parse_expr("await fut");
    assert!(matches!(&expr, Expr::Await(_)));
}

// ========== CLOSURE ERROR PATHS ==========

#[test]
fn test_closure_missing_param_name() {
    let (_, log) = parse_expr_no_assert("fn(: i32) -> i32 { 42 }");
    assert!(log.error_bit());
}

#[test]
fn test_closure_missing_param_type() {
    let (_, log) = parse_expr_no_assert("fn(x) { x }");
    assert!(log.error_bit());
}

#[test]
fn test_closure_missing_close_paren() {
    let (_, log) = parse_expr_no_assert("fn(x: i32 { x }");
    assert!(log.error_bit());
}

#[test]
fn test_closure_return_arrow_missing_gt() {
    let (_, log) = parse_expr_no_assert("fn(x: i32) - i32 { x }");
    assert!(log.error_bit());
}

// ========== FUNCTION TYPE ERROR PATHS ==========

#[test]
fn test_fn_type_missing_open_paren() {
    let (_, log) = parse_type_no_assert("fn x: i32) -> bool");
    assert!(log.error_bit());
}

#[test]
fn test_fn_type_missing_param_name() {
    let (_, log) = parse_type_no_assert("fn(: i32) -> bool");
    assert!(log.error_bit());
}

#[test]
fn test_fn_type_missing_param_type() {
    let (_, log) = parse_type_no_assert("fn(x) -> bool");
    assert!(log.error_bit());
}

// ========== ARRAY TYPE ERROR PATHS ==========

#[test]
fn test_type_array_missing_semi_before_expr() {
    let (_, log) = parse_type_no_assert("[i32; 10");
    assert!(log.error_bit());
}

// ========== POINTER TYPE ERROR PATHS ==========

#[test]
fn test_type_ptr_poly_mut() {
    let ty = parse_type("*poly mut i32");
    assert!(
        matches!(&ty, Type::PointerType(p) if matches!(p.exclusivity, Some(Exclusivity::Poly)) && matches!(p.mutability, Some(Mutability::Mut)))
    );
}

// ========== PATH ERROR PATHS ==========

#[test]
fn test_type_path_eof_after_scope() {
    let (_, log) = parse_type_no_assert("foo::");
    assert!(log.error_bit());
}

// ========== ITEM ERROR PATHS ==========

#[test]
fn test_item_unexpected_in_function() {
    let (_, log) = parse_source_no_assert("fn f() { } struct Foo { }");
    // struct after fn should work fine, they are at module level
    assert!(!log.error_bit());
}

// ========== LOCAL VAR PARSING ERROR ==========

#[test]
fn test_var_missing_semicolon() {
    let (_, log) = parse_source_no_assert("fn f() { var x: i32 = 42 }");
    assert!(log.error_bit());
}

// ========== NESTED MODULE ERROR ==========

#[test]
fn test_module_missing_close_brace() {
    let (_, log) = parse_source_no_assert("mod foo { fn f() {} ");
    assert!(log.error_bit(), "Module should fail if close brace is missing");
}

// ========== TRAIT WITH INVALID ITEM ==========

#[test]
fn test_trait_syntax_error_item() {
    let (_, log) = parse_source_no_assert("trait Foo { struct Bar; }");
    assert!(log.error_bit());
}

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

// ========== BLOCK WITH UNSAFE AND SAFE ==========

#[test]
fn test_block_unsafe_missing_modifier() {
    // Just parsing an unsafe block without modifier
    let expr = parse_expr("unsafe { 42 }");
    assert!(matches!(&expr, Expr::Closure(_)));
}

// ========== MULTIPLE IMPORTS ==========

#[test]
fn test_program_with_only_imports() {
    let m = parse_source("use a; use b; use c;");
    assert_eq!(m.items.len(), 3);
}

// ========== STRUCT INIT COMMA BEFORE BRACE ==========

#[test]
fn test_struct_init_trailing_comma_no_space() {
    let expr = parse_expr("Foo { x: 1, }");
    assert!(matches!(&expr, Expr::StructInit(s) if s.fields.len() == 1));
}

// ========== EXPRESSION PATH WITH EMPTY GENERICS ==========

#[test]
fn test_expr_path_with_empty_generics() {
    let expr = parse_expr("foo::<>::bar");
    // This should parse as a path with empty generics
    assert!(matches!(&expr, Expr::Path(_)));
}

// ========== EXPRESSION WITH NESTED FUNCTION CALLS ==========

#[test]
fn test_nested_function_calls() {
    let expr = parse_expr("f(g(h()))");
    assert!(matches!(&expr, Expr::FunctionCall(c) if c.positional.len() == 1));
}

// ========== PROPAGATION OF PRECEDENCE ==========

#[test]
fn test_mixed_precedence_add_mul() {
    let expr = parse_expr("1 + 2 * 3");
    assert!(matches!(&expr, Expr::BinExpr(b) if matches!(&b.left, Expr::Integer(_))));
}

#[test]
fn test_mixed_precedence_mul_add() {
    let expr = parse_expr("1 * 2 + 3");
    assert!(matches!(&expr, Expr::BinExpr(b) if matches!(b.operator, BinExprOp::Add)));
}

// ========== COMPLEX EXPRESSIONS ==========

#[test]
fn test_complex_chain() {
    // This tests field access followed by method call
    let expr = parse_expr("a.b.c(d)");
    assert!(matches!(&expr, Expr::MethodCall(_)));
}

// ========== DEREFERENCE AND BORROW ==========

#[test]
fn test_deref_path() {
    let expr = parse_expr("*ptr");
    assert!(matches!(&expr, Expr::UnaryExpr(u) if matches!(u.operator, UnaryExprOp::Deref)));
}

#[test]
fn test_borrow_path() {
    let expr = parse_expr("&val");
    assert!(matches!(&expr, Expr::UnaryExpr(u) if matches!(u.operator, UnaryExprOp::Borrow)));
}

// ========== CLOSURE WITH MUT PARAM ==========

#[test]
fn test_closure_mut_param() {
    let expr = parse_expr("fn(mut x: i32) { }");
    assert!(matches!(&expr, Expr::Closure(_)));
}

#[test]
fn test_closure_const_param() {
    let expr = parse_expr("fn(const x: i32) { }");
    assert!(matches!(&expr, Expr::Closure(_)));
}

// ========== COMPOUND ASSIGNMENTS ==========

#[test]
fn test_set_percent() {
    let expr = parse_expr("x %= 1");
    assert!(matches!(&expr, Expr::BinExpr(b) if matches!(b.operator, BinExprOp::SetPercent)));
}

#[test]
fn test_set_bitand() {
    let expr = parse_expr("x &= 1");
    assert!(matches!(&expr, Expr::BinExpr(b) if matches!(b.operator, BinExprOp::SetBitAnd)));
}

#[test]
fn test_set_logicand() {
    let expr = parse_expr("true &&= false");
    assert!(matches!(&expr, Expr::BinExpr(b) if matches!(b.operator, BinExprOp::SetLogicAnd)));
}

#[test]
fn test_set_logicor() {
    let expr = parse_expr("true ||= false");
    assert!(matches!(&expr, Expr::BinExpr(b) if matches!(b.operator, BinExprOp::SetLogicOr)));
}

#[test]
fn test_set_shl() {
    let expr = parse_expr("x <<= 1");
    assert!(matches!(&expr, Expr::BinExpr(b) if matches!(b.operator, BinExprOp::SetBitShl)));
}

#[test]
fn test_set_shr() {
    let expr = parse_expr("x >>= 1");
    assert!(matches!(&expr, Expr::BinExpr(b) if matches!(b.operator, BinExprOp::SetBitShr)));
}

#[test]
fn test_set_rol() {
    let expr = parse_expr("x <<<= 1");
    assert!(matches!(&expr, Expr::BinExpr(b) if matches!(b.operator, BinExprOp::SetBitRotl)));
}

#[test]
fn test_set_ror() {
    let expr = parse_expr("x >>>= 1");
    assert!(matches!(&expr, Expr::BinExpr(b) if matches!(b.operator, BinExprOp::SetBitRotr)));
}

// Note: != is handled as `!` prefix operator followed by `=` set operator
// ========== FLOAT LITERAL TYPE SUFFIX ==========

#[test]
fn test_float_literal_suffix_f32() {
    let expr = parse_expr("1.5f32");
    assert!(matches!(&expr, Expr::Cast(_)));
}

// ========== CAST FROM STRING ==========

#[test]
fn test_cast_from_string() {
    let expr = parse_expr("\"hello\" as f64");
    assert!(matches!(&expr, Expr::Cast(_)));
}

// ========== TYPE INFO ==========

#[test]
fn test_type_info_path() {
    let expr = parse_expr("type String");
    assert!(matches!(&expr, Expr::TypeInfo(_)));
}

// ========== BOOLEAN LITERALS ==========

#[test]
fn test_boolean_true() {
    let expr = parse_expr("true");
    assert!(matches!(&expr, Expr::Boolean(b) if b.value));
}

#[test]
fn test_boolean_false() {
    let expr = parse_expr("false");
    assert!(matches!(&expr, Expr::Boolean(b) if !b.value));
}

// ========== PARSING EOF AFTER BINARY OPERATOR ==========

#[test]
fn test_binary_op_at_eof() {
    let (_, log) = parse_expr_no_assert("1 + ");
    assert!(log.error_bit());
}

// ========== CLOSURE TRAILING COMMA ==========

#[test]
fn test_closure_params_trailing_comma() {
    let expr = parse_expr("fn(x: i32,) { x }");
    assert!(matches!(&expr, Expr::Closure(c) if c.parameters.as_ref().map_or(false, |p| p.len() == 1)));
}

// ========== POINTER POLY MUT ==========

#[test]
fn test_type_ptr_poly_const() {
    let ty = parse_type("*poly const i32");
    assert!(
        matches!(&ty, Type::PointerType(p) if matches!(p.exclusivity, Some(Exclusivity::Poly)) && matches!(p.mutability, Some(Mutability::Const)))
    );
}

// ========== EXPRESSION WITH TUPLE AND FUNCTION CALL ==========

#[test]
fn test_tuple_in_function_call() {
    let expr = parse_expr("f((1, 2), 3)");
    assert!(matches!(&expr, Expr::FunctionCall(_)));
}

// ========== ANNOTATED ATTRIBUTES ==========

#[test]
fn test_attributes_with_nonempty_comma() {
    let f = single_function(parse_source("fn [a, b] foo() {}"));
    assert!(f.attributes.is_some());
}

// ========== EXPRESSION AS CAST TARGET ==========

#[test]
fn test_cast_of_expression_result() {
    let expr = parse_expr("(1 + 2) as f64");
    assert!(matches!(&expr, Expr::Cast(_)));
}

// ========== LINKED OPERATIONS ==========

#[test]
fn test_assign_plus() {
    let expr = parse_expr("x += 1");
    assert!(matches!(&expr, Expr::BinExpr(b) if matches!(b.operator, BinExprOp::SetPlus)));
}

#[test]
fn test_assign_minus() {
    let expr = parse_expr("x -= 1");
    assert!(matches!(&expr, Expr::BinExpr(b) if matches!(b.operator, BinExprOp::SetMinus)));
}

#[test]
fn test_assign_times() {
    let expr = parse_expr("x *= 2");
    assert!(matches!(&expr, Expr::BinExpr(b) if matches!(b.operator, BinExprOp::SetTimes)));
}

#[test]
fn test_assign_divide() {
    let expr = parse_expr("x /= 2");
    assert!(matches!(&expr, Expr::BinExpr(b) if matches!(b.operator, BinExprOp::SetSlash)));
}

// ========== LOGICAL OPERATORS ==========

#[test]
fn test_logic_and() {
    let expr = parse_expr("true && false");
    assert!(matches!(&expr, Expr::BinExpr(b) if matches!(b.operator, BinExprOp::LogicAnd)));
}

#[test]
fn test_logic_or() {
    let expr = parse_expr("true || false");
    assert!(matches!(&expr, Expr::BinExpr(b) if matches!(b.operator, BinExprOp::LogicOr)));
}

// ========== COMPARISONS ==========

#[test]
fn test_lt() {
    let expr = parse_expr("1 < 2");
    assert!(matches!(&expr, Expr::BinExpr(b) if matches!(b.operator, BinExprOp::LogicLt)));
}

#[test]
fn test_le() {
    let expr = parse_expr("1 <= 2");
    assert!(matches!(&expr, Expr::BinExpr(b) if matches!(b.operator, BinExprOp::LogicLe)));
}

#[test]
fn test_gt() {
    let expr = parse_expr("1 > 2");
    assert!(matches!(&expr, Expr::BinExpr(b) if matches!(b.operator, BinExprOp::LogicGt)));
}

#[test]
fn test_ge() {
    let expr = parse_expr("1 >= 2");
    assert!(matches!(&expr, Expr::BinExpr(b) if matches!(b.operator, BinExprOp::LogicGe)));
}

// ========== COMPARISON EQ ==========

#[test]
fn test_eq() {
    let expr = parse_expr("1 == 2");
    assert!(matches!(&expr, Expr::BinExpr(b) if matches!(b.operator, BinExprOp::LogicEq)));
}

// ========== BIT OPERATORS ==========

#[test]
fn test_shl() {
    let expr = parse_expr("1 << 2");
    assert!(matches!(&expr, Expr::BinExpr(b) if matches!(b.operator, BinExprOp::BitShl)));
}

#[test]
fn test_shr() {
    let expr = parse_expr("1 >> 2");
    assert!(matches!(&expr, Expr::BinExpr(b) if matches!(b.operator, BinExprOp::BitShr)));
}

#[test]
fn test_rol() {
    let expr = parse_expr("1 <<< 2");
    assert!(matches!(&expr, Expr::BinExpr(b) if matches!(b.operator, BinExprOp::BitRol)));
}

#[test]
fn test_ror() {
    let expr = parse_expr("1 >>> 2");
    assert!(matches!(&expr, Expr::BinExpr(b) if matches!(b.operator, BinExprOp::BitRor)));
}

// ========== SET OPERATIONS ==========

#[test]
fn test_set_bit_or() {
    let expr = parse_expr("x |= 1");
    assert!(matches!(&expr, Expr::BinExpr(b) if matches!(b.operator, BinExprOp::SetBitOr)));
}

#[test]
fn test_set_bit_xor() {
    let expr = parse_expr("x ^= 1");
    assert!(matches!(&expr, Expr::BinExpr(b) if matches!(b.operator, BinExprOp::SetBitXor)));
}

// ========== RANGE OPERATOR ==========

#[test]
fn test_range_expression() {
    let expr = parse_expr("x..y");
    assert!(matches!(&expr, Expr::BinExpr(b) if matches!(b.operator, BinExprOp::Range)));
}
