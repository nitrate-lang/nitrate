use crate::{
    context::Ast2HirCtx,
    diagnosis::HirErr,
    ty::{
        lower_array_type, lower_function_type, lower_pointer_type, lower_reference_type, lower_refinement_type,
        lower_slice_type, lower_tuple_type, lower_type, lower_type_path, lower_type_potential,
    },
};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::{Store, prelude::*, using_storage};
use nitrate_nstring::NString;
use nitrate_token::IntegerKind;
use nitrate_tree::ast::{self as ast, SrcPos, SrcSpan, SymbolKind};
use nitrate_tree_resolve::ImportContext;
use std::ops::Deref;

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

fn tp(t: &TypeId) -> &Type {
    t.deref()
}

// ===== lower_type: 16 primitive tests =====
#[test]
fn lt_bool() {
    run(|c, l| {
        assert_eq!(
            lower_type(
                ast::Type::Bool(ast::Bool {
                    span: SrcSpan::default()
                }),
                c,
                l
            ),
            Ok(Type::Bool {
                span: SrcPos::default()
            })
        );
    })
}
#[test]
fn lt_u8() {
    run(|c, l| {
        assert_eq!(
            lower_type(
                ast::Type::UInt8(ast::UInt8 {
                    span: SrcSpan::default()
                }),
                c,
                l
            ),
            Ok(Type::U8 {
                span: SrcPos::default()
            })
        );
    })
}
#[test]
fn lt_u16() {
    run(|c, l| {
        assert_eq!(
            lower_type(
                ast::Type::UInt16(ast::UInt16 {
                    span: SrcSpan::default()
                }),
                c,
                l
            ),
            Ok(Type::U16 {
                span: SrcPos::default()
            })
        );
    })
}
#[test]
fn lt_u32() {
    run(|c, l| {
        assert_eq!(
            lower_type(
                ast::Type::UInt32(ast::UInt32 {
                    span: SrcSpan::default()
                }),
                c,
                l
            ),
            Ok(Type::U32 {
                span: SrcPos::default()
            })
        );
    })
}
#[test]
fn lt_u64() {
    run(|c, l| {
        assert_eq!(
            lower_type(
                ast::Type::UInt64(ast::UInt64 {
                    span: SrcSpan::default()
                }),
                c,
                l
            ),
            Ok(Type::U64 {
                span: SrcPos::default()
            })
        );
    })
}
#[test]
fn lt_u128() {
    run(|c, l| {
        assert_eq!(
            lower_type(
                ast::Type::UInt128(ast::UInt128 {
                    span: SrcSpan::default()
                }),
                c,
                l
            ),
            Ok(Type::U128 {
                span: SrcPos::default()
            })
        );
    })
}
#[test]
fn lt_usize() {
    run(|c, l| {
        assert_eq!(
            lower_type(
                ast::Type::USize(ast::USize {
                    span: SrcSpan::default()
                }),
                c,
                l
            ),
            Ok(Type::USize {
                span: SrcPos::default()
            })
        );
    })
}
#[test]
fn lt_i8() {
    run(|c, l| {
        assert_eq!(
            lower_type(
                ast::Type::Int8(ast::Int8 {
                    span: SrcSpan::default()
                }),
                c,
                l
            ),
            Ok(Type::I8 {
                span: SrcPos::default()
            })
        );
    })
}
#[test]
fn lt_i16() {
    run(|c, l| {
        assert_eq!(
            lower_type(
                ast::Type::Int16(ast::Int16 {
                    span: SrcSpan::default()
                }),
                c,
                l
            ),
            Ok(Type::I16 {
                span: SrcPos::default()
            })
        );
    })
}
#[test]
fn lt_i32() {
    run(|c, l| {
        assert_eq!(
            lower_type(
                ast::Type::Int32(ast::Int32 {
                    span: SrcSpan::default()
                }),
                c,
                l
            ),
            Ok(Type::I32 {
                span: SrcPos::default()
            })
        );
    })
}
#[test]
fn lt_i64() {
    run(|c, l| {
        assert_eq!(
            lower_type(
                ast::Type::Int64(ast::Int64 {
                    span: SrcSpan::default()
                }),
                c,
                l
            ),
            Ok(Type::I64 {
                span: SrcPos::default()
            })
        );
    })
}
#[test]
fn lt_i128() {
    run(|c, l| {
        assert_eq!(
            lower_type(
                ast::Type::Int128(ast::Int128 {
                    span: SrcSpan::default()
                }),
                c,
                l
            ),
            Ok(Type::I128 {
                span: SrcPos::default()
            })
        );
    })
}
#[test]
fn lt_f32() {
    run(|c, l| {
        assert_eq!(
            lower_type(
                ast::Type::Float32(ast::Float32 {
                    span: SrcSpan::default()
                }),
                c,
                l
            ),
            Ok(Type::F32 {
                span: SrcPos::default()
            })
        );
    })
}
#[test]
fn lt_f64() {
    run(|c, l| {
        assert_eq!(
            lower_type(
                ast::Type::Float64(ast::Float64 {
                    span: SrcSpan::default()
                }),
                c,
                l
            ),
            Ok(Type::F64 {
                span: SrcPos::default()
            })
        );
    })
}
#[test]
fn lt_infer() {
    run(|c, l| {
        lower_type(
            ast::Type::InferType(ast::InferType {
                span: SrcSpan::default(),
            }),
            c,
            l,
        )
        .unwrap();
    })
}

#[test]
fn lt_syntax_error() {
    run(|c, l| {
        assert!(
            lower_type(
                ast::Type::SyntaxError(ast::TypeSyntaxError {
                    span: SrcSpan::default()
                }),
                c,
                l
            )
            .is_err()
        );
    })
}

#[test]
fn lt_type_path_unresolved() {
    run(|c, l| {
        assert!(
            lower_type(
                ast::Type::TypePath(Box::new(ast::TypePath {
                    span: SrcSpan::default(),
                    segments: vec![],
                    resolved_path: None,
                })),
                c,
                l
            )
            .is_err()
        );
    })
}

#[test]
fn lt_parentheses() {
    run(|c, l| {
        assert!(
            lower_type(
                ast::Type::Parentheses(Box::new(ast::TypeParentheses {
                    span: SrcSpan::default(),
                    inner: ast::Type::Bool(ast::Bool {
                        span: SrcSpan::default()
                    }),
                })),
                c,
                l,
            )
            .unwrap()
            .is_bool()
        );
    })
}

// ===== lower_type_path =====

#[test]
fn lt_struct() {
    run(|c, l| {
        let ty: NString = "Foo".into();
        c.ast_symbol_map.insert(ty.clone(), SymbolKind::Struct);
        c.tab.get_struct_or_insert_placeholder(&ty);
        let tp = ast::TypePath {
            span: SrcSpan::default(),
            segments: vec![ast::TypePathSegment {
                span: SrcSpan::default(),
                name: "Foo".into(),
                type_arguments: None,
            }],
            resolved_path: Some(ty),
        };
        let r = lower_type_path(tp, c, l).unwrap();
        assert!(r.is_struct());
    })
}

// ===== lower_tuple_type =====

#[test]
fn lt_tuple_empty() {
    run(|c, l| {
        assert_eq!(
            lower_tuple_type(
                ast::TupleType {
                    span: SrcSpan::default(),
                    element_types: vec![],
                },
                c,
                l
            ),
            Ok(Type::Unit {
                span: SrcPos::default()
            })
        );
    })
}

#[test]
fn lt_tuple_single() {
    run(|c, l| {
        let r = lower_tuple_type(
            ast::TupleType {
                span: SrcSpan::default(),
                element_types: vec![ast::Type::Bool(ast::Bool {
                    span: SrcSpan::default(),
                })],
            },
            c,
            l,
        )
        .unwrap();
        assert!(r.is_tuple());
    })
}

#[test]
fn lt_tuple_multi() {
    run(|c, l| {
        let r = lower_tuple_type(
            ast::TupleType {
                span: SrcSpan::default(),
                element_types: vec![
                    ast::Type::Bool(ast::Bool {
                        span: SrcSpan::default(),
                    }),
                    ast::Type::Int32(ast::Int32 {
                        span: SrcSpan::default(),
                    }),
                ],
            },
            c,
            l,
        )
        .unwrap();
        assert!(r.is_tuple());
    })
}

// ===== lower_array_type =====

#[test]
fn lt_array_5() {
    run(|c, l| {
        let r = lower_array_type(
            ast::ArrayType {
                span: SrcSpan::default(),
                element_type: ast::Type::Int32(ast::Int32 {
                    span: SrcSpan::default(),
                }),
                len: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    span: SrcSpan::default(),
                    value: 5,
                    kind: IntegerKind::Dec,
                })),
            },
            c,
            l,
        )
        .unwrap();
        match r {
            Type::UnresolvedArray { element_type, .. } => {
                assert!(matches!(element_type.deref(), Type::I32 { .. }));
            }
            _ => panic!("expected UnresolvedArray"),
        }
    })
}

#[test]
fn lt_array_zero() {
    run(|c, l| {
        let r = lower_array_type(
            ast::ArrayType {
                span: SrcSpan::default(),
                element_type: ast::Type::UInt8(ast::UInt8 {
                    span: SrcSpan::default(),
                }),
                len: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    span: SrcSpan::default(),
                    value: 0,
                    kind: IntegerKind::Dec,
                })),
            },
            c,
            l,
        )
        .unwrap();
        match r {
            Type::UnresolvedArray { .. } => {}
            _ => panic!("expected UnresolvedArray"),
        }
    })
}

// ===== lower_function_type =====

#[test]
fn lt_fn_empty() {
    run(|c, l| {
        let r = lower_function_type(
            ast::FunctionType {
                span: SrcSpan::default(),
                attributes: None,
                parameters: vec![],
                return_type: None,
            },
            c,
            l,
        )
        .unwrap();
        assert!(r.is_function());
    })
}

#[test]
fn lt_fn_with_params() {
    run(|c, l| {
        let r = lower_function_type(
            ast::FunctionType {
                span: SrcSpan::default(),
                attributes: None,
                parameters: vec![
                    ast::FuncTypeParam {
                        span: SrcSpan::default(),
                        attributes: None,
                        name: "x".into(),
                        ty: ast::Type::Int32(ast::Int32 {
                            span: SrcSpan::default(),
                        }),
                    },
                    ast::FuncTypeParam {
                        span: SrcSpan::default(),
                        attributes: None,
                        name: "y".into(),
                        ty: ast::Type::Bool(ast::Bool {
                            span: SrcSpan::default(),
                        }),
                    },
                ],
                return_type: Some(ast::Type::Float64(ast::Float64 {
                    span: SrcSpan::default(),
                })),
            },
            c,
            l,
        )
        .unwrap();
        assert!(r.is_function());
    })
}

// ===== lower_reference_type =====

#[test]
fn lt_ref_immut() {
    run(|c, l| {
        let r = lower_reference_type(
            ast::ReferenceType {
                span: SrcSpan::default(),
                lifetime: None,
                exclusivity: None,
                mutability: None,
                to: ast::Type::Int32(ast::Int32 {
                    span: SrcSpan::default(),
                }),
            },
            c,
            l,
        )
        .unwrap();
        assert!(r.is_reference());
    })
}

#[test]
fn lt_ref_mut() {
    run(|c, l| {
        let r = lower_reference_type(
            ast::ReferenceType {
                span: SrcSpan::default(),
                lifetime: None,
                exclusivity: None,
                mutability: Some(ast::Mutability::Mut),
                to: ast::Type::Bool(ast::Bool {
                    span: SrcSpan::default(),
                }),
            },
            c,
            l,
        )
        .unwrap();
        assert!(r.is_reference());
    })
}

// ===== lower_pointer_type =====

#[test]
fn lt_ptr() {
    run(|c, l| {
        let r = lower_pointer_type(
            ast::PointerType {
                span: SrcSpan::default(),
                lifetime: None,
                exclusivity: None,
                mutability: None,
                to: ast::Type::Int32(ast::Int32 {
                    span: SrcSpan::default(),
                }),
            },
            c,
            l,
        )
        .unwrap();
        assert!(r.is_pointer());
    })
}

// ===== Error-returning functions =====

#[test]
fn lt_slice_outside_ref() {
    run(|c, l| {
        assert!(
            lower_slice_type(
                ast::SliceType {
                    span: SrcSpan::default(),
                    element_type: ast::Type::Int32(ast::Int32 {
                        span: SrcSpan::default(),
                    })
                },
                c,
                l
            )
            .is_err()
        );
    })
}

#[test]
fn lt_type_potential() {
    run(|c, l| {
        assert!(
            lower_type_potential(
                ast::TypePotential {
                    span: SrcSpan::default(),
                    body: ast::Block {
                        span: SrcSpan::default(),
                        safety: None,
                        elements: vec![],
                    }
                },
                c,
                l
            )
            .is_err()
        );
    })
}

#[test]
fn lt_lifetime() {
    run(|c, l| {
        assert!(
            crate::ty::lower_lifetime_type(
                ast::Lifetime {
                    span: SrcSpan::default(),
                    name: "static".into()
                },
                c,
                l
            )
            .is_err()
        );
    })
}
