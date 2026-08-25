//! Targeted tests that exercise specific lower-level branches for coverage:
//! unit literals, constant-operand signedness, padding cells during field
//! search, and anonymous struct aggregate fallback.

use crate::test_common::{Harness, fn_binary};
use nitrate_mir::prelude as mir;

/// `MirLiteral::Unit` as a value inside a tuple aggregate (not a void return)
/// forces the `gen_literal` Unit arm.
#[test]
fn unit_literal_inside_tuple_aggregate() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let tuple_ty = mir::MirType::Tuple {
            element_types: thin_vec::thin_vec![mir::MirType::Unit.into(), mir::MirType::I32.into()],
        };
        let tuple_id: mir::MirTypeId = tuple_ty.into();
        let mut f = b.start_function("f".into(), tuple_id);
        let tmp = f.new_temp(tuple_id, false);
        f.create_block();
        f.push_assign(
            mir::Place::Local(tmp.clone()),
            mir::Rvalue::Aggregate(
                mir::AggregateKind::Tuple,
                thin_vec::thin_vec![
                    mir::Operand::Constant(mir::MirLiteral::Unit),
                    mir::Operand::Constant(mir::MirLiteral::I32(7)),
                ],
            ),
        );
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(tmp))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module), "module invalid: {ir}");
    assert!(ir.contains("{ {}, i32 }"), "expected a unit+i32 tuple type: {ir}");
}

/// A constant Bool operand in a logical op drives `operand_signedness`'s Bool
/// arm (which returns unsigned).
#[test]
fn constant_bool_operand_signedness() {
    let h = Harness::new();
    let ir = h.build_ir(|b| {
        fn_binary(
            b,
            "f",
            mir::MirType::Bool.into(),
            mir::MirType::Bool.into(),
            mir::MirType::Bool.into(),
            mir::MirBinaryOp::LogicAnd,
        );
    });
    assert!(ir.contains("and"), "expected logical and: {ir}");
}

/// A constant float operand drives `operand_signedness`'s `None` fallback.
#[test]
fn constant_float_operand_signedness_is_none() {
    let h = Harness::new();
    let ir = h.build_ir(|b| {
        fn_binary(
            b,
            "f",
            mir::MirType::F32.into(),
            mir::MirType::F32.into(),
            mir::MirType::Bool.into(),
            mir::MirBinaryOp::Lt,
        );
    });
    assert!(ir.contains("fcmp"), "expected float compare: {ir}");
}

/// A struct whose layout begins with a padding cell forces the field lookup to
/// iterate over a `Padding` cell (the `_ => false` arm) before finding the
/// field.
#[test]
fn field_lookup_iterates_padding_cell_first() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let struct_ty = mir::MirType::Struct {
            name: "PaddedFirst".into(),
            fields: thin_vec::thin_vec![("x".into(), mir::MirType::I32.into())],
            layout: thin_vec::thin_vec![
                mir::MirStructLayoutCell::Padding(std::num::NonZeroU32::new(4).unwrap()),
                mir::MirStructLayoutCell::Field { field_name: "x".into() },
            ],
        };
        let struct_id: mir::MirTypeId = struct_ty.into();
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let s = f.new_temp(struct_id, false);
        f.create_block();
        let field = mir::Place::Field {
            base: Box::new(mir::Place::Local(s)),
            field_name: "x".into(),
        };
        f.ret(Some(mir::Operand::Copy(field)));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("getelementptr"), "expected GEP: {ir}");
}

/// A struct aggregate whose named type has not been registered in the module
/// falls back to an anonymous struct type.
#[test]
fn struct_aggregate_anonymous_fallback() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        // Note: The destination type is a tuple (anonymous), yet the aggregate
        // kind names a struct "S" which does not exist in the module. This
        // forces gen_aggregate to fall back to an anonymous struct built from
        // the operand types.
        let dst_ty = mir::MirType::Tuple {
            element_types: thin_vec::thin_vec![mir::MirType::I32.into()],
        };
        let dst_id: mir::MirTypeId = dst_ty.into();
        let mut f = b.start_function("f".into(), dst_id);
        let tmp = f.new_temp(dst_id, false);
        f.create_block();
        f.push_assign(
            mir::Place::Local(tmp.clone()),
            mir::Rvalue::Aggregate(
                mir::AggregateKind::Struct("S".into(), thin_vec::thin_vec!["x".into()]),
                thin_vec::thin_vec![mir::Operand::Constant(mir::MirLiteral::I32(9))],
            ),
        );
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(tmp))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module), "module invalid: {ir}");
    assert!(ir.contains("{ i32 }"), "expected anonymous struct: {ir}");
}

/// Constant string operand in a binary-op-free expression (as a function
/// return) exercises the `Constant(Str)` literal arm.
#[test]
fn constant_str_literal_arm() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::Str.into());
        f.create_block();
        f.ret(Some(mir::Operand::Constant(mir::MirLiteral::Str("gap".into()))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("gap"), "expected string literal: {ir}");
}

/// Constant byte-string literal arm (BStr) as fat pointer.
#[test]
fn constant_bstr_literal_arm() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let slice_ty = mir::MirType::SliceRef {
            exclusive: false,
            mutable: false,
            element_type: mir::MirType::U8.into(),
        };
        let slice_id: mir::MirTypeId = slice_ty.into();
        let mut f = b.start_function("f".into(), slice_id);
        f.create_block();
        f.ret(Some(mir::Operand::Constant(mir::MirLiteral::BStr(
            thin_vec::thin_vec![1, 2],
        ))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    let width = h.llvm.ptr_size() * 8;
    assert!(ir.contains(&format!("i{width} 2")), "expected len 2: {ir}");
}
