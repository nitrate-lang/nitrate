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
