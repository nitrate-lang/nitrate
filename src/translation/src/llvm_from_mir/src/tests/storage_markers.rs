//! Tests for `StorageLive` and `StorageDead` statements. These are no-ops in
//! LLVM lowering (the alloca already models storage), but they appear in real
//! lowered MIR and must not interfere with code generation.

use crate::test_common::Harness;
use nitrate_mir::prelude as mir;

#[test]
fn storage_live_statement_is_noop() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let x = f.new_temp(mir::MirType::I32.into(), false);
        f.create_block();
        f.push_storage_live(x.clone());
        f.push_assign(
            mir::Place::Local(x.clone()),
            mir::Rvalue::Use(mir::Operand::Constant(mir::MirLiteral::I32(1))),
        );
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(x))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("store i32 1"), "expected store: {ir}");
}

#[test]
fn storage_dead_statement_is_noop() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let x = f.new_temp(mir::MirType::I32.into(), false);
        f.create_block();
        f.push_storage_live(x.clone());
        f.push_assign(
            mir::Place::Local(x.clone()),
            mir::Rvalue::Use(mir::Operand::Constant(mir::MirLiteral::I32(7))),
        );
        f.push_storage_dead(x);
        f.ret(Some(mir::Operand::Constant(mir::MirLiteral::I32(0))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("store i32 7"), "expected store: {ir}");
}

#[test]
fn storage_live_and_dead_around_multiple_locals() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let a = f.new_temp(mir::MirType::I32.into(), false);
        let b = f.new_temp(mir::MirType::I32.into(), false);
        f.create_block();
        f.push_storage_live(a.clone());
        f.push_storage_live(b.clone());
        f.push_assign(
            mir::Place::Local(a.clone()),
            mir::Rvalue::Use(mir::Operand::Constant(mir::MirLiteral::I32(2))),
        );
        f.push_assign(
            mir::Place::Local(b.clone()),
            mir::Rvalue::BinaryOp {
                op: mir::MirBinaryOp::Add,
                lhs: mir::Operand::Copy(mir::Place::Local(a)),
                rhs: mir::Operand::Constant(mir::MirLiteral::I32(3)),
            },
        );
        f.push_storage_dead(a);
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(b))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("add"), "expected add: {ir}");
}
