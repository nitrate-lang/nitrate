use crate::{context::Ast2HirCtx, expr::lower_expr};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::{Store, prelude::*, using_storage};
use nitrate_token::IntegerKind;
use nitrate_tree::ast::{self as ast};
use nitrate_tree_resolve::ImportContext;

fn ctx_log() -> (Ast2HirCtx, CompilerLog) {
    let log = CompilerLog::default();
    let import_ctx = ImportContext::new("test".into(), "test.nit".into());
    let ctx = Ast2HirCtx::new(PtrSize::U64, import_ctx);
    (ctx, log)
}

fn run<R>(f: impl FnOnce(&mut Ast2HirCtx, &CompilerLog) -> R) -> R {
    let store = Store::new();
    using_storage(&store, || {
        let (mut ctx, log) = ctx_log();
        f(&mut ctx, &log)
    })
}

// Boolean literals
#[test]
fn expr_bool_true() {
    run(|c, l| {
        assert_eq!(
            lower_expr(ast::Expr::Boolean(ast::BooleanLit { value: true }), c, l).unwrap(),
            Value::Bool(true)
        );
    })
}
#[test]
fn expr_bool_false() {
    run(|c, l| {
        assert_eq!(
            lower_expr(ast::Expr::Boolean(ast::BooleanLit { value: false }), c, l).unwrap(),
            Value::Bool(false)
        );
    })
}

// Integer literals
#[test]
fn expr_integer_42() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::Integer(Box::new(ast::IntegerLit {
                value: 42,
                kind: IntegerKind::Dec,
            })),
            c,
            l,
        )
        .unwrap();
        assert!(matches!(r, Value::InferredInteger(v) if *v == 42));
    })
}
#[test]
fn expr_integer_hex() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::Integer(Box::new(ast::IntegerLit {
                value: 255,
                kind: IntegerKind::Hex,
            })),
            c,
            l,
        )
        .unwrap();
        assert!(matches!(r, Value::InferredInteger(v) if *v == 255));
    })
}
#[test]
fn expr_integer_large() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::Integer(Box::new(ast::IntegerLit {
                value: u128::MAX,
                kind: IntegerKind::Dec,
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_inferred_integer());
    })
}

// Float literals
#[test]
fn expr_float() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::Float(ast::FloatLit {
                value: std::str::FromStr::from_str("3.14").unwrap(),
            }),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_inferred_float());
    })
}
#[test]
fn expr_float_zero() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::Float(ast::FloatLit {
                value: std::str::FromStr::from_str("0.0").unwrap(),
            }),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_inferred_float());
    })
}

// String literals
#[test]
fn expr_string_hello() {
    run(|c, l| {
        let r = lower_expr(ast::Expr::String(ast::StringLit { value: "hello".into() }), c, l).unwrap();
        assert!(matches!(r, Value::StringLit(s) if *s == *"hello"));
    })
}
#[test]
fn expr_string_empty() {
    run(|c, l| {
        let r = lower_expr(ast::Expr::String(ast::StringLit { value: "".into() }), c, l).unwrap();
        assert!(r.is_string_lit());
    })
}

// Bstring literals
#[test]
fn expr_bstring() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::BString(Box::new(ast::BStringLit { value: vec![1, 2, 3] })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_bstring_lit());
    })
}

// List literals
#[test]
fn expr_list_empty() {
    run(|c, l| {
        let r = lower_expr(ast::Expr::List(Box::new(ast::List { elements: vec![] })), c, l).unwrap();
        assert!(r.is_list());
    })
}
#[test]
fn expr_list_ints() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::List(Box::new(ast::List {
                elements: vec![
                    ast::Expr::Integer(Box::new(ast::IntegerLit {
                        value: 1,
                        kind: IntegerKind::Dec,
                    })),
                    ast::Expr::Integer(Box::new(ast::IntegerLit {
                        value: 2,
                        kind: IntegerKind::Dec,
                    })),
                ],
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_list());
    })
}

// Tuple literals
#[test]
fn expr_tuple_empty() {
    run(|c, l| {
        let r = lower_expr(ast::Expr::Tuple(Box::new(ast::Tuple { elements: vec![] })), c, l).unwrap();
        assert!(r.is_tuple());
    })
}
#[test]
fn expr_tuple_values() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::Tuple(Box::new(ast::Tuple {
                elements: vec![
                    ast::Expr::Boolean(ast::BooleanLit { value: true }),
                    ast::Expr::Integer(Box::new(ast::IntegerLit {
                        value: 1,
                        kind: IntegerKind::Dec,
                    })),
                ],
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_tuple());
    })
}

// Parentheses
#[test]
fn expr_parens() {
    run(|c, l| {
        assert_eq!(
            lower_expr(
                ast::Expr::Parentheses(Box::new(ast::ExprParentheses {
                    inner: ast::Expr::Boolean(ast::BooleanLit { value: true })
                })),
                c,
                l
            )
            .unwrap(),
            Value::Bool(true)
        );
    })
}

// Unary expressions
#[test]
fn expr_unary_not() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::UnaryExpr(Box::new(ast::UnaryExpr {
                operator: ast::UnaryExprOp::Not,
                operand: ast::Expr::Boolean(ast::BooleanLit { value: true }),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_unary());
    })
}
#[test]
fn expr_unary_neg() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::UnaryExpr(Box::new(ast::UnaryExpr {
                operator: ast::UnaryExprOp::Sub,
                operand: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 5,
                    kind: IntegerKind::Dec,
                })),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_unary());
    })
}
#[test]
fn expr_unary_pos() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::UnaryExpr(Box::new(ast::UnaryExpr {
                operator: ast::UnaryExprOp::Add,
                operand: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 3,
                    kind: IntegerKind::Dec,
                })),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_unary());
    })
}
#[test]
fn expr_unary_deref() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::UnaryExpr(Box::new(ast::UnaryExpr {
                operator: ast::UnaryExprOp::Deref,
                operand: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 1,
                    kind: IntegerKind::Dec,
                })),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_deref());
    })
}
#[test]
fn expr_unary_borrow() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::UnaryExpr(Box::new(ast::UnaryExpr {
                operator: ast::UnaryExprOp::Borrow,
                operand: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 1,
                    kind: IntegerKind::Dec,
                })),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_borrow());
    })
}
#[test]
fn expr_unary_typeof() {
    run(|c, l| {
        assert!(
            lower_expr(
                ast::Expr::UnaryExpr(Box::new(ast::UnaryExpr {
                    operator: ast::UnaryExprOp::Typeof,
                    operand: ast::Expr::Integer(Box::new(ast::IntegerLit {
                        value: 1,
                        kind: IntegerKind::Dec
                    }))
                })),
                c,
                l
            )
            .is_err()
        );
    })
}

// Binary expressions
#[test]
fn expr_binary_add() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::BinExpr(Box::new(ast::BinExpr {
                operator: ast::BinExprOp::Add,
                left: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 1,
                    kind: IntegerKind::Dec,
                })),
                right: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 2,
                    kind: IntegerKind::Dec,
                })),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_binary());
    })
}
#[test]
fn expr_binary_sub() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::BinExpr(Box::new(ast::BinExpr {
                operator: ast::BinExprOp::Sub,
                left: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 5,
                    kind: IntegerKind::Dec,
                })),
                right: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 3,
                    kind: IntegerKind::Dec,
                })),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_binary());
    })
}
#[test]
fn expr_binary_mul() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::BinExpr(Box::new(ast::BinExpr {
                operator: ast::BinExprOp::Mul,
                left: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 2,
                    kind: IntegerKind::Dec,
                })),
                right: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 3,
                    kind: IntegerKind::Dec,
                })),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_binary());
    })
}
#[test]
fn expr_binary_div() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::BinExpr(Box::new(ast::BinExpr {
                operator: ast::BinExprOp::Div,
                left: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 6,
                    kind: IntegerKind::Dec,
                })),
                right: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 2,
                    kind: IntegerKind::Dec,
                })),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_binary());
    })
}
#[test]
fn expr_binary_mod() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::BinExpr(Box::new(ast::BinExpr {
                operator: ast::BinExprOp::Mod,
                left: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 7,
                    kind: IntegerKind::Dec,
                })),
                right: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 3,
                    kind: IntegerKind::Dec,
                })),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_binary());
    })
}
#[test]
fn expr_binary_and() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::BinExpr(Box::new(ast::BinExpr {
                operator: ast::BinExprOp::BitAnd,
                left: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 1,
                    kind: IntegerKind::Dec,
                })),
                right: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 2,
                    kind: IntegerKind::Dec,
                })),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_binary());
    })
}
#[test]
fn expr_binary_or() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::BinExpr(Box::new(ast::BinExpr {
                operator: ast::BinExprOp::BitOr,
                left: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 1,
                    kind: IntegerKind::Dec,
                })),
                right: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 2,
                    kind: IntegerKind::Dec,
                })),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_binary());
    })
}
#[test]
fn expr_binary_xor() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::BinExpr(Box::new(ast::BinExpr {
                operator: ast::BinExprOp::BitXor,
                left: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 1,
                    kind: IntegerKind::Dec,
                })),
                right: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 2,
                    kind: IntegerKind::Dec,
                })),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_binary());
    })
}
#[test]
fn expr_binary_shl() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::BinExpr(Box::new(ast::BinExpr {
                operator: ast::BinExprOp::BitShl,
                left: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 1,
                    kind: IntegerKind::Dec,
                })),
                right: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 4,
                    kind: IntegerKind::Dec,
                })),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_binary());
    })
}
#[test]
fn expr_binary_shr() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::BinExpr(Box::new(ast::BinExpr {
                operator: ast::BinExprOp::BitShr,
                left: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 8,
                    kind: IntegerKind::Dec,
                })),
                right: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 1,
                    kind: IntegerKind::Dec,
                })),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_binary());
    })
}
#[test]
fn expr_binary_rol() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::BinExpr(Box::new(ast::BinExpr {
                operator: ast::BinExprOp::BitRol,
                left: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 1,
                    kind: IntegerKind::Dec,
                })),
                right: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 1,
                    kind: IntegerKind::Dec,
                })),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_binary());
    })
}
#[test]
fn expr_binary_ror() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::BinExpr(Box::new(ast::BinExpr {
                operator: ast::BinExprOp::BitRor,
                left: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 2,
                    kind: IntegerKind::Dec,
                })),
                right: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 1,
                    kind: IntegerKind::Dec,
                })),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_binary());
    })
}
#[test]
fn expr_binary_eq() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::BinExpr(Box::new(ast::BinExpr {
                operator: ast::BinExprOp::LogicEq,
                left: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 1,
                    kind: IntegerKind::Dec,
                })),
                right: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 1,
                    kind: IntegerKind::Dec,
                })),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_binary());
    })
}
#[test]
fn expr_binary_ne() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::BinExpr(Box::new(ast::BinExpr {
                operator: ast::BinExprOp::LogicNe,
                left: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 1,
                    kind: IntegerKind::Dec,
                })),
                right: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 2,
                    kind: IntegerKind::Dec,
                })),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_binary());
    })
}
#[test]
fn expr_binary_lt() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::BinExpr(Box::new(ast::BinExpr {
                operator: ast::BinExprOp::LogicLt,
                left: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 1,
                    kind: IntegerKind::Dec,
                })),
                right: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 2,
                    kind: IntegerKind::Dec,
                })),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_binary());
    })
}
#[test]
fn expr_binary_gt() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::BinExpr(Box::new(ast::BinExpr {
                operator: ast::BinExprOp::LogicGt,
                left: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 2,
                    kind: IntegerKind::Dec,
                })),
                right: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 1,
                    kind: IntegerKind::Dec,
                })),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_binary());
    })
}
#[test]
fn expr_binary_le() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::BinExpr(Box::new(ast::BinExpr {
                operator: ast::BinExprOp::LogicLe,
                left: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 1,
                    kind: IntegerKind::Dec,
                })),
                right: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 2,
                    kind: IntegerKind::Dec,
                })),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_binary());
    })
}
#[test]
fn expr_binary_ge() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::BinExpr(Box::new(ast::BinExpr {
                operator: ast::BinExprOp::LogicGe,
                left: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 2,
                    kind: IntegerKind::Dec,
                })),
                right: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 1,
                    kind: IntegerKind::Dec,
                })),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_binary());
    })
}
#[test]
fn expr_binary_andand() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::BinExpr(Box::new(ast::BinExpr {
                operator: ast::BinExprOp::LogicAnd,
                left: ast::Expr::Boolean(ast::BooleanLit { value: true }),
                right: ast::Expr::Boolean(ast::BooleanLit { value: false }),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_binary());
    })
}
#[test]
fn expr_binary_oror() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::BinExpr(Box::new(ast::BinExpr {
                operator: ast::BinExprOp::LogicOr,
                left: ast::Expr::Boolean(ast::BooleanLit { value: true }),
                right: ast::Expr::Boolean(ast::BooleanLit { value: false }),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_binary());
    })
}

// Assign (Set) operations
#[test]
fn expr_assign() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::BinExpr(Box::new(ast::BinExpr {
                operator: ast::BinExprOp::Set,
                left: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 1,
                    kind: IntegerKind::Dec,
                })),
                right: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 2,
                    kind: IntegerKind::Dec,
                })),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_assign());
    })
}
#[test]
fn expr_assign_add() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::BinExpr(Box::new(ast::BinExpr {
                operator: ast::BinExprOp::SetPlus,
                left: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 1,
                    kind: IntegerKind::Dec,
                })),
                right: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 2,
                    kind: IntegerKind::Dec,
                })),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_assign());
    })
}
#[test]
fn expr_assign_sub() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::BinExpr(Box::new(ast::BinExpr {
                operator: ast::BinExprOp::SetMinus,
                left: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 5,
                    kind: IntegerKind::Dec,
                })),
                right: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 3,
                    kind: IntegerKind::Dec,
                })),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_assign());
    })
}
#[test]
fn expr_assign_mul() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::BinExpr(Box::new(ast::BinExpr {
                operator: ast::BinExprOp::SetTimes,
                left: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 2,
                    kind: IntegerKind::Dec,
                })),
                right: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 3,
                    kind: IntegerKind::Dec,
                })),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_assign());
    })
}
#[test]
fn expr_assign_div() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::BinExpr(Box::new(ast::BinExpr {
                operator: ast::BinExprOp::SetSlash,
                left: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 6,
                    kind: IntegerKind::Dec,
                })),
                right: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 2,
                    kind: IntegerKind::Dec,
                })),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_assign());
    })
}
#[test]
fn expr_assign_mod() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::BinExpr(Box::new(ast::BinExpr {
                operator: ast::BinExprOp::SetPercent,
                left: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 7,
                    kind: IntegerKind::Dec,
                })),
                right: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 3,
                    kind: IntegerKind::Dec,
                })),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_assign());
    })
}
#[test]
fn expr_assign_and() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::BinExpr(Box::new(ast::BinExpr {
                operator: ast::BinExprOp::SetBitAnd,
                left: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 1,
                    kind: IntegerKind::Dec,
                })),
                right: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 2,
                    kind: IntegerKind::Dec,
                })),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_assign());
    })
}
#[test]
fn expr_assign_or() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::BinExpr(Box::new(ast::BinExpr {
                operator: ast::BinExprOp::SetBitOr,
                left: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 1,
                    kind: IntegerKind::Dec,
                })),
                right: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 2,
                    kind: IntegerKind::Dec,
                })),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_assign());
    })
}
#[test]
fn expr_assign_xor() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::BinExpr(Box::new(ast::BinExpr {
                operator: ast::BinExprOp::SetBitXor,
                left: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 1,
                    kind: IntegerKind::Dec,
                })),
                right: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 2,
                    kind: IntegerKind::Dec,
                })),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_assign());
    })
}
#[test]
fn expr_assign_shl() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::BinExpr(Box::new(ast::BinExpr {
                operator: ast::BinExprOp::SetBitShl,
                left: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 1,
                    kind: IntegerKind::Dec,
                })),
                right: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 4,
                    kind: IntegerKind::Dec,
                })),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_assign());
    })
}
#[test]
fn expr_assign_shr() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::BinExpr(Box::new(ast::BinExpr {
                operator: ast::BinExprOp::SetBitShr,
                left: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 8,
                    kind: IntegerKind::Dec,
                })),
                right: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 1,
                    kind: IntegerKind::Dec,
                })),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_assign());
    })
}
#[test]
fn expr_range_unimplemented() {
    run(|c, l| {
        assert!(
            lower_expr(
                ast::Expr::BinExpr(Box::new(ast::BinExpr {
                    operator: ast::BinExprOp::Range,
                    left: ast::Expr::Integer(Box::new(ast::IntegerLit {
                        value: 1,
                        kind: IntegerKind::Dec
                    })),
                    right: ast::Expr::Integer(Box::new(ast::IntegerLit {
                        value: 10,
                        kind: IntegerKind::Dec
                    }))
                })),
                c,
                l
            )
            .is_err()
        );
    })
}

// If expressions
#[test]
fn expr_if_true() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::If(Box::new(ast::If {
                condition: ast::Expr::Boolean(ast::BooleanLit { value: true }),
                true_branch: ast::Block {
                    safety: None,
                    elements: vec![ast::BlockItem::Expr(ast::Expr::Integer(Box::new(ast::IntegerLit {
                        value: 1,
                        kind: IntegerKind::Dec,
                    })))],
                },
                false_branch: None,
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_if());
    })
}
#[test]
fn expr_if_else() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::If(Box::new(ast::If {
                condition: ast::Expr::Boolean(ast::BooleanLit { value: true }),
                true_branch: ast::Block {
                    safety: None,
                    elements: vec![ast::BlockItem::Expr(ast::Expr::Integer(Box::new(ast::IntegerLit {
                        value: 1,
                        kind: IntegerKind::Dec,
                    })))],
                },
                false_branch: Some(ast::ElseIf::Block(ast::Block {
                    safety: None,
                    elements: vec![ast::BlockItem::Expr(ast::Expr::Integer(Box::new(ast::IntegerLit {
                        value: 2,
                        kind: IntegerKind::Dec,
                    })))],
                })),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_if());
    })
}
#[test]
fn expr_if_elseif() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::If(Box::new(ast::If {
                condition: ast::Expr::Boolean(ast::BooleanLit { value: false }),
                true_branch: ast::Block {
                    safety: None,
                    elements: vec![],
                },
                false_branch: Some(ast::ElseIf::If(Box::new(ast::If {
                    condition: ast::Expr::Boolean(ast::BooleanLit { value: true }),
                    true_branch: ast::Block {
                        safety: None,
                        elements: vec![],
                    },
                    false_branch: None,
                }))),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_if());
    })
}

// While loops
#[test]
fn expr_while_true() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::While(Box::new(ast::WhileLoop {
                condition: Some(ast::Expr::Boolean(ast::BooleanLit { value: true })),
                body: ast::Block {
                    safety: None,
                    elements: vec![],
                },
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_while());
    })
}
#[test]
fn expr_while_uncond() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::While(Box::new(ast::WhileLoop {
                condition: None,
                body: ast::Block {
                    safety: None,
                    elements: vec![],
                },
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_while());
    })
}

// Break / Continue
#[test]
fn expr_break() {
    run(|c, l| {
        let r = lower_expr(ast::Expr::Break(Box::new(ast::Break { label: None })), c, l).unwrap();
        assert!(r.is_break());
    })
}
#[test]
fn expr_break_labeled() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::Break(Box::new(ast::Break {
                label: Some("outer".into()),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_break());
    })
}
#[test]
fn expr_continue() {
    run(|c, l| {
        let r = lower_expr(ast::Expr::Continue(Box::new(ast::Continue { label: None })), c, l).unwrap();
        assert!(r.is_continue());
    })
}
#[test]
fn expr_continue_labeled() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::Continue(Box::new(ast::Continue {
                label: Some("outer".into()),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_continue());
    })
}

// Return
#[test]
fn expr_return() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::Return(Box::new(ast::Return {
                value: Some(ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 42,
                    kind: IntegerKind::Dec,
                }))),
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_return());
    })
}
#[test]
fn expr_return_void() {
    run(|c, l| {
        let r = lower_expr(ast::Expr::Return(Box::new(ast::Return { value: None })), c, l).unwrap();
        assert!(r.is_return());
    })
}

// Block
#[test]
fn expr_block_empty() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::Block(Box::new(ast::Block {
                safety: None,
                elements: vec![],
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_block());
    })
}
#[test]
fn expr_block_with_expr() {
    run(|c, l| {
        let r = lower_expr(
            ast::Expr::Block(Box::new(ast::Block {
                safety: None,
                elements: vec![ast::BlockItem::Expr(ast::Expr::Integer(Box::new(ast::IntegerLit {
                    value: 1,
                    kind: IntegerKind::Dec,
                })))],
            })),
            c,
            l,
        )
        .unwrap();
        assert!(r.is_block());
    })
}

// Unimplemented features that return Err
#[test]
fn expr_closure_unimplemented() {
    run(|c, l| {
        assert!(
            lower_expr(
                ast::Expr::Closure(Box::new(ast::Closure {
                    attributes: None,
                    parameters: None,
                    return_type: None,
                    definition: ast::Block {
                        safety: None,
                        elements: vec![]
                    }
                })),
                c,
                l
            )
            .is_err()
        );
    })
}
#[test]
fn expr_type_info_unimplemented() {
    run(|c, l| {
        assert!(
            lower_expr(
                ast::Expr::TypeInfo(Box::new(ast::TypeInfo {
                    the: ast::Type::Bool(ast::Bool)
                })),
                c,
                l
            )
            .is_err()
        );
    })
}
#[test]
fn expr_index_access_unimplemented() {
    run(|c, l| {
        assert!(
            lower_expr(
                ast::Expr::IndexAccess(Box::new(ast::IndexAccess {
                    collection: ast::Expr::Integer(Box::new(ast::IntegerLit {
                        value: 0,
                        kind: IntegerKind::Dec
                    })),
                    index: ast::Expr::Integer(Box::new(ast::IntegerLit {
                        value: 1,
                        kind: IntegerKind::Dec
                    }))
                })),
                c,
                l
            )
            .is_err()
        );
    })
}
#[test]
fn expr_match_unimplemented() {
    run(|c, l| {
        assert!(
            lower_expr(
                ast::Expr::Match(Box::new(ast::Match {
                    condition: ast::Expr::Boolean(ast::BooleanLit { value: true }),
                    cases: vec![],
                    default_case: None
                })),
                c,
                l
            )
            .is_err()
        );
    })
}
#[test]
fn expr_for_unimplemented() {
    run(|c, l| {
        assert!(
            lower_expr(
                ast::Expr::For(Box::new(ast::ForEach {
                    attributes: None,
                    bindings: vec!["x".into()],
                    iterable: ast::Expr::Integer(Box::new(ast::IntegerLit {
                        value: 0,
                        kind: IntegerKind::Dec
                    })),
                    body: ast::Block {
                        safety: None,
                        elements: vec![]
                    }
                })),
                c,
                l
            )
            .is_err()
        );
    })
}
#[test]
fn expr_await_unimplemented() {
    run(|c, l| {
        assert!(
            lower_expr(
                ast::Expr::Await(Box::new(ast::Await {
                    future: ast::Expr::Integer(Box::new(ast::IntegerLit {
                        value: 0,
                        kind: IntegerKind::Dec
                    }))
                })),
                c,
                l
            )
            .is_err()
        );
    })
}

// Syntax error
#[test]
fn expr_syntax_error() {
    run(|c, l| {
        assert!(lower_expr(ast::Expr::SyntaxError(ast::ExprSyntaxError), c, l).is_err());
    })
}
