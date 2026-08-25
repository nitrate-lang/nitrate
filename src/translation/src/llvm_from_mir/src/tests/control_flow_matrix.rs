//! Matrix of control-flow shapes: loops, switch arms, block arguments on each
//! terminator kind, and unreachable/return combinations.

use crate::test_common::Harness;
use nitrate_mir::prelude as mir;

#[test]
fn if_then_else_dynamic_condition() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let bool_ty: mir::MirTypeId = mir::MirType::Bool.into();
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let cond = f.add_param("cond".into(), bool_ty, false);
        let then_b = f.reserve_block();
        let else_b = f.reserve_block();
        f.create_block();
        f.if_br(mir::Operand::Copy(mir::Place::Local(cond)), then_b, else_b);
        f.switch_to_block(then_b);
        f.ret(Some(mir::Operand::Constant(mir::MirLiteral::I32(1))));
        f.switch_to_block(else_b);
        f.ret(Some(mir::Operand::Constant(mir::MirLiteral::I32(0))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("br i1"), "expected conditional branch: {ir}");
}

#[test]
fn loop_with_exit_edge() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let bool_ty: mir::MirTypeId = mir::MirType::Bool.into();
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let cond = f.add_param("cond".into(), bool_ty, false);
        let body = f.reserve_block();
        let exit = f.reserve_block();
        f.create_block();
        f.goto(body);
        f.switch_to_block(body);
        f.if_br(mir::Operand::Copy(mir::Place::Local(cond)), body, exit);
        f.switch_to_block(exit);
        f.ret(Some(mir::Operand::Constant(mir::MirLiteral::I32(0))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("br i1"), "expected loop condition: {ir}");
}

#[test]
fn switch_four_arms() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let u8: mir::MirTypeId = mir::MirType::U8.into();
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let discr = f.add_param("x".into(), u8, false);
        let a = f.reserve_block();
        let b1 = f.reserve_block();
        let c = f.reserve_block();
        let d = f.reserve_block();
        f.create_block();
        f.set_terminator(mir::Terminator::SwitchInt {
            discr: mir::Operand::Copy(mir::Place::Local(discr)),
            targets: thin_vec::thin_vec![
                (0u128, a, thin_vec::ThinVec::new()),
                (1u128, b1, thin_vec::ThinVec::new()),
                (2u128, c, thin_vec::ThinVec::new()),
                (3u128, d, thin_vec::ThinVec::new()),
            ],
            otherwise: d,
            otherwise_args: thin_vec::ThinVec::new(),
        });
        f.switch_to_block(a);
        f.ret(Some(mir::Operand::Constant(mir::MirLiteral::I32(1))));
        f.switch_to_block(b1);
        f.ret(Some(mir::Operand::Constant(mir::MirLiteral::I32(2))));
        f.switch_to_block(c);
        f.ret(Some(mir::Operand::Constant(mir::MirLiteral::I32(3))));
        f.switch_to_block(d);
        f.ret(Some(mir::Operand::Constant(mir::MirLiteral::I32(4))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("switch"), "expected switch: {ir}");
}

#[test]
fn goto_with_args_single_phi() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let i32: mir::MirTypeId = mir::MirType::I32.into();
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let merge = f.reserve_block_with_args(&[i32]);
        let src = f.reserve_block();
        f.create_block();
        f.goto(src);
        f.switch_to_block(src);
        f.goto_with_args(
            merge.block,
            thin_vec::thin_vec![mir::Operand::Constant(mir::MirLiteral::I32(7))],
        );
        f.switch_to_block(merge.block);
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(merge.arg_locals[0].clone()))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("phi"), "expected phi: {ir}");
}

#[test]
fn if_br_with_args_two_phis() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let bool_ty: mir::MirTypeId = mir::MirType::Bool.into();
        let i32: mir::MirTypeId = mir::MirType::I32.into();
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let cond = f.add_param("cond".into(), bool_ty, false);
        let merge = f.reserve_block_with_args(&[i32, i32]);
        let then_b = f.reserve_block();
        let else_b = f.reserve_block();
        f.create_block();
        f.if_br(mir::Operand::Copy(mir::Place::Local(cond)), then_b, else_b);
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
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(merge.arg_locals[0].clone()))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert_eq!(ir.matches(" = phi ").count(), 2, "{ir}");
}

#[test]
fn switch_with_args_phis() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let u8: mir::MirTypeId = mir::MirType::U8.into();
        let i32: mir::MirTypeId = mir::MirType::I32.into();
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let discr = f.add_param("x".into(), u8, false);
        let merge = f.reserve_block_with_args(&[i32]);
        let a = f.reserve_block();
        f.create_block();
        f.set_terminator(mir::Terminator::SwitchInt {
            discr: mir::Operand::Copy(mir::Place::Local(discr)),
            targets: thin_vec::thin_vec![(
                1u128,
                a,
                thin_vec::thin_vec![mir::Operand::Constant(mir::MirLiteral::I32(42))]
            )],
            otherwise: merge.block,
            otherwise_args: thin_vec::thin_vec![mir::Operand::Constant(mir::MirLiteral::I32(0))],
        });
        f.switch_to_block(a);
        f.goto_with_args(
            merge.block,
            thin_vec::thin_vec![mir::Operand::Constant(mir::MirLiteral::I32(99))],
        );
        f.switch_to_block(merge.block);
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(merge.arg_locals[0].clone()))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("phi"), "expected phi from switch arms: {ir}");
}

#[test]
fn pure_loop_no_exit() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::Never.into());
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
fn unreachable_then_return_blocks() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let dead = f.reserve_block();
        let alive = f.reserve_block();
        f.create_block();
        f.goto(alive);
        f.switch_to_block(dead);
        f.unreachable();
        f.switch_to_block(alive);
        f.ret(Some(mir::Operand::Constant(mir::MirLiteral::I32(5))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("unreachable"), "expected unreachable block: {ir}");
}

#[test]
fn deep_chain_of_gotos() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let b1 = b;
        let mut f = b1.start_function("f".into(), mir::MirType::I32.into());
        let blocks: Vec<_> = (0..8).map(|_| f.reserve_block()).collect();
        f.create_block();
        f.goto(blocks[0]);
        for i in 0..8 {
            f.switch_to_block(blocks[i]);
            if i + 1 < 8 {
                f.goto(blocks[i + 1]);
            } else {
                f.ret(Some(mir::Operand::Constant(mir::MirLiteral::I32(0))));
            }
        }
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.matches("br label").count() >= 8, "{ir}");
}

#[test]
fn return_with_unit_value_in_void_fn() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::Unit.into());
        f.create_block();
        f.ret(Some(mir::Operand::Constant(mir::MirLiteral::Unit)));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("ret void"), "expected ret void: {ir}");
}
