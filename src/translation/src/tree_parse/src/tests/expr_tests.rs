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



// ========== EXPECTED OPEN BRACKET ==========
#[test]
fn test_err_expected_open_bracket() {
    // Trigger by trying to parse something after attribute syntax
    let (_, log) = parse_expr_no_assert("a[");
    assert!(log.error_bit());
}


// ========== EXPECTED OPEN BRACKET IN EXPR ==========
#[test]
fn test_err_expr_index_missing_expr() {
    let (_, log) = parse_expr_no_assert("a[]");
    // Parse `a[]` - `a` is a path, then `[]` is index with missing expression
    assert!(log.error_bit());
}


// ========== EMPTY TUPLE ==========


// ========== FOR LOOP VARIADIC BINDING EXPECTED END ==========
#[test]
fn test_err_for_binding_expected_end() {
    let (_, log) = parse_source_no_assert("fn f() { for (x, y in items { } }");
    assert!(log.error_bit());
}


// ========== STRUCT INIT FIELD NAME BEFORE COLON ==========
#[test]
fn test_struct_init_field_name() {
    let expr = parse_expr("Foo { x: 42 }");
    assert!(matches!(&expr, Expr::StructInit(s) if s.fields.len() == 1));
}


// ========== EXPRESSION IN BLOCK ITEM ==========
#[test]
fn test_block_unsafe_expr() {
    let (_, log) = parse_source_no_assert("fn f() { unsafe { 42 } }");
    assert!(!log.error_bit());
}


// ========== TYPE AS EXPR PREFIX ==========
#[test]
fn test_type_info_parse() {
    let expr = parse_expr("type u8");
    assert!(matches!(&expr, Expr::TypeInfo(_)));
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


// ========== TUPLE TYPE IN EXPRESSION ==========
#[test]
fn test_tuple_type_expr() {
    let expr = parse_expr("(1, 2, 3)");
    assert!(matches!(&expr, Expr::Tuple(t) if t.elements.len() == 3));
}


// ========== PAREN EXPRESSION ==========
#[test]
fn test_paren_expression() {
    let expr = parse_expr("(42,)");
    assert!(matches!(&expr, Expr::Tuple(t) if t.elements.len() == 1));
}


// ========== PARENTHESIZED TYPE IN EXPRESSION ==========
#[test]
fn test_paren_type_path_expr() {
    let expr = parse_expr("(foo)");
    // With no comma, this should be Parentheses, not Tuple
    assert!(matches!(&expr, Expr::Parentheses(_)));
}


// ========== CLOSURE WITH DEFAULT PARAM ==========
#[test]
fn test_closure_default_param() {
    let expr = parse_expr("fn(x: i32 = 42) { x }");
    assert!(matches!(&expr, Expr::Closure(_)));
}


// Push coverage for expr.rs and ty.rs uncovered paths

// ========== EXPR: PREFIX WITH NESTED PARENS ==========
#[test]
fn test_nested_prefix_parens() {
    let expr = parse_expr("(&x)");
    assert!(matches!(&expr, Expr::Parentheses(_)));
}


// ========== EXPR: DELETE_PRECEDENCE_EDGE ==========
#[test]
fn test_expr_precedence_rewind() {
    // This tests the rewind path when binop precedence is lower than min
    let expr = parse_expr("1 + 2 + 3");
    // Left associative, so (1+2)+3
    assert!(matches!(&expr, Expr::BinExpr(b) if matches!(b.operator, BinExprOp::Add)));
}


// Note: literal suffix with bool keyword after boolean literal - depends on lexer spacing
// Skipped because "true bool" parses as boolean expression followed by path

// ========== EXPR: TYPE INFO WITH COMPLEX TYPE ==========
#[test]
fn test_type_info_complex() {
    let expr = parse_expr("type (&i32)");
    assert!(matches!(&expr, Expr::TypeInfo(_)));
}


// ========== EXPR: CLOSURE RETURN ARROW ==========
#[test]
fn test_closure_with_return_arrow_missing_gt() {
    let (_, log) = parse_expr_no_assert("fn(x: i32) - i32 { x }");
    assert!(log.error_bit());
}


// ========== EXPR: CLOSURE PARAM DEFAULT ==========
#[test]
fn test_closure_param_default() {
    let expr = parse_expr("fn(x: i32 = 42) { }");
    assert!(matches!(&expr, Expr::Closure(_)));
}


// ========== BREAK/CONTINUE EDGE CASES ==========


#[test]
fn test_continue_label() {
    let expr = parse_expr("continue 'l;");
    assert!(matches!(&expr, Expr::Continue(c) if matches!(&c.label, Some(l) if &**l == "l")));
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
fn test_closure_no_params_no_return() {
    let expr = parse_expr("fn { 42 }");
    assert!(matches!(&expr, Expr::Closure(c) if c.parameters.is_none() && c.return_type.is_none()));
}


#[test]
fn test_block_in_block() {
    let f = single_function(parse_source("fn f() { { 42 } }"));
    assert!(f.definition.is_some());
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


// ========== WHILE LOOP EDGE CASES ==========


// ========== UNSAFE BLOCK MODIFIER EDGE CASES ==========


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


// ========== AWAIT EDGE CASES ==========


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


// ========== ITEM ERROR PATHS ==========

#[test]
fn test_item_unexpected_in_function() {
    let (_, log) = parse_source_no_assert("fn f() { } struct Foo { }");
    // struct after fn should work fine, they are at module level
    assert!(!log.error_bit());
}


// ========== BLOCK WITH UNSAFE AND SAFE ==========


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


// ========== CAST FROM STRING ==========

#[test]
fn test_cast_from_string() {
    let expr = parse_expr("\"hello\" as f64");
    assert!(matches!(&expr, Expr::Cast(_)));
}


// ========== CLOSURE TRAILING COMMA ==========

#[test]
fn test_closure_params_trailing_comma() {
    let expr = parse_expr("fn(x: i32,) { x }");
    assert!(matches!(&expr, Expr::Closure(c) if c.parameters.as_ref().map_or(false, |p| p.len() == 1)));
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

