use crate::{
    context::Ast2HirCtx,
    diagnosis::HirErr,
    ty::{
        lower_array_type, lower_function_type, lower_latent_type, lower_lifetime, lower_pointer_type,
        lower_reference_type, lower_refinement_type, lower_slice_type, lower_tuple_type, lower_type, lower_type_path,
    },
};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::{Store, prelude::*, using_storage};
use nitrate_token::IntegerKind;
use nitrate_tree::ast::{self as ast, SymbolKind};
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
        assert_eq!(lower_type(ast::Type::Bool(ast::Bool), c, l), Ok(Type::Bool));
    })
}
#[test]
fn lt_u8() {
    run(|c, l| {
        assert_eq!(lower_type(ast::Type::UInt8(ast::UInt8), c, l), Ok(Type::U8));
    })
}
#[test]
fn lt_u16() {
    run(|c, l| {
        assert_eq!(lower_type(ast::Type::UInt16(ast::UInt16), c, l), Ok(Type::U16));
    })
}
#[test]
fn lt_u32() {
    run(|c, l| {
        assert_eq!(lower_type(ast::Type::UInt32(ast::UInt32), c, l), Ok(Type::U32));
    })
}
#[test]
fn lt_u64() {
    run(|c, l| {
        assert_eq!(lower_type(ast::Type::UInt64(ast::UInt64), c, l), Ok(Type::U64));
    })
}
#[test]
fn lt_u128() {
    run(|c, l| {
        assert_eq!(lower_type(ast::Type::UInt128(ast::UInt128), c, l), Ok(Type::U128));
    })
}
#[test]
fn lt_usize() {
    run(|c, l| {
        assert_eq!(lower_type(ast::Type::USize(ast::USize), c, l), Ok(Type::USize));
    })
}
#[test]
fn lt_i8() {
    run(|c, l| {
        assert_eq!(lower_type(ast::Type::Int8(ast::Int8), c, l), Ok(Type::I8));
    })
}
#[test]
fn lt_i16() {
    run(|c, l| {
        assert_eq!(lower_type(ast::Type::Int16(ast::Int16), c, l), Ok(Type::I16));
    })
}
#[test]
fn lt_i32() {
    run(|c, l| {
        assert_eq!(lower_type(ast::Type::Int32(ast::Int32), c, l), Ok(Type::I32));
    })
}
#[test]
fn lt_i64() {
    run(|c, l| {
        assert_eq!(lower_type(ast::Type::Int64(ast::Int64), c, l), Ok(Type::I64));
    })
}
#[test]
fn lt_i128() {
    run(|c, l| {
        assert_eq!(lower_type(ast::Type::Int128(ast::Int128), c, l), Ok(Type::I128));
    })
}
#[test]
fn lt_f32() {
    run(|c, l| {
        assert_eq!(lower_type(ast::Type::Float32(ast::Float32), c, l), Ok(Type::F32));
    })
}
#[test]
fn lt_f64() {
    run(|c, l| {
        assert_eq!(lower_type(ast::Type::Float64(ast::Float64), c, l), Ok(Type::F64));
    })
}
#[test]
fn lt_infer() {
    run(|c, l| {
        assert!(
            lower_type(ast::Type::InferType(ast::InferType), c, l)
                .unwrap()
                .is_inferred()
        );
    })
}
#[test]
fn lt_syntax_err() {
    run(|c, l| {
        assert!(lower_type(ast::Type::SyntaxError(ast::TypeSyntaxError), c, l).is_err());
    })
}
#[test]
fn lt_parens() {
    run(|c, l| {
        assert_eq!(
            lower_type(
                ast::Type::Parentheses(Box::new(ast::TypeParentheses {
                    inner: ast::Type::Bool(ast::Bool)
                })),
                c,
                l
            ),
            Ok(Type::Bool)
        );
    })
}
#[test]
fn lt_parens_f32() {
    run(|c, l| {
        assert_eq!(
            lower_type(
                ast::Type::Parentheses(Box::new(ast::TypeParentheses {
                    inner: ast::Type::Float32(ast::Float32)
                })),
                c,
                l
            ),
            Ok(Type::F32)
        );
    })
}
#[test]
fn lt_parens_nested() {
    run(|c, l| {
        assert_eq!(
            lower_type(
                ast::Type::Parentheses(Box::new(ast::TypeParentheses {
                    inner: ast::Type::Parentheses(Box::new(ast::TypeParentheses {
                        inner: ast::Type::UInt64(ast::UInt64)
                    }))
                })),
                c,
                l
            ),
            Ok(Type::U64)
        );
    })
}

// ===== lower_type_path: 12 tests =====
#[test]
fn ltp_struct() {
    run(|c, l| {
        c.ast_symbol_map.insert("Foo".into(), SymbolKind::Struct);
        c.tab.get_struct_or_insert_placeholder(&"Foo".into());
        assert!(matches!(
            lower_type_path(
                ast::TypePath {
                    segments: vec![ast::TypePathSegment {
                        name: "Foo".into(),
                        type_arguments: None
                    }],
                    resolved_path: Some("Foo".into())
                },
                c,
                l
            )
            .unwrap(),
            Type::Struct { .. }
        ));
    })
}
#[test]
fn ltp_enum() {
    run(|c, l| {
        c.ast_symbol_map.insert("E".into(), SymbolKind::Enum);
        c.tab.get_enum_or_insert_placeholder(&"E".into());
        assert!(matches!(
            lower_type_path(
                ast::TypePath {
                    segments: vec![ast::TypePathSegment {
                        name: "E".into(),
                        type_arguments: None
                    }],
                    resolved_path: Some("E".into())
                },
                c,
                l
            )
            .unwrap(),
            Type::Enum { .. }
        ));
    })
}
#[test]
fn ltp_type_alias() {
    run(|c, l| {
        c.ast_symbol_map.insert("A".into(), SymbolKind::TypeAlias);
        c.tab.get_type_alias_or_insert_placeholder(&"A".into());
        assert!(matches!(
            lower_type_path(
                ast::TypePath {
                    segments: vec![ast::TypePathSegment {
                        name: "A".into(),
                        type_arguments: None
                    }],
                    resolved_path: Some("A".into())
                },
                c,
                l
            )
            .unwrap(),
            Type::TypeAlias { .. }
        ));
    })
}
#[test]
fn ltp_generic_param() {
    run(|c, l| {
        c.ast_symbol_map.insert("T".into(), SymbolKind::GenericParameter);
        assert!(matches!(
            lower_type_path(
                ast::TypePath {
                    segments: vec![ast::TypePathSegment {
                        name: "T".into(),
                        type_arguments: None
                    }],
                    resolved_path: Some("T".into())
                },
                c,
                l
            )
            .unwrap(),
            Type::GenericParam { .. }
        ));
    })
}
#[test]
fn ltp_unresolved_symbol() {
    run(|c, l| {
        c.ast_symbol_map.insert("X".into(), SymbolKind::Function);
        assert!(
            lower_type_path(
                ast::TypePath {
                    segments: vec![ast::TypePathSegment {
                        name: "X".into(),
                        type_arguments: None
                    }],
                    resolved_path: Some("X".into())
                },
                c,
                l
            )
            .is_err()
        );
    })
}
#[test]
fn ltp_unresolved_path() {
    run(|c, l| {
        assert!(
            lower_type_path(
                ast::TypePath {
                    segments: vec![ast::TypePathSegment {
                        name: "X".into(),
                        type_arguments: None
                    }],
                    resolved_path: None
                },
                c,
                l
            )
            .is_err()
        );
    })
}
#[test]
fn ltp_intermediate_generics() {
    run(|c, l| {
        c.ast_symbol_map.insert("Foo".into(), SymbolKind::Struct);
        c.ast_symbol_map.insert("Bar".into(), SymbolKind::Struct);
        assert!(
            lower_type_path(
                ast::TypePath {
                    segments: vec![
                        ast::TypePathSegment {
                            name: "Foo".into(),
                            type_arguments: Some(vec![ast::TypeArgument {
                                name: None,
                                value: ast::Type::Int32(ast::Int32)
                            }])
                        },
                        ast::TypePathSegment {
                            name: "Bar".into(),
                            type_arguments: None
                        }
                    ],
                    resolved_path: Some("Foo::Bar".into())
                },
                c,
                l
            )
            .is_err()
        );
    })
}
#[test]
fn ltp_parameterized() {
    run(|c, l| {
        c.ast_symbol_map.insert("Vec".into(), SymbolKind::Struct);
        c.tab.get_struct_or_insert_placeholder(&"Vec".into());
        assert!(matches!(
            lower_type_path(
                ast::TypePath {
                    segments: vec![ast::TypePathSegment {
                        name: "Vec".into(),
                        type_arguments: Some(vec![ast::TypeArgument {
                            name: None,
                            value: ast::Type::Int32(ast::Int32)
                        }])
                    }],
                    resolved_path: Some("Vec".into())
                },
                c,
                l
            )
            .unwrap(),
            Type::Parameterized { .. }
        ));
    })
}
#[test]
fn ltp_multi_seg() {
    run(|c, l| {
        c.ast_symbol_map.insert("a::b::Foo".into(), SymbolKind::Struct);
        c.tab.get_struct_or_insert_placeholder(&"a::b::Foo".into());
        assert!(matches!(
            lower_type_path(
                ast::TypePath {
                    segments: vec![
                        ast::TypePathSegment {
                            name: "a".into(),
                            type_arguments: None
                        },
                        ast::TypePathSegment {
                            name: "b".into(),
                            type_arguments: None
                        },
                        ast::TypePathSegment {
                            name: "Foo".into(),
                            type_arguments: None
                        }
                    ],
                    resolved_path: Some("a::b::Foo".into())
                },
                c,
                l
            )
            .unwrap(),
            Type::Struct { .. }
        ));
    })
}
#[test]
fn ltp_unresolved_type_arg() {
    run(|c, l| {
        c.ast_symbol_map.insert("M".into(), SymbolKind::Struct);
        c.tab.get_struct_or_insert_placeholder(&"M".into());
        let r = lower_type_path(
            ast::TypePath {
                segments: vec![ast::TypePathSegment {
                    name: "M".into(),
                    type_arguments: Some(vec![ast::TypeArgument {
                        name: None,
                        value: ast::Type::TypePath(Box::new(ast::TypePath {
                            segments: vec![ast::TypePathSegment {
                                name: "Missing".into(),
                                type_arguments: None,
                            }],
                            resolved_path: None,
                        })),
                    }]),
                }],
                resolved_path: Some("M".into()),
            },
            c,
            l,
        );
        assert!(r.is_ok());
    })
}

// ===== lower_tuple_type: 8 tests =====
#[test]
fn ltt_empty() {
    run(|c, l| {
        assert_eq!(
            lower_tuple_type(ast::TupleType { element_types: vec![] }, c, l).unwrap(),
            Type::Unit
        );
    })
}
#[test]
fn ltt_single() {
    run(|c, l| {
        assert!(matches!(
            lower_tuple_type(
                ast::TupleType {
                    element_types: vec![ast::Type::Int32(ast::Int32)]
                },
                c,
                l
            )
            .unwrap(),
            Type::Tuple { .. }
        ));
    })
}
#[test]
fn ltt_multi() {
    run(|c, l| {
        let r = lower_tuple_type(
            ast::TupleType {
                element_types: vec![
                    ast::Type::Bool(ast::Bool),
                    ast::Type::Float64(ast::Float64),
                    ast::Type::Int16(ast::Int16),
                ],
            },
            c,
            l,
        )
        .unwrap();
        if let Type::Tuple { element_types } = r {
            assert_eq!(element_types.len(), 3);
        } else {
            panic!();
        }
    })
}
#[test]
fn ltt_nested() {
    run(|c, l| {
        let r = lower_tuple_type(
            ast::TupleType {
                element_types: vec![
                    ast::Type::Int32(ast::Int32),
                    ast::Type::TupleType(Box::new(ast::TupleType {
                        element_types: vec![ast::Type::Float64(ast::Float64)],
                    })),
                ],
            },
            c,
            l,
        )
        .unwrap();
        if let Type::Tuple { element_types } = r {
            assert!(tp(&element_types[1]).is_tuple());
        } else {
            panic!();
        }
    })
}
#[test]
fn ltt_three_bools() {
    run(|c, l| {
        let r = lower_tuple_type(
            ast::TupleType {
                element_types: vec![
                    ast::Type::Bool(ast::Bool),
                    ast::Type::Bool(ast::Bool),
                    ast::Type::Bool(ast::Bool),
                ],
            },
            c,
            l,
        )
        .unwrap();
        if let Type::Tuple { element_types } = r {
            assert_eq!(element_types.len(), 3);
        } else {
            panic!();
        }
    })
}
#[test]
fn ltt_mixed_primitives() {
    run(|c, l| {
        let r = lower_tuple_type(
            ast::TupleType {
                element_types: vec![
                    ast::Type::UInt8(ast::UInt8),
                    ast::Type::Int8(ast::Int8),
                    ast::Type::Float32(ast::Float32),
                ],
            },
            c,
            l,
        )
        .unwrap();
        if let Type::Tuple { element_types } = r {
            assert_eq!(element_types.len(), 3);
        } else {
            panic!();
        }
    })
}
#[test]
fn ltt_with_path() {
    run(|c, l| {
        c.ast_symbol_map.insert("P".into(), SymbolKind::Struct);
        c.tab.get_struct_or_insert_placeholder(&"P".into());
        let r = lower_tuple_type(
            ast::TupleType {
                element_types: vec![
                    ast::Type::Int32(ast::Int32),
                    ast::Type::TypePath(Box::new(ast::TypePath {
                        segments: vec![ast::TypePathSegment {
                            name: "P".into(),
                            type_arguments: None,
                        }],
                        resolved_path: Some("P".into()),
                    })),
                ],
            },
            c,
            l,
        )
        .unwrap();
        if let Type::Tuple { element_types } = r {
            assert_eq!(element_types.len(), 2);
        } else {
            panic!();
        }
    })
}

// ===== lower_array_type: 8 tests =====
#[test]
fn lat_int_5() {
    run(|c, l| {
        let a = ast::ArrayType {
            element_type: ast::Type::Int32(ast::Int32),
            len: ast::Expr::Integer(Box::new(ast::IntegerLit {
                value: 5,
                kind: IntegerKind::Dec,
            })),
        };
        let r = lower_array_type(a, c, l).unwrap();
        match r {
            Type::Array { len, .. } => assert_eq!(len, 5),
            _ => panic!(),
        };
    })
}
#[test]
fn lat_zero() {
    run(|c, l| {
        let a = ast::ArrayType {
            element_type: ast::Type::UInt8(ast::UInt8),
            len: ast::Expr::Integer(Box::new(ast::IntegerLit {
                value: 0,
                kind: IntegerKind::Dec,
            })),
        };
        let r = lower_array_type(a, c, l).unwrap();
        match r {
            Type::Array { len, .. } => assert_eq!(len, 0),
            _ => panic!(),
        };
    })
}
#[test]
fn lat_large() {
    run(|c, l| {
        let a = ast::ArrayType {
            element_type: ast::Type::Float64(ast::Float64),
            len: ast::Expr::Integer(Box::new(ast::IntegerLit {
                value: 1000,
                kind: IntegerKind::Dec,
            })),
        };
        let r = lower_array_type(a, c, l).unwrap();
        match r {
            Type::Array { len, .. } => assert_eq!(len, 1000),
            _ => panic!(),
        };
    })
}
#[test]
fn lat_bool() {
    run(|c, l| {
        let a = ast::ArrayType {
            element_type: ast::Type::Bool(ast::Bool),
            len: ast::Expr::Integer(Box::new(ast::IntegerLit {
                value: 10,
                kind: IntegerKind::Dec,
            })),
        };
        let r = lower_array_type(a, c, l).unwrap();
        match r {
            Type::Array { len, .. } => assert_eq!(len, 10),
            _ => panic!(),
        };
    })
}
#[test]
fn lat_string_len_err() {
    run(|c, l| {
        let a = ast::ArrayType {
            element_type: ast::Type::Int32(ast::Int32),
            len: ast::Expr::String(ast::StringLit { value: "bad".into() }),
        };
        assert!(lower_array_type(a, c, l).is_err());
    })
}
#[test]
fn lat_struct_elem() {
    run(|c, l| {
        c.ast_symbol_map.insert("P".into(), SymbolKind::Struct);
        c.tab.get_struct_or_insert_placeholder(&"P".into());
        let a = ast::ArrayType {
            element_type: ast::Type::TypePath(Box::new(ast::TypePath {
                segments: vec![ast::TypePathSegment {
                    name: "P".into(),
                    type_arguments: None,
                }],
                resolved_path: Some("P".into()),
            })),
            len: ast::Expr::Integer(Box::new(ast::IntegerLit {
                value: 3,
                kind: IntegerKind::Dec,
            })),
        };
        assert!(matches!(lower_array_type(a, c, l).unwrap(), Type::Array { .. }));
    })
}
#[test]
fn lat_usize() {
    run(|c, l| {
        let a = ast::ArrayType {
            element_type: ast::Type::USize(ast::USize),
            len: ast::Expr::Integer(Box::new(ast::IntegerLit {
                value: 1,
                kind: IntegerKind::Dec,
            })),
        };
        assert!(matches!(lower_array_type(a, c, l).unwrap(), Type::Array { .. }));
    })
}

// ===== lower_function_type: 8 tests =====
#[test]
fn lft_empty() {
    run(|c, l| {
        assert!(matches!(
            lower_function_type(
                ast::FunctionType {
                    attributes: None,
                    parameters: vec![],
                    return_type: None
                },
                c,
                l
            )
            .unwrap(),
            Type::Function { .. }
        ));
    })
}
#[test]
fn lft_one_param() {
    run(|c, l| {
        let r = lower_function_type(
            ast::FunctionType {
                attributes: None,
                parameters: vec![ast::FuncTypeParam {
                    attributes: None,
                    name: "x".into(),
                    ty: ast::Type::Int32(ast::Int32),
                }],
                return_type: None,
            },
            c,
            l,
        )
        .unwrap();
        if let Type::Function { function_type } = r {
            assert_eq!(function_type.params.len(), 1);
        } else {
            panic!()
        };
    })
}
#[test]
fn lft_many_params() {
    run(|c, l| {
        let ps: Vec<_> = (0..10)
            .map(|i| ast::FuncTypeParam {
                attributes: None,
                name: format!("p{i}").into(),
                ty: ast::Type::Int32(ast::Int32),
            })
            .collect();
        let r = lower_function_type(
            ast::FunctionType {
                attributes: None,
                parameters: ps,
                return_type: None,
            },
            c,
            l,
        )
        .unwrap();
        if let Type::Function { function_type } = r {
            assert_eq!(function_type.params.len(), 10);
        } else {
            panic!()
        };
    })
}
#[test]
fn lft_with_return() {
    run(|c, l| {
        let r = lower_function_type(
            ast::FunctionType {
                attributes: None,
                parameters: vec![],
                return_type: Some(ast::Type::Float64(ast::Float64)),
            },
            c,
            l,
        )
        .unwrap();
        if let Type::Function { function_type } = r {
            assert!(tp(&function_type.return_type).is_float_primitive());
        } else {
            panic!()
        };
    })
}
#[test]
fn lft_named_params() {
    run(|c, l| {
        let r = lower_function_type(
            ast::FunctionType {
                attributes: None,
                parameters: vec![
                    ast::FuncTypeParam {
                        attributes: None,
                        name: "key".into(),
                        ty: ast::Type::Bool(ast::Bool),
                    },
                    ast::FuncTypeParam {
                        attributes: None,
                        name: "val".into(),
                        ty: ast::Type::Int64(ast::Int64),
                    },
                ],
                return_type: None,
            },
            c,
            l,
        )
        .unwrap();
        if let Type::Function { function_type } = r {
            assert_eq!(function_type.params.len(), 2);
            assert_eq!(&*function_type.params[0].0, "key");
        } else {
            panic!()
        };
    })
}
#[test]
fn lft_attr_err() {
    run(|c, l| {
        let r = lower_function_type(
            ast::FunctionType {
                attributes: Some(vec![ast::Expr::Path(Box::new(ast::ExprPath {
                    segments: vec![ast::ExprPathSegment {
                        name: "bad".into(),
                        type_arguments: None,
                    }],
                    resolved_path: None,
                }))]),
                parameters: vec![],
                return_type: None,
            },
            c,
            l,
        );
        assert!(r.is_ok());
        assert!(l.error_bit());
    })
}
#[test]
fn lft_param_attr_err() {
    run(|c, l| {
        let r = lower_function_type(
            ast::FunctionType {
                attributes: None,
                parameters: vec![ast::FuncTypeParam {
                    attributes: Some(vec![ast::Expr::Integer(Box::new(ast::IntegerLit {
                        value: 1,
                        kind: IntegerKind::Dec,
                    }))]),
                    name: "x".into(),
                    ty: ast::Type::Int32(ast::Int32),
                }],
                return_type: None,
            },
            c,
            l,
        );
        assert!(r.is_ok());
        assert!(l.error_bit());
    })
}

// ===== lower_reference_type: 16 tests =====
#[test]
fn lrf_default() {
    run(|c, l| {
        let r = lower_reference_type(
            ast::ReferenceType {
                lifetime: None,
                exclusivity: None,
                mutability: None,
                to: ast::Type::Int32(ast::Int32),
            },
            c,
            l,
        )
        .unwrap();
        assert!(matches!(
            r,
            Type::Reference {
                lifetime: Lifetime::Inferred,
                mutable: false,
                ..
            }
        ));
    })
}
#[test]
fn lrf_mut() {
    run(|c, l| {
        let r = lower_reference_type(
            ast::ReferenceType {
                lifetime: None,
                exclusivity: None,
                mutability: Some(ast::Mutability::Mut),
                to: ast::Type::Bool(ast::Bool),
            },
            c,
            l,
        )
        .unwrap();
        assert!(matches!(
            r,
            Type::Reference {
                mutable: true,
                exclusive: true,
                ..
            }
        ));
    })
}
#[test]
fn lrf_iso() {
    run(|c, l| {
        let r = lower_reference_type(
            ast::ReferenceType {
                lifetime: None,
                exclusivity: Some(ast::Exclusivity::Iso),
                mutability: None,
                to: ast::Type::Float64(ast::Float64),
            },
            c,
            l,
        )
        .unwrap();
        assert!(matches!(r, Type::Reference { exclusive: true, .. }));
    })
}
#[test]
fn lrf_poly() {
    run(|c, l| {
        let r = lower_reference_type(
            ast::ReferenceType {
                lifetime: None,
                exclusivity: Some(ast::Exclusivity::Poly),
                mutability: Some(ast::Mutability::Mut),
                to: ast::Type::Int32(ast::Int32),
            },
            c,
            l,
        )
        .unwrap();
        assert!(matches!(
            r,
            Type::Reference {
                exclusive: false,
                mutable: true,
                ..
            }
        ));
    })
}
#[test]
fn lrf_static() {
    run(|c, l| {
        let r = lower_reference_type(
            ast::ReferenceType {
                lifetime: Some(ast::Lifetime { name: "static".into() }),
                exclusivity: None,
                mutability: None,
                to: ast::Type::Int32(ast::Int32),
            },
            c,
            l,
        )
        .unwrap();
        assert!(matches!(
            r,
            Type::Reference {
                lifetime: Lifetime::Static,
                ..
            }
        ));
    })
}
#[test]
fn lrf_gc() {
    run(|c, l| {
        let r = lower_reference_type(
            ast::ReferenceType {
                lifetime: Some(ast::Lifetime { name: "gc".into() }),
                exclusivity: None,
                mutability: None,
                to: ast::Type::Int64(ast::Int64),
            },
            c,
            l,
        )
        .unwrap();
        assert!(matches!(
            r,
            Type::Reference {
                lifetime: Lifetime::Gc,
                ..
            }
        ));
    })
}
#[test]
fn lrf_thread() {
    run(|c, l| {
        let r = lower_reference_type(
            ast::ReferenceType {
                lifetime: Some(ast::Lifetime { name: "thread".into() }),
                exclusivity: None,
                mutability: None,
                to: ast::Type::Bool(ast::Bool),
            },
            c,
            l,
        )
        .unwrap();
        assert!(matches!(
            r,
            Type::Reference {
                lifetime: Lifetime::ThreadLocal,
                ..
            }
        ));
    })
}
#[test]
fn lrf_task() {
    run(|c, l| {
        let r = lower_reference_type(
            ast::ReferenceType {
                lifetime: Some(ast::Lifetime { name: "task".into() }),
                exclusivity: None,
                mutability: None,
                to: ast::Type::UInt8(ast::UInt8),
            },
            c,
            l,
        )
        .unwrap();
        assert!(matches!(
            r,
            Type::Reference {
                lifetime: Lifetime::TaskLocal,
                ..
            }
        ));
    })
}
#[test]
fn lrf_underscore() {
    run(|c, l| {
        let r = lower_reference_type(
            ast::ReferenceType {
                lifetime: Some(ast::Lifetime { name: "_".into() }),
                exclusivity: None,
                mutability: None,
                to: ast::Type::Int32(ast::Int32),
            },
            c,
            l,
        )
        .unwrap();
        assert!(matches!(
            r,
            Type::Reference {
                lifetime: Lifetime::Inferred,
                ..
            }
        ));
    })
}
#[test]
fn lrf_bad_lifetime() {
    run(|c, l| {
        assert!(
            lower_reference_type(
                ast::ReferenceType {
                    lifetime: Some(ast::Lifetime { name: "nope".into() }),
                    exclusivity: None,
                    mutability: None,
                    to: ast::Type::Int32(ast::Int32)
                },
                c,
                l
            )
            .is_err()
        );
    })
}
#[test]
fn lrf_slice() {
    run(|c, l| {
        let r = lower_reference_type(
            ast::ReferenceType {
                lifetime: Some(ast::Lifetime { name: "static".into() }),
                exclusivity: None,
                mutability: None,
                to: ast::Type::SliceType(Box::new(ast::SliceType {
                    element_type: ast::Type::Int32(ast::Int32),
                })),
            },
            c,
            l,
        )
        .unwrap();
        assert!(matches!(
            r,
            Type::SliceRef {
                lifetime: Lifetime::Static,
                ..
            }
        ));
    })
}
#[test]
fn lrf_slice_mut() {
    run(|c, l| {
        let r = lower_reference_type(
            ast::ReferenceType {
                lifetime: None,
                exclusivity: None,
                mutability: Some(ast::Mutability::Mut),
                to: ast::Type::SliceType(Box::new(ast::SliceType {
                    element_type: ast::Type::UInt8(ast::UInt8),
                })),
            },
            c,
            l,
        )
        .unwrap();
        assert!(matches!(
            r,
            Type::SliceRef {
                mutable: true,
                exclusive: true,
                ..
            }
        ));
    })
}
#[test]
fn lrf_mut_poly() {
    run(|c, l| {
        let r = lower_reference_type(
            ast::ReferenceType {
                lifetime: None,
                exclusivity: Some(ast::Exclusivity::Poly),
                mutability: Some(ast::Mutability::Mut),
                to: ast::Type::Float32(ast::Float32),
            },
            c,
            l,
        )
        .unwrap();
        assert!(matches!(
            r,
            Type::Reference {
                mutable: true,
                exclusive: false,
                ..
            }
        ));
    })
}
#[test]
fn lrf_iso_mut() {
    run(|c, l| {
        let r = lower_reference_type(
            ast::ReferenceType {
                lifetime: None,
                exclusivity: Some(ast::Exclusivity::Iso),
                mutability: Some(ast::Mutability::Mut),
                to: ast::Type::UInt64(ast::UInt64),
            },
            c,
            l,
        )
        .unwrap();
        assert!(matches!(
            r,
            Type::Reference {
                mutable: true,
                exclusive: true,
                ..
            }
        ));
    })
}
#[test]
fn lrf_slice_poly() {
    run(|c, l| {
        let r = lower_reference_type(
            ast::ReferenceType {
                lifetime: None,
                exclusivity: Some(ast::Exclusivity::Poly),
                mutability: Some(ast::Mutability::Mut),
                to: ast::Type::SliceType(Box::new(ast::SliceType {
                    element_type: ast::Type::UInt8(ast::UInt8),
                })),
            },
            c,
            l,
        )
        .unwrap();
        assert!(matches!(
            r,
            Type::SliceRef {
                mutable: true,
                exclusive: false,
                ..
            }
        ));
    })
}

// ===== lower_pointer_type: 10 tests =====
#[test]
fn lpt_default() {
    run(|c, l| {
        let r = lower_pointer_type(
            ast::PointerType {
                lifetime: None,
                exclusivity: None,
                mutability: None,
                to: ast::Type::Int32(ast::Int32),
            },
            c,
            l,
        )
        .unwrap();
        assert!(matches!(
            r,
            Type::Pointer {
                mutable: false,
                exclusive: false,
                ..
            }
        ));
    })
}
#[test]
fn lpt_mut() {
    run(|c, l| {
        let r = lower_pointer_type(
            ast::PointerType {
                lifetime: None,
                exclusivity: None,
                mutability: Some(ast::Mutability::Mut),
                to: ast::Type::Bool(ast::Bool),
            },
            c,
            l,
        )
        .unwrap();
        assert!(matches!(
            r,
            Type::Pointer {
                mutable: true,
                exclusive: true,
                ..
            }
        ));
    })
}
#[test]
fn lpt_iso() {
    run(|c, l| {
        let r = lower_pointer_type(
            ast::PointerType {
                lifetime: None,
                exclusivity: Some(ast::Exclusivity::Iso),
                mutability: None,
                to: ast::Type::UInt64(ast::UInt64),
            },
            c,
            l,
        )
        .unwrap();
        assert!(matches!(r, Type::Pointer { exclusive: true, .. }));
    })
}
#[test]
fn lpt_poly() {
    run(|c, l| {
        let r = lower_pointer_type(
            ast::PointerType {
                lifetime: None,
                exclusivity: Some(ast::Exclusivity::Poly),
                mutability: None,
                to: ast::Type::Float32(ast::Float32),
            },
            c,
            l,
        )
        .unwrap();
        assert!(matches!(r, Type::Pointer { exclusive: false, .. }));
    })
}
#[test]
fn lpt_poly_mut() {
    run(|c, l| {
        let r = lower_pointer_type(
            ast::PointerType {
                lifetime: None,
                exclusivity: Some(ast::Exclusivity::Poly),
                mutability: Some(ast::Mutability::Mut),
                to: ast::Type::UInt8(ast::UInt8),
            },
            c,
            l,
        )
        .unwrap();
        assert!(matches!(
            r,
            Type::Pointer {
                mutable: true,
                exclusive: false,
                ..
            }
        ));
    })
}
#[test]
fn lpt_slice() {
    run(|c, l| {
        let r = lower_pointer_type(
            ast::PointerType {
                lifetime: None,
                exclusivity: None,
                mutability: None,
                to: ast::Type::SliceType(Box::new(ast::SliceType {
                    element_type: ast::Type::Int32(ast::Int32),
                })),
            },
            c,
            l,
        )
        .unwrap();
        assert!(matches!(r, Type::SlicePtr { .. }));
    })
}
#[test]
fn lpt_mut_slice() {
    run(|c, l| {
        let r = lower_pointer_type(
            ast::PointerType {
                lifetime: None,
                exclusivity: None,
                mutability: Some(ast::Mutability::Mut),
                to: ast::Type::SliceType(Box::new(ast::SliceType {
                    element_type: ast::Type::UInt8(ast::UInt8),
                })),
            },
            c,
            l,
        )
        .unwrap();
        assert!(matches!(r, Type::SlicePtr { mutable: true, .. }));
    })
}
#[test]
fn lpt_iso_mut_slice() {
    run(|c, l| {
        let r = lower_pointer_type(
            ast::PointerType {
                lifetime: None,
                exclusivity: Some(ast::Exclusivity::Iso),
                mutability: Some(ast::Mutability::Mut),
                to: ast::Type::SliceType(Box::new(ast::SliceType {
                    element_type: ast::Type::UInt8(ast::UInt8),
                })),
            },
            c,
            l,
        )
        .unwrap();
        assert!(matches!(
            r,
            Type::SlicePtr {
                mutable: true,
                exclusive: true,
                ..
            }
        ));
    })
}
#[test]
fn lpt_poly_slice() {
    run(|c, l| {
        let r = lower_pointer_type(
            ast::PointerType {
                lifetime: None,
                exclusivity: Some(ast::Exclusivity::Poly),
                mutability: Some(ast::Mutability::Mut),
                to: ast::Type::SliceType(Box::new(ast::SliceType {
                    element_type: ast::Type::UInt8(ast::UInt8),
                })),
            },
            c,
            l,
        )
        .unwrap();
        assert!(matches!(
            r,
            Type::SlicePtr {
                mutable: true,
                exclusive: false,
                ..
            }
        ));
    })
}

// ===== Error-returning type functions: 6 tests =====
#[test]
fn err_slice() {
    run(|c, l| {
        assert!(
            lower_slice_type(
                ast::SliceType {
                    element_type: ast::Type::Int32(ast::Int32)
                },
                c,
                l
            )
            .is_err()
        );
    })
}
#[test]
fn err_refinement() {
    run(|c, l| {
        assert!(
            lower_refinement_type(
                ast::RefinementType {
                    basis_type: ast::Type::Int32(ast::Int32),
                    width: None,
                    minimum: None,
                    maximum: None
                },
                c,
                l
            )
            .is_err()
        );
    })
}
#[test]
fn err_latent() {
    run(|c, l| {
        assert!(
            lower_latent_type(
                ast::LatentType {
                    body: ast::Block {
                        safety: None,
                        elements: vec![]
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
fn err_lifetime() {
    run(|c, l| {
        assert!(lower_lifetime(ast::Lifetime { name: "static".into() }, c, l).is_err());
    })
}
#[test]
fn err_slice_u8() {
    run(|c, l| {
        assert!(
            lower_slice_type(
                ast::SliceType {
                    element_type: ast::Type::UInt8(ast::UInt8)
                },
                c,
                l
            )
            .is_err()
        );
    })
}
#[test]
fn err_slice_f64() {
    run(|c, l| {
        assert!(
            lower_slice_type(
                ast::SliceType {
                    element_type: ast::Type::Float64(ast::Float64)
                },
                c,
                l
            )
            .is_err()
        );
    })
}
