//! Tests for the suspended constraint solver (hir_solve2).
//!
//! These tests verify the core constraint graph, unification, bounds analysis,
//! substitution, and the full end-to-end solving pipeline.

use crate::bounds::{
    Bounds, check_bounds_against_constraint, check_literal_against_refinement, compute_binary_bounds,
    compute_unary_bounds, extract_bounds_from_type, integer_primitive_bounds, lit_to_i128,
};
use crate::constraints::{ConstraintGraph, is_arithmetic_op, is_comparison_or_logical_op};
use crate::diagnosis::TypeErr;
use crate::monomorphize::{MonoCacheKey, Monomorphizer};
use crate::substitution::Substitution;
use nitrate_hir::{BinaryOp, Lit, PtrSize, Store, SymbolTab, Type, TypeId, UnaryOp, Value, ValueId, using_storage};
use nitrate_nstring::NString;
use nitrate_tree::ByteSpan;
use ordered_float::OrderedFloat;
use std::collections::{BTreeSet, HashMap, HashSet};

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
fn lit(l: Lit) -> nitrate_hir::LiteralId {
    nitrate_hir::LiteralId::from(l)
}

// ── Bounds tests ────────────────────────────────────────────────────

#[test]
fn test_bounds_int_primitive() {
    store(|_| {
        assert_eq!(integer_primitive_bounds(&*u8t()), Some(Bounds::unsigned(0, 255)));
        assert_eq!(integer_primitive_bounds(&*u16t()), Some(Bounds::unsigned(0, 65535)));
        assert_eq!(
            integer_primitive_bounds(&*u32t()),
            Some(Bounds::unsigned(0, 4294967295))
        );
        assert_eq!(
            integer_primitive_bounds(&*u64t()),
            Some(Bounds::unsigned(0, 18446744073709551615))
        );
        assert_eq!(
            integer_primitive_bounds(&*u128t()),
            Some(Bounds::unsigned(0, u128::MAX))
        );
        assert_eq!(
            integer_primitive_bounds(&*uszt()),
            Some(Bounds::unsigned(0, 18446744073709551615))
        );
        assert_eq!(integer_primitive_bounds(&*i8t()), Some(Bounds::signed(-128, 127)));
        assert_eq!(integer_primitive_bounds(&*i16t()), Some(Bounds::signed(-32768, 32767)));
        assert_eq!(
            integer_primitive_bounds(&*i32t()),
            Some(Bounds::signed(-2147483648, 2147483647))
        );
        assert_eq!(
            integer_primitive_bounds(&*i64t()),
            Some(Bounds::signed(-9223372036854775808, 9223372036854775807))
        );
        assert_eq!(
            integer_primitive_bounds(&*i128t()),
            Some(Bounds::signed(i128::MIN, i128::MAX))
        );
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
        assert_eq!(extract_bounds_from_type(&*r), Some(Bounds::new(0, 100)));
        assert_eq!(
            extract_bounds_from_type(&*i32t()),
            Some(Bounds::signed(-2147483648, 2147483647))
        );
        assert_eq!(extract_bounds_from_type(&*boolt()), None);
    });
}

#[test]
fn test_bounds_binary() {
    store(|_| {
        assert_eq!(
            compute_binary_bounds(&BinaryOp::Add, Bounds::new(1, 10), Bounds::new(20, 30)),
            Some(Bounds::new(21, 40))
        );
        assert_eq!(
            compute_binary_bounds(&BinaryOp::Sub, Bounds::new(10, 20), Bounds::new(1, 5)),
            Some(Bounds::new(5, 19))
        );
        assert_eq!(
            compute_binary_bounds(&BinaryOp::Mul, Bounds::new(-5, 5), Bounds::new(-5, 5)),
            Some(Bounds::new(-25, 25))
        );
    });
}

#[test]
fn test_bounds_unary() {
    store(|_| {
        assert_eq!(
            compute_unary_bounds(&UnaryOp::Add, Bounds::new(-5, 10)),
            Bounds::new(-5, 10)
        );
        assert_eq!(
            compute_unary_bounds(&UnaryOp::Sub, Bounds::new(-5, 10)),
            Bounds::new(-10, 5)
        );
        assert_eq!(
            compute_unary_bounds(&UnaryOp::Not, Bounds::new(0, 255)),
            Bounds::new(!255_i128, !0_i128 as u128)
        );
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
        assert!(check_bounds_against_constraint(Bounds::new(10, 50), &*r));
        assert!(!check_bounds_against_constraint(Bounds::new(10, 200), &*r));
        assert!(check_bounds_against_constraint(Bounds::new(10, 200), &*i32t()));
        assert!(check_literal_against_refinement(42, &*r));
        assert!(!check_literal_against_refinement(200, &*r));
    });
}

#[test]
fn test_bounds_lit() {
    store(|_| {
        assert_eq!(lit_to_i128(&Lit::U8(42)), Some(42));
        assert_eq!(lit_to_i128(&Lit::I8(-5)), Some(-5));
        assert_eq!(lit_to_i128(&Lit::I16(-1000)), Some(-1000));
        assert_eq!(lit_to_i128(&Lit::I32(-100000)), Some(-100000));
        assert_eq!(lit_to_i128(&Lit::Bool(true)), None);
        assert_eq!(lit_to_i128(&Lit::F32(OrderedFloat(1.5))), None);
        assert_eq!(lit_to_i128(&Lit::F64(OrderedFloat(2.5))), None);
    });
}

// ── Constraint / operation classification tests ─────────────────────

#[test]
fn test_op_classification() {
    assert!(is_comparison_or_logical_op(&BinaryOp::Lt));
    assert!(!is_comparison_or_logical_op(&BinaryOp::Add));
    assert!(is_arithmetic_op(&BinaryOp::Ror));
    assert!(!is_arithmetic_op(&BinaryOp::Lt));
}

// ── Constraint graph tests ──────────────────────────────────────────

#[test]
fn test_constraint_graph_fresh_var() {
    let mut graph = ConstraintGraph::new();
    let v1 = graph.fresh_var();
    let v2 = graph.fresh_var();
    assert_ne!(v1, v2);
}

#[test]
fn test_constraint_graph_concrete_equality() {
    store(|_| {
        let mut graph = ConstraintGraph::new();
        let v = graph.fresh_var();
        // Variable = Concrete
        graph.add_equality(
            crate::constraints::CanonicalType::Variable(v),
            crate::constraints::CanonicalType::Concrete(i32t()),
            crate::constraints::ConstraintSource::Other,
        );
        assert!(graph.solve_equalities());
        assert_eq!(
            graph.resolve(&crate::constraints::CanonicalType::Variable(v)),
            Some(i32t())
        );
    });
}

#[test]
fn test_constraint_graph_conflict() {
    store(|_| {
        let mut graph = ConstraintGraph::new();
        let v = graph.fresh_var();
        // Variable = i32, then Variable = f64 → conflict
        graph.add_equality(
            crate::constraints::CanonicalType::Variable(v),
            crate::constraints::CanonicalType::Concrete(i32t()),
            crate::constraints::ConstraintSource::Other,
        );
        graph.add_equality(
            crate::constraints::CanonicalType::Variable(v),
            crate::constraints::CanonicalType::Concrete(f64t()),
            crate::constraints::ConstraintSource::Other,
        );
        graph.solve_equalities();
        let errors = graph.drain_errors();
        assert!(!errors.is_empty());
    });
}

// ── Substitution tests ──────────────────────────────────────────────

#[test]
fn test_subst_generic() {
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
            Type::I32 {
                span: ByteSpan::default(),
            },
            Type::F64 {
                span: ByteSpan::default(),
            },
        ] {
            assert_eq!(sub.apply(leaf), *leaf);
        }
    });
}

// ── MonoCacheKey tests ──────────────────────────────────────────────

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

// ── Range name tests ────────────────────────────────────────────────

#[test]
fn test_range_struct_names() {
    assert_eq!(crate::range_struct_name(true, true, false), "Range");
    assert_eq!(crate::range_struct_name(true, true, true), "RangeInclusive");
    assert_eq!(crate::range_struct_name(true, false, false), "RangeFrom");
    assert_eq!(crate::range_struct_name(false, true, false), "RangeTo");
    assert_eq!(crate::range_struct_name(false, true, true), "RangeToInclusive");
    assert_eq!(crate::range_struct_name(false, false, false), "RangeFull");
}
