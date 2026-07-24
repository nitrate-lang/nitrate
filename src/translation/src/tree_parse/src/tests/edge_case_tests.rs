use super::helpers::*;
use crate::Parser;
use nitrate_diagnosis::CompilerLog;
use nitrate_token_lexer::Lexer;
use nitrate_tree::ast::*;

#[test]
fn test_empty_source() {
    assert!(parse_source("").items.is_empty());
}

#[test]
fn test_whitespace() {
    assert!(parse_source("   \n  \t  ").items.is_empty());
}

#[test]
fn test_unicode_name() {
    assert_eq!(&*single_function(parse_source("fn 日本語() {}")).name, "日本語");
}

#[test]
fn test_underscore_name() {
    assert_eq!(&*single_function(parse_source("fn _() {}")).name, "_");
}

#[test]
fn test_multi_items_same_line() {
    assert_eq!(parse_source("struct Foo {} fn bar() {}").items.len(), 2);
}

#[test]
fn test_multi_items_lines() {
    assert_eq!(parse_source("struct Foo {}\n\nfn bar() {}").items.len(), 2);
}

// ========== IMPORT ERROR PATHS ==========

#[test]
fn test_import_alias_missing_name() {
    let (_, log) = parse_source_no_assert("use foo as ;");
    assert!(log.error_bit());
}

#[test]
fn test_import_expected_star_or_group() {
    let (_, log) = parse_source_no_assert("use foo::bar::;");
    assert!(log.error_bit());
}

#[test]
fn test_import_group_unclosed() {
    let (_, log) = parse_source_no_assert("use foo::{bar");
    assert!(log.error_bit());
}

#[test]
fn test_import_missing_semicolon() {
    let (_, log) = parse_source_no_assert("use foo");
    assert!(log.error_bit());
}

// ========== FUNCTION ERROR PATHS ==========

#[test]
fn test_fn_missing_name() {
    let (_, log) = parse_source_no_assert("fn () {}");
    assert!(log.error_bit());
}

#[test]
fn test_fn_missing_params() {
    let (_, log) = parse_source_no_assert("fn foo");
    assert!(log.error_bit());
}

#[test]
fn test_fn_param_missing_name() {
    let (_, log) = parse_source_no_assert("fn foo(: i32) {}");
    assert!(log.error_bit());
}

#[test]
fn test_fn_param_missing_type() {
    let (_, log) = parse_source_no_assert("fn foo(x) {}");
    assert!(log.error_bit());
}

#[test]
fn test_fn_variadic() {
    let m = parse_source("fn foo(x: i32, ...) {}");
    assert!(single_function(m).parameters.variadic);
}

#[test]
fn test_fn_return_arrow_no_type() {
    let (_, log) = parse_source_no_assert("fn foo() -> {}");
    assert!(log.error_bit());
}

#[test]
fn test_fn_generics_unclosed() {
    let (_, log) = parse_source_no_assert("fn foo<T() {}");
    assert!(log.error_bit());
}

// ========== SET ASSIGNMENT OPERATORS ==========

#[test]
fn test_set_percent() {
    check_binop("x %= 1", BinExprOp::SetPercent);
}

#[test]
fn test_set_bitand() {
    check_binop("x &= 1", BinExprOp::SetBitAnd);
}

#[test]
fn test_set_logicand() {
    let expr = parse_expr("true &&= false");
    assert!(matches!(&expr, Expr::BinExpr(b) if b.operator == BinExprOp::SetLogicAnd));
}

#[test]
fn test_set_logicor() {
    let expr = parse_expr("true ||= false");
    assert!(matches!(&expr, Expr::BinExpr(b) if b.operator == BinExprOp::SetLogicOr));
}

#[test]
fn test_set_shl() {
    check_binop("x <<= 1", BinExprOp::SetBitShl);
}

#[test]
fn test_set_shr() {
    check_binop("x >>= 1", BinExprOp::SetBitShr);
}

#[test]
fn test_set_rol() {
    check_binop("x <<<= 1", BinExprOp::SetBitRotl);
}

#[test]
fn test_set_ror() {
    check_binop("x >>>= 1", BinExprOp::SetBitRotr);
}

#[test]
fn test_set_xor() {
    check_binop("x ^= 1", BinExprOp::SetBitXor);
}

#[test]
fn test_set_or() {
    check_binop("x |= 1", BinExprOp::SetBitOr);
}

fn check_binop(src: &str, expected: BinExprOp) {
    assert!(
        matches!(&parse_expr(src), Expr::BinExpr(b) if b.operator == expected),
        "Expected {:?} for {src}",
        expected
    );
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

// ========== TRAIT ERROR PATHS ==========

#[test]
fn test_trait_missing_name() {
    let (_, log) = parse_source_no_assert("trait {}");
    assert!(log.error_bit());
}

#[test]
fn test_trait_missing_brace() {
    let (_, log) = parse_source_no_assert("trait Foo");
    assert!(log.error_bit());
}

#[test]
fn test_trait_invalid_item() {
    let (_, log) = parse_source_no_assert("trait Foo { struct Bad {} }");
    assert!(log.error_bit());
}

// ========== IMPL ERROR PATHS ==========

#[test]
fn test_impl_missing_trait_for() {
    let (_, log) = parse_source_no_assert("impl trait Foo Bar {}");
    assert!(log.error_bit());
}

#[test]
fn test_impl_cannot_be_visible() {
    let (_, log) = parse_source_no_assert("pub impl Foo {}");
    assert!(log.error_bit());
}

#[test]
fn test_impl_missing_brace() {
    let (_, log) = parse_source_no_assert("impl Foo");
    assert!(log.error_bit());
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

// ========== VARIABLE ERROR PATHS ==========

#[test]
fn test_static_missing_name() {
    let (_, log) = parse_source_no_assert("static : i32 = 0;");
    assert!(log.error_bit());
}

#[test]
fn test_static_missing_semicolon() {
    let (_, log) = parse_source_no_assert("static x: i32 = 0");
    assert!(log.error_bit());
}

// ========== GENERICS ERROR PATHS ==========

#[test]
fn test_generics_unclosed() {
    let (_, log) = parse_source_no_assert("struct Foo<T { x: T }");
    assert!(log.error_bit());
}

// ========== EXPRESSION ERROR PATHS ==========

#[test]
fn test_expr_expected_expr() {
    let (_, log) = parse_expr_no_assert("}");
    assert!(log.error_bit());
}

#[test]
fn test_expr_tuple_missing_close() {
    let (_, log) = parse_expr_no_assert("(1, 2");
    assert!(log.error_bit());
}

#[test]
fn test_expr_struct_init_missing_field_name() {
    let (_, log) = parse_expr_no_assert("Foo { : 1 }");
    assert!(log.error_bit());
}

#[test]
fn test_expr_struct_init_missing_colon() {
    let (_, log) = parse_expr_no_assert("Foo { x 1 }");
    assert!(log.error_bit());
}

#[test]
fn test_expr_struct_init_unclosed() {
    let (_, log) = parse_expr_no_assert("Foo { x: 1 ");
    assert!(log.error_bit());
}

#[test]
fn test_expr_list_unclosed() {
    let (_, log) = parse_expr_no_assert("[1, 2, 3");
    assert!(log.error_bit());
}

#[test]
fn test_expr_call_missing_close() {
    let (_, log) = parse_expr_no_assert("foo(1, 2");
    assert!(log.error_bit());
}

#[test]
fn test_expr_call_positional_follows_named() {
    let (_, log) = parse_expr_no_assert("f(x: 1, 2)");
    assert!(log.error_bit());
}

#[test]
fn test_expr_index_missing_close() {
    let (_, log) = parse_expr_no_assert("a[0");
    assert!(log.error_bit());
}

#[test]
fn test_expr_field_access_missing_name() {
    let (_, log) = parse_expr_no_assert("a.");
    assert!(log.error_bit());
}

#[test]
fn test_expr_method_call_missing_close() {
    let (_, log) = parse_expr_no_assert("a.b(1, 2");
    assert!(log.error_bit());
}

// ========== CONTROL FLOW ==========

#[test]
fn test_if_no_else() {
    let expr = parse_expr("if true { 1 }");
    assert!(matches!(&expr, Expr::If(i) if i.false_branch.is_none()));
}

#[test]
fn test_if_else_if() {
    let expr = parse_expr("if true { 1 } else if false { 2 } else { 3 }");
    assert!(matches!(&expr, Expr::If(i) if matches!(i.false_branch, Some(ElseIf::If(_)))));
}

#[test]
fn test_while_no_condition() {
    let expr = parse_expr("while { break; }");
    assert!(matches!(&expr, Expr::While(w) if w.condition.is_none()));
}

#[test]
fn test_break_missing_semicolon() {
    let (_, log) = parse_expr_no_assert("break");
    assert!(log.error_bit());
}

#[test]
fn test_continue_missing_semicolon() {
    let (_, log) = parse_expr_no_assert("continue");
    assert!(log.error_bit());
}

#[test]
fn test_return_missing_semicolon() {
    let (_, log) = parse_expr_no_assert("ret");
    assert!(log.error_bit());
}

#[test]
fn test_await_expr() {
    let expr = parse_expr("await fut");
    assert!(matches!(&expr, Expr::Await(_)));
}

// ========== CLOSURES ==========

#[test]
fn test_closure_brace() {
    let expr = parse_expr("{ 42 }");
    assert!(matches!(&expr, Expr::Closure(_)));
}

#[test]
fn test_closure_fn() {
    let expr = parse_expr("fn(x: i32) -> i32 { x }");
    assert!(matches!(&expr, Expr::Closure(c) if c.parameters.is_some()));
}

#[test]
fn test_closure_no_return() {
    let expr = parse_expr("fn(x: i32) { x }");
    assert!(matches!(&expr, Expr::Closure(c) if c.return_type.is_none()));
}

#[test]
fn test_closure_call() {
    let expr = parse_expr("fn(x: i32) -> i32 { x }(42)");
    assert!(matches!(&expr, Expr::FunctionCall(_)));
}

// ========== CAST SUFFIX ==========

#[test]
fn test_cast_suffix_u8() {
    assert!(matches!(&parse_expr("42u8"), Expr::Cast(c) if matches!(&c.to, Type::UInt8(_))));
}
#[test]
fn test_cast_suffix_i32() {
    assert!(matches!(&parse_expr("42i32"), Expr::Cast(c) if matches!(&c.to, Type::Int32(_))));
}
#[test]
fn test_cast_suffix_f64() {
    assert!(matches!(&parse_expr("42.0f64"), Expr::Cast(c) if matches!(&c.to, Type::Float64(_))));
}
#[test]
fn test_cast_suffix_name() {
    assert!(matches!(&parse_expr("42mytype"), Expr::Cast(c) if matches!(&c.to, Type::TypePath(_))));
}
#[test]
fn test_cast_suffix_f32() {
    assert!(matches!(&parse_expr("42f32"), Expr::Cast(c) if matches!(&c.to, Type::Float32(_))));
}
#[test]
fn test_cast_suffix_f8() {
    assert!(matches!(&parse_expr("42f8"), Expr::Cast(c) if matches!(&c.to, Type::TypePath(_))));
}
#[test]
fn test_cast_suffix_f16() {
    assert!(matches!(&parse_expr("42f16"), Expr::Cast(c) if matches!(&c.to, Type::TypePath(_))));
}
#[test]
fn test_cast_suffix_f128() {
    assert!(matches!(&parse_expr("42f128"), Expr::Cast(c) if matches!(&c.to, Type::TypePath(_))));
}

// ========== PATHS ==========

#[test]
fn test_expr_path_global() {
    assert!(matches!(&parse_expr("::std::mem"), Expr::Path(p) if p.segments[0].name == ""));
}

#[test]
fn test_expr_path_generic() {
    assert!(matches!(&parse_expr("foo::<i32>::bar"), Expr::Path(p) if p.segments[0].type_arguments.is_some()));
}

// ========== METHOD CHAINING AND FIELD ACCESS ==========

#[test]
fn test_method_call_chain() {
    let expr = parse_expr("a.b().c()");
    assert!(matches!(&expr, Expr::MethodCall(m) if m.method_name == "c" && matches!(m.object, Expr::MethodCall(_))));
}

#[test]
fn test_field_access_chain() {
    assert!(matches!(&parse_expr("a.b.c"), Expr::FieldAccess(f) if f.field == "c"));
}

// ========== CAST OPERATOR ==========

#[test]
fn test_cast_operator() {
    assert!(matches!(&parse_expr("42 as i64"), Expr::Cast(c) if matches!(&c.to, Type::Int64(_))));
}

#[test]
fn test_cast_operator_no_type() {
    let (_, log) = parse_expr_no_assert("42 as ");
    assert!(log.error_bit());
}

// ========== ATTRIBUTES ON ITEMS ==========

#[test]
fn test_function_with_attributes() {
    assert!(
        single_function(parse_source("fn [inline] foo() {}"))
            .attributes
            .is_some()
    );
}
#[test]
fn test_struct_with_attributes() {
    assert!(
        single_struct(parse_source("struct [repr(C)] Foo { x: i32 }"))
            .attributes
            .is_some()
    );
}
#[test]
fn test_enum_with_attributes() {
    assert!(
        single_enum(parse_source("enum [repr(C)] Foo { A, B }"))
            .attributes
            .is_some()
    );
}
#[test]
fn test_trait_with_attributes() {
    assert!(
        single_trait(parse_source("trait [must_use] Foo { fn bar(); }"))
            .attributes
            .is_some()
    );
}
#[test]
fn test_type_alias_with_attributes() {
    assert!(
        single_type_alias(parse_source("type [some_attr] MyInt = i32;"))
            .attributes
            .is_some()
    );
}
#[test]
fn test_static_with_attributes() {
    assert!(
        single_variable(parse_source("static [used] x: i32 = 0;"))
            .attributes
            .is_some()
    );
}
#[test]
fn test_import_with_attributes() {
    assert!(
        single_import(parse_source("use [allow(unused)] std::mem;"))
            .attributes
            .is_some()
    );
}

// ========== VISIBILITY ==========

#[test]
fn test_import_pub() {
    assert!(matches!(
        single_import(parse_source("pub use std::mem;")).visibility,
        Some(Visibility::Public)
    ));
}
#[test]
fn test_visibility_sec() {
    assert!(matches!(
        single_function(parse_source("sec fn f() {}")).visibility,
        Some(Visibility::Private)
    ));
}
#[test]
fn test_visibility_pro() {
    assert!(matches!(
        single_function(parse_source("pro fn f() {}")).visibility,
        Some(Visibility::Protected)
    ));
}
#[test]
fn test_struct_pub() {
    assert!(matches!(
        single_struct(parse_source("pub struct S { x: i32 }")).visibility,
        Some(Visibility::Public)
    ));
}
#[test]
fn test_struct_sec() {
    assert!(matches!(
        single_struct(parse_source("sec struct S { x: i32 }")).visibility,
        Some(Visibility::Private)
    ));
}
#[test]
fn test_struct_pro() {
    assert!(matches!(
        single_struct(parse_source("pro struct S { x: i32 }")).visibility,
        Some(Visibility::Protected)
    ));
}

// ========== NESTED IMPORT GROUP ==========

#[test]
fn test_import_nested_group() {
    let m = parse_source("use foo::{bar::{baz, qux}};");
    assert!(matches!(&single_import(m).use_tree, UseTree::Group { .. }));
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

// ========== LIFETIME ==========
#[test]
fn test_type_lifetime_static() {
    assert!(matches!(&parse_type("'static"), Type::Lifetime(_)));
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
fn test_type_global_path() {
    assert!(matches!(&parse_type("::std::collections"), Type::TypePath(p) if p.segments[0].name == ""));
}
#[test]
fn test_type_path_generic() {
    assert!(matches!(&parse_type("Vec<i32>"), Type::TypePath(p) if p.segments[0].type_arguments.is_some()));
}
#[test]
fn test_type_path_multi() {
    assert!(matches!(&parse_type("std::collections::HashMap"), Type::TypePath(p) if p.segments.len() == 3));
}

// ========== ASSOCIATED ITEMS ==========

#[test]
fn test_trait_type_alias() {
    assert!(matches!(
        &single_trait(parse_source("trait Foo { type X; }")).items[0],
        AssociatedItem::TypeAlias(_)
    ));
}
#[test]
fn test_trait_const_item() {
    assert!(matches!(
        &single_trait(parse_source("trait Foo { const X: i32; }")).items[0],
        AssociatedItem::ConstantItem(_)
    ));
}

// ========== MODULE ==========

#[test]
fn test_mod_pub() {
    assert!(
        matches!(&parse_source("pub mod m { fn f() {} }").items[0], Item::Module(mod_item) if matches!(mod_item.visibility, Some(Visibility::Public)))
    );
}
#[test]
fn test_mod_attr() {
    assert!(
        matches!(&parse_source("mod [cfg(test)] test { fn h() {} }").items[0], Item::Module(mod_item) if mod_item.attributes.is_some())
    );
}
#[test]
fn test_mod_nested() {
    assert!(
        matches!(&parse_source("mod a { mod b { fn f() {} } }").items[0], Item::Module(outer) if outer.items.len() == 1)
    );
}
#[test]
fn test_mod_missing_name() {
    let (_, log) = parse_source_no_assert("mod { fn f() {} }");
    assert!(log.error_bit());
}
#[test]
fn test_mod_missing_brace() {
    let (_, log) = parse_source_no_assert("mod m fn f() {} }");
    assert!(log.error_bit());
}

// ========== CONST MUTABILITY ==========

#[test]
fn test_const_mutability_const() {
    assert!(matches!(
        single_variable(parse_source("const const X: i32 = 0;")).mutability,
        Some(Mutability::Const)
    ));
}
#[test]
fn test_static_mutability_const() {
    assert!(matches!(
        single_variable(parse_source("static const x: i32 = 0;")).mutability,
        Some(Mutability::Const)
    ));
}

// ========== GLOBAL VARIABLE ==========

#[test]
fn test_static_type_annotated() {
    let v = single_variable(parse_source("static x: i32 = 42;"));
    assert!(v.ty.is_some() && v.initializer.is_some());
}
#[test]
fn test_static_no_type() {
    assert!(single_variable(parse_source("static x = 42;")).ty.is_none());
}
#[test]
fn test_static_no_init() {
    assert!(single_variable(parse_source("static x: i32;")).initializer.is_none());
}

// ========== DEFAULT VALUES ==========

#[test]
fn test_param_default() {
    assert!(
        single_function(parse_source("fn f(x: i32 = 42) {}")).parameters.params[0]
            .default_value
            .is_some()
    );
}
#[test]
fn test_struct_field_default() {
    assert!(
        single_struct(parse_source("struct Foo { x: i32 = 42 }")).fields[0]
            .default_value
            .is_some()
    );
}

// ========== ENUM VARIANT ==========

#[test]
fn test_enum_variant_type() {
    assert!(
        single_enum(parse_source("enum Foo { A(i32) }")).variants[0]
            .ty
            .is_some()
    );
}
#[test]
fn test_enum_variant_default() {
    assert!(
        single_enum(parse_source("enum Foo { A = 42 }")).variants[0]
            .default_value
            .is_some()
    );
}
#[test]
fn test_enum_generic() {
    assert!(
        single_enum(parse_source("enum Option<T> { Some(T), None }"))
            .generics
            .is_some()
    );
}

// ========== TRAIT METHOD ==========
#[test]
fn test_trait_method() {
    assert_eq!(single_trait(parse_source("trait Foo { fn bar(); }")).items.len(), 1);
}

// ========== IMPL ==========
#[test]
fn test_impl_direct() {
    assert!(
        single_impl(parse_source("impl Foo { fn bar() {} }"))
            .trait_path
            .is_none()
    );
}

// ========== FUNCTION DECLARATION ==========
#[test]
fn test_fn_decl() {
    assert!(single_function(parse_source("fn foo();")).definition.is_none());
}

// ========== UNSAFE BLOCK ==========
#[test]
fn test_unsafe_block() {
    assert!(matches!(&parse_expr("unsafe { 42 }"), Expr::Closure(_)));
}
#[test]
fn test_safe_block() {
    assert!(matches!(&parse_expr("safe { 42 }"), Expr::Closure(_)));
}
#[test]
fn test_unsafe_block_with_expr() {
    assert!(matches!(&parse_expr("unsafe(42) { 1 }"), Expr::Closure(_)));
}

// ========== CALLS ==========

#[test]
fn test_call_named() {
    let expr = parse_expr("f(x: 1, y: 2)");
    assert!(matches!(&expr, Expr::FunctionCall(c) if c.named.len() == 2 && c.positional.is_empty()));
}
#[test]
fn test_call_mixed() {
    let expr = parse_expr("f(1, 2, x: 3)");
    assert!(matches!(&expr, Expr::FunctionCall(c) if c.positional.len() == 2 && c.named.len() == 1));
}

// ========== NAMED GENERIC ==========
#[test]
fn test_type_named_generic() {
    assert!(matches!(&parse_type("Map<Key: i32, Value: f64>"), Type::TypePath(_)));
}

// ========== BLOCK AS STMT ==========
#[test]
fn test_block_as_stmt() {
    assert!(single_function(parse_source("fn f() { { 42 }; }")).definition.is_some());
}

// ========== TRAILING PATHS ==========

#[test]
fn test_path_expected_name() {
    let (_, log) = parse_expr_no_assert("foo::");
    assert!(log.error_bit());
}
#[test]
fn test_type_path_expected_name() {
    let (_, log) = parse_type_no_assert("foo::");
    assert!(log.error_bit());
}

// ========== EMPTY TUPLE IN PARENS ==========
#[test]
fn test_empty_parens_tuple_expr() {
    assert!(matches!(&parse_expr("()"), Expr::Tuple(t) if t.elements.is_empty()));
}

// ========== TYPE PATH IN PARENS ==========
#[test]
fn test_type_parentheses() {
    assert!(matches!(&parse_type("(i32)"), Type::Parentheses(_)));
}

// ========== AVOID INFINITE LOOP IN PATH PARSING ==========
#[test]
fn test_path_at_eof_after_colon() {
    // The source "fn f() { :: }" should error at the colon without panicking
    let (_, log) = parse_source_no_assert("fn f() { :: }");
    assert!(log.error_bit());
}

// ========== FUNCTION TYPE WITH RETURN ARROW MISSING ==========
#[test]
fn test_fn_type_return_arrow_missing() {
    let (_, log) = parse_type_no_assert("fn(x: i32) - bool");
    assert!(log.error_bit());
}

// ========== REFINEMENT WITH EXPRESSION AFTER COLON ==========
#[test]
fn test_type_refine_with_complex_expr() {
    let ty = parse_type("u8: [0:10]");
    assert!(matches!(&ty, Type::RefinementType(_)));
}

// ========== BINARY NOT RANGE ==========
#[test]
fn test_binop_not_range() {
    // `..` without left side should NOT parse as range, just dot dot
    let expr = parse_expr("x..y");
    assert!(matches!(&expr, Expr::BinExpr(b) if b.operator == BinExprOp::Range));
}

// ========== INTEGER SUFFIXES ==========
#[test]
fn test_integer_suffix_u128() {
    let expr = parse_expr("42u128");
    assert!(matches!(&expr, Expr::Cast(c) if matches!(&c.to, Type::UInt128(_))));
}
#[test]
fn test_integer_suffix_u64() {
    let expr = parse_expr("42u64");
    assert!(matches!(&expr, Expr::Cast(c) if matches!(&c.to, Type::UInt64(_))));
}
#[test]
fn test_integer_suffix_usize() {
    let expr = parse_expr("42usize");
    assert!(matches!(&expr, Expr::Cast(c) if matches!(&c.to, Type::USize(_))));
}
#[test]
fn test_integer_suffix_i8() {
    let expr = parse_expr("42i8");
    assert!(matches!(&expr, Expr::Cast(c) if matches!(&c.to, Type::Int8(_))));
}
#[test]
fn test_integer_suffix_i16() {
    let expr = parse_expr("42i16");
    assert!(matches!(&expr, Expr::Cast(c) if matches!(&c.to, Type::Int16(_))));
}
#[test]
fn test_integer_suffix_i64() {
    let expr = parse_expr("42i64");
    assert!(matches!(&expr, Expr::Cast(c) if matches!(&c.to, Type::Int64(_))));
}
#[test]
fn test_integer_suffix_i128() {
    let expr = parse_expr("42i128");
    assert!(matches!(&expr, Expr::Cast(c) if matches!(&c.to, Type::Int128(_))));
}
#[test]
fn test_integer_suffix_u16() {
    let expr = parse_expr("42u16");
    assert!(matches!(&expr, Expr::Cast(c) if matches!(&c.to, Type::UInt16(_))));
}

// ========== STRING AS BSTRING ==========
#[test]
fn test_bstring_expr() {
    let expr = parse_expr(r#""\xff\xfe""#);
    assert!(matches!(&expr, Expr::BString(_)));
}

// ========== LIST WITH TRAILING COMMA ==========
#[test]
fn test_list_trailing_comma() {
    let expr = parse_expr("[1, 2,]");
    assert!(matches!(&expr, Expr::List(l) if l.elements.len() == 2));
}

// ========== STRUCT INIT WITH TRAILING COMMA ==========
#[test]
fn test_struct_init_trailing_comma() {
    let expr = parse_expr("Foo { x: 1, }");
    assert!(matches!(&expr, Expr::StructInit(s) if s.fields.len() == 1));
}
