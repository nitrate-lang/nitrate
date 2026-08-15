//! Tests for control-flow terminators: goto, if/else, switch, and block
//! arguments (lowered to phi nodes).

use crate::test_common::Harness;
use nitrate_mir::prelude as mir;

#[test]
fn goto_branch_emits_unconditional_branch() {
    let h = Harness::new();
    let ir = h.build_ir(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::Unit.into());
        let target = f.reserve_block();
        f.create_block();
        f.goto(target);
        f.switch_to_block(target);
        f.ret(None);
        f.finish_function();
    });
    assert!(ir.contains("br label"), "expected unconditional branch: {ir}");
}

#[test]
fn if_else_emits_conditional_branch() {
    let h = Harness::new();
    let ir = h.build_ir(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::Unit.into());
        let cond = f.new_temp(mir::MirType::Bool.into(), false);
        let then_b = f.reserve_block();
        let else_b = f.reserve_block();
        f.create_block();
        f.if_br(mir::Operand::Copy(mir::Place::Local(cond)), then_b, else_b);
        f.switch_to_block(then_b);
        f.ret(None);
        f.switch_to_block(else_b);
        f.ret(None);
        f.finish_function();
    });
    assert!(ir.contains("br i1"), "expected conditional branch: {ir}");
}

#[test]
fn switch_int_emits_switch_instruction() {
    let h = Harness::new();
    let ir = h.build_ir(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::Unit.into());
        let discr = f.new_temp(mir::MirType::U8.into(), false);
        let a = f.reserve_block();
        let otherwise = f.reserve_block();
        f.create_block();
        f.set_terminator(mir::Terminator::SwitchInt {
            discr: mir::Operand::Copy(mir::Place::Local(discr)),
            targets: thin_vec::thin_vec![(1u128, a, thin_vec::ThinVec::new())],
            otherwise,
            otherwise_args: thin_vec::ThinVec::new(),
        });
        f.switch_to_block(a);
        f.ret(None);
        f.switch_to_block(otherwise);
        f.ret(None);
        f.finish_function();
    });
    assert!(ir.contains("switch"), "expected switch instruction: {ir}");
}

#[test]
fn switch_int_with_wide_value_uses_arbitrary_precision() {
    let h = Harness::new();
    let ir = h.build_ir(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::Unit.into());
        let discr = f.new_temp(mir::MirType::U128.into(), false);
        let a = f.reserve_block();
        let otherwise = f.reserve_block();
        f.create_block();
        f.set_terminator(mir::Terminator::SwitchInt {
            discr: mir::Operand::Copy(mir::Place::Local(discr)),
            targets: thin_vec::thin_vec![(u128::MAX, a, thin_vec::ThinVec::new())],
            otherwise,
            otherwise_args: thin_vec::ThinVec::new(),
        });
        f.switch_to_block(a);
        f.ret(None);
        f.switch_to_block(otherwise);
        f.ret(None);
        f.finish_function();
    });
    assert!(ir.contains("switch"), "expected switch for 128-bit discr: {ir}");
}

#[test]
fn block_arguments_lower_to_phi_nodes() {
    let h = Harness::new();
    let ir = h.build_ir(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let merge = f.reserve_block_with_args(&[mir::MirType::I32.into()]);
        let left = f.reserve_block();
        let right = f.reserve_block();
        f.create_block();
        f.if_br(mir::Operand::Constant(mir::MirLiteral::Bool(true)), left, right);

        f.switch_to_block(left);
        f.goto_with_args(
            merge.block,
            thin_vec::thin_vec![mir::Operand::Constant(mir::MirLiteral::I32(10))],
        );

        f.switch_to_block(right);
        f.goto_with_args(
            merge.block,
            thin_vec::thin_vec![mir::Operand::Constant(mir::MirLiteral::I32(20))],
        );

        f.switch_to_block(merge.block);
        let arg_local = merge.arg_locals[0].clone();
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(arg_local))));
        f.finish_function();
    });
    assert!(ir.contains("phi"), "expected phi node from block arguments: {ir}");
}

#[test]
fn if_with_multiple_block_arguments_creates_multiple_phis() {
    let h = Harness::new();
    let ir = h.build_ir(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let merge = f.reserve_block_with_args(&[mir::MirType::I32.into(), mir::MirType::I32.into()]);
        let then_b = f.reserve_block();
        let else_b = f.reserve_block();
        f.create_block();
        f.if_br(mir::Operand::Constant(mir::MirLiteral::Bool(false)), then_b, else_b);

        f.switch_to_block(then_b);
        f.goto_with_args(
            merge.block,
            thin_vec::thin_vec![
                mir::Operand::Constant(mir::MirLiteral::I32(1)),
                mir::Operand::Constant(mir::MirLiteral::I32(2)),
            ],
        );

        f.switch_to_block(else_b);
        f.goto_with_args(
            merge.block,
            thin_vec::thin_vec![
                mir::Operand::Constant(mir::MirLiteral::I32(3)),
                mir::Operand::Constant(mir::MirLiteral::I32(4)),
            ],
        );

        f.switch_to_block(merge.block);
        let arg_local = merge.arg_locals[0].clone();
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(arg_local))));
        f.finish_function();
    });
    let phi_count = ir.matches("phi").count();
    assert_eq!(phi_count, 2, "expected two phi nodes: {ir}");
}

#[test]
fn unreachable_terminator_emits_unreachable() {
    let h = Harness::new();
    let ir = h.build_ir(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::Unit.into());
        f.create_block();
        f.unreachable();
        f.finish_function();
    });
    assert!(ir.contains("unreachable"), "expected unreachable: {ir}");
}

#[test]
fn return_void_emits_ret_void() {
    let h = Harness::new();
    let ir = h.build_ir(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::Unit.into());
        f.create_block();
        f.ret(None);
        f.finish_function();
    });
    assert!(ir.contains("ret void"), "expected `ret void`: {ir}");
}

#[test]
fn return_value_emits_ret_value() {
    let h = Harness::new();
    let ir = h.build_ir(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        f.create_block();
        f.ret(Some(mir::Operand::Constant(mir::MirLiteral::I32(5))));
        f.finish_function();
    });
    assert!(ir.contains("ret i32 5"), "expected `ret i32 5`: {ir}");
}

#[test]
fn empty_function_unreachable_is_valid() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::Unit.into());
        f.create_block();
        f.unreachable();
        f.finish_function();
    });
    assert!(h.verify(&module));
}

#[test]
fn self_loop_block_verified() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::Unit.into());
        let body = f.reserve_block();
        f.create_block();
        f.goto(body);
        f.switch_to_block(body);
        f.goto(body);
        f.finish_function();
    });
    assert!(h.verify(&module));
}

#[test]
fn diamond_control_flow_with_phi_verified() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let merge = f.reserve_block_with_args(&[mir::MirType::I32.into()]);
        let a = f.reserve_block();
        let c = f.reserve_block();
        f.create_block();
        f.if_br(mir::Operand::Constant(mir::MirLiteral::Bool(true)), a, c);
        f.switch_to_block(a);
        f.goto_with_args(
            merge.block,
            thin_vec::thin_vec![mir::Operand::Constant(mir::MirLiteral::I32(1))],
        );
        f.switch_to_block(c);
        f.goto_with_args(
            merge.block,
            thin_vec::thin_vec![mir::Operand::Constant(mir::MirLiteral::I32(2))],
        );
        f.switch_to_block(merge.block);
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(merge.arg_locals[0].clone()))));
        f.finish_function();
    });
    assert!(h.verify(&module));
}
