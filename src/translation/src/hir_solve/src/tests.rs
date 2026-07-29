use crate::bounds::{
    check_bounds_against_constraint, check_literal_against_refinement, compute_binary_bounds, compute_unary_bounds,
    extract_bounds_from_type, integer_primitive_bounds, lit_to_i128,
};
use crate::constraints::{
    NodeAction, TypeConstraint, is_arithmetic_op, is_comparison_or_logical_op, propagate_to_children,
};
use crate::diagnosis::TypeErr;
use crate::solver::{MAX_MONO_DEPTH, MonoCacheKey, Solver, resolve_function, resolve_global};
use crate::substitution::Substitution;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::{
    BinaryOp, Block, BlockElement, BlockId, BlockSafety, Function, FunctionId, Lit, LocalKind, LocalVariable,
    LocalVariableId, Parameter, ParameterId, PtrSize, Store, StructDef, StructDefId, StructField,
    StructMemoryLayoutCell, SymbolTab, Type, TypeId, UnaryOp, Value, ValueId, Visibility, using_storage,
};
use nitrate_nstring::NString;
use nitrate_tree::ByteSpan;
use ordered_float::OrderedFloat;
use std::collections::{BTreeMap, BTreeSet, HashSet};
use thin_vec::ThinVec;

fn span(s: u32, e: u32) -> ByteSpan {
    ByteSpan { start: s, end: e }
}
fn store<R>(f: impl FnOnce(&Store) -> R) -> R {
    let s = Store::new();
    using_storage(&s, || f(&s))
}
fn symtab() -> SymbolTab {
    SymbolTab::new(PtrSize::U64)
}
fn i32t() -> TypeId {
    TypeId::from(Type::I32 {
        span: ByteSpan::default(),
    })
}
fn i64t() -> TypeId {
    TypeId::from(Type::I64 {
        span: ByteSpan::default(),
    })
}
fn u8t() -> TypeId {
    TypeId::from(Type::U8 {
        span: ByteSpan::default(),
    })
}
fn u16t() -> TypeId {
    TypeId::from(Type::U16 {
        span: ByteSpan::default(),
    })
}
fn u32t() -> TypeId {
    TypeId::from(Type::U32 {
        span: ByteSpan::default(),
    })
}
fn u64t() -> TypeId {
    TypeId::from(Type::U64 {
        span: ByteSpan::default(),
    })
}
fn u128t() -> TypeId {
    TypeId::from(Type::U128 {
        span: ByteSpan::default(),
    })
}
fn uszt() -> TypeId {
    TypeId::from(Type::USize {
        span: ByteSpan::default(),
    })
}
fn i8t() -> TypeId {
    TypeId::from(Type::I8 {
        span: ByteSpan::default(),
    })
}
fn i16t() -> TypeId {
    TypeId::from(Type::I16 {
        span: ByteSpan::default(),
    })
}
fn i128t() -> TypeId {
    TypeId::from(Type::I128 {
        span: ByteSpan::default(),
    })
}
fn f64t() -> TypeId {
    TypeId::from(Type::F64 {
        span: ByteSpan::default(),
    })
}
fn boolt() -> TypeId {
    TypeId::from(Type::Bool {
        span: ByteSpan::default(),
    })
}
fn unitt() -> TypeId {
    TypeId::from(Type::Unit {
        span: ByteSpan::default(),
    })
}
fn sv(v: Value) -> ValueId {
    ValueId::from(v)
}
fn i32v(v: i32) -> ValueId {
    sv(Value::I32 {
        span: ByteSpan::default(),
        value: v,
    })
}
fn boolv(v: bool) -> ValueId {
    sv(Value::Bool {
        span: ByteSpan::default(),
        value: v,
    })
}
fn inf_int(v: u128) -> ValueId {
    sv(Value::InferredInteger {
        span: ByteSpan::default(),
        value: Box::new(v),
    })
}
fn inf_flt(v: f64) -> ValueId {
    sv(Value::InferredFloat {
        span: ByteSpan::default(),
        value: OrderedFloat(v),
    })
}
fn param(n: &str, ty: TypeId) -> ParameterId {
    ParameterId::from(Parameter {
        span: ByteSpan::default(),
        attributes: BTreeSet::new(),
        is_mutable: false,
        name: NString::from(n),
        ty,
        default_value: None,
    })
}
fn local(n: &str, ty: TypeId, init: ValueId) -> LocalVariableId {
    LocalVariableId::from(LocalVariable {
        span: ByteSpan::default(),
        kind: LocalKind::Let,
        attributes: BTreeSet::new(),
        is_mutable: false,
        name: NString::from(n),
        ty,
        initializer: init,
    })
}
fn mkfunc(
    name: &str,
    params: Vec<ParameterId>,
    rt: TypeId,
    body: Option<Vec<BlockElement>>,
    gens: Option<BTreeMap<NString, Option<TypeId>>>,
) -> FunctionId {
    FunctionId::from(Function {
        span: ByteSpan::default(),
        visibility: Visibility::Pub,
        attributes: BTreeSet::new(),
        name: NString::from(name),
        mangled_name: NString::from(name),
        generics: gens,
        params,
        return_type: rt,
        body,
    })
}
fn lit(l: Lit) -> nitrate_hir::LiteralId {
    nitrate_hir::LiteralId::from(l)
}

fn mkstruct(name: &str, fields: Vec<(&str, TypeId)>, generics: Option<Vec<&str>>) -> StructDefId {
    let fmap: BTreeMap<NString, StructField> = fields
        .into_iter()
        .map(|(n, t)| {
            (
                NString::from(n),
                StructField {
                    span: ByteSpan::default(),
                    visibility: Visibility::Pub,
                    attributes: BTreeSet::new(),
                    name: NString::from(n),
                    ty: t,
                    default_value: None,
                },
            )
        })
        .collect();
    let layout: Vec<StructMemoryLayoutCell> = fmap
        .keys()
        .map(|n| StructMemoryLayoutCell::Field { field_name: n.clone() })
        .collect();
    let gmap = generics.map(|g| {
        let mut m = BTreeMap::new();
        for (i, gn) in g.into_iter().enumerate() {
            m.insert(
                NString::from(gn),
                Some(TypeId::from(Type::GenericParam {
                    span: ByteSpan::default(),
                    index: i as u32,
                    name: NString::from(gn),
                })),
            );
        }
        m
    });
    StructDefId::from(StructDef {
        span: ByteSpan::default(),
        visibility: Visibility::Pub,
        name: NString::from(name),
        attributes: BTreeSet::new(),
        fields: fmap,
        generics: gmap,
        layout: layout.into(),
    })
}

// ── Bounds tests ────────────────────────────────────────────────────────

#[test]
fn test_int_bounds_all() {
    store(|_| {
        assert_eq!(integer_primitive_bounds(&*u8t()), Some((0, 255)));
        assert_eq!(integer_primitive_bounds(&*u16t()), Some((0, 65535)));
        assert_eq!(integer_primitive_bounds(&*u32t()), Some((0, 4294967295)));
        assert_eq!(integer_primitive_bounds(&*u64t()), Some((0, 18446744073709551615)));
        assert_eq!(integer_primitive_bounds(&*u128t()), Some((0, i128::MAX)));
        assert_eq!(integer_primitive_bounds(&*uszt()), Some((0, 18446744073709551615)));
        assert_eq!(integer_primitive_bounds(&*i8t()), Some((-128, 127)));
        assert_eq!(integer_primitive_bounds(&*i16t()), Some((-32768, 32767)));
        assert_eq!(integer_primitive_bounds(&*i32t()), Some((-2147483648, 2147483647)));
        assert_eq!(
            integer_primitive_bounds(&*i64t()),
            Some((-9223372036854775808, 9223372036854775807))
        );
        assert_eq!(integer_primitive_bounds(&*i128t()), Some((i128::MIN, i128::MAX)));
        assert_eq!(integer_primitive_bounds(&*boolt()), None);
        assert_eq!(integer_primitive_bounds(&*unitt()), None);
        assert_eq!(integer_primitive_bounds(&*f64t()), None);
    });
}

#[test]
fn test_extract_bounds() {
    store(|_| {
        let r = TypeId::from(Type::Refine {
            span: ByteSpan::default(),
            base: i32t(),
            min: lit(Lit::I32(0)),
            max: lit(Lit::I32(100)),
        });
        assert_eq!(extract_bounds_from_type(&*r), Some((0, 100)));
        assert_eq!(extract_bounds_from_type(&*i32t()), Some((-2147483648, 2147483647)));
        assert_eq!(extract_bounds_from_type(&*boolt()), None);
    });
}

#[test]
fn test_lit_to_i128() {
    store(|_| {
        assert_eq!(lit_to_i128(&Lit::U8(42)), Some(42));
        assert_eq!(lit_to_i128(&Lit::U16(300)), Some(300));
        assert_eq!(lit_to_i128(&Lit::I8(-5)), Some(-5));
        assert_eq!(lit_to_i128(&Lit::I128(i128::MIN + 1)), Some(i128::MIN + 1));
        assert_eq!(lit_to_i128(&Lit::Unit), None);
        assert_eq!(lit_to_i128(&Lit::Bool(true)), None);
    });
}

#[test]
fn test_binary_bounds() {
    store(|_| {
        assert_eq!(compute_binary_bounds(&BinaryOp::Add, (1, 10), (20, 30)), Some((21, 40)));
        assert_eq!(compute_binary_bounds(&BinaryOp::Sub, (10, 20), (1, 5)), Some((5, 19)));
        assert_eq!(compute_binary_bounds(&BinaryOp::Mul, (-5, 5), (-5, 5)), Some((-25, 25)));
        assert_eq!(compute_binary_bounds(&BinaryOp::Div, (10, 20), (2, 5)), Some((2, 10)));
        assert_eq!(
            compute_binary_bounds(&BinaryOp::Div, (10, 10), (0, 0)),
            Some((i128::MIN, i128::MAX))
        );
        assert_eq!(compute_binary_bounds(&BinaryOp::Mod, (0, 100), (10, 10)), Some((0, 9)));
        assert_eq!(
            compute_binary_bounds(&BinaryOp::And, (0, 255), (0, 255)),
            Some((0, 255))
        );
        assert_eq!(compute_binary_bounds(&BinaryOp::Or, (0, 8), (0, 4)), Some((0, 8)));
        assert_eq!(compute_binary_bounds(&BinaryOp::Xor, (0, 8), (0, 4)), Some((0, 8)));
        assert_eq!(
            compute_binary_bounds(&BinaryOp::Shl, (1, 1), (1, 1)),
            Some((i128::MIN, i128::MAX))
        );
        assert_eq!(compute_binary_bounds(&BinaryOp::Shr, (8, 64), (1, 3)), Some((1, 32)));
        assert_eq!(compute_binary_bounds(&BinaryOp::Lt, (0, 10), (0, 10)), None);
    });
}

#[test]
fn test_unary_bounds() {
    store(|_| {
        assert_eq!(compute_unary_bounds(&UnaryOp::Add, (-5, 10)), (-5, 10));
        assert_eq!(compute_unary_bounds(&UnaryOp::Sub, (-5, 10)), (-10, 5));
        assert_eq!(compute_unary_bounds(&UnaryOp::Not, (0, 255)), (!255, !0));
    });
}

#[test]
fn test_check_bounds() {
    store(|_| {
        let r = TypeId::from(Type::Refine {
            span: ByteSpan::default(),
            base: i32t(),
            min: lit(Lit::I32(0)),
            max: lit(Lit::I32(100)),
        });
        assert!(check_bounds_against_constraint((10, 50), &*r));
        assert!(!check_bounds_against_constraint((10, 200), &*r));
        assert!(check_bounds_against_constraint((10, 200), &*i32t()));
    });
}

#[test]
fn test_check_literal_refine() {
    store(|_| {
        let r = TypeId::from(Type::Refine {
            span: ByteSpan::default(),
            base: i32t(),
            min: lit(Lit::I32(0)),
            max: lit(Lit::I32(100)),
        });
        assert!(check_literal_against_refinement(42, &*r));
        assert!(!check_literal_against_refinement(200, &*r));
        assert!(check_literal_against_refinement(999, &*i32t()));
    });
}

#[test]
fn test_type_constraint() {
    store(|_| {
        let eq = TypeConstraint::Equal(i32t());
        assert_eq!(eq.type_id(), i32t());
        let sub = TypeConstraint::SubtypeOf(u32t());
        assert_eq!(sub.type_id(), u32t());
    });
}

#[test]
fn test_node_action() {
    let _ = NodeAction::NoChange;
    store(|_| {
        let r = NodeAction::Replace(Value::Unit {
            span: ByteSpan::default(),
        });
        assert!(matches!(r, NodeAction::Replace(..)));
    });
}

#[test]
fn test_is_ops() {
    store(|_| {
        assert!(is_comparison_or_logical_op(&BinaryOp::Lt));
        assert!(is_comparison_or_logical_op(&BinaryOp::Eq));
        assert!(!is_comparison_or_logical_op(&BinaryOp::Add));
        assert!(is_arithmetic_op(&BinaryOp::Add));
        assert!(is_arithmetic_op(&BinaryOp::Mul));
        assert!(!is_arithmetic_op(&BinaryOp::Lt));
    });
}

#[test]
fn test_propagate() {
    store(|_| {
        let mut s = HashSet::new();
        s.insert(TypeConstraint::Equal(i32t()));
        s.insert(TypeConstraint::SubtypeOf(u32t()));
        let p = propagate_to_children(&s);
        assert_eq!(p.len(), 2);
        for c in &p {
            assert!(matches!(c, TypeConstraint::Equal(..)));
        }
    });
}

// ── Substitution tests ──────────────────────────────────────────────────

#[test]
fn test_subst_generic() {
    store(|_| {
        let mut sub = Substitution::default();
        sub.mapping.insert(0, i32t());
        let r = sub.apply(&Type::GenericParam {
            span: ByteSpan::default(),
            index: 0,
            name: NString::from("T"),
        });
        assert!(matches!(r, Type::I32 { .. }));
        let r2 = sub.apply(&Type::GenericParam {
            span: ByteSpan::default(),
            index: 99,
            name: NString::from("Z"),
        });
        assert!(matches!(r2, Type::GenericParam { index: 99, .. }));
    });
}

#[test]
fn test_subst_inferred() {
    store(|_| {
        let mut sub = Substitution::default();
        sub.mapping.insert(5, i64t());
        let r = sub.apply(&Type::Inferred {
            span: ByteSpan::default(),
            id: std::num::NonZeroU32::new(6).unwrap(),
            name: None,
        });
        assert!(matches!(r, Type::Inferred { .. }));
        let r = sub.apply(&Type::Inferred {
            span: ByteSpan::default(),
            id: std::num::NonZeroU32::new(5).unwrap(),
            name: None,
        });
        assert!(matches!(r, Type::I64 { .. }));
    });
}

#[test]
fn test_subst_compound() {
    store(|_| {
        let mut sub = Substitution::default();
        sub.mapping.insert(0, u8t());
        let r = sub.apply(&Type::Array {
            span: ByteSpan::default(),
            element_type: TypeId::from(Type::GenericParam {
                span: ByteSpan::default(),
                index: 0,
                name: NString::from("T"),
            }),
            len: 10,
        });
        assert!(matches!(r, Type::Array { len: 10, .. }));
        let r2 = sub.apply(&Type::Struct {
            span: ByteSpan::default(),
            def: mkstruct("S", vec![("x", i32t())], None),
        });
        assert!(matches!(r2, Type::Struct { .. }));
    });
}

#[test]
fn test_subst_leaf() {
    store(|_| {
        let sub = Substitution::default();
        let leaves = vec![
            Type::Never {
                span: ByteSpan::default(),
            },
            Type::Unit {
                span: ByteSpan::default(),
            },
            Type::Bool {
                span: ByteSpan::default(),
            },
            Type::U8 {
                span: ByteSpan::default(),
            },
            Type::I32 {
                span: ByteSpan::default(),
            },
            Type::InferredInteger {
                span: ByteSpan::default(),
            },
            Type::InferredFloat {
                span: ByteSpan::default(),
            },
        ];
        for leaf in leaves {
            assert_eq!(sub.apply(&leaf), leaf);
        }
    });
}

#[test]
fn test_mono_cache_key() {
    store(|_| {
        let a = MonoCacheKey::new(42, &[(0, i32t()), (1, u32t())]);
        let b = MonoCacheKey::new(42, &[(0, i32t()), (1, u32t())]);
        let c = MonoCacheKey::new(43, &[(0, i32t()), (1, u32t())]);
        assert_eq!(a, b);
        assert_ne!(a, c);
    });
}

#[test]
fn test_unify() {
    store(|_| {
        let mut sub = Substitution::default();
        Solver::unify_types_with_subst(
            &Type::I32 {
                span: ByteSpan::default(),
            },
            &Type::GenericParam {
                span: ByteSpan::default(),
                index: 0,
                name: NString::from("T"),
            },
            &mut sub,
        );
        assert!(sub.mapping.contains_key(&0));
    });
}

#[test]
fn test_unify_nested() {
    store(|_| {
        let mut sub = Substitution::default();
        Solver::unify_types_with_subst(
            &Type::Pointer {
                span: ByteSpan::default(),
                lifetime: nitrate_hir::Lifetime::Inferred,
                exclusive: false,
                mutable: false,
                to: TypeId::from(Type::I32 {
                    span: ByteSpan::default(),
                }),
            },
            &Type::Pointer {
                span: ByteSpan::default(),
                lifetime: nitrate_hir::Lifetime::Inferred,
                exclusive: false,
                mutable: false,
                to: TypeId::from(Type::GenericParam {
                    span: ByteSpan::default(),
                    index: 0,
                    name: NString::from("T"),
                }),
            },
            &mut sub,
        );
        assert!(sub.mapping.contains_key(&0));
    });
}

// ── Solver integration tests ────────────────────────────────────────────

#[test]
fn test_resolve_function_simple() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = symtab();
        let mut f = Function {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            name: NString::from("f"),
            mangled_name: NString::from("f"),
            generics: None,
            params: vec![],
            return_type: i32t(),
            body: Some(vec![BlockElement::Expr(i32v(42))]),
        };
        assert!(resolve_function(&mut f, &mut s, &log).is_ok());
    });
}

#[test]
fn test_resolve_inferred_int() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = symtab();
        let mut f = Function {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            name: NString::from("f"),
            mangled_name: NString::from("f"),
            generics: None,
            params: vec![],
            return_type: i32t(),
            body: Some(vec![BlockElement::Expr(inf_int(42))]),
        };
        assert!(resolve_function(&mut f, &mut s, &log).is_ok());
        if let Some(BlockElement::Expr(last)) = f.body.as_ref().and_then(|b| b.last()) {
            assert!(matches!(&*last.borrow(), Value::I32 { value: 42, .. }));
        }
    });
}

#[test]
fn test_resolve_inferred_float() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = symtab();
        let mut f = Function {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            name: NString::from("f"),
            mangled_name: NString::from("f"),
            generics: None,
            params: vec![],
            return_type: f64t(),
            body: Some(vec![BlockElement::Expr(inf_flt(3.14))]),
        };
        assert!(resolve_function(&mut f, &mut s, &log).is_ok());
    });
}

#[test]
fn test_resolve_binary() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = symtab();
        let bin = sv(Value::Binary {
            span: ByteSpan::default(),
            left: i32v(10),
            op: BinaryOp::Add,
            right: i32v(20),
        });
        let mut f = Function {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            name: NString::from("f"),
            mangled_name: NString::from("f"),
            generics: None,
            params: vec![],
            return_type: i32t(),
            body: Some(vec![BlockElement::Expr(bin)]),
        };
        assert!(resolve_function(&mut f, &mut s, &log).is_ok());
    });
}

#[test]
fn test_resolve_comparison() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = symtab();
        let cmp = sv(Value::Binary {
            span: ByteSpan::default(),
            left: i32v(10),
            op: BinaryOp::Lt,
            right: i32v(20),
        });
        let mut f = Function {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            name: NString::from("f"),
            mangled_name: NString::from("f"),
            generics: None,
            params: vec![],
            return_type: boolt(),
            body: Some(vec![BlockElement::Expr(cmp)]),
        };
        assert!(resolve_function(&mut f, &mut s, &log).is_ok());
    });
}

#[test]
fn test_resolve_local() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = symtab();
        let l = local("x", i32t(), i32v(42));
        let mut f = Function {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            name: NString::from("f"),
            mangled_name: NString::from("f"),
            generics: None,
            params: vec![],
            return_type: unitt(),
            body: Some(vec![BlockElement::Local(l)]),
        };
        assert!(resolve_function(&mut f, &mut s, &log).is_ok());
    });
}

#[test]
fn test_resolve_param_return() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = symtab();
        let p = param("x", i32t());
        let ps = sv(Value::ParameterSymbol {
            span: ByteSpan::default(),
            id: p.clone(),
        });
        s.add_parameter(p.clone());
        let r = sv(Value::Return {
            span: ByteSpan::default(),
            value: ps,
        });
        let mut f = Function {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            name: NString::from("f"),
            mangled_name: NString::from("f"),
            generics: None,
            params: vec![p],
            return_type: i32t(),
            body: Some(vec![BlockElement::Expr(r)]),
        };
        assert!(resolve_function(&mut f, &mut s, &log).is_ok());
    });
}

#[test]
fn test_resolve_cast() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = symtab();
        let c = sv(Value::Cast {
            span: ByteSpan::default(),
            value: i32v(42),
            target_type: i64t(),
        });
        let mut f = Function {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            name: NString::from("f"),
            mangled_name: NString::from("f"),
            generics: None,
            params: vec![],
            return_type: i64t(),
            body: Some(vec![BlockElement::Expr(c)]),
        };
        assert!(resolve_function(&mut f, &mut s, &log).is_ok());
    });
}

#[test]
fn test_resolve_if() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = symtab();
        let tb = Block {
            span: ByteSpan::default(),
            safety: BlockSafety::Safe,
            elements: vec![BlockElement::Expr(i32v(1))],
        };
        let fb = Block {
            span: ByteSpan::default(),
            safety: BlockSafety::Safe,
            elements: vec![BlockElement::Expr(i32v(2))],
        };
        let iv = sv(Value::If {
            span: ByteSpan::default(),
            condition: boolv(true),
            true_branch: BlockId::from(tb),
            false_branch: Some(BlockId::from(fb)),
        });
        let mut f = Function {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            name: NString::from("f"),
            mangled_name: NString::from("f"),
            generics: None,
            params: vec![],
            return_type: i32t(),
            body: Some(vec![BlockElement::Expr(iv)]),
        };
        assert!(resolve_function(&mut f, &mut s, &log).is_ok());
    });
}

#[test]
fn test_resolve_mismatched_if() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = symtab();
        let tb = Block {
            span: ByteSpan::default(),
            safety: BlockSafety::Safe,
            elements: vec![BlockElement::Expr(i32v(1))],
        };
        let fe = sv(Value::F64 {
            span: ByteSpan::default(),
            value: OrderedFloat(2.0),
        });
        let fb = Block {
            span: ByteSpan::default(),
            safety: BlockSafety::Safe,
            elements: vec![BlockElement::Expr(fe)],
        };
        let iv = sv(Value::If {
            span: ByteSpan::default(),
            condition: boolv(true),
            true_branch: BlockId::from(tb),
            false_branch: Some(BlockId::from(fb)),
        });
        let mut f = Function {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            name: NString::from("f"),
            mangled_name: NString::from("f"),
            generics: None,
            params: vec![],
            return_type: unitt(),
            body: Some(vec![BlockElement::Expr(iv)]),
        };
        assert!(resolve_function(&mut f, &mut s, &log).is_err());
    });
}

#[test]
fn test_resolve_unary() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = symtab();
        let u = sv(Value::Unary {
            span: ByteSpan::default(),
            op: UnaryOp::Sub,
            operand: i32v(5),
        });
        let mut f = Function {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            name: NString::from("f"),
            mangled_name: NString::from("f"),
            generics: None,
            params: vec![],
            return_type: i32t(),
            body: Some(vec![BlockElement::Expr(u)]),
        };
        assert!(resolve_function(&mut f, &mut s, &log).is_ok());
    });
}

#[test]
fn test_resolve_block() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = symtab();
        let b = Block {
            span: ByteSpan::default(),
            safety: BlockSafety::Safe,
            elements: vec![BlockElement::Expr(i32v(7))],
        };
        let bv = sv(Value::Block {
            span: ByteSpan::default(),
            block: BlockId::from(b),
        });
        let mut f = Function {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            name: NString::from("f"),
            mangled_name: NString::from("f"),
            generics: None,
            params: vec![],
            return_type: i32t(),
            body: Some(vec![BlockElement::Expr(bv)]),
        };
        assert!(resolve_function(&mut f, &mut s, &log).is_ok());
    });
}

#[test]
fn test_resolve_list() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = symtab();
        let l = sv(Value::List {
            span: ByteSpan::default(),
            elements: vec![i32v(1), i32v(2)].into(),
        });
        let mut f = Function {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            name: NString::from("f"),
            mangled_name: NString::from("f"),
            generics: None,
            params: vec![],
            return_type: unitt(),
            body: Some(vec![BlockElement::Expr(l)]),
        };
        assert!(resolve_function(&mut f, &mut s, &log).is_ok());
    });
}

#[test]
fn test_resolve_tuple() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = symtab();
        let t = sv(Value::Tuple {
            span: ByteSpan::default(),
            elements: vec![i32v(1), boolv(true)].into(),
        });
        let mut f = Function {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            name: NString::from("f"),
            mangled_name: NString::from("f"),
            generics: None,
            params: vec![],
            return_type: unitt(),
            body: Some(vec![BlockElement::Expr(t)]),
        };
        assert!(resolve_function(&mut f, &mut s, &log).is_ok());
    });
}

#[test]
fn test_resolve_while() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = symtab();
        let bb = Block {
            span: ByteSpan::default(),
            safety: BlockSafety::Safe,
            elements: vec![BlockElement::Expr(i32v(1))],
        };
        let w = sv(Value::While {
            span: ByteSpan::default(),
            condition: boolv(true),
            body: BlockId::from(bb),
        });
        let mut f = Function {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            name: NString::from("f"),
            mangled_name: NString::from("f"),
            generics: None,
            params: vec![],
            return_type: unitt(),
            body: Some(vec![BlockElement::Expr(w)]),
        };
        assert!(resolve_function(&mut f, &mut s, &log).is_ok());
    });
}

#[test]
fn test_resolve_loop() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = symtab();
        let bb = Block {
            span: ByteSpan::default(),
            safety: BlockSafety::Safe,
            elements: vec![BlockElement::Expr(i32v(1))],
        };
        let l = sv(Value::Loop {
            span: ByteSpan::default(),
            body: BlockId::from(bb),
        });
        let mut f = Function {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            name: NString::from("f"),
            mangled_name: NString::from("f"),
            generics: None,
            params: vec![],
            return_type: unitt(),
            body: Some(vec![BlockElement::Expr(l)]),
        };
        assert!(resolve_function(&mut f, &mut s, &log).is_ok());
    });
}

#[test]
fn test_resolve_break_continue() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = symtab();
        let bv = sv(Value::Break {
            span: ByteSpan::default(),
            label: None,
        });
        let cv = sv(Value::Continue {
            span: ByteSpan::default(),
            label: None,
        });
        let bb = Block {
            span: ByteSpan::default(),
            safety: BlockSafety::Safe,
            elements: vec![BlockElement::Expr(bv), BlockElement::Expr(cv)],
        };
        let l = sv(Value::Loop {
            span: ByteSpan::default(),
            body: BlockId::from(bb),
        });
        let mut f = Function {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            name: NString::from("f"),
            mangled_name: NString::from("f"),
            generics: None,
            params: vec![],
            return_type: unitt(),
            body: Some(vec![BlockElement::Expr(l)]),
        };
        assert!(resolve_function(&mut f, &mut s, &log).is_ok());
    });
}

#[test]
fn test_resolve_struct() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = symtab();
        let sd = mkstruct("Point", vec![("x", i32t()), ("y", i32t())], None);
        s.add_struct(sd);
        let sd2 = mkstruct("Point", vec![("x", i32t()), ("y", i32t())], None);
        let fields: ThinVec<(NString, ValueId)> =
            vec![(NString::from("x"), i32v(10)), (NString::from("y"), i32v(20))].into();
        let so = sv(Value::StructObject {
            span: ByteSpan::default(),
            struct_def: sd2,
            fields,
        });
        let mut f = Function {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            name: NString::from("f"),
            mangled_name: NString::from("f"),
            generics: None,
            params: vec![],
            return_type: unitt(),
            body: Some(vec![BlockElement::Expr(so)]),
        };
        assert!(resolve_function(&mut f, &mut s, &log).is_ok());
    });
}

#[test]
fn test_resolve_generic_struct() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = symtab();
        let sd = mkstruct(
            "Pair",
            vec![
                (
                    "first",
                    TypeId::from(Type::GenericParam {
                        span: ByteSpan::default(),
                        index: 0,
                        name: NString::from("T"),
                    }),
                ),
                (
                    "second",
                    TypeId::from(Type::GenericParam {
                        span: ByteSpan::default(),
                        index: 0,
                        name: NString::from("T"),
                    }),
                ),
            ],
            Some(vec!["T"]),
        );
        let fields: ThinVec<(NString, ValueId)> =
            vec![(NString::from("first"), i32v(1)), (NString::from("second"), i32v(2))].into();
        let so = sv(Value::StructObject {
            span: ByteSpan::default(),
            struct_def: sd,
            fields,
        });
        let mut f = Function {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            name: NString::from("f"),
            mangled_name: NString::from("f"),
            generics: None,
            params: vec![],
            return_type: unitt(),
            body: Some(vec![BlockElement::Expr(so)]),
        };
        assert!(resolve_function(&mut f, &mut s, &log).is_ok());
    });
}

#[test]
fn test_resolve_index_access() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = symtab();
        let list = sv(Value::List {
            span: ByteSpan::default(),
            elements: vec![i32v(1), i32v(2)].into(),
        });
        let idx = sv(Value::USize {
            span: ByteSpan::default(),
            bits: 64,
            value: 0,
        });
        let ia = sv(Value::IndexAccess {
            span: ByteSpan::default(),
            collection: list,
            index: idx,
        });
        let mut f = Function {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            name: NString::from("f"),
            mangled_name: NString::from("f"),
            generics: None,
            params: vec![],
            return_type: i32t(),
            body: Some(vec![BlockElement::Expr(ia)]),
        };
        assert!(resolve_function(&mut f, &mut s, &log).is_ok());
    });
}

#[test]
fn test_resolve_assign() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = symtab();
        let lv = local("x", i32t(), i32v(0));
        let ls = sv(Value::LocalVariableSymbol {
            span: ByteSpan::default(),
            id: lv.clone(),
        });
        let a = sv(Value::Assign {
            span: ByteSpan::default(),
            place: ls,
            value: i32v(42),
        });
        let mut f = Function {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            name: NString::from("f"),
            mangled_name: NString::from("f"),
            generics: None,
            params: vec![],
            return_type: unitt(),
            body: Some(vec![BlockElement::Local(lv), BlockElement::Expr(a)]),
        };
        assert!(resolve_function(&mut f, &mut s, &log).is_ok());
    });
}

#[test]
fn test_resolve_borrow_deref() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = symtab();
        let br = sv(Value::Borrow {
            span: ByteSpan::default(),
            exclusive: false,
            mutable: false,
            place: i32v(42),
        });
        let dr = sv(Value::Deref {
            span: ByteSpan::default(),
            place: br,
        });
        let mut f = Function {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            name: NString::from("f"),
            mangled_name: NString::from("f"),
            generics: None,
            params: vec![],
            return_type: i32t(),
            body: Some(vec![BlockElement::Expr(dr)]),
        };
        assert!(resolve_function(&mut f, &mut s, &log).is_ok());
    });
}

#[test]
fn test_resolve_global_var() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = symtab();
        let mut g = nitrate_hir::GlobalVariable {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            is_mutable: false,
            name: NString::from("C"),
            mangled_name: NString::from("C"),
            ty: TypeId::from(Type::Inferred {
                span: ByteSpan::default(),
                id: std::num::NonZeroU32::new(1).unwrap(),
                name: None,
            }),
            initializer: i32v(42),
        };
        assert!(resolve_global(&mut g, &mut s, &log).is_ok());
        let mut g2 = nitrate_hir::GlobalVariable {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            is_mutable: false,
            name: NString::from("M"),
            mangled_name: NString::from("M"),
            ty: i32t(),
            initializer: i32v(99),
        };
        assert!(resolve_global(&mut g2, &mut s, &log).is_ok());
    });
}

#[test]
fn test_solver_new() {
    store(|_| {
        let mut s = symtab();
        let solver = Solver::new(&mut s);
        assert!(solver.constraints.is_empty());
        assert!(solver.errors.is_empty());
        assert_eq!(solver.mono_counter, 0);
        assert_eq!(solver.mono_depth, 0);
    });
}

#[test]
fn test_solver_monomorphize() {
    store(|_| {
        let mut s = symtab();
        let p = param(
            "x",
            TypeId::from(Type::GenericParam {
                span: ByteSpan::default(),
                index: 0,
                name: NString::from("T"),
            }),
        );
        let bv = sv(Value::ParameterSymbol {
            span: ByteSpan::default(),
            id: p.clone(),
        });
        let fid = mkfunc(
            "gf",
            vec![p],
            TypeId::from(Type::GenericParam {
                span: ByteSpan::default(),
                index: 0,
                name: NString::from("T"),
            }),
            Some(vec![BlockElement::Expr(bv)]),
            Some({
                let mut m = BTreeMap::new();
                m.insert(
                    NString::from("T"),
                    Some(TypeId::from(Type::GenericParam {
                        span: ByteSpan::default(),
                        index: 0,
                        name: NString::from("T"),
                    })),
                );
                m
            }),
        );
        let mut sub = Substitution::default();
        sub.mapping.insert(0, i32t());
        let mut solver = Solver::new(&mut s);
        let mid = solver.monomorphize_function(&fid, &sub);
        let mf = mid.borrow();
        assert!(mf.name.contains("mono"));
        assert!(matches!(&*mf.return_type, Type::I32 { .. }));
        let sdd = mkstruct(
            "GP",
            vec![(
                "f",
                TypeId::from(Type::GenericParam {
                    span: ByteSpan::default(),
                    index: 0,
                    name: NString::from("T"),
                }),
            )],
            Some(vec!["T"]),
        );
        let mid2 = solver.monomorphize_struct(&sdd, &sub);
        let ms = mid2.borrow();
        assert!(ms.name.contains("mono"));
        assert!(ms.generics.is_none());
    });
}

#[test]
fn test_solver_mono_caching() {
    store(|_| {
        let mut s = symtab();
        let p = param(
            "x",
            TypeId::from(Type::GenericParam {
                span: ByteSpan::default(),
                index: 0,
                name: NString::from("T"),
            }),
        );
        let bv = sv(Value::ParameterSymbol {
            span: ByteSpan::default(),
            id: p.clone(),
        });
        let fid = mkfunc(
            "gf",
            vec![p],
            TypeId::from(Type::GenericParam {
                span: ByteSpan::default(),
                index: 0,
                name: NString::from("T"),
            }),
            Some(vec![BlockElement::Expr(bv)]),
            Some({
                let mut m = BTreeMap::new();
                m.insert(
                    NString::from("T"),
                    Some(TypeId::from(Type::GenericParam {
                        span: ByteSpan::default(),
                        index: 0,
                        name: NString::from("T"),
                    })),
                );
                m
            }),
        );
        let mut sub = Substitution::default();
        sub.mapping.insert(0, i32t());
        let mut solver = Solver::new(&mut s);
        let id1 = solver.monomorphize_function(&fid, &sub);
        let id2 = solver.monomorphize_function(&fid, &sub);
        assert_eq!(id1.as_usize(), id2.as_usize());
    });
}

#[test]
fn test_solver_mono_depth() {
    store(|_| {
        let mut s = symtab();
        let mut solver = Solver::new(&mut s);
        solver.mono_depth = MAX_MONO_DEPTH;
        let r = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
            solver.monomorphize_function(
                &mkfunc("f", vec![param("x", i32t())], i32t(), None, None),
                &Substitution::default(),
            );
        }));
        assert!(r.is_err());
    });
}

#[test]
fn test_solver_enum_variant() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = symtab();
        let ed = nitrate_hir::EnumDef {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            name: NString::from("E"),
            attributes: BTreeSet::new(),
            generics: None,
            variants: vec![nitrate_hir::EnumVariant {
                span: ByteSpan::default(),
                attributes: BTreeSet::new(),
                name: NString::from("A"),
                ty: i32t(),
                default_value: None,
            }]
            .into(),
        };
        let edi = nitrate_hir::EnumDefId::from(ed);
        let ev = sv(Value::EnumVariant {
            span: ByteSpan::default(),
            enum_def: edi,
            variant: NString::from("A"),
            value: i32v(42),
        });
        let mut f = Function {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            name: NString::from("f"),
            mangled_name: NString::from("f"),
            generics: None,
            params: vec![],
            return_type: unitt(),
            body: Some(vec![BlockElement::Expr(ev)]),
        };
        assert!(resolve_function(&mut f, &mut s, &log).is_ok());
    });
}

#[test]
fn test_diagnostic() {
    store(|_| {
        use nitrate_diagnosis::FormattableDiagnosticGroup;
        let e = TypeErr::IntegerLiteralOutOfRange {
            span: span(0, 5),
            value: 999,
            target_type: i32t(),
        };
        assert_eq!(e.group_id(), nitrate_diagnosis::DiagnosticGroupId::Type);
        assert_eq!(e.variant_id(), 0);
        assert!(e.format().message.contains("999"));
        let e2 = TypeErr::MismatchedBranchTypes {
            span: span(0, 10),
            true_type: i32t(),
            false_type: i64t(),
        };
        assert_eq!(e2.variant_id(), 5);
        assert!(e2.format().message.contains("i32"));
        let e3 = TypeErr::MethodNotFound {
            span: span(0, 1),
            method_name: "foo".into(),
            receiver_type: i32t(),
        };
        assert_eq!(e3.variant_id(), 14);
        assert!(e3.format().message.contains("foo"));
    });
}
