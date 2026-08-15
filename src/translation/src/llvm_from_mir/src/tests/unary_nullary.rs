//! Tests for unary and nullary rvalue lowering.

use crate::test_common::Harness;
use nitrate_mir::prelude as mir;
use ordered_float::OrderedFloat;

fn unary(h: &Harness, op: mir::MirUnaryOp, operand: mir::Operand, ty: mir::MirType) -> String {
    h.build_ir(|b| {
        let ty_id: mir::MirTypeId = ty.into();
        let mut f = b.start_function("f".into(), ty_id);
        let tmp = f.new_temp(ty_id, false);
        f.create_block();
        f.push_assign(mir::Place::Local(tmp.clone()), mir::Rvalue::UnaryOp { op, operand });
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(tmp))));
        f.finish_function();
    })
}

#[test]
fn neg_i32_is_sub_from_zero() {
    let h = Harness::new();
    let ir = unary(
        &h,
        mir::MirUnaryOp::Neg,
        mir::Operand::Constant(mir::MirLiteral::I32(3)),
        mir::MirType::I32,
    );
    assert!(ir.contains("sub i32 0"), "expected integer negate: {ir}");
}

#[test]
fn neg_u64_is_sub_from_zero() {
    let h = Harness::new();
    let ir = unary(
        &h,
        mir::MirUnaryOp::Neg,
        mir::Operand::Constant(mir::MirLiteral::U64(1)),
        mir::MirType::U64,
    );
    assert!(ir.contains("sub i64 0"), "expected unsigned negate: {ir}");
}

#[test]
fn neg_f32_is_fneg() {
    let h = Harness::new();
    let ir = unary(
        &h,
        mir::MirUnaryOp::Neg,
        mir::Operand::Constant(mir::MirLiteral::F32(OrderedFloat(1.0))),
        mir::MirType::F32,
    );
    assert!(ir.contains("fneg"), "expected float negate: {ir}");
}

#[test]
fn neg_f64_is_fneg() {
    let h = Harness::new();
    let ir = unary(
        &h,
        mir::MirUnaryOp::Neg,
        mir::Operand::Constant(mir::MirLiteral::F64(OrderedFloat(1.0))),
        mir::MirType::F64,
    );
    assert!(ir.contains("fneg"), "expected float negate: {ir}");
}

#[test]
fn not_i32_is_bitwise_not() {
    let h = Harness::new();
    let ir = unary(
        &h,
        mir::MirUnaryOp::Not,
        mir::Operand::Constant(mir::MirLiteral::I32(0)),
        mir::MirType::I32,
    );
    assert!(ir.contains("xor") || ir.contains("not"), "expected bitwise not: {ir}");
}

#[test]
fn not_u8_is_bitwise_not() {
    let h = Harness::new();
    let ir = unary(
        &h,
        mir::MirUnaryOp::Not,
        mir::Operand::Constant(mir::MirLiteral::U8(0)),
        mir::MirType::U8,
    );
    assert!(ir.contains("xor") || ir.contains("not"), "expected bitwise not: {ir}");
}

#[test]
fn size_of_i32_is_4() {
    let h = Harness::new();
    let width_ty = if h.llvm.ptr_size() == 8 { "i64" } else { "i32" };
    let ir = h.build_ir(|b| {
        let usize_id: mir::MirTypeId = mir::MirType::USize.into();
        let i32_id: mir::MirTypeId = mir::MirType::I32.into();
        let mut f = b.start_function("f".into(), usize_id);
        let tmp = f.new_temp(usize_id, false);
        f.create_block();
        f.push_assign(
            mir::Place::Local(tmp.clone()),
            mir::Rvalue::NullaryOp(mir::NullaryOp::SizeOf, i32_id),
        );
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(tmp))));
        f.finish_function();
    });
    assert!(ir.contains(&format!("{width_ty} 4")), "expected size 4: {ir}");
}

#[test]
fn size_of_u128_is_16() {
    let h = Harness::new();
    let width_ty = if h.llvm.ptr_size() == 8 { "i64" } else { "i32" };
    let ir = h.build_ir(|b| {
        let usize_id: mir::MirTypeId = mir::MirType::USize.into();
        let u128_id: mir::MirTypeId = mir::MirType::U128.into();
        let mut f = b.start_function("f".into(), usize_id);
        let tmp = f.new_temp(usize_id, false);
        f.create_block();
        f.push_assign(
            mir::Place::Local(tmp.clone()),
            mir::Rvalue::NullaryOp(mir::NullaryOp::SizeOf, u128_id),
        );
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(tmp))));
        f.finish_function();
    });
    assert!(ir.contains(&format!("{width_ty} 16")), "expected size 16: {ir}");
}

#[test]
fn size_of_unit_is_zero() {
    let h = Harness::new();
    let width_ty = if h.llvm.ptr_size() == 8 { "i64" } else { "i32" };
    let ir = h.build_ir(|b| {
        let usize_id: mir::MirTypeId = mir::MirType::USize.into();
        let unit_id: mir::MirTypeId = mir::MirType::Unit.into();
        let mut f = b.start_function("f".into(), usize_id);
        let tmp = f.new_temp(usize_id, false);
        f.create_block();
        f.push_assign(
            mir::Place::Local(tmp.clone()),
            mir::Rvalue::NullaryOp(mir::NullaryOp::SizeOf, unit_id),
        );
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(tmp))));
        f.finish_function();
    });
    assert!(ir.contains(&format!("{width_ty} 0")), "expected size 0: {ir}");
}

#[test]
fn align_of_i32_is_4() {
    let h = Harness::new();
    let width_ty = if h.llvm.ptr_size() == 8 { "i64" } else { "i32" };
    let ir = h.build_ir(|b| {
        let usize_id: mir::MirTypeId = mir::MirType::USize.into();
        let i32_id: mir::MirTypeId = mir::MirType::I32.into();
        let mut f = b.start_function("f".into(), usize_id);
        let tmp = f.new_temp(usize_id, false);
        f.create_block();
        f.push_assign(
            mir::Place::Local(tmp.clone()),
            mir::Rvalue::NullaryOp(mir::NullaryOp::AlignOf, i32_id),
        );
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(tmp))));
        f.finish_function();
    });
    assert!(ir.contains(&format!("{width_ty} 4")), "expected alignment 4: {ir}");
}

#[test]
fn align_of_u64_is_8() {
    let h = Harness::new();
    let width_ty = if h.llvm.ptr_size() == 8 { "i64" } else { "i32" };
    let ir = h.build_ir(|b| {
        let usize_id: mir::MirTypeId = mir::MirType::USize.into();
        let u64_id: mir::MirTypeId = mir::MirType::U64.into();
        let mut f = b.start_function("f".into(), usize_id);
        let tmp = f.new_temp(usize_id, false);
        f.create_block();
        f.push_assign(
            mir::Place::Local(tmp.clone()),
            mir::Rvalue::NullaryOp(mir::NullaryOp::AlignOf, u64_id),
        );
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(tmp))));
        f.finish_function();
    });
    assert!(ir.contains(&format!("{width_ty} 8")), "expected alignment 8: {ir}");
}

#[test]
fn unary_op_assigns_to_temp() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let i32_id: mir::MirTypeId = mir::MirType::I32.into();
        let mut f = b.start_function("f".into(), i32_id);
        let x = f.new_temp(i32_id, false);
        let y = f.new_temp(i32_id, false);
        f.create_block();
        f.push_assign(
            mir::Place::Local(x.clone()),
            mir::Rvalue::Use(mir::Operand::Constant(mir::MirLiteral::I32(5))),
        );
        f.push_assign(
            mir::Place::Local(y.clone()),
            mir::Rvalue::UnaryOp {
                op: mir::MirUnaryOp::Neg,
                operand: mir::Operand::Copy(mir::Place::Local(x)),
            },
        );
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(y))));
        f.finish_function();
    });
    assert!(h.verify(&module));
}
