//! Matrix of rvalue tests: Ref, Len, NullaryOp sizes/alignments across type
//! shapes, and aggregate kinds.

use crate::test_common::Harness;
use nitrate_mir::prelude as mir;

/// Build a `SizeOf` function for the given type (constructed inside TLS) and
/// assert the emitted byte count constant.
fn nullary_size_of(make_ty: impl FnOnce() -> mir::MirType, expect: &str) {
    let h = Harness::new();
    let module = h.build_module(move |b| {
        let usize_id: mir::MirTypeId = mir::MirType::USize.into();
        let ty: mir::MirTypeId = make_ty().into();
        let mut f = b.start_function("f".into(), usize_id);
        let tmp = f.new_temp(usize_id, false);
        f.create_block();
        f.push_assign(
            mir::Place::Local(tmp.clone()),
            mir::Rvalue::NullaryOp(mir::NullaryOp::SizeOf, ty),
        );
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(tmp))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module), "module invalid: {ir}");
    assert!(ir.contains(expect), "expected `{}` in {ir}", expect);
}

/// Build an `AlignOf` function for the given type (constructed inside TLS) and
/// assert the emitted alignment constant.
fn nullary_align_of(make_ty: impl FnOnce() -> mir::MirType, expect: &str) {
    let h = Harness::new();
    let module = h.build_module(move |b| {
        let usize_id: mir::MirTypeId = mir::MirType::USize.into();
        let ty: mir::MirTypeId = make_ty().into();
        let mut f = b.start_function("f".into(), usize_id);
        let tmp = f.new_temp(usize_id, false);
        f.create_block();
        f.push_assign(
            mir::Place::Local(tmp.clone()),
            mir::Rvalue::NullaryOp(mir::NullaryOp::AlignOf, ty),
        );
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(tmp))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module), "module invalid: {ir}");
    assert!(ir.contains(expect), "expected `{}` in {ir}", expect);
}

macro_rules! size_of {
    ($name:ident, $ty:expr, $expect:expr) => {
        #[test]
        fn $name() {
            nullary_size_of(|| $ty, $expect);
        }
    };
}

macro_rules! align_of {
    ($name:ident, $ty:expr, $expect:expr) => {
        #[test]
        fn $name() {
            nullary_align_of(|| $ty, $expect);
        }
    };
}

size_of!(size_of_bool_is_1, mir::MirType::Bool, "i64 1");
size_of!(size_of_u8_is_1, mir::MirType::U8, "i64 1");
size_of!(size_of_u16_is_2, mir::MirType::U16, "i64 2");
size_of!(size_of_u32_is_4, mir::MirType::U32, "i64 4");
size_of!(size_of_u64_is_8, mir::MirType::U64, "i64 8");
size_of!(size_of_u128_is_16, mir::MirType::U128, "i64 16");
size_of!(size_of_f32_is_4, mir::MirType::F32, "i64 4");
size_of!(size_of_f64_is_8, mir::MirType::F64, "i64 8");

#[test]
fn size_of_array_is_elem_times_len() {
    nullary_size_of(
        || mir::MirType::Array {
            element_type: mir::MirType::I32.into(),
            len: 4,
        },
        "i64 16",
    );
}

#[test]
fn size_of_tuple_is_sum_of_fields() {
    nullary_size_of(
        || mir::MirType::Tuple {
            element_types: thin_vec::thin_vec![mir::MirType::I32.into(), mir::MirType::I64.into()],
        },
        "i64 16",
    );
}

align_of!(align_of_u8_is_1, mir::MirType::U8, "i64 1");
align_of!(align_of_u16_is_2, mir::MirType::U16, "i64 2");
align_of!(align_of_u32_is_4, mir::MirType::U32, "i64 4");
align_of!(align_of_u64_is_8, mir::MirType::U64, "i64 8");
align_of!(align_of_f32_is_4, mir::MirType::F32, "i64 4");
align_of!(align_of_f64_is_8, mir::MirType::F64, "i64 8");

#[test]
fn ref_rvalue_to_static_is_address() {
    let h = Harness::new();
    let mut module = h.build_module(|b| {
        let ref_ty = mir::MirType::Reference {
            exclusive: false,
            mutable: false,
            to: mir::MirType::I32.into(),
        };
        let ref_id: mir::MirTypeId = ref_ty.into();
        let mut f = b.start_function("f".into(), ref_id);
        let r = f.new_temp(ref_id, false);
        f.create_block();
        f.push_assign(
            mir::Place::Local(r.clone()),
            mir::Rvalue::Ref {
                region: mir::BorrowKind::Shared,
                place: mir::Place::Static("g".into()),
            },
        );
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(r))));
        f.finish_function();
    });
    let gty: mir::MirTypeId = mir::using_storage(&h.store, || mir::MirType::I32.into());
    module.globals.push(mir::MirGlobal {
        name: "g".into(),
        ty: gty,
        body: None,
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("@g"), "expected reference to global: {ir}");
}

#[test]
fn ref_rvalue_mutable() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let ref_ty = mir::MirType::Reference {
            exclusive: true,
            mutable: true,
            to: mir::MirType::I32.into(),
        };
        let ref_id: mir::MirTypeId = ref_ty.into();
        let mut f = b.start_function("f".into(), ref_id);
        let x = f.new_temp(mir::MirType::I32.into(), false);
        f.create_block();
        f.push_assign(
            mir::Place::Local(x.clone()),
            mir::Rvalue::Use(mir::Operand::Constant(mir::MirLiteral::I32(1))),
        );
        let r = f.new_temp(ref_id, false);
        f.push_assign(
            mir::Place::Local(r.clone()),
            mir::Rvalue::Ref {
                region: mir::BorrowKind::Mutable,
                place: mir::Place::Local(x),
            },
        );
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(r))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(!ir.contains("load i32"), "borrow should not load: {ir}");
}

#[test]
fn len_of_bstr_is_literal_length() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let slice_ty = mir::MirType::SliceRef {
            exclusive: false,
            mutable: false,
            element_type: mir::MirType::U8.into(),
        };
        let slice_id: mir::MirTypeId = slice_ty.into();
        let mut f = b.start_function("f".into(), mir::MirType::USize.into());
        let s = f.new_temp(slice_id, false);
        f.create_block();
        f.push_assign(
            mir::Place::Local(s.clone()),
            mir::Rvalue::Use(mir::Operand::Constant(mir::MirLiteral::BStr(thin_vec::thin_vec![
                1, 2, 3, 4, 5
            ]))),
        );
        let len_tmp = f.new_temp(mir::MirType::USize.into(), false);
        f.push_assign(mir::Place::Local(len_tmp), mir::Rvalue::Len(mir::Place::Local(s)));
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(len_tmp))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    let width = h.llvm.ptr_size() * 8;
    assert!(ir.contains(&format!("i{width} 5")), "expected length 5: {ir}");
}

#[test]
fn checked_binary_op_lowers_same_as_unchecked() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let i32: mir::MirTypeId = mir::MirType::I32.into();
        let mut f = b.start_function("f".into(), i32);
        let a = f.add_param("a".into(), i32, false);
        let b = f.add_param("b".into(), i32, false);
        let tmp = f.new_temp(i32, false);
        f.create_block();
        f.push_assign(
            mir::Place::Local(tmp.clone()),
            mir::Rvalue::CheckedBinaryOp {
                op: mir::MirBinaryOp::Add,
                lhs: mir::Operand::Copy(mir::Place::Local(a)),
                rhs: mir::Operand::Copy(mir::Place::Local(b)),
            },
        );
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(tmp))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("add"), "expected add for checked op: {ir}");
}

#[test]
fn aggregate_empty_tuple() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let tuple_ty = mir::MirType::Tuple {
            element_types: Default::default(),
        };
        let tuple_id: mir::MirTypeId = tuple_ty.into();
        let mut f = b.start_function("f".into(), tuple_id);
        let tmp = f.new_temp(tuple_id, false);
        f.create_block();
        f.push_assign(
            mir::Place::Local(tmp.clone()),
            mir::Rvalue::Aggregate(mir::AggregateKind::Tuple, thin_vec::ThinVec::new()),
        );
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(tmp))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("ret {}"), "expected empty tuple: {ir}");
}

#[test]
fn aggregate_array_literal_same_values() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let arr_ty = mir::MirType::Array {
            element_type: mir::MirType::U8.into(),
            len: 2,
        };
        let arr_id: mir::MirTypeId = arr_ty.into();
        let mut f = b.start_function("f".into(), arr_id);
        let tmp = f.new_temp(arr_id, false);
        f.create_block();
        f.push_assign(
            mir::Place::Local(tmp.clone()),
            mir::Rvalue::Aggregate(
                mir::AggregateKind::Array(mir::MirType::U8.into()),
                thin_vec::thin_vec![
                    mir::Operand::Constant(mir::MirLiteral::U8(7)),
                    mir::Operand::Constant(mir::MirLiteral::U8(7)),
                ],
            ),
        );
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(tmp))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("[2 x i8]"), "expected [2 x i8]: {ir}");
}
