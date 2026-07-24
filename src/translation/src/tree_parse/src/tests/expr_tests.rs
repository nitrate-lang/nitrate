use super::helpers::*;
use nitrate_tree::ast::*;

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
fn test_expr_call_mixed() {
    let expr = parse_expr("f(1, 2, x: 3)");
    assert!(matches!(&expr, Expr::FunctionCall(c) if c.positional.len() == 2 && c.named.len() == 1));
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
fn test_cast_operator_no_type() {
    let (_, log) = parse_expr_no_assert("42 as ");
    assert!(log.error_bit());
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
fn test_struct_init_trailing_comma() {
    let expr = parse_expr("Foo { x: 1, }");
    assert!(matches!(&expr, Expr::StructInit(s) if s.fields.len() == 1));
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
fn test_if_else_if() {
    let expr = parse_expr("if true { 1 } else if false { 2 } else { 3 }");
    assert!(matches!(&expr, Expr::If(i) if matches!(i.false_branch, Some(ElseIf::If(_)))));
}

#[test]
fn test_expr_while() {
    assert!(matches!(&parse_expr("while true { break; }"), Expr::While(_)));
}

#[test]
fn test_while_no_condition() {
    let expr = parse_expr("while { break; }");
    assert!(matches!(&expr, Expr::While(w) if w.condition.is_none()));
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

#[test]
fn test_list_trailing_comma() {
    let expr = parse_expr("[1, 2,]");
    assert!(matches!(&expr, Expr::List(l) if l.elements.len() == 2));
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

// ========== UNSAFE / SAFE BLOCKS ==========

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

// ========== PATHS ==========

#[test]
fn test_expr_path_global() {
    assert!(matches!(&parse_expr("::std::mem"), Expr::Path(p) if p.segments[0].name == ""));
}

#[test]
fn test_expr_path_generic() {
    assert!(matches!(&parse_expr("foo::<i32>::bar"), Expr::Path(p) if p.segments[0].type_arguments.is_some()));
}

// ========== METHOD CHAINING / FIELD ACCESS ==========

#[test]
fn test_method_call_chain() {
    let expr = parse_expr("a.b().c()");
    assert!(matches!(&expr, Expr::MethodCall(m) if m.method_name == "c" && matches!(m.object, Expr::MethodCall(_))));
}

#[test]
fn test_field_access_chain() {
    assert!(matches!(&parse_expr("a.b.c"), Expr::FieldAccess(f) if f.field == "c"));
}

// ========== BLOCK AS STMT ==========

#[test]
fn test_block_as_stmt() {
    assert!(single_function(parse_source("fn f() { { 42 }; }")).definition.is_some());
}

// ========== BSTRING ==========

#[test]
fn test_bstring_expr() {
    let expr = parse_expr(r#""\xff\xfe""#);
    assert!(matches!(&expr, Expr::BString(_)));
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
fn test_path_expected_name() {
    let (_, log) = parse_expr_no_assert("foo::");
    assert!(log.error_bit());
}

#[test]
fn test_path_at_eof_after_colon() {
    // The source "fn f() { :: }" should error at the colon without panicking
    let (_, log) = parse_source_no_assert("fn f() { :: }");
    assert!(log.error_bit());
}
