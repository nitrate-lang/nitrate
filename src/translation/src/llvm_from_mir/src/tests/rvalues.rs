//! Tests for rvalue lowering: Use, Ref, Len, aggregates, and nullary ops.

use crate::test_common::Harness;
use nitrate_mir::prelude as mir;

fn assign_return(h: &Harness, rv: mir::Rvalue, dest_ty: mir::MirType) -> String {
    h.build_ir(|b| {
        let dest_id: mir::MirTypeId = dest_ty.into();
        let mut f = b.start_function("f".into(), dest_id);
        let tmp = f.new_temp(dest_id, false);
        f.create_block();
        f.push_assign(mir::Place::Local(tmp.clone()), rv);
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(tmp))));
        f.finish_function();
    })
}

#[test]
fn use_operand_loads_from_place() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let x = f.new_temp(mir::MirType::I32.into(), false);
        f.create_block();
        f.push_assign(
            mir::Place::Local(x.clone()),
            mir::Rvalue::Use(mir::Operand::Constant(mir::MirLiteral::I32(9))),
        );
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(x))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("ret i32 9"), "expected use rvalue: {ir}");
}

#[test]
fn ref_rvalue_produces_pointer_without_copy() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let ref_ty = mir::MirType::Reference {
            exclusive: false,
            mutable: false,
            to: mir::MirType::I32.into(),
        };
        let ref_id: mir::MirTypeId = ref_ty.into();
        let mut f = b.start_function("f".into(), ref_id);
        let x = f.new_temp(mir::MirType::I32.into(), false);
        f.create_block();
        f.push_assign(
            mir::Place::Local(x.clone()),
            mir::Rvalue::Use(mir::Operand::Constant(mir::MirLiteral::I32(5))),
        );
        let r = f.new_temp(ref_id, false);
        f.push_assign(
            mir::Place::Local(r.clone()),
            mir::Rvalue::Ref {
                region: mir::BorrowKind::Shared,
                place: mir::Place::Local(x),
            },
        );
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(r))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    // The borrow value is the address itself; the pointee is not loaded.
    assert!(!ir.contains("load i32"), "borrow should not copy pointee: {ir}");
}

#[test]
fn size_of_returns_type_size() {
    let h = Harness::new();
    let ir = assign_return(
        &h,
        mir::Rvalue::NullaryOp(mir::NullaryOp::SizeOf, mir::MirType::I64.into()),
        mir::MirType::USize,
    );
    assert!(ir.contains("i64 8"), "expected size constant: {ir}");
}

#[test]
fn align_of_returns_type_alignment() {
    let h = Harness::new();
    let ir = assign_return(
        &h,
        mir::Rvalue::NullaryOp(mir::NullaryOp::AlignOf, mir::MirType::I64.into()),
        mir::MirType::USize,
    );
    assert!(ir.contains("ret"), "expected align constant: {ir}");
}

#[test]
fn tuple_aggregate_builds_struct() {
    let h = Harness::new();
    let tuple_ty = mir::MirType::Tuple {
        element_types: thin_vec::thin_vec![mir::MirType::I32.into(), mir::MirType::I32.into()],
    };
    let ir = assign_return(
        &h,
        mir::Rvalue::Aggregate(
            mir::AggregateKind::Tuple,
            thin_vec::thin_vec![
                mir::Operand::Constant(mir::MirLiteral::I32(1)),
                mir::Operand::Constant(mir::MirLiteral::I32(2)),
            ],
        ),
        tuple_ty,
    );
    assert!(ir.contains("{ i32, i32 }"), "expected tuple type: {ir}");
}

#[test]
fn array_aggregate_builds_array() {
    let h = Harness::new();
    let arr_ty = mir::MirType::Array {
        element_type: mir::MirType::I32.into(),
        len: 3,
    };
    let ir = assign_return(
        &h,
        mir::Rvalue::Aggregate(
            mir::AggregateKind::Array(mir::MirType::I32.into()),
            thin_vec::thin_vec![
                mir::Operand::Constant(mir::MirLiteral::I32(1)),
                mir::Operand::Constant(mir::MirLiteral::I32(2)),
                mir::Operand::Constant(mir::MirLiteral::I32(3)),
            ],
        ),
        arr_ty,
    );
    assert!(ir.contains("[3 x i32]"), "expected array type: {ir}");
}

#[test]
fn struct_aggregate_uses_named_type() {
    let h = Harness::new();
    let struct_ty = mir::MirType::Struct {
        name: "S".into(),
        fields: thin_vec::thin_vec![("x".into(), mir::MirType::I32.into())],
        layout: thin_vec::thin_vec![mir::MirStructLayoutCell::Field { field_name: "x".into() }],
    };
    let ir = assign_return(
        &h,
        mir::Rvalue::Aggregate(
            mir::AggregateKind::Struct("S".into(), thin_vec::thin_vec!["x".into()]),
            thin_vec::thin_vec![mir::Operand::Constant(mir::MirLiteral::I32(7))],
        ),
        struct_ty,
    );
    assert!(ir.contains("%S"), "expected named struct in IR: {ir}");
}

#[test]
fn enum_aggregate_writes_discriminant() {
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
    assert!(ir.contains("store"), "expected enum stores: {ir}");
}

#[test]
fn len_of_slice_loads_length_field() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let slice_ty = mir::MirType::SliceRef {
            exclusive: false,
            mutable: false,
            element_type: mir::MirType::I8.into(),
        };
        let slice_id: mir::MirTypeId = slice_ty.into();
        let mut f = b.start_function("f".into(), mir::MirType::USize.into());
        let s = f.new_temp(slice_id, false);
        f.create_block();
        f.push_assign(
            mir::Place::Local(s.clone()),
            mir::Rvalue::Use(mir::Operand::Constant(mir::MirLiteral::BStr(thin_vec::thin_vec![
                1u8, 2, 3
            ]))),
        );
        let len_tmp = f.new_temp(mir::MirType::USize.into(), false);
        f.push_assign(mir::Place::Local(len_tmp), mir::Rvalue::Len(mir::Place::Local(s)));
        f.ret(Some(mir::Operand::Constant(mir::MirLiteral::USize {
            bits: 64,
            value: 0,
        })));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("len_load"), "expected len load: {ir}");
}
