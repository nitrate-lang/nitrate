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
    Arguments, BinaryOp, Block, BlockElement, BlockId, BlockSafety, Function, FunctionId, GlobalVariableId, Lit,
    LocalKind, LocalVariable, LocalVariableId, Parameter, ParameterId, PtrSize, Store, StructDef, StructDefId,
    StructField, StructMemoryLayoutCell, SymbolTab, Type, TypeId, UnaryOp, Value, ValueId, Visibility, using_storage,
};
use nitrate_nstring::NString;
use nitrate_tree::ByteSpan;
use ordered_float::OrderedFloat;
use std::collections::{BTreeMap, BTreeSet, HashSet};
use std::vec;
use thin_vec::ThinVec;

fn span(s: u32, e: u32) -> ByteSpan {
    ByteSpan { start: s, end: e }
}
fn store<R>(f: impl FnOnce(&Store) -> R) -> R {
    let s = Store::new();
    using_storage(&s, || f(&s))
}
fn sym() -> SymbolTab {
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
fn f32t() -> TypeId {
    TypeId::from(Type::F32 {
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
fn f64v(v: f64) -> ValueId {
    sv(Value::F64 {
        span: ByteSpan::default(),
        value: OrderedFloat(v),
    })
}
fn i64v(v: i64) -> ValueId {
    sv(Value::I64 {
        span: ByteSpan::default(),
        value: v,
    })
}
fn i8v(v: i8) -> ValueId {
    sv(Value::I8 {
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
fn mkstruct(name: &str, fields: Vec<(&str, TypeId)>, generics: Option<Vec<&str>>, ns: Option<&str>) -> StructDefId {
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
    let full_name = if let Some(ns_val) = ns {
        format!("pkg::{}::{}", ns_val, name)
    } else {
        name.to_string()
    };
    let gmap = generics.map(|g| {
        let mut m = BTreeMap::new();
        for (i, gn) in g.into_iter().enumerate() {
            m.insert(NString::from(gn), Some(GP(i as u32, gn)));
        }
        m
    });
    StructDefId::from(StructDef {
        span: ByteSpan::default(),
        visibility: Visibility::Pub,
        name: NString::from(full_name),
        attributes: BTreeSet::new(),
        fields: fmap,
        generics: gmap,
        layout: layout.into(),
    })
}
fn GP(idx: u32, name: &str) -> TypeId {
    TypeId::from(Type::GenericParam {
        span: ByteSpan::default(),
        index: idx,
        name: NString::from(name),
    })
}
fn resolve_func(body: Vec<BlockElement>, rt: TypeId, log: &CompilerLog, sym: &mut SymbolTab) -> Result<(), ()> {
    let mut f = Function {
        span: ByteSpan::default(),
        visibility: Visibility::Pub,
        attributes: BTreeSet::new(),
        name: NString::from("f"),
        mangled_name: NString::from("f"),
        generics: None,
        params: vec![],
        return_type: rt,
        body: Some(body),
    };
    resolve_function(&mut f, sym, log)
}

// ═══ BOUNDS ═══

#[test]
fn test_bounds_int_primitive() {
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
        assert_eq!(integer_primitive_bounds(&*f64t()), None);
        assert_eq!(integer_primitive_bounds(&*f32t()), None);
    });
}

#[test]
fn test_bounds_extract() {
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
fn test_bounds_extract_refine_with_non_literal_bounds() {
    store(|_| {
        let r = TypeId::from(Type::Refine {
            span: ByteSpan::default(),
            base: i32t(),
            min: lit(Lit::Unit),
            max: lit(Lit::Unit),
        });
        assert_eq!(extract_bounds_from_type(&*r), None);
    });
}

#[test]
fn test_bounds_lit() {
    store(|_| {
        assert_eq!(lit_to_i128(&Lit::U8(42)), Some(42));
        assert_eq!(lit_to_i128(&Lit::U16(300)), Some(300));
        assert_eq!(lit_to_i128(&Lit::U32(70000)), Some(70000));
        assert_eq!(lit_to_i128(&Lit::U64(1 << 40)), Some(1 << 40));
        assert_eq!(lit_to_i128(&Lit::U128(1 << 50)), Some(1 << 50));
        assert_eq!(lit_to_i128(&Lit::USize(64, 12345)), Some(12345));
        assert_eq!(lit_to_i128(&Lit::I8(-5)), Some(-5));
        assert_eq!(lit_to_i128(&Lit::I16(-1000)), Some(-1000));
        assert_eq!(lit_to_i128(&Lit::I32(-100000)), Some(-100000));
        assert_eq!(lit_to_i128(&Lit::I64(-1 << 40)), Some(-(1 << 40)));
        assert_eq!(lit_to_i128(&Lit::I128(i128::MIN + 1)), Some(i128::MIN + 1));
        assert_eq!(lit_to_i128(&Lit::Unit), None);
        assert_eq!(lit_to_i128(&Lit::Bool(true)), None);
        assert_eq!(lit_to_i128(&Lit::F32(OrderedFloat(1.5))), None);
        assert_eq!(lit_to_i128(&Lit::F64(OrderedFloat(2.5))), None);
    });
}

#[test]
fn test_bounds_binary() {
    store(|_| {
        assert_eq!(compute_binary_bounds(&BinaryOp::Add, (1, 10), (20, 30)), Some((21, 40)));
        assert_eq!(compute_binary_bounds(&BinaryOp::Sub, (10, 20), (1, 5)), Some((5, 19)));
        assert_eq!(compute_binary_bounds(&BinaryOp::Mul, (-5, 5), (-5, 5)), Some((-25, 25)));
        assert_eq!(
            compute_binary_bounds(&BinaryOp::Mul, (i128::MAX, i128::MAX), (2, 2)),
            Some((i128::MAX, i128::MAX))
        );
        assert_eq!(compute_binary_bounds(&BinaryOp::Div, (10, 20), (2, 5)), Some((2, 10)));
        assert_eq!(
            compute_binary_bounds(&BinaryOp::Div, (10, 10), (-2, 2)),
            Some((-10, 10))
        );
        assert_eq!(
            compute_binary_bounds(&BinaryOp::Div, (10, 10), (0, 0)),
            Some((i128::MIN, i128::MAX))
        );
        assert_eq!(compute_binary_bounds(&BinaryOp::Div, (1, 1), (-2, 2)), Some((-1, 1)));
        assert_eq!(
            compute_binary_bounds(&BinaryOp::Div, (10, 20), (-10, -2)),
            Some((-10, -1))
        );
        assert_eq!(compute_binary_bounds(&BinaryOp::Mod, (0, 100), (10, 10)), Some((0, 9)));
        assert_eq!(
            compute_binary_bounds(&BinaryOp::Mod, (0, 100), (-10, 10)),
            Some((i128::MIN, i128::MAX))
        );
        assert_eq!(
            compute_binary_bounds(&BinaryOp::And, (0, 255), (0, 255)),
            Some((0, 255))
        );
        assert_eq!(
            compute_binary_bounds(&BinaryOp::And, (-128, 127), (0, 255)),
            Some((-128, 255))
        );
        assert_eq!(compute_binary_bounds(&BinaryOp::Or, (0, 8), (0, 4)), Some((0, 8)));
        assert_eq!(compute_binary_bounds(&BinaryOp::Xor, (0, 8), (0, 4)), Some((0, 8)));
        assert_eq!(
            compute_binary_bounds(&BinaryOp::Shl, (1, 1), (1, 1)),
            Some((i128::MIN, i128::MAX))
        );
        assert_eq!(compute_binary_bounds(&BinaryOp::Shr, (8, 64), (1, 3)), Some((1, 32)));
        assert_eq!(
            compute_binary_bounds(&BinaryOp::Shr, (8, 64), (-1, 3)),
            Some((i128::MIN, i128::MAX))
        );
        assert_eq!(compute_binary_bounds(&BinaryOp::Shr, (8, 64), (0, 3)), Some((1, 64)));
        assert_eq!(
            compute_binary_bounds(&BinaryOp::Rol, (1, 1), (1, 1)),
            Some((i128::MIN, i128::MAX))
        );
        assert_eq!(compute_binary_bounds(&BinaryOp::Ror, (8, 64), (1, 3)), Some((1, 32)));
        for op in &[
            BinaryOp::Lt,
            BinaryOp::Gt,
            BinaryOp::Lte,
            BinaryOp::Gte,
            BinaryOp::Eq,
            BinaryOp::Ne,
            BinaryOp::LogicAnd,
            BinaryOp::LogicOr,
        ] {
            assert_eq!(compute_binary_bounds(op, (0, 10), (0, 10)), None);
        }
    });
}

#[test]
fn test_bounds_unary() {
    store(|_| {
        assert_eq!(compute_unary_bounds(&UnaryOp::Add, (-5, 10)), (-5, 10));
        assert_eq!(compute_unary_bounds(&UnaryOp::Sub, (-5, 10)), (-10, 5));
        assert_eq!(compute_unary_bounds(&UnaryOp::Sub, (i128::MIN, 5)), (-5, i128::MAX));
        assert_eq!(compute_unary_bounds(&UnaryOp::Not, (0, 255)), (!255, !0));
    });
}

#[test]
fn test_bounds_check() {
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
        assert!(check_literal_against_refinement(42, &*r));
        assert!(!check_literal_against_refinement(200, &*r));
        assert!(check_literal_against_refinement(0, &*r));
        assert!(check_literal_against_refinement(999, &*i32t()));
        let r2 = TypeId::from(Type::Refine {
            span: ByteSpan::default(),
            base: u64t(),
            min: lit(Lit::U64(0)),
            max: lit(Lit::U64(u64::MAX)),
        });
        assert!(check_literal_against_refinement(u64::MAX as u128, &*r2));
        let r3 = TypeId::from(Type::Refine {
            span: ByteSpan::default(),
            base: i128t(),
            min: lit(Lit::I128(-100)),
            max: lit(Lit::I128(200)),
        });
        assert!(!check_literal_against_refinement(1000, &*r3));
    });
}

#[test]
fn test_check_literal_against_refinement_above_i128_max() {
    store(|_| {
        let val = 170141183460469231731687303715884105728u128;
        let r = TypeId::from(Type::Refine {
            span: ByteSpan::default(),
            base: u128t(),
            min: lit(Lit::U128(0)),
            max: lit(Lit::U128(200)),
        });
        assert!(!check_literal_against_refinement(val, &*r));
        let r2 = TypeId::from(Type::Refine {
            span: ByteSpan::default(),
            base: u64t(),
            min: lit(Lit::U64(0)),
            max: lit(Lit::U64(u64::MAX)),
        });
        assert!(!check_literal_against_refinement(val, &*r2));
        let r3 = TypeId::from(Type::Refine {
            span: ByteSpan::default(),
            base: i128t(),
            min: lit(Lit::I128(-100)),
            max: lit(Lit::I128(-50)),
        });
        assert!(!check_literal_against_refinement(val, &*r3));
    });
}

// ═══ CONSTRAINTS ═══

#[test]
fn test_constraints() {
    store(|_| {
        let eq = TypeConstraint::Equal(i32t());
        assert_eq!(eq.type_id(), i32t());
        let sub = TypeConstraint::SubtypeOf(u32t());
        assert_eq!(sub.type_id(), u32t());
        let _ = NodeAction::NoChange;
        let r = NodeAction::Replace(Value::Unit {
            span: ByteSpan::default(),
        });
        assert!(matches!(r, NodeAction::Replace(..)));
        assert!(is_comparison_or_logical_op(&BinaryOp::Lt));
        assert!(!is_comparison_or_logical_op(&BinaryOp::Add));
        assert!(is_arithmetic_op(&BinaryOp::Ror));
        assert!(!is_arithmetic_op(&BinaryOp::Lt));
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

#[test]
fn test_constraints_eq_type() {
    store(|_| {
        let c = TypeConstraint::eq_type(Type::I32 {
            span: ByteSpan::default(),
        });
        assert!(matches!(c, TypeConstraint::Equal(_)));
        assert_eq!(c.type_id(), i32t());
    });
}

#[test]
fn test_propagate_to_children_empty() {
    let p = propagate_to_children(&HashSet::new());
    assert!(p.is_empty());
}

// ═══ SUBSTITUTION ═══

#[test]
fn test_subst_generic_inferred() {
    store(|_| {
        let mut sub = Substitution::default();
        sub.mapping.insert(0, i32t());
        assert!(matches!(
            sub.apply(&Type::GenericParam {
                span: ByteSpan::default(),
                index: 0,
                name: NString::from("T")
            }),
            Type::I32 { .. }
        ));
        assert!(matches!(
            sub.apply(&Type::GenericParam {
                span: ByteSpan::default(),
                index: 99,
                name: NString::from("Z")
            }),
            Type::GenericParam { index: 99, .. }
        ));
        sub.mapping.insert(5, i64t());
        assert!(matches!(
            sub.apply(&Type::Inferred {
                span: ByteSpan::default(),
                id: std::num::NonZeroU32::new(5).unwrap(),
                name: None
            }),
            Type::I64 { .. }
        ));
        assert!(matches!(
            sub.apply(&Type::Inferred {
                span: ByteSpan::default(),
                id: std::num::NonZeroU32::new(6).unwrap(),
                name: None
            }),
            Type::Inferred { .. }
        ));
    });
}

#[test]
fn test_subst_compound() {
    store(|_| {
        let mut sub = Substitution::default();
        sub.mapping.insert(0, u8t());
        sub.mapping.insert(1, i16t());
        sub.mapping.insert(2, f64t());
        sub.mapping.insert(3, boolt());
        assert!(matches!(
            sub.apply(&Type::Tuple {
                span: ByteSpan::default(),
                element_types: vec![GP(0, "T"), GP(1, "U")].into()
            }),
            Type::Tuple { .. }
        ));
        assert!(matches!(
            sub.apply(&Type::Reference {
                span: ByteSpan::default(),
                lifetime: nitrate_hir::Lifetime::Inferred,
                exclusive: false,
                mutable: false,
                to: GP(0, "T")
            }),
            Type::Reference { .. }
        ));
        assert!(matches!(
            sub.apply(&Type::Pointer {
                span: ByteSpan::default(),
                lifetime: nitrate_hir::Lifetime::Inferred,
                exclusive: false,
                mutable: true,
                to: GP(0, "T")
            }),
            Type::Pointer { .. }
        ));
        assert!(matches!(
            sub.apply(&Type::SliceRef {
                span: ByteSpan::default(),
                lifetime: nitrate_hir::Lifetime::Inferred,
                exclusive: false,
                mutable: false,
                element_type: GP(0, "T")
            }),
            Type::SliceRef { .. }
        ));
        assert!(matches!(
            sub.apply(&Type::SlicePtr {
                span: ByteSpan::default(),
                lifetime: nitrate_hir::Lifetime::Inferred,
                exclusive: false,
                mutable: false,
                element_type: GP(0, "T")
            }),
            Type::SlicePtr { .. }
        ));
        assert!(matches!(
            sub.apply(&Type::Refine {
                span: ByteSpan::default(),
                base: GP(0, "T"),
                min: lit(Lit::I32(0)),
                max: lit(Lit::I32(100))
            }),
            Type::Refine { .. }
        ));
        assert!(matches!(
            sub.apply(&Type::Array {
                span: ByteSpan::default(),
                element_type: GP(0, "T"),
                len: 10
            }),
            Type::Array { .. }
        ));
        assert!(matches!(
            sub.apply(&Type::Struct {
                span: ByteSpan::default(),
                def: mkstruct("S", vec![("x", i32t())], None, None)
            }),
            Type::Struct { .. }
        ));
        let base = Type::Struct {
            span: ByteSpan::default(),
            def: mkstruct("P", vec![("x", i32t())], None, None),
        };
        assert!(matches!(
            sub.apply(&Type::Parameterized {
                span: ByteSpan::default(),
                base: TypeId::from(base),
                args: Arguments {
                    positional: vec![i32t()].into(),
                    named: ThinVec::new()
                }
            }),
            Type::Struct { .. }
        ));
        assert!(matches!(
            sub.apply(&Type::TraitObject {
                span: ByteSpan::default(),
                bounds: vec![].into()
            }),
            Type::TraitObject { .. }
        ));
        let r10 = sub.apply(&Type::Function {
            span: ByteSpan::default(),
            function_type: Box::new(nitrate_hir::FunctionType {
                attributes: BTreeSet::new(),
                params: vec![(NString::from("x"), GP(2, "T"))].into(),
                return_type: GP(3, "U"),
            }),
        });
        assert!(matches!(r10, Type::Function { .. }));
        let ed = nitrate_hir::EnumDef {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            name: NString::from("E"),
            attributes: BTreeSet::new(),
            generics: None,
            variants: ThinVec::new(),
        };
        assert!(matches!(
            sub.apply(&Type::Enum {
                span: ByteSpan::default(),
                def: nitrate_hir::EnumDefId::from(ed)
            }),
            Type::Enum { .. }
        ));
    });
}

#[test]
fn test_subst_leaf() {
    store(|_| {
        let sub = Substitution::default();
        for leaf in &[
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
            Type::U16 {
                span: ByteSpan::default(),
            },
            Type::U32 {
                span: ByteSpan::default(),
            },
            Type::U64 {
                span: ByteSpan::default(),
            },
            Type::U128 {
                span: ByteSpan::default(),
            },
            Type::USize {
                span: ByteSpan::default(),
            },
            Type::I8 {
                span: ByteSpan::default(),
            },
            Type::I16 {
                span: ByteSpan::default(),
            },
            Type::I32 {
                span: ByteSpan::default(),
            },
            Type::I64 {
                span: ByteSpan::default(),
            },
            Type::I128 {
                span: ByteSpan::default(),
            },
            Type::F32 {
                span: ByteSpan::default(),
            },
            Type::F64 {
                span: ByteSpan::default(),
            },
            Type::InferredInteger {
                span: ByteSpan::default(),
            },
            Type::InferredFloat {
                span: ByteSpan::default(),
            },
        ] {
            assert_eq!(sub.apply(leaf), *leaf);
        }
    });
}

#[test]
fn test_subst_type_alias() {
    store(|s| {
        let mut sub = Substitution::default();
        sub.mapping.insert(0, i32t());
        let aid = s.store_type_alias(nitrate_hir::TypeAliasDef {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            name: NString::from("A"),
            generics: None,
            type_id: GP(0, "T"),
        });
        assert!(matches!(
            sub.apply(&Type::TypeAlias {
                span: ByteSpan::default(),
                def: aid
            }),
            Type::I32 { .. }
        ));
    });
}

// ═══ MONO CACHE KEY ═══

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

// ═══ UNIFY ═══

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
        Solver::unify_types_with_subst(
            &Type::GenericParam {
                span: ByteSpan::default(),
                index: 0,
                name: NString::from("T"),
            },
            &Type::I32 {
                span: ByteSpan::default(),
            },
            &mut sub,
        );
        assert!(sub.mapping.contains_key(&0));
        Solver::unify_types_with_subst(
            &Type::I32 {
                span: ByteSpan::default(),
            },
            &Type::Bool {
                span: ByteSpan::default(),
            },
            &mut sub,
        );
        assert!(sub.mapping.is_empty() || sub.mapping.len() == 1);
        let mut sub2 = Substitution::default();
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
                to: GP(0, "T"),
            },
            &mut sub2,
        );
        assert!(sub2.mapping.contains_key(&0));
        let mut sub3 = Substitution::default();
        Solver::unify_types_with_subst(
            &Type::Tuple {
                span: ByteSpan::default(),
                element_types: vec![
                    TypeId::from(Type::I32 {
                        span: ByteSpan::default(),
                    }),
                    TypeId::from(Type::Bool {
                        span: ByteSpan::default(),
                    }),
                ]
                .into(),
            },
            &Type::Tuple {
                span: ByteSpan::default(),
                element_types: vec![GP(0, "T"), GP(1, "U")].into(),
            },
            &mut sub3,
        );
        assert!(sub3.mapping.contains_key(&0) && sub3.mapping.contains_key(&1));
        let mut sub4 = Substitution::default();
        sub4.mapping.insert(0, i32t());
        Solver::unify_types_with_subst(
            &Type::U32 {
                span: ByteSpan::default(),
            },
            &Type::GenericParam {
                span: ByteSpan::default(),
                index: 0,
                name: NString::from("T"),
            },
            &mut sub4,
        );
        assert!(matches!(&*sub4.mapping[&0], Type::I32 { .. }));
        let mut sub5 = Substitution::default();
        Solver::unify_types_with_subst(
            &Type::I32 {
                span: ByteSpan::default(),
            },
            &Type::SliceRef {
                span: ByteSpan::default(),
                lifetime: nitrate_hir::Lifetime::Inferred,
                exclusive: false,
                mutable: false,
                element_type: GP(0, "T"),
            },
            &mut sub5,
        );
        assert!(sub5.mapping.is_empty());
    });
}

#[test]
fn test_unify_slice_ptr_and_ref() {
    store(|_| {
        let mut sub = Substitution::default();
        Solver::unify_types_with_subst(
            &Type::SlicePtr {
                span: ByteSpan::default(),
                lifetime: nitrate_hir::Lifetime::Inferred,
                exclusive: false,
                mutable: false,
                element_type: TypeId::from(Type::I32 {
                    span: ByteSpan::default(),
                }),
            },
            &Type::SlicePtr {
                span: ByteSpan::default(),
                lifetime: nitrate_hir::Lifetime::Inferred,
                exclusive: false,
                mutable: false,
                element_type: GP(0, "T"),
            },
            &mut sub,
        );
        assert!(sub.mapping.contains_key(&0));
        let mut sub2 = Substitution::default();
        Solver::unify_types_with_subst(
            &Type::SliceRef {
                span: ByteSpan::default(),
                lifetime: nitrate_hir::Lifetime::Inferred,
                exclusive: false,
                mutable: false,
                element_type: TypeId::from(Type::I32 {
                    span: ByteSpan::default(),
                }),
            },
            &Type::SliceRef {
                span: ByteSpan::default(),
                lifetime: nitrate_hir::Lifetime::Inferred,
                exclusive: false,
                mutable: false,
                element_type: GP(0, "T"),
            },
            &mut sub2,
        );
        assert!(sub2.mapping.contains_key(&0));
        let mut sub3 = Substitution::default();
        Solver::unify_types_with_subst(
            &Type::Array {
                span: ByteSpan::default(),
                element_type: TypeId::from(Type::I32 {
                    span: ByteSpan::default(),
                }),
                len: 10,
            },
            &Type::Array {
                span: ByteSpan::default(),
                element_type: GP(0, "T"),
                len: 10,
            },
            &mut sub3,
        );
        assert!(sub3.mapping.contains_key(&0));
        let mut sub4 = Substitution::default();
        Solver::unify_types_with_subst(
            &Type::I32 {
                span: ByteSpan::default(),
            },
            &Type::Function {
                span: ByteSpan::default(),
                function_type: Box::new(nitrate_hir::FunctionType {
                    attributes: BTreeSet::new(),
                    params: vec![(NString::from("x"), GP(0, "T"))].into(),
                    return_type: GP(1, "U"),
                }),
            },
            &mut sub4,
        );
        assert!(sub4.mapping.is_empty());
    });
}

// ═══ SOLVER ═══

#[test]
fn test_solver_new() {
    store(|_| {
        let mut s = sym();
        let solver = Solver::new(&mut s);
        assert!(solver.constraints.is_empty() && solver.errors.is_empty() && solver.mono_counter == 0);
    });
}

#[test]
fn test_effective_bounds() {
    store(|_| {
        let mut s = sym();
        let solver = Solver::new(&mut s);
        assert_eq!(solver.get_effective_bounds(&i32v(42)), Some((-2147483648, 2147483647)));
        assert_eq!(
            solver.get_effective_bounds(&sv(Value::I8 {
                span: ByteSpan::default(),
                value: -5
            })),
            Some((-128, 127))
        );
        assert_eq!(
            solver.get_effective_bounds(&sv(Value::I16 {
                span: ByteSpan::default(),
                value: 100
            })),
            Some((-32768, 32767))
        );
        assert_eq!(
            solver.get_effective_bounds(&sv(Value::U8 {
                span: ByteSpan::default(),
                value: 10
            })),
            Some((0, 255))
        );
        assert_eq!(
            solver.get_effective_bounds(&sv(Value::U16 {
                span: ByteSpan::default(),
                value: 10
            })),
            Some((0, 65535))
        );
        let ps = sv(Value::ParameterSymbol {
            span: ByteSpan::default(),
            id: param("x", i32t()),
        });
        let bounds = solver.get_effective_bounds(&ps);
        assert!(bounds.is_none() || bounds == Some((-2147483648, 2147483647)));
    });
}

#[test]
fn test_effective_bounds_inferred() {
    store(|_| {
        let mut s = sym();
        let solver = Solver::new(&mut s);
        assert_eq!(
            solver.get_effective_bounds(&sv(Value::USize {
                span: ByteSpan::default(),
                bits: 64,
                value: 42
            })),
            Some((0, 18446744073709551615))
        );
        assert_eq!(solver.get_effective_bounds(&inf_int(42)), Some((42, 42)));
        assert_eq!(
            solver.get_effective_bounds(&sv(Value::U128 {
                span: ByteSpan::default(),
                value: Box::new(100)
            })),
            Some((0, i128::MAX))
        );
        assert_eq!(
            solver.get_effective_bounds(&sv(Value::I128 {
                span: ByteSpan::default(),
                value: Box::new(-5)
            })),
            Some((i128::MIN, i128::MAX))
        );
        let gv = nitrate_hir::GlobalVariable {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            is_mutable: false,
            name: NString::from("G"),
            mangled_name: NString::from("G"),
            ty: i32t(),
            initializer: i32v(0),
        };
        let gid = GlobalVariableId::from(gv);
        let gs = sv(Value::GlobalVariableSymbol {
            span: ByteSpan::default(),
            id: gid,
        });
        assert_eq!(solver.get_effective_bounds(&gs), Some((-2147483648, 2147483647)));
        assert_eq!(
            solver.get_effective_bounds(&sv(Value::U32 {
                span: ByteSpan::default(),
                value: 10
            })),
            Some((0, 4294967295))
        );
        assert_eq!(
            solver.get_effective_bounds(&sv(Value::U64 {
                span: ByteSpan::default(),
                value: 10
            })),
            Some((0, 18446744073709551615))
        );
        assert_eq!(solver.get_effective_bounds(&i32v(42)), Some((-2147483648, 2147483647)));
        assert_eq!(
            solver.get_effective_bounds(&sv(Value::I64 {
                span: ByteSpan::default(),
                value: 99
            })),
            Some((-9223372036854775808, 9223372036854775807))
        );
        assert_eq!(
            solver.get_effective_bounds(&sv(Value::F64 {
                span: ByteSpan::default(),
                value: OrderedFloat(1.0)
            })),
            None
        );
        assert_eq!(solver.get_effective_bounds(&boolv(true)), None);
        assert_eq!(
            solver.get_effective_bounds(&sv(Value::StringLit {
                span: ByteSpan::default(),
                value: "hi".into()
            })),
            None
        );
        assert_eq!(
            solver.get_effective_bounds(&sv(Value::U128 {
                span: ByteSpan::default(),
                value: Box::new(0)
            })),
            Some((0, i128::MAX))
        );
    });
}

#[test]
fn test_solver_with_inferred_int_constrained() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let inferred = inf_int(42);
        assert!(resolve_func(vec![BlockElement::Expr(inferred)], i32t(), &log, &mut s).is_ok());
    });
}

#[test]
fn test_resolve_simple() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        assert!(resolve_func(vec![BlockElement::Expr(i32v(42))], i32t(), &log, &mut s).is_ok());
    });
}

#[test]
fn test_resolve_empty_body() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let mut f = Function {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            name: NString::from("f"),
            mangled_name: NString::from("f"),
            generics: None,
            params: vec![],
            return_type: unitt(),
            body: None,
        };
        assert!(resolve_function(&mut f, &mut s, &log).is_ok());
    });
}

#[test]
fn test_resolve_literals() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        for v in &[
            i32v(1),
            i8v(1),
            i64v(1),
            boolv(true),
            sv(Value::Unit {
                span: ByteSpan::default(),
            }),
            inf_int(42),
            inf_flt(3.14),
            sv(Value::I16 {
                span: ByteSpan::default(),
                value: 1,
            }),
            sv(Value::U8 {
                span: ByteSpan::default(),
                value: 1,
            }),
            sv(Value::U16 {
                span: ByteSpan::default(),
                value: 1,
            }),
            sv(Value::U32 {
                span: ByteSpan::default(),
                value: 1,
            }),
            sv(Value::U64 {
                span: ByteSpan::default(),
                value: 1,
            }),
            sv(Value::I128 {
                span: ByteSpan::default(),
                value: Box::new(1),
            }),
            sv(Value::F32 {
                span: ByteSpan::default(),
                value: OrderedFloat(1.0),
            }),
            sv(Value::F64 {
                span: ByteSpan::default(),
                value: OrderedFloat(1.0),
            }),
            sv(Value::StringLit {
                span: ByteSpan::default(),
                value: "hi".into(),
            }),
            sv(Value::BStringLit {
                span: ByteSpan::default(),
                value: vec![1u8].into(),
            }),
        ] {
            assert!(resolve_func(vec![BlockElement::Expr(v.clone())], unitt(), &log, &mut s).is_ok());
        }
    });
}

#[test]
fn test_resolve_binary_ops() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        for op in &[
            BinaryOp::Add,
            BinaryOp::Sub,
            BinaryOp::Mul,
            BinaryOp::Div,
            BinaryOp::Mod,
            BinaryOp::And,
            BinaryOp::Or,
            BinaryOp::Xor,
            BinaryOp::Shl,
            BinaryOp::Shr,
            BinaryOp::Rol,
            BinaryOp::Ror,
        ] {
            assert!(
                resolve_func(
                    vec![BlockElement::Expr(sv(Value::Binary {
                        span: ByteSpan::default(),
                        left: i32v(10),
                        op: op.clone(),
                        right: i32v(20)
                    }))],
                    i32t(),
                    &log,
                    &mut s
                )
                .is_ok()
            );
        }
    });
}

#[test]
fn test_resolve_binary_ambiguous() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let result = resolve_func(
            vec![BlockElement::Expr(sv(Value::Binary {
                span: ByteSpan::default(),
                left: i32v(10),
                op: BinaryOp::Add,
                right: i64v(20),
            }))],
            unitt(),
            &log,
            &mut s,
        );
        let _ = result;
    });
}

#[test]
fn test_resolve_binary_with_inferred_ops() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        assert!(
            resolve_func(
                vec![BlockElement::Expr(sv(Value::Binary {
                    span: ByteSpan::default(),
                    left: inf_int(10),
                    op: BinaryOp::Add,
                    right: i32v(20)
                }))],
                i32t(),
                &log,
                &mut s
            )
            .is_ok()
        );
    });
}

#[test]
fn test_resolve_cmp_ops() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        for op in &[
            BinaryOp::Lt,
            BinaryOp::Gt,
            BinaryOp::Lte,
            BinaryOp::Gte,
            BinaryOp::Eq,
            BinaryOp::Ne,
            BinaryOp::LogicAnd,
            BinaryOp::LogicOr,
        ] {
            assert!(
                resolve_func(
                    vec![BlockElement::Expr(sv(Value::Binary {
                        span: ByteSpan::default(),
                        left: i32v(10),
                        op: op.clone(),
                        right: i32v(20)
                    }))],
                    boolt(),
                    &log,
                    &mut s
                )
                .is_ok()
            );
        }
    });
}

#[test]
fn test_resolve_unary_ops() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        for op in &[UnaryOp::Add, UnaryOp::Sub] {
            assert!(
                resolve_func(
                    vec![BlockElement::Expr(sv(Value::Unary {
                        span: ByteSpan::default(),
                        op: op.clone(),
                        operand: i32v(5)
                    }))],
                    i32t(),
                    &log,
                    &mut s
                )
                .is_ok()
            );
        }
        assert!(
            resolve_func(
                vec![BlockElement::Expr(sv(Value::Unary {
                    span: ByteSpan::default(),
                    op: UnaryOp::Not,
                    operand: boolv(true)
                }))],
                boolt(),
                &log,
                &mut s
            )
            .is_ok()
        );
    });
}

#[test]
fn test_resolve_local() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        assert!(
            resolve_func(
                vec![BlockElement::Local(local("x", i32t(), i32v(42)))],
                unitt(),
                &log,
                &mut s
            )
            .is_ok()
        );
        assert!(
            resolve_func(
                vec![BlockElement::Local(local(
                    "x",
                    TypeId::from(Type::Inferred {
                        span: ByteSpan::default(),
                        id: std::num::NonZeroU32::new(1).unwrap(),
                        name: None
                    }),
                    inf_int(42)
                ))],
                unitt(),
                &log,
                &mut s
            )
            .is_ok()
        );
    });
}

#[test]
fn test_resolve_local_with_parameterized_type() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let sd = mkstruct(
            "Pair",
            vec![("first", GP(0, "T")), ("second", GP(0, "T"))],
            Some(vec!["T"]),
            None,
        );
        s.add_struct(sd.clone());
        let param_type = Type::Parameterized {
            span: ByteSpan::default(),
            base: TypeId::from(Type::Struct {
                span: ByteSpan::default(),
                def: sd.clone(),
            }),
            args: Arguments {
                positional: vec![i32t()].into(),
                named: ThinVec::new(),
            },
        };
        let fields: ThinVec<(NString, ValueId)> =
            vec![(NString::from("first"), i32v(1)), (NString::from("second"), i32v(2))].into();
        let obj = sv(Value::StructObject {
            span: ByteSpan::default(),
            struct_def: sd.clone(),
            fields,
        });
        let lv = local("p", TypeId::from(param_type), obj);
        assert!(resolve_func(vec![BlockElement::Local(lv)], unitt(), &log, &mut s).is_ok());
    });
}

#[test]
fn test_resolve_return() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let p = param("x", i32t());
        s.add_parameter(p.clone());
        let r = sv(Value::Return {
            span: ByteSpan::default(),
            value: sv(Value::ParameterSymbol {
                span: ByteSpan::default(),
                id: p.clone(),
            }),
        });
        assert!(resolve_func(vec![BlockElement::Expr(r)], i32t(), &log, &mut s).is_ok());
        let r2 = sv(Value::Return {
            span: ByteSpan::default(),
            value: i32v(42),
        });
        let mut f = Function {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            name: NString::from("f"),
            mangled_name: NString::from("f"),
            generics: None,
            params: vec![p],
            return_type: unitt(),
            body: Some(vec![BlockElement::Expr(r2)]),
        };
        let _ = resolve_function(&mut f, &mut s, &log);
    });
}

#[test]
fn test_resolve_cast() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        assert!(
            resolve_func(
                vec![BlockElement::Expr(sv(Value::Cast {
                    span: ByteSpan::default(),
                    value: i32v(42),
                    target_type: i64t()
                }))],
                i64t(),
                &log,
                &mut s
            )
            .is_ok()
        );
    });
}

#[test]
fn test_resolve_if() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let tb = Block {
            span: ByteSpan::default(),
            safety: BlockSafety::Safe,
            elements: vec![BlockElement::Expr(i32v(1))],
        };
        assert!(
            resolve_func(
                vec![BlockElement::Expr(sv(Value::If {
                    span: ByteSpan::default(),
                    condition: boolv(true),
                    true_branch: BlockId::from(tb),
                    false_branch: None
                }))],
                unitt(),
                &log,
                &mut s
            )
            .is_ok()
        );
        let tb2 = Block {
            span: ByteSpan::default(),
            safety: BlockSafety::Safe,
            elements: vec![BlockElement::Expr(i32v(1))],
        };
        let fb2 = Block {
            span: ByteSpan::default(),
            safety: BlockSafety::Safe,
            elements: vec![BlockElement::Expr(i32v(2))],
        };
        assert!(
            resolve_func(
                vec![BlockElement::Expr(sv(Value::If {
                    span: ByteSpan::default(),
                    condition: boolv(true),
                    true_branch: BlockId::from(tb2),
                    false_branch: Some(BlockId::from(fb2))
                }))],
                i32t(),
                &log,
                &mut s
            )
            .is_ok()
        );
        let tb3 = Block {
            span: ByteSpan::default(),
            safety: BlockSafety::Safe,
            elements: vec![BlockElement::Expr(sv(Value::Break {
                span: ByteSpan::default(),
                label: None,
            }))],
        };
        let fb3 = Block {
            span: ByteSpan::default(),
            safety: BlockSafety::Safe,
            elements: vec![BlockElement::Expr(i32v(1))],
        };
        assert!(
            resolve_func(
                vec![BlockElement::Expr(sv(Value::If {
                    span: ByteSpan::default(),
                    condition: boolv(true),
                    true_branch: BlockId::from(tb3),
                    false_branch: Some(BlockId::from(fb3))
                }))],
                unitt(),
                &log,
                &mut s
            )
            .is_ok()
        );
    });
}

#[test]
fn test_resolve_if_mismatched() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let tb = Block {
            span: ByteSpan::default(),
            safety: BlockSafety::Safe,
            elements: vec![BlockElement::Expr(i32v(1))],
        };
        let fb = Block {
            span: ByteSpan::default(),
            safety: BlockSafety::Safe,
            elements: vec![BlockElement::Expr(f64v(2.0))],
        };
        assert!(
            resolve_func(
                vec![BlockElement::Expr(sv(Value::If {
                    span: ByteSpan::default(),
                    condition: boolv(true),
                    true_branch: BlockId::from(tb),
                    false_branch: Some(BlockId::from(fb))
                }))],
                unitt(),
                &log,
                &mut s
            )
            .is_err()
        );
    });
}

#[test]
fn test_resolve_block() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let b = Block {
            span: ByteSpan::default(),
            safety: BlockSafety::Safe,
            elements: vec![BlockElement::Expr(i32v(7))],
        };
        assert!(
            resolve_func(
                vec![BlockElement::Expr(sv(Value::Block {
                    span: ByteSpan::default(),
                    block: BlockId::from(b)
                }))],
                i32t(),
                &log,
                &mut s
            )
            .is_ok()
        );
    });
}

#[test]
fn test_resolve_list_tuple() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        assert!(
            resolve_func(
                vec![BlockElement::Expr(sv(Value::List {
                    span: ByteSpan::default(),
                    elements: vec![i32v(1), i32v(2)].into()
                }))],
                unitt(),
                &log,
                &mut s
            )
            .is_ok()
        );
        assert!(
            resolve_func(
                vec![BlockElement::Expr(sv(Value::List {
                    span: ByteSpan::default(),
                    elements: vec![i32v(5), inf_int(10)].into()
                }))],
                unitt(),
                &log,
                &mut s
            )
            .is_ok()
        );
        assert!(
            resolve_func(
                vec![BlockElement::Expr(sv(Value::Tuple {
                    span: ByteSpan::default(),
                    elements: vec![i32v(1), boolv(true)].into()
                }))],
                unitt(),
                &log,
                &mut s
            )
            .is_ok()
        );
    });
}

#[test]
fn test_resolve_while_loop() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let bb = Block {
            span: ByteSpan::default(),
            safety: BlockSafety::Safe,
            elements: vec![BlockElement::Expr(i32v(1))],
        };
        assert!(
            resolve_func(
                vec![BlockElement::Expr(sv(Value::While {
                    span: ByteSpan::default(),
                    condition: boolv(true),
                    body: BlockId::from(bb)
                }))],
                unitt(),
                &log,
                &mut s
            )
            .is_ok()
        );
        let bb2 = Block {
            span: ByteSpan::default(),
            safety: BlockSafety::Safe,
            elements: vec![BlockElement::Expr(i32v(1))],
        };
        assert!(
            resolve_func(
                vec![BlockElement::Expr(sv(Value::Loop {
                    span: ByteSpan::default(),
                    body: BlockId::from(bb2)
                }))],
                unitt(),
                &log,
                &mut s
            )
            .is_ok()
        );
    });
}

#[test]
fn test_resolve_break_continue() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let bb = Block {
            span: ByteSpan::default(),
            safety: BlockSafety::Safe,
            elements: vec![
                BlockElement::Expr(sv(Value::Break {
                    span: ByteSpan::default(),
                    label: None,
                })),
                BlockElement::Expr(sv(Value::Continue {
                    span: ByteSpan::default(),
                    label: None,
                })),
            ],
        };
        assert!(
            resolve_func(
                vec![BlockElement::Expr(sv(Value::Loop {
                    span: ByteSpan::default(),
                    body: BlockId::from(bb)
                }))],
                unitt(),
                &log,
                &mut s
            )
            .is_ok()
        );
        let bv2 = sv(Value::Break {
            span: ByteSpan::default(),
            label: Some(NString::from("outer")),
        });
        let bb2 = Block {
            span: ByteSpan::default(),
            safety: BlockSafety::Safe,
            elements: vec![BlockElement::Expr(bv2)],
        };
        assert!(
            resolve_func(
                vec![BlockElement::Expr(sv(Value::Loop {
                    span: ByteSpan::default(),
                    body: BlockId::from(bb2)
                }))],
                unitt(),
                &log,
                &mut s
            )
            .is_ok()
        );
    });
}

#[test]
fn test_resolve_struct() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let sd = mkstruct("Point", vec![("x", i32t()), ("y", i32t())], None, None);
        s.add_struct(sd);
        let sd2 = mkstruct("Point", vec![("x", i32t()), ("y", i32t())], None, None);
        let fields: ThinVec<(NString, ValueId)> =
            vec![(NString::from("x"), i32v(10)), (NString::from("y"), i32v(20))].into();
        assert!(
            resolve_func(
                vec![BlockElement::Expr(sv(Value::StructObject {
                    span: ByteSpan::default(),
                    struct_def: sd2,
                    fields
                }))],
                unitt(),
                &log,
                &mut s
            )
            .is_ok()
        );
        let sd3 = mkstruct("OnlyI32", vec![("val", i32t())], None, None);
        s.add_struct(sd3);
        let fields3: ThinVec<(NString, ValueId)> = vec![(NString::from("val"), i32v(99))].into();
        let sd4 = sv(Value::StructObject {
            span: ByteSpan::default(),
            struct_def: mkstruct("OnlyI32", vec![("val", i32t())], None, None),
            fields: fields3,
        });
        assert!(resolve_func(vec![BlockElement::Expr(sd4)], unitt(), &log, &mut s).is_ok());
    });
}

#[test]
fn test_resolve_generic_struct() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let sd = mkstruct(
            "Pair",
            vec![("first", GP(0, "T")), ("second", GP(0, "T"))],
            Some(vec!["T"]),
            None,
        );
        let fields: ThinVec<(NString, ValueId)> =
            vec![(NString::from("first"), i32v(1)), (NString::from("second"), i32v(2))].into();
        assert!(
            resolve_func(
                vec![BlockElement::Expr(sv(Value::StructObject {
                    span: ByteSpan::default(),
                    struct_def: sd,
                    fields
                }))],
                unitt(),
                &log,
                &mut s
            )
            .is_ok()
        );
    });
}

#[test]
fn test_resolve_generic_struct_float_fields() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let sd = mkstruct(
            "Pair",
            vec![("first", GP(0, "T")), ("second", GP(0, "T"))],
            Some(vec!["T"]),
            None,
        );
        let fields: ThinVec<(NString, ValueId)> = vec![
            (NString::from("first"), f64v(1.0)),
            (NString::from("second"), f64v(2.0)),
        ]
        .into();
        assert!(
            resolve_func(
                vec![BlockElement::Expr(sv(Value::StructObject {
                    span: ByteSpan::default(),
                    struct_def: sd,
                    fields
                }))],
                unitt(),
                &log,
                &mut s
            )
            .is_ok()
        );
    });
}

#[test]
fn test_resolve_generic_struct_unsolved_inferred() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let sd = mkstruct(
            "Pair",
            vec![("first", GP(0, "T")), ("second", GP(0, "T"))],
            Some(vec!["T"]),
            None,
        );
        let fields: ThinVec<(NString, ValueId)> = vec![
            (NString::from("first"), inf_int(1)),
            (NString::from("second"), inf_int(2)),
        ]
        .into();
        let result = resolve_func(
            vec![BlockElement::Expr(sv(Value::StructObject {
                span: ByteSpan::default(),
                struct_def: sd,
                fields,
            }))],
            unitt(),
            &log,
            &mut s,
        );
        let _ = result;
    });
}

#[test]
fn test_resolve_generic_struct_from_constraints() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let sd = mkstruct(
            "Pair",
            vec![("first", GP(0, "T")), ("second", GP(0, "T"))],
            Some(vec!["T"]),
            None,
        );
        s.add_struct(sd.clone());
        let fields: ThinVec<(NString, ValueId)> = vec![
            (NString::from("first"), inf_int(1)),
            (NString::from("second"), inf_int(2)),
        ]
        .into();
        let param_type = TypeId::from(Type::Parameterized {
            span: ByteSpan::default(),
            base: TypeId::from(Type::Struct {
                span: ByteSpan::default(),
                def: sd.clone(),
            }),
            args: Arguments {
                positional: vec![i32t()].into(),
                named: ThinVec::new(),
            },
        });
        let obj = sv(Value::StructObject {
            span: ByteSpan::default(),
            struct_def: sd,
            fields,
        });
        let lv = local("p", param_type, obj);
        let _ = resolve_func(vec![BlockElement::Local(lv)], unitt(), &log, &mut s);
    });
}

#[test]
fn test_resolve_field_access() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let sd = mkstruct("Pt", vec![("x", i32t()), ("y", i32t())], None, None);
        s.add_struct(sd);
        let sd2 = mkstruct("Pt", vec![("x", i32t()), ("y", i32t())], None, None);
        let fields: ThinVec<(NString, ValueId)> =
            vec![(NString::from("x"), i32v(10)), (NString::from("y"), i32v(20))].into();
        let so = sv(Value::StructObject {
            span: ByteSpan::default(),
            struct_def: sd2,
            fields,
        });
        assert!(
            resolve_func(
                vec![BlockElement::Expr(sv(Value::FieldAccess {
                    span: ByteSpan::default(),
                    expr: so,
                    field_name: NString::from("x")
                }))],
                i32t(),
                &log,
                &mut s
            )
            .is_ok()
        );
    });
}

#[test]
fn test_resolve_index_access() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let list = sv(Value::List {
            span: ByteSpan::default(),
            elements: vec![i32v(1), i32v(2)].into(),
        });
        let idx = sv(Value::USize {
            span: ByteSpan::default(),
            bits: 64,
            value: 0,
        });
        assert!(
            resolve_func(
                vec![BlockElement::Expr(sv(Value::IndexAccess {
                    span: ByteSpan::default(),
                    collection: list,
                    index: idx
                }))],
                i32t(),
                &log,
                &mut s
            )
            .is_ok()
        );
    });
}

#[test]
fn test_resolve_index_access_with_list_and_constraint() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let list = sv(Value::List {
            span: ByteSpan::default(),
            elements: vec![i32v(1), i32v(2), i32v(3)].into(),
        });
        let idx = sv(Value::USize {
            span: ByteSpan::default(),
            bits: 64,
            value: 0,
        });
        let idx_access = sv(Value::IndexAccess {
            span: ByteSpan::default(),
            collection: list,
            index: idx,
        });
        assert!(
            resolve_func(
                vec![BlockElement::Local(local("r", i32t(), idx_access))],
                unitt(),
                &log,
                &mut s
            )
            .is_ok()
        );
    });
}

#[test]
fn test_resolve_assign() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let lv = local("x", i32t(), i32v(0));
        let ls = sv(Value::LocalVariableSymbol {
            span: ByteSpan::default(),
            id: lv.clone(),
        });
        assert!(
            resolve_func(
                vec![
                    BlockElement::Local(lv),
                    BlockElement::Expr(sv(Value::Assign {
                        span: ByteSpan::default(),
                        place: ls,
                        value: i32v(42)
                    }))
                ],
                unitt(),
                &log,
                &mut s
            )
            .is_ok()
        );
    });
}

#[test]
fn test_resolve_borrow_deref() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let br = sv(Value::Borrow {
            span: ByteSpan::default(),
            exclusive: true,
            mutable: true,
            place: i32v(42),
        });
        assert!(
            resolve_func(
                vec![BlockElement::Expr(sv(Value::Deref {
                    span: ByteSpan::default(),
                    place: br
                }))],
                i32t(),
                &log,
                &mut s
            )
            .is_ok()
        );
        let br2 = sv(Value::Borrow {
            span: ByteSpan::default(),
            exclusive: false,
            mutable: false,
            place: i32v(42),
        });
        assert!(resolve_func(vec![BlockElement::Expr(br2)], unitt(), &log, &mut s).is_ok());
    });
}

#[test]
fn test_resolve_global() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
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
        let mut g3 = nitrate_hir::GlobalVariable {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            is_mutable: false,
            name: NString::from("S"),
            mangled_name: NString::from("S"),
            ty: unitt(),
            initializer: sv(Value::StringLit {
                span: ByteSpan::default(),
                value: "hi".into(),
            }),
        };
        assert!(resolve_global(&mut g3, &mut s, &log).is_ok());
    });
}

#[test]
fn test_resolve_call() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let p = param("x", i32t());
        let ret = sv(Value::Return {
            span: ByteSpan::default(),
            value: sv(Value::ParameterSymbol {
                span: ByteSpan::default(),
                id: p.clone(),
            }),
        });
        let fid = mkfunc("add", vec![p], i32t(), Some(vec![BlockElement::Expr(ret)]), None);
        let call = sv(Value::Call {
            span: ByteSpan::default(),
            callee: sv(Value::FunctionSymbol {
                span: ByteSpan::default(),
                id: fid,
            }),
            args: Arguments {
                positional: vec![i32v(42)].into(),
                named: ThinVec::new(),
            },
        });
        assert!(resolve_func(vec![BlockElement::Expr(call)], i32t(), &log, &mut s).is_ok());
    });
}

#[test]
fn test_resolve_call_named() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let p = param("x", i32t());
        let ret = sv(Value::Return {
            span: ByteSpan::default(),
            value: sv(Value::ParameterSymbol {
                span: ByteSpan::default(),
                id: p.clone(),
            }),
        });
        let fid = mkfunc("f", vec![p], i32t(), Some(vec![BlockElement::Expr(ret)]), None);
        let call = sv(Value::Call {
            span: ByteSpan::default(),
            callee: sv(Value::FunctionSymbol {
                span: ByteSpan::default(),
                id: fid,
            }),
            args: Arguments {
                positional: ThinVec::new(),
                named: vec![(NString::from("x"), i32v(42))].into(),
            },
        });
        assert!(resolve_func(vec![BlockElement::Expr(call)], i32t(), &log, &mut s).is_ok());
    });
}

#[test]
fn test_resolve_call_generic() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let p = param("x", GP(0, "T"));
        let ret = sv(Value::Return {
            span: ByteSpan::default(),
            value: sv(Value::ParameterSymbol {
                span: ByteSpan::default(),
                id: p.clone(),
            }),
        });
        let mut gens = BTreeMap::new();
        gens.insert(NString::from("T"), Some(GP(0, "T")));
        let fid = mkfunc(
            "id",
            vec![p],
            GP(0, "T"),
            Some(vec![BlockElement::Expr(ret)]),
            Some(gens),
        );
        let call = sv(Value::Call {
            span: ByteSpan::default(),
            callee: sv(Value::FunctionSymbol {
                span: ByteSpan::default(),
                id: fid,
            }),
            args: Arguments {
                positional: vec![i32v(42)].into(),
                named: ThinVec::new(),
            },
        });
        assert!(resolve_func(vec![BlockElement::Expr(call)], i32t(), &log, &mut s).is_ok());
    });
}

#[test]
fn test_resolve_call_generic_named() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let p = param("x", GP(0, "T"));
        let ret = sv(Value::Return {
            span: ByteSpan::default(),
            value: sv(Value::ParameterSymbol {
                span: ByteSpan::default(),
                id: p.clone(),
            }),
        });
        let mut gens = BTreeMap::new();
        gens.insert(NString::from("T"), Some(GP(0, "T")));
        let fid = mkfunc(
            "id",
            vec![p],
            GP(0, "T"),
            Some(vec![BlockElement::Expr(ret)]),
            Some(gens),
        );
        let call = sv(Value::Call {
            span: ByteSpan::default(),
            callee: sv(Value::FunctionSymbol {
                span: ByteSpan::default(),
                id: fid,
            }),
            args: Arguments {
                positional: ThinVec::new(),
                named: vec![(NString::from("x"), i32v(42))].into(),
            },
        });
        assert!(resolve_func(vec![BlockElement::Expr(call)], i32t(), &log, &mut s).is_ok());
    });
}

#[test]
fn test_resolve_call_generic_mismatch() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let p = param("x", GP(0, "T"));
        let ret = sv(Value::Return {
            span: ByteSpan::default(),
            value: sv(Value::ParameterSymbol {
                span: ByteSpan::default(),
                id: p.clone(),
            }),
        });
        let mut gens = BTreeMap::new();
        gens.insert(NString::from("T"), Some(GP(0, "T")));
        let fid = mkfunc(
            "id",
            vec![p],
            GP(0, "T"),
            Some(vec![BlockElement::Expr(ret)]),
            Some(gens),
        );
        let call = sv(Value::Call {
            span: ByteSpan::default(),
            callee: sv(Value::FunctionSymbol {
                span: ByteSpan::default(),
                id: fid,
            }),
            args: Arguments {
                positional: ThinVec::new(),
                named: ThinVec::new(),
            },
        });
        let _ = resolve_func(vec![BlockElement::Expr(call)], i32t(), &log, &mut s);
    });
}

#[test]
fn test_resolve_call_generic_empty_generics() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let mut gens = BTreeMap::new();
        gens.insert(NString::from("T"), Some(GP(0, "T")));
        let fid = mkfunc("gen", vec![], GP(0, "T"), None, Some(gens));
        let call = sv(Value::Call {
            span: ByteSpan::default(),
            callee: sv(Value::FunctionSymbol {
                span: ByteSpan::default(),
                id: fid,
            }),
            args: Arguments {
                positional: ThinVec::new(),
                named: ThinVec::new(),
            },
        });
        let _ = resolve_func(vec![BlockElement::Expr(call)], unitt(), &log, &mut s);
    });
}

#[test]
fn test_resolve_method_call() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let sd = mkstruct("Pt", vec![("x", i32t()), ("y", i32t())], None, None);
        let sd2 = mkstruct("Pt", vec![("x", i32t()), ("y", i32t())], None, None);
        s.add_struct(sd);
        let st_id = TypeId::from(Type::Struct {
            span: ByteSpan::default(),
            def: sd2,
        });
        let p = param("self", st_id);
        let ret = sv(Value::Return {
            span: ByteSpan::default(),
            value: sv(Value::ParameterSymbol {
                span: ByteSpan::default(),
                id: p.clone(),
            }),
        });
        let mid = mkfunc("get_x", vec![p], i32t(), Some(vec![BlockElement::Expr(ret)]), None);
        s.add_method(st_id, NString::from("get_x"), mid);
        let sd3 = mkstruct("Pt", vec![("x", i32t()), ("y", i32t())], None, None);
        let fields: ThinVec<(NString, ValueId)> =
            vec![(NString::from("x"), i32v(10)), (NString::from("y"), i32v(20))].into();
        let obj = sv(Value::StructObject {
            span: ByteSpan::default(),
            struct_def: sd3,
            fields,
        });
        let mc = sv(Value::MethodCall {
            span: ByteSpan::default(),
            object: obj,
            method_name: NString::from("get_x"),
            args: Arguments {
                positional: ThinVec::new(),
                named: ThinVec::new(),
            },
        });
        assert!(resolve_func(vec![BlockElement::Expr(mc)], i32t(), &log, &mut s).is_ok());
    });
}

#[test]
fn test_resolve_method_call_named_args() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let sd = mkstruct("Pt", vec![("x", i32t()), ("y", i32t())], None, None);
        let sd2 = mkstruct("Pt", vec![("x", i32t()), ("y", i32t())], None, None);
        s.add_struct(sd);
        let st_id = TypeId::from(Type::Struct {
            span: ByteSpan::default(),
            def: sd2,
        });
        let p = param("self", st_id);
        let ret = sv(Value::Return {
            span: ByteSpan::default(),
            value: sv(Value::ParameterSymbol {
                span: ByteSpan::default(),
                id: p.clone(),
            }),
        });
        let mid = mkfunc("get_x", vec![p], i32t(), Some(vec![BlockElement::Expr(ret)]), None);
        s.add_method(st_id, NString::from("get_x"), mid);
        let sd3 = mkstruct("Pt", vec![("x", i32t()), ("y", i32t())], None, None);
        let fields: ThinVec<(NString, ValueId)> =
            vec![(NString::from("x"), i32v(10)), (NString::from("y"), i32v(20))].into();
        let obj = sv(Value::StructObject {
            span: ByteSpan::default(),
            struct_def: sd3,
            fields,
        });
        let mc = sv(Value::MethodCall {
            span: ByteSpan::default(),
            object: obj.clone(),
            method_name: NString::from("get_x"),
            args: Arguments {
                positional: ThinVec::new(),
                named: vec![(NString::from("self"), obj)].into(),
            },
        });
        let _ = resolve_func(vec![BlockElement::Expr(mc)], i32t(), &log, &mut s);
    });
}

#[test]
fn test_resolve_method_not_found() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let mc = sv(Value::MethodCall {
            span: ByteSpan::default(),
            object: i32v(42),
            method_name: NString::from("nonexistent"),
            args: Arguments {
                positional: ThinVec::new(),
                named: ThinVec::new(),
            },
        });
        let _ = resolve_func(vec![BlockElement::Expr(mc)], unitt(), &log, &mut s);
    });
}

#[test]
fn test_resolve_method_call_generic() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let sd = mkstruct("Pt", vec![("x", GP(0, "T")), ("y", GP(0, "T"))], Some(vec!["T"]), None);
        s.add_struct(sd.clone());
        let st_id = TypeId::from(Type::Struct {
            span: ByteSpan::default(),
            def: sd.clone(),
        });
        let p = param("self", st_id);
        let ret = sv(Value::Return {
            span: ByteSpan::default(),
            value: sv(Value::ParameterSymbol {
                span: ByteSpan::default(),
                id: p.clone(),
            }),
        });
        let mid = mkfunc("get_x", vec![p], i32t(), Some(vec![BlockElement::Expr(ret)]), None);
        s.add_method(
            TypeId::from(Type::Struct {
                span: ByteSpan::default(),
                def: sd.clone(),
            }),
            NString::from("get_x"),
            mid,
        );
        let fields: ThinVec<(NString, ValueId)> =
            vec![(NString::from("x"), i32v(10)), (NString::from("y"), i32v(20))].into();
        let obj = sv(Value::StructObject {
            span: ByteSpan::default(),
            struct_def: sd,
            fields,
        });
        let mc = sv(Value::MethodCall {
            span: ByteSpan::default(),
            object: obj,
            method_name: NString::from("get_x"),
            args: Arguments {
                positional: ThinVec::new(),
                named: ThinVec::new(),
            },
        });
        assert!(resolve_func(vec![BlockElement::Expr(mc)], i32t(), &log, &mut s).is_ok());
    });
}

#[test]
fn test_resolve_enum_variant() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
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
        assert!(
            resolve_func(
                vec![BlockElement::Expr(sv(Value::EnumVariant {
                    span: ByteSpan::default(),
                    enum_def: edi,
                    variant: NString::from("A"),
                    value: i32v(42)
                }))],
                unitt(),
                &log,
                &mut s
            )
            .is_ok()
        );
    });
}

#[test]
fn test_resolve_enum_variant_nested_ty() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let ct = TypeId::from(Type::Tuple {
            span: ByteSpan::default(),
            element_types: vec![i32t(), i64t()].into(),
        });
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
                ty: ct,
                default_value: None,
            }]
            .into(),
        };
        let edi = nitrate_hir::EnumDefId::from(ed);
        let inner = sv(Value::Tuple {
            span: ByteSpan::default(),
            elements: vec![i32v(1), i64v(2)].into(),
        });
        assert!(
            resolve_func(
                vec![BlockElement::Expr(sv(Value::EnumVariant {
                    span: ByteSpan::default(),
                    enum_def: edi,
                    variant: NString::from("A"),
                    value: inner
                }))],
                unitt(),
                &log,
                &mut s
            )
            .is_ok()
        );
    });
}

#[test]
fn test_monomorphize() {
    store(|_| {
        let mut s = sym();
        let p = param("x", GP(0, "T"));
        let bv = sv(Value::ParameterSymbol {
            span: ByteSpan::default(),
            id: p.clone(),
        });
        let mut gens = BTreeMap::new();
        gens.insert(NString::from("T"), Some(GP(0, "T")));
        let fid = mkfunc(
            "gf",
            vec![p],
            GP(0, "T"),
            Some(vec![BlockElement::Expr(bv)]),
            Some(gens),
        );
        let mut sub = Substitution::default();
        sub.mapping.insert(0, i32t());
        let mut solver = Solver::new(&mut s);
        let mid = solver.monomorphize_function(&fid, &sub);
        assert!(mid.borrow().name.contains("mono"));
        assert!(matches!(&*mid.borrow().return_type, Type::I32 { .. }));
        let sd = mkstruct("GP", vec![("f", GP(0, "T"))], Some(vec!["T"]), None);
        let mid2 = solver.monomorphize_struct(&sd, &sub);
        assert!(mid2.borrow().name.contains("mono") && mid2.borrow().generics.is_none());
        assert_eq!(
            solver.monomorphize_function(&fid, &sub).as_usize(),
            solver.monomorphize_function(&fid, &sub).as_usize()
        );
        let mid3 = solver.monomorphize_struct(&sd, &sub);
        assert_eq!(mid2.as_usize(), mid3.as_usize());
    });
}

#[test]
fn test_monomorphize_with_body_containing_local() {
    store(|_| {
        let mut s = sym();
        let p = param("x", GP(0, "T"));
        let body = vec![BlockElement::Local(local(
            "y",
            GP(0, "T"),
            sv(Value::ParameterSymbol {
                span: ByteSpan::default(),
                id: p.clone(),
            }),
        ))];
        let mut gens = BTreeMap::new();
        gens.insert(NString::from("T"), Some(GP(0, "T")));
        let fid = mkfunc("gf2", vec![p], GP(0, "T"), Some(body), Some(gens));
        let mut sub = Substitution::default();
        sub.mapping.insert(0, i32t());
        let mut solver = Solver::new(&mut s);
        let mid = solver.monomorphize_function(&fid, &sub);
        assert!(mid.borrow().name.contains("mono"));
    });
}

#[test]
fn test_monomorphize_struct_with_generic_not_in_fields() {
    store(|_| {
        let mut s = sym();
        let sd = mkstruct("Wrapper", vec![("x", i32t())], Some(vec!["T"]), None);
        let mut sub = Substitution::default();
        sub.mapping.insert(0, i32t());
        let mut solver = Solver::new(&mut s);
        let mid = solver.monomorphize_struct(&sd, &sub);
        assert!(mid.borrow().name.contains("mono"));
        assert!(mid.borrow().generics.is_none());
    });
}

#[test]
fn test_mono_depth_limit() {
    store(|_| {
        let mut s = sym();
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
fn test_mono_struct_depth_limit() {
    store(|_| {
        let mut s = sym();
        let mut solver = Solver::new(&mut s);
        solver.mono_depth = MAX_MONO_DEPTH;
        let r = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
            solver.monomorphize_struct(
                &mkstruct("S", vec![("x", i32t())], None, None),
                &Substitution::default(),
            );
        }));
        assert!(r.is_err());
    });
}

// ═══ INFERRED INTEGER/FLOAT SOLVING ═══

#[test]
fn test_solve_inferred_int_to_u8() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let inferred = inf_int(42);
        assert!(
            resolve_func(
                vec![BlockElement::Local(local("x", u8t(), inferred))],
                unitt(),
                &log,
                &mut s
            )
            .is_ok()
        );
    });
}

#[test]
fn test_solve_inferred_int_to_u16() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let inferred = inf_int(42);
        assert!(
            resolve_func(
                vec![BlockElement::Local(local("x", u16t(), inferred))],
                unitt(),
                &log,
                &mut s
            )
            .is_ok()
        );
    });
}

#[test]
fn test_solve_inferred_int_to_u32() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let inferred = inf_int(42);
        assert!(
            resolve_func(
                vec![BlockElement::Local(local("x", u32t(), inferred))],
                unitt(),
                &log,
                &mut s
            )
            .is_ok()
        );
    });
}

#[test]
fn test_solve_inferred_int_to_u64() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let inferred = inf_int(42);
        assert!(
            resolve_func(
                vec![BlockElement::Local(local("x", u64t(), inferred))],
                unitt(),
                &log,
                &mut s
            )
            .is_ok()
        );
    });
}

#[test]
fn test_solve_inferred_int_to_i16() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let inferred = inf_int(42);
        assert!(
            resolve_func(
                vec![BlockElement::Local(local("x", i16t(), inferred))],
                unitt(),
                &log,
                &mut s
            )
            .is_ok()
        );
    });
}

#[test]
fn test_solve_inferred_int_to_i64() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let inferred = inf_int(42);
        assert!(
            resolve_func(
                vec![BlockElement::Local(local("x", i64t(), inferred))],
                unitt(),
                &log,
                &mut s
            )
            .is_ok()
        );
    });
}

#[test]
fn test_solve_inferred_int_to_i128() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let inferred = inf_int(42);
        assert!(
            resolve_func(
                vec![BlockElement::Local(local("x", i128t(), inferred))],
                unitt(),
                &log,
                &mut s
            )
            .is_ok()
        );
    });
}

#[test]
fn test_solve_inferred_int_to_u128() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let inferred = inf_int(42);
        assert!(
            resolve_func(
                vec![BlockElement::Local(local("x", u128t(), inferred))],
                unitt(),
                &log,
                &mut s
            )
            .is_ok()
        );
    });
}

#[test]
fn test_solve_inferred_int_refinement_constraint() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let inferred = inf_int(42);
        let refine = TypeId::from(Type::Refine {
            span: ByteSpan::default(),
            base: i32t(),
            min: lit(Lit::I32(0)),
            max: lit(Lit::I32(100)),
        });
        assert!(
            resolve_func(
                vec![BlockElement::Local(local("x", refine, inferred))],
                unitt(),
                &log,
                &mut s
            )
            .is_ok()
        );
    });
}

#[test]
fn test_solve_inferred_int_refinement_out_of_bounds() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let inferred = inf_int(200);
        let refine = TypeId::from(Type::Refine {
            span: ByteSpan::default(),
            base: i32t(),
            min: lit(Lit::I32(0)),
            max: lit(Lit::I32(100)),
        });
        let result = resolve_func(
            vec![BlockElement::Local(local("x", refine, inferred))],
            unitt(),
            &log,
            &mut s,
        );
        let _ = result;
    });
}

#[test]
fn test_solve_inferred_int_non_integer_constraint() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let inferred = inf_int(42);
        let result = resolve_func(
            vec![BlockElement::Local(local("x", boolt(), inferred))],
            unitt(),
            &log,
            &mut s,
        );
        let _ = result;
    });
}

#[test]
fn test_solve_inferred_float_f32() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let inferred = inf_flt(3.14);
        assert!(
            resolve_func(
                vec![BlockElement::Local(local("x", f32t(), inferred))],
                unitt(),
                &log,
                &mut s
            )
            .is_ok()
        );
    });
}

#[test]
fn test_solve_inferred_float_f64() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let inferred = inf_flt(3.14);
        assert!(
            resolve_func(
                vec![BlockElement::Local(local("x", f64t(), inferred))],
                unitt(),
                &log,
                &mut s
            )
            .is_ok()
        );
    });
}

#[test]
fn test_solve_inferred_float_non_float_constraint() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let inferred = inf_flt(3.14);
        let result = resolve_func(
            vec![BlockElement::Local(local("x", boolt(), inferred))],
            unitt(),
            &log,
            &mut s,
        );
        let _ = result;
    });
}

// ═══ INFERRED LITERAL DEFAULTING ═══

#[test]
fn test_inferred_literal_defaulting_in_block() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let inner_block = Block {
            span: ByteSpan::default(),
            safety: BlockSafety::Safe,
            elements: vec![BlockElement::Expr(inf_int(99))],
        };
        let val = sv(Value::Block {
            span: ByteSpan::default(),
            block: BlockId::from(inner_block),
        });
        assert!(resolve_func(vec![BlockElement::Expr(val)], unitt(), &log, &mut s).is_ok());
    });
}

#[test]
fn test_inferred_literal_defaulting_in_if() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let tb = Block {
            span: ByteSpan::default(),
            safety: BlockSafety::Safe,
            elements: vec![BlockElement::Expr(inf_int(42))],
        };
        let fb = Block {
            span: ByteSpan::default(),
            safety: BlockSafety::Safe,
            elements: vec![BlockElement::Expr(inf_int(43))],
        };
        let val = sv(Value::If {
            span: ByteSpan::default(),
            condition: boolv(true),
            true_branch: BlockId::from(tb),
            false_branch: Some(BlockId::from(fb)),
        });
        assert!(resolve_func(vec![BlockElement::Expr(val)], unitt(), &log, &mut s).is_ok());
    });
}

// ═══ TYPE CONTAINING GENERIC PARAM ═══

#[test]
fn test_type_contains_generic_param_indirect() {
    store(|_| {
        let sd = mkstruct(
            "Holder",
            vec![(
                "ptr",
                TypeId::from(Type::Pointer {
                    span: ByteSpan::default(),
                    lifetime: nitrate_hir::Lifetime::Inferred,
                    exclusive: false,
                    mutable: false,
                    to: GP(0, "T"),
                }),
            )],
            Some(vec!["T"]),
            None,
        );
        let fields: ThinVec<(NString, ValueId)> = vec![(NString::from("ptr"), i32v(42))].into();
        let log = CompilerLog::default();
        let mut s = sym();
        let result = resolve_func(
            vec![BlockElement::Expr(sv(Value::StructObject {
                span: ByteSpan::default(),
                struct_def: sd,
                fields,
            }))],
            unitt(),
            &log,
            &mut s,
        );
        let _ = result;
    });
}

// ═══ DIAGNOSIS ═══

#[test]
fn test_diagnostic_all_variants() {
    store(|_| {
        use nitrate_diagnosis::{DiagnosticGroupId, FormattableDiagnosticGroup, Origin};
        let refine = TypeId::from(Type::Refine {
            span: ByteSpan::default(),
            base: i32t(),
            min: lit(Lit::I32(0)),
            max: lit(Lit::I32(100)),
        });
        let cases: Vec<(TypeErr, u16, &str)> = vec![
            (
                TypeErr::IntegerLiteralOutOfRange {
                    span: span(0, 1),
                    value: 256,
                    target_type: u8t(),
                },
                0,
                "256",
            ),
            (
                TypeErr::IntegerLiteralUnsatisfiable {
                    span: span(0, 1),
                    value: 42,
                    unsatisfiable_type: boolt(),
                },
                1,
                "42",
            ),
            (
                TypeErr::FloatLiteralUnsatisfiable {
                    span: span(0, 1),
                    value: OrderedFloat(3.14),
                    unsatisfiable_type: boolt(),
                },
                2,
                "3.14",
            ),
            (
                TypeErr::IntegerLiteralOutOfRefinementBounds {
                    span: span(0, 1),
                    value: 200,
                    refinement_type: refine,
                },
                3,
                "200",
            ),
            (
                TypeErr::OperationResultOutOfRefinementBounds {
                    span: span(0, 1),
                    refinement_type: refine,
                    computed_min: 0,
                    computed_max: 200,
                },
                4,
                "200",
            ),
            (
                TypeErr::MismatchedBranchTypes {
                    span: span(0, 1),
                    true_type: i32t(),
                    false_type: i64t(),
                },
                5,
                "i32",
            ),
            (
                TypeErr::CannotInferTypeArgs {
                    span: span(0, 1),
                    generic_name: "Foo".into(),
                    reason: "test".into(),
                },
                6,
                "Foo",
            ),
            (
                TypeErr::AmbiguousType {
                    span: span(0, 1),
                    description: "test".into(),
                },
                8,
                "test",
            ),
            (
                TypeErr::UnboundGenericParam {
                    span: span(0, 1),
                    param_name: "T".into(),
                    generic_name: "Box".into(),
                },
                11,
                "T",
            ),
            (
                TypeErr::MethodNotFound {
                    span: span(0, 1),
                    method_name: "bar".into(),
                    receiver_type: i32t(),
                },
                14,
                "bar",
            ),
        ];
        for (err, vid, expected) in &cases {
            assert_eq!(err.group_id(), DiagnosticGroupId::Type);
            assert_eq!(err.variant_id(), *vid);
            let info = err.format();
            assert!(
                info.message.contains(expected),
                "{} not in {:?}",
                expected,
                info.message
            );
        }
        let with_span = TypeErr::IntegerLiteralOutOfRange {
            span: span(10, 20),
            value: 0,
            target_type: i32t(),
        }
        .format();
        if let Origin::Span(s) = &with_span.origin {
            assert_eq!(s.start.offset, 10);
        }
        let with_point = TypeErr::IntegerLiteralOutOfRange {
            span: ByteSpan::default(),
            value: 0,
            target_type: i32t(),
        }
        .format();
        assert!(matches!(with_point.origin, Origin::Point(_)));
    });
}

#[test]
fn test_diagnostic_refine_bounds_format() {
    store(|_| {
        use nitrate_diagnosis::FormattableDiagnosticGroup;
        let r = TypeId::from(Type::Refine {
            span: ByteSpan::default(),
            base: i32t(),
            min: lit(Lit::I32(0)),
            max: lit(Lit::I32(100)),
        });
        let e = TypeErr::OperationResultOutOfRefinementBounds {
            span: span(0, 3),
            refinement_type: r,
            computed_min: 0,
            computed_max: 200,
        };
        let info = e.format();
        assert!(info.message.contains("expected"));
        let e2 = TypeErr::OperationResultOutOfRefinementBounds {
            span: span(0, 3),
            refinement_type: i32t(),
            computed_min: 0,
            computed_max: 200,
        };
        let info2 = e2.format();
        assert!(!info2.message.is_empty());
    });
}

// ═══ SPECIAL VISITOR PATHS ═══

#[test]
fn test_resolve_global_initializer_with_inferred_type() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let mut g = nitrate_hir::GlobalVariable {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            is_mutable: false,
            name: NString::from("V"),
            mangled_name: NString::from("V"),
            ty: TypeId::from(Type::Inferred {
                span: ByteSpan::default(),
                id: std::num::NonZeroU32::new(1).unwrap(),
                name: None,
            }),
            initializer: sv(Value::Binary {
                span: ByteSpan::default(),
                left: i32v(10),
                op: BinaryOp::Add,
                right: i32v(20),
            }),
        };
        assert!(resolve_global(&mut g, &mut s, &log).is_ok());
    });
}

#[test]
fn test_resolve_binary_with_refinement_bounds() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let refine = TypeId::from(Type::Refine {
            span: ByteSpan::default(),
            base: i32t(),
            min: lit(Lit::I32(0)),
            max: lit(Lit::I32(100)),
        });
        let binop = sv(Value::Binary {
            span: ByteSpan::default(),
            left: i32v(10),
            op: BinaryOp::Add,
            right: i32v(20),
        });
        let result = resolve_func(
            vec![BlockElement::Local(local("r", refine, binop))],
            unitt(),
            &log,
            &mut s,
        );
        let _ = result;
    });
}

#[test]
fn test_resolve_binary_with_refinement_bounds_out_of_range() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let refine = TypeId::from(Type::Refine {
            span: ByteSpan::default(),
            base: u8t(),
            min: lit(Lit::U8(0)),
            max: lit(Lit::U8(10)),
        });
        let binop = sv(Value::Binary {
            span: ByteSpan::default(),
            left: u8v(10),
            op: BinaryOp::Add,
            right: u8v(20),
        });
        let result = resolve_func(
            vec![BlockElement::Local(local("r", refine, binop))],
            unitt(),
            &log,
            &mut s,
        );
        let _ = result;
    });
}

fn u8v(v: u8) -> ValueId {
    sv(Value::U8 {
        span: ByteSpan::default(),
        value: v,
    })
}

#[test]
fn test_resolve_binary_default_fallback() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let binop = sv(Value::Binary {
            span: ByteSpan::default(),
            left: inf_int(10),
            op: BinaryOp::Add,
            right: inf_int(20),
        });
        assert!(resolve_func(vec![BlockElement::Expr(binop)], unitt(), &log, &mut s).is_ok());
    });
}

#[test]
fn test_resolve_list_with_inferred_elements_and_parent_constraint() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let list = sv(Value::List {
            span: ByteSpan::default(),
            elements: vec![inf_int(1), inf_int(2)].into(),
        });
        assert!(
            resolve_func(
                vec![BlockElement::Local(local("xs", u32t(), list))],
                unitt(),
                &log,
                &mut s
            )
            .is_ok()
        );
    });
}

#[test]
fn test_method_call_generic_method() {
    store(|_| {
        let log = CompilerLog::default();
        let mut s = sym();
        let sd = mkstruct("Pt", vec![("x", GP(0, "T")), ("y", GP(0, "T"))], Some(vec!["T"]), None);
        s.add_struct(sd.clone());
        let st_id = TypeId::from(Type::Struct {
            span: ByteSpan::default(),
            def: sd.clone(),
        });
        let p = param("self", st_id);
        let ret = sv(Value::Return {
            span: ByteSpan::default(),
            value: sv(Value::ParameterSymbol {
                span: ByteSpan::default(),
                id: p.clone(),
            }),
        });
        let mid = mkfunc("get_x", vec![p], GP(0, "T"), Some(vec![BlockElement::Expr(ret)]), None);
        s.add_method(
            TypeId::from(Type::Struct {
                span: ByteSpan::default(),
                def: sd.clone(),
            }),
            NString::from("get_x"),
            mid,
        );
        let fields: ThinVec<(NString, ValueId)> =
            vec![(NString::from("x"), i32v(10)), (NString::from("y"), i32v(20))].into();
        let obj = sv(Value::StructObject {
            span: ByteSpan::default(),
            struct_def: sd,
            fields,
        });
        let mc = sv(Value::MethodCall {
            span: ByteSpan::default(),
            object: obj,
            method_name: NString::from("get_x"),
            args: Arguments {
                positional: ThinVec::new(),
                named: ThinVec::new(),
            },
        });
        assert!(resolve_func(vec![BlockElement::Expr(mc)], i32t(), &log, &mut s).is_ok());
    });
}

#[test]
fn test_monomorphize_with_cast() {
    store(|_| {
        let mut s = sym();
        let body = vec![BlockElement::Expr(sv(Value::Cast {
            span: ByteSpan::default(),
            value: i32v(42),
            target_type: GP(0, "T"),
        }))];
        let mut gens = BTreeMap::new();
        gens.insert(NString::from("T"), Some(GP(0, "T")));
        let fid = mkfunc("cast_fn", vec![], GP(0, "T"), Some(body), Some(gens));
        let mut sub = Substitution::default();
        sub.mapping.insert(0, i64t());
        let mut solver = Solver::new(&mut s);
        let mid = solver.monomorphize_function(&fid, &sub);
        assert!(mid.borrow().name.contains("mono"));
    });
}

#[test]
fn test_monomorphize_with_generic_struct_value() {
    store(|_| {
        let mut s = sym();
        let sd = mkstruct("GP", vec![("f", GP(0, "T"))], Some(vec!["T"]), None);
        let body = vec![BlockElement::Expr(sv(Value::StructObject {
            span: ByteSpan::default(),
            struct_def: sd,
            fields: vec![(
                NString::from("f"),
                sv(Value::ParameterSymbol {
                    span: ByteSpan::default(),
                    id: param("x", GP(0, "T")),
                }),
            )]
            .into(),
        }))];
        let p = param("x", GP(0, "T"));
        let mut gens = BTreeMap::new();
        gens.insert(NString::from("T"), Some(GP(0, "T")));
        let fid = mkfunc("make_gp", vec![p], GP(0, "T"), Some(body), Some(gens));
        let mut sub = Substitution::default();
        sub.mapping.insert(0, i32t());
        let mut solver = Solver::new(&mut s);
        let mid = solver.monomorphize_function(&fid, &sub);
        assert!(mid.borrow().name.contains("mono"));
    });
}
