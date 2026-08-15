//! Regression tests for specific bugs fixed in `llvm_from_mir`.
//!
//! Each test documents a concrete miscompilation or panic that was fixed:
//! - Signed comparison/division semantics derived from the wrong type
//! - Static globals loading incorrect types
//! - Enum aggregates collapsing to `Unit`
//! - Block-argument index arithmetic producing out-of-order phi mismatches

use crate::test_common::{Harness, fn_binary};
use nitrate_mir::prelude as mir;

/// A signed comparison was emitted as an unsigned comparison because the
/// signedness was derived from the boolean result type. It must come from the
/// left operand's type.
#[test]
fn signed_comparison_uses_signed_predicate() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        fn_binary(
            b,
            "cmp",
            mir::MirType::Bool.into(),
            mir::MirBinaryOp::Lt,
            mir::Operand::Constant(mir::MirLiteral::I32(-1)),
            mir::Operand::Constant(mir::MirLiteral::I32(1)),
        );
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("icmp slt"), "signed `-1 < 1` must use `slt`, got: {ir}");
}

/// Unsigned division was emitted as signed division (`sdiv`), which yields the
/// wrong result for values with the high bit set.
#[test]
fn unsigned_division_uses_udiv() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        fn_binary(
            b,
            "div",
            mir::MirType::U32.into(),
            mir::MirBinaryOp::Div,
            mir::Operand::Constant(mir::MirLiteral::U32(u32::MAX)),
            mir::Operand::Constant(mir::MirLiteral::U32(2)),
        );
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("udiv"), "unsigned division must be `udiv`, got: {ir}");
}

/// Signed shift-right was emitted as logical shift (`lshr`), which fails to
/// sign-extend negative values.
#[test]
fn signed_shift_right_uses_ashr() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        fn_binary(
            b,
            "shr",
            mir::MirType::I64.into(),
            mir::MirBinaryOp::Shr,
            mir::Operand::Constant(mir::MirLiteral::I64(-8)),
            mir::Operand::Constant(mir::MirLiteral::I64(1)),
        );
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("ashr"), "signed shift must be `ashr`, got: {ir}");
}

/// A static global's type was guessed from the LLVM type, causing every struct
/// static to be loaded as `Unit`. Now the MIR type is preserved.
#[test]
fn static_global_loads_its_mir_type() {
    let h = Harness::new();
    let mut module = h.build_module(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        f.create_block();
        f.ret(Some(mir::Operand::Copy(mir::Place::Static("g".into()))));
        f.finish_function();
    });
    let ty: mir::MirTypeId = mir::using_storage(&h.store, || mir::MirType::I32.into());
    module.globals.push(mir::MirGlobal {
        name: "g".into(),
        ty,
        body: None,
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("load i32"), "global must load as i32, got: {ir}");
}

/// Enum aggregates previously collapsed to a constant `Unit`, losing the
/// payload and discriminant. They must now build a tagged struct.
#[test]
fn enum_aggregate_preserves_payload_and_tag() {
    let h = Harness::new();
    let ir = h.build_ir(|b| {
        let enum_ty = mir::MirType::Enum {
            name: "E".into(),
            variants: thin_vec::thin_vec![
                mir::MirEnumVariant {
                    name: "A".into(),
                    payload: Some(mir::MirType::I32.into())
                },
                mir::MirEnumVariant {
                    name: "B".into(),
                    payload: None
                },
            ],
        };
        let enum_id: mir::MirTypeId = enum_ty.into();
        let mut f = b.start_function("f".into(), enum_id);
        let tmp = f.new_temp(enum_id, false);
        f.create_block();
        f.push_assign(
            mir::Place::Local(tmp.clone()),
            mir::Rvalue::Aggregate(
                mir::AggregateKind::Enum {
                    name: "E".into(),
                    variant_name: "A".into(),
                    variant_index: 0,
                    enum_ty: enum_id,
                },
                thin_vec::thin_vec![mir::Operand::Constant(mir::MirLiteral::I32(42))],
            ),
        );
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(tmp))));
        f.finish_function();
    });
    assert!(ir.contains("store"), "enum aggregate must store: {ir}");
    assert!(!ir.contains("ret {}"), "enum must not collapse to unit: {ir}");
}

/// SetDiscriminant previously was a no-op. It must write the tag field.
#[test]
fn set_discriminant_writes_tag_field() {
    let h = Harness::new();
    let ir = h.build_ir(|b| {
        let enum_ty = mir::MirType::Enum {
            name: "E".into(),
            variants: thin_vec::thin_vec![
                mir::MirEnumVariant {
                    name: "A".into(),
                    payload: None
                },
                mir::MirEnumVariant {
                    name: "B".into(),
                    payload: None
                },
            ],
        };
        let enum_id: mir::MirTypeId = enum_ty.into();
        let mut f = b.start_function("f".into(), enum_id);
        let e = f.new_temp(enum_id, false);
        f.create_block();
        f.push_set_discriminant(mir::Place::Local(e), 1);
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(e))));
        f.finish_function();
    });
    assert!(ir.contains("store i8 1"), "set_discriminant must store tag=1: {ir}");
}

/// Block arguments were mapped to locals via fragile index arithmetic that
/// broke when blocks were emitted out of order. The direct recording of arg
/// locals keeps phi stores correct regardless of order.
#[test]
fn block_argument_phi_verifies_with_multiple_args() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let merge = f.reserve_block_with_args(&[mir::MirType::I32.into(), mir::MirType::I32.into()]);
        let a = f.reserve_block();
        let c = f.reserve_block();
        f.create_block();
        f.if_br(mir::Operand::Constant(mir::MirLiteral::Bool(true)), a, c);
        f.switch_to_block(a);
        f.goto_with_args(
            merge.block,
            thin_vec::thin_vec![
                mir::Operand::Constant(mir::MirLiteral::I32(1)),
                mir::Operand::Constant(mir::MirLiteral::I32(2)),
            ],
        );
        f.switch_to_block(c);
        f.goto_with_args(
            merge.block,
            thin_vec::thin_vec![
                mir::Operand::Constant(mir::MirLiteral::I32(3)),
                mir::Operand::Constant(mir::MirLiteral::I32(4)),
            ],
        );
        f.switch_to_block(merge.block);
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(merge.arg_locals[0].clone()))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    let phis = ir.matches("phi").count();
    assert_eq!(phis, 2, "each block argument must lower to its own phi: {ir}");
}

/// String statics used to report `Unit` type, so loading a string global was
/// impossible. A `Place::Static` of a string must load as `ptr`.
#[test]
fn string_static_maps_to_str_type() {
    let h = Harness::new();
    let ir = h.build_ir(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::Str.into());
        f.create_block();
        f.ret(Some(mir::Operand::Copy(mir::Place::Static("__nitrate_str_0".into()))));
        f.finish_function();
    });
    assert!(ir.contains("ptr"), "string literal static must be a pointer: {ir}");
}

/// Struct field GEP previously errored when a field name referenced a padding
/// cell. A field lookup in the layout must only match `Field` cells.
#[test]
fn field_gep_skips_padding_cells() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let struct_ty = mir::MirType::Struct {
            name: "Padded".into(),
            fields: thin_vec::thin_vec![("a".into(), mir::MirType::I8.into())],
            layout: thin_vec::thin_vec![
                mir::MirStructLayoutCell::Field { field_name: "a".into() },
                mir::MirStructLayoutCell::Padding(std::num::NonZeroU32::new(3).unwrap()),
            ],
        };
        let struct_id: mir::MirTypeId = struct_ty.into();
        let mut f = b.start_function("f".into(), mir::MirType::I8.into());
        let p = f.new_temp(struct_id, false);
        f.create_block();
        let field = mir::Place::Field {
            base: Box::new(mir::Place::Local(p)),
            field_name: "a".into(),
        };
        f.ret(Some(mir::Operand::Copy(field)));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("getelementptr"), "expected field GEP: {ir}");
}
