// Comprehensive parser tests
use super::helpers::*;
use nitrate_tree::ast::*;

// ============================
// STRUCT TESTS
// ============================
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

// ============================
// ENUM TESTS
// ============================
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

// ============================
// FUNCTION TESTS
// ============================
#[test]
fn test_fn_empty() {
    let f = single_function(parse_source("fn foo() {}"));
    assert_eq!(&*f.name, "foo");
    assert!(f.definition.is_some());
}

#[test]
fn test_fn_params() {
    let f = single_function(parse_source("fn add(x: i32, y: i32) {}"));
    assert_eq!(f.parameters.params.len(), 2);
}

#[test]
fn test_fn_return() {
    let f = single_function(parse_source("fn add(x: i32, y: i32) -> i32 { 42 }"));
    assert!(f.return_type.is_some());
}

#[test]
fn test_fn_decl() {
    assert!(single_function(parse_source("fn foo();")).definition.is_none());
}

#[test]
fn test_fn_pub() {
    assert!(matches!(
        single_function(parse_source("pub fn f() {}")).visibility,
        Some(Visibility::Public)
    ));
}
#[test]
fn test_fn_sec() {
    assert!(matches!(
        single_function(parse_source("sec fn f() {}")).visibility,
        Some(Visibility::Private)
    ));
}
#[test]
fn test_fn_pro() {
    assert!(matches!(
        single_function(parse_source("pro fn f() {}")).visibility,
        Some(Visibility::Protected)
    ));
}

#[test]
fn test_fn_variadic() {
    assert!(
        single_function(parse_source("fn f(x: i32, ...) {}"))
            .parameters
            .variadic
    );
}
#[test]
fn test_fn_mut_param() {
    assert!(matches!(
        single_function(parse_source("fn f(mut x: i32) {}")).parameters.params[0].mutability,
        Some(Mutability::Mut)
    ));
}
#[test]
fn test_fn_const_param() {
    assert!(matches!(
        single_function(parse_source("fn f(const x: i32) {}")).parameters.params[0].mutability,
        Some(Mutability::Const)
    ));
}
#[test]
fn test_fn_attr() {
    assert!(
        single_function(parse_source("fn [inline] foo() {}"))
            .attributes
            .is_some()
    );
}
#[test]
fn test_fn_default() {
    assert!(
        single_function(parse_source("fn f(x: i32 = 42) {}")).parameters.params[0]
            .default_value
            .is_some()
    );
}

// ============================
// TYPE TESTS
// ============================
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

// ============================
// LITERAL TESTS
// ============================
#[test]
fn test_int_dec() {
    assert!(matches!(&parse_expr("42"), Expr::Integer(i) if i.value == 42));
}
#[test]
fn test_int_hex() {
    assert!(matches!(&parse_expr("0xFF"), Expr::Integer(i) if i.value == 255));
}
#[test]
fn test_int_bin() {
    assert!(matches!(&parse_expr("0b1010"), Expr::Integer(i) if i.value == 10));
}
#[test]
fn test_int_oct() {
    assert!(matches!(&parse_expr("0o77"), Expr::Integer(i) if i.value == 63));
}
#[test]
fn test_int_underscore() {
    assert!(matches!(&parse_expr("1_000"), Expr::Integer(i) if i.value == 1000));
}
#[test]
fn test_int_zero() {
    assert!(matches!(&parse_expr("0"), Expr::Integer(i) if i.value == 0));
}
#[test]
fn test_float() {
    assert!(matches!(&parse_expr("3.14"), Expr::Float(_)));
}
#[test]
fn test_float_zero() {
    assert!(matches!(&parse_expr("0.5"), Expr::Float(_)));
}
#[test]
fn test_str() {
    assert!(matches!(&parse_expr("\"hello\""), Expr::String(s) if s.value == "hello"));
}
#[test]
fn test_str_empty() {
    assert!(matches!(&parse_expr("\"\""), Expr::String(_)));
}
#[test]
fn test_true() {
    assert!(matches!(&parse_expr("true"), Expr::Boolean(b) if b.value));
}
#[test]
fn test_false() {
    assert!(matches!(&parse_expr("false"), Expr::Boolean(b) if !b.value));
}

#[test]
fn test_suffix_u8() {
    assert!(matches!(&parse_expr("42u8"), Expr::Cast(c) if matches!(&c.to, Type::UInt8(_))));
}
#[test]
fn test_suffix_i32() {
    assert!(matches!(&parse_expr("42i32"), Expr::Cast(c) if matches!(&c.to, Type::Int32(_))));
}
#[test]
fn test_suffix_f64() {
    assert!(matches!(&parse_expr("42.0f64"), Expr::Cast(c) if matches!(&c.to, Type::Float64(_))));
}

// ============================
// BINARY OPERATOR TESTS
// ============================
fn check_binop(src: &str, expected: BinExprOp) {
    assert!(
        matches!(&parse_expr(src), Expr::BinExpr(b) if b.operator == expected),
        "Expected {:?} for {src}",
        expected
    );
}

#[test]
fn test_binop_add() {
    check_binop("1 + 2", BinExprOp::Add);
}
#[test]
fn test_binop_sub() {
    check_binop("1 - 2", BinExprOp::Sub);
}
#[test]
fn test_binop_mul() {
    check_binop("1 * 2", BinExprOp::Mul);
}
#[test]
fn test_binop_div() {
    check_binop("1 / 2", BinExprOp::Div);
}
#[test]
fn test_binop_mod() {
    check_binop("1 % 2", BinExprOp::Mod);
}
#[test]
fn test_binop_bitand() {
    check_binop("1 & 2", BinExprOp::BitAnd);
}
#[test]
fn test_binop_bitor() {
    check_binop("1 | 2", BinExprOp::BitOr);
}
#[test]
fn test_binop_bitxor() {
    check_binop("1 ^ 2", BinExprOp::BitXor);
}
#[test]
fn test_binop_shl() {
    check_binop("1 << 2", BinExprOp::BitShl);
}
#[test]
fn test_binop_shr() {
    check_binop("1 >> 2", BinExprOp::BitShr);
}
#[test]
fn test_binop_rol() {
    check_binop("1 <<< 2", BinExprOp::BitRol);
}
#[test]
fn test_binop_ror() {
    check_binop("1 >>> 2", BinExprOp::BitRor);
}
#[test]
fn test_binop_and() {
    check_binop("true && false", BinExprOp::LogicAnd);
}
#[test]
fn test_binop_or() {
    check_binop("true || false", BinExprOp::LogicOr);
}
#[test]
fn test_binop_eq() {
    check_binop("1 == 2", BinExprOp::LogicEq);
}
#[test]
fn test_binop_lt() {
    check_binop("1 < 2", BinExprOp::LogicLt);
}
#[test]
fn test_binop_gt() {
    check_binop("1 > 2", BinExprOp::LogicGt);
}
#[test]
fn test_binop_le() {
    check_binop("1 <= 2", BinExprOp::LogicLe);
}
#[test]
fn test_binop_ge() {
    check_binop("1 >= 2", BinExprOp::LogicGe);
}
#[test]
fn test_binop_assign() {
    check_binop("x = 42", BinExprOp::Set);
}
#[test]
fn test_binop_add_eq() {
    check_binop("x += 1", BinExprOp::SetPlus);
}
#[test]
fn test_binop_sub_eq() {
    check_binop("x -= 1", BinExprOp::SetMinus);
}
#[test]
fn test_binop_mul_eq() {
    check_binop("x *= 2", BinExprOp::SetTimes);
}
#[test]
fn test_binop_div_eq() {
    check_binop("x /= 2", BinExprOp::SetSlash);
}
#[test]
fn test_binop_range() {
    check_binop("0..10", BinExprOp::Range);
}

// ============================
// UNARY TESTS
// ============================
#[test]
fn test_unary_neg() {
    assert!(matches!(&parse_expr("-42"), Expr::UnaryExpr(u) if u.operator == UnaryExprOp::Sub));
}
#[test]
fn test_unary_not() {
    assert!(matches!(&parse_expr("!true"), Expr::UnaryExpr(u) if u.operator == UnaryExprOp::Not));
}
#[test]
fn test_unary_borrow() {
    assert!(matches!(&parse_expr("&x"), Expr::UnaryExpr(u) if u.operator == UnaryExprOp::Borrow));
}
#[test]
fn test_unary_deref() {
    assert!(matches!(&parse_expr("*ptr"), Expr::UnaryExpr(u) if u.operator == UnaryExprOp::Deref));
}
#[test]
fn test_unary_plus() {
    assert!(matches!(&parse_expr("+42"), Expr::UnaryExpr(u) if u.operator == UnaryExprOp::Add));
}
#[test]
fn test_unary_typeof() {
    assert!(matches!(&parse_expr("typeof x"), Expr::UnaryExpr(u) if u.operator == UnaryExprOp::Typeof));
}

// ============================
// EXPRESSION TESTS
// ============================
#[test]
fn test_expr_path() {
    assert!(matches!(&parse_expr("foo"), Expr::Path(p) if p.segments[0].name == "foo"));
}

#[test]
fn test_expr_call_no_args() {
    assert!(matches!(&parse_expr("foo()"), Expr::FunctionCall(c) if c.positional.is_empty()));
}

#[test]
fn test_expr_call_positional() {
    assert!(matches!(&parse_expr("f(1, 2, 3)"), Expr::FunctionCall(c) if c.positional.len() == 3));
}

#[test]
fn test_expr_call_named() {
    assert!(matches!(&parse_expr("f(x: 1, y: 2)"), Expr::FunctionCall(c) if c.named.len() == 2));
}

#[test]
fn test_expr_method_call() {
    assert!(matches!(&parse_expr("a.b()"), Expr::MethodCall(c) if c.method_name == "b"));
}

#[test]
fn test_expr_field() {
    assert!(matches!(&parse_expr("a.b"), Expr::FieldAccess(f) if f.field == "b"));
}

#[test]
fn test_expr_index() {
    assert!(matches!(&parse_expr("a[0]"), Expr::IndexAccess(_)));
}

#[test]
fn test_expr_cast() {
    assert!(matches!(&parse_expr("42 as i64"), Expr::Cast(c) if matches!(&c.to, Type::Int64(_))));
}

#[test]
fn test_expr_parens() {
    assert!(matches!(&parse_expr("(42)"), Expr::Parentheses(_)));
}

#[test]
fn test_expr_tuple2() {
    assert!(matches!(&parse_expr("(1, 2)"), Expr::Tuple(t) if t.elements.len() == 2));
}

#[test]
fn test_expr_tuple_empty() {
    assert!(matches!(&parse_expr("()"), Expr::Tuple(t) if t.elements.is_empty()));
}

#[test]
fn test_expr_struct_init() {
    assert!(matches!(&parse_expr("Foo { x: 1 }"), Expr::StructInit(s) if s.fields.len() == 1));
}

#[test]
fn test_expr_if() {
    assert!(matches!(&parse_expr("if true { 1 } else { 2 }"), Expr::If(_)));
}

#[test]
fn test_expr_if_no_else() {
    assert!(matches!(&parse_expr("if true { 1 }"), Expr::If(i) if i.false_branch.is_none()));
}

#[test]
fn test_expr_while() {
    assert!(matches!(&parse_expr("while true { break; }"), Expr::While(_)));
}

#[test]
fn test_expr_break() {
    assert!(matches!(&parse_expr("break;"), Expr::Break(_)));
}
#[test]
fn test_expr_break_label() {
    assert!(matches!(&parse_expr("break 'l;"), Expr::Break(b) if matches!(&b.label, Some(l) if &**l == "l")));
}
#[test]
fn test_expr_continue() {
    assert!(matches!(&parse_expr("continue;"), Expr::Continue(_)));
}
#[test]
fn test_expr_return() {
    assert!(matches!(&parse_expr("ret;"), Expr::Return(r) if r.value.is_none()));
}
#[test]
fn test_expr_return_val() {
    assert!(matches!(&parse_expr("ret 42;"), Expr::Return(r) if r.value.is_some()));
}
#[test]
fn test_expr_type_info() {
    assert!(matches!(&parse_expr("type i32"), Expr::TypeInfo(t) if matches!(&t.the, Type::Int32(_))));
}
#[test]
fn test_expr_await() {
    assert!(matches!(&parse_expr("await fut"), Expr::Await(_)));
}

#[test]
fn test_expr_list_empty() {
    assert!(matches!(&parse_expr("[]"), Expr::List(l) if l.elements.is_empty()));
}
#[test]
fn test_expr_list() {
    assert!(matches!(&parse_expr("[1, 2, 3]"), Expr::List(l) if l.elements.len() == 3));
}

// ============================
// GENERICS TESTS
// ============================
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

// ============================
// TRAIT TESTS
// ============================
#[test]
fn test_trait_empty() {
    assert!(single_trait(parse_source("trait F {}")).items.is_empty());
}
#[test]
fn test_trait_method() {
    assert_eq!(single_trait(parse_source("trait F { fn foo(); }")).items.len(), 1);
}
#[test]
fn test_trait_const() {
    assert!(matches!(
        &single_trait(parse_source("trait F { const X: i32; }")).items[0],
        AssociatedItem::ConstantItem(_)
    ));
}
#[test]
fn test_trait_type() {
    assert!(matches!(
        &single_trait(parse_source("trait F { type X; }")).items[0],
        AssociatedItem::TypeAlias(_)
    ));
}
#[test]
fn test_trait_pub() {
    assert!(matches!(
        single_trait(parse_source("pub trait F {}")).visibility,
        Some(Visibility::Public)
    ));
}
#[test]
fn test_trait_attr() {
    assert!(
        single_trait(parse_source("trait [must_use] F { fn foo(); }"))
            .attributes
            .is_some()
    );
}

// ============================
// IMPL TESTS
// ============================
#[test]
fn test_impl_trait_for() {
    assert!(
        single_impl(parse_source("impl trait Foo for Bar { fn m() {} }"))
            .trait_path
            .is_some()
    );
}

// ============================
// IMPORT TESTS
// ============================
#[test]
fn test_import_simple() {
    assert!(matches!(
        &single_import(parse_source("use std::mem;")).use_tree,
        UseTree::Single { .. }
    ));
}
#[test]
fn test_import_glob() {
    assert!(matches!(
        &single_import(parse_source("use std::*;")).use_tree,
        UseTree::UseAll { .. }
    ));
}
#[test]
fn test_import_group() {
    assert!(matches!(
        &single_import(parse_source("use std::{a, b};")).use_tree,
        UseTree::Group { .. }
    ));
}
#[test]
fn test_import_alias() {
    assert!(matches!(
        &single_import(parse_source("use std::mem as m;")).use_tree,
        UseTree::Alias { .. }
    ));
}
#[test]
fn test_import_pub() {
    assert!(matches!(
        single_import(parse_source("pub use std::mem;")).visibility,
        Some(Visibility::Public)
    ));
}
#[test]
fn test_import_attr() {
    assert!(
        single_import(parse_source("use [allow(unused)] std::mem;"))
            .attributes
            .is_some()
    );
}

// ============================
// VARIABLE TESTS
// ============================
#[test]
fn test_static_var() {
    let v = single_variable(parse_source("static x: i32 = 42;"));
    assert_eq!(&*v.name, "x");
    assert!(v.ty.is_some());
    assert!(v.initializer.is_some());
}
#[test]
fn test_const_var() {
    let v = single_variable(parse_source("const X: i32 = 100;"));
    assert_eq!(&*v.name, "X");
}
#[test]
fn test_static_mut_var() {
    assert!(matches!(
        single_variable(parse_source("static mut x: i32 = 0;")).mutability,
        Some(Mutability::Mut)
    ));
}
#[test]
fn test_static_no_type() {
    assert!(single_variable(parse_source("static x = 42;")).ty.is_none());
}
#[test]
fn test_static_no_init() {
    assert!(single_variable(parse_source("static x: i32;")).initializer.is_none());
}
#[test]
fn test_var_pub() {
    assert!(matches!(
        single_variable(parse_source("pub static x: i32 = 0;")).visibility,
        Some(Visibility::Public)
    ));
}
#[test]
fn test_var_attr() {
    assert!(
        single_variable(parse_source("static [used] x: i32 = 0;"))
            .attributes
            .is_some()
    );
}

// ============================
// MODULE TESTS
// ============================
#[test]
fn test_mod() {
    assert!(matches!(&parse_source("mod m { fn f() {} }").items[0], Item::Module(m) if m.items.len() == 1));
}
#[test]
fn test_mod_pub() {
    assert!(
        matches!(&parse_source("pub mod m { fn f() {} }").items[0], Item::Module(m) if matches!(m.visibility, Some(Visibility::Public)))
    );
}
#[test]
fn test_mod_attr() {
    assert!(
        matches!(&parse_source("mod [cfg(test)] test { fn h() {} }").items[0], Item::Module(m) if m.attributes.is_some())
    );
}
#[test]
fn test_mod_nested() {
    assert!(
        matches!(&parse_source("mod a { mod b { fn f() {} } }").items[0], Item::Module(outer) if outer.items.len() == 1)
    );
}

// ============================
// EDGE CASES
// ============================
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

// ============================
// INTEGRATION
// ============================
#[test]
fn test_program_simple() {
    let m = parse_source(
        "struct Point { x: i32, y: i32 }
        fn main() { let p = Point { x: 0, y: 0 }; }",
    );
    assert_eq!(m.items.len(), 2);
}

#[test]
fn test_program_with_imports() {
    let m = parse_source(
        "use std::mem;
        use std::io::{self, Write};
        fn main() {}",
    );
    assert_eq!(m.items.len(), 3);
}

#[test]
fn test_program_visibility() {
    let m = parse_source(
        "pub fn f1() {} sec fn f2() {} pro fn f3() {}
        pub struct S { pub x: i32, sec y: i32, pro z: i32 }",
    );
    assert_eq!(m.items.len(), 4);
}

#[test]
fn test_program_consts() {
    let m = parse_source("const PI: f64 = 3.14; const E: f64 = 2.71; static APP: str = \"My\";");
    assert_eq!(m.items.len(), 3);
}

#[test]
fn test_program_enums() {
    let m = parse_source(
        "enum Option<T> { Some(T), None }
        enum Result<T, E> { Ok(T), Err(E) }",
    );
    assert_eq!(m.items.len(), 2);
}

#[test]
fn test_program_aliases() {
    let m = parse_source("type Int = i32; type Float = f64; type Pair = (Int, Float);");
    assert_eq!(m.items.len(), 3);
}

#[test]
fn test_program_variadic() {
    let m = parse_source(
        "fn print(fmt: str, ...) {}
        fn main() { print(\"hello\"); }",
    );
    assert_eq!(m.items.len(), 2);
}

#[test]
fn test_program_with_refinements() {
    let m = parse_source(
        "struct Temperature { celsius: i8: [-128:127] }
        fn check(t: Temperature) -> bool { true }",
    );
    assert_eq!(m.items.len(), 2);
}

#[test]
fn test_program_attributes() {
    let m = parse_source(
        "struct [derive(Debug)] [repr(C)] Vec2 { x: f64, y: f64 }
        fn [inline] dot(a: Vec2, b: Vec2) -> f64 { 42.0 }",
    );
    assert_eq!(m.items.len(), 2);
}

#[test]
fn test_program_modules() {
    let m = parse_source(
        "mod math { fn add(x: i32, y: i32) -> i32 { 42 } }
        mod io { fn print(s: String) {} }
        fn main() { math::add(1, 2); }",
    );
    assert_eq!(m.items.len(), 3);
}
