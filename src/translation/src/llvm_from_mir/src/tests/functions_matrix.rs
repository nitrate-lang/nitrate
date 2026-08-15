//! Matrix-style tests for function signatures, bodies, and call shapes.

use crate::test_common::Harness;
use nitrate_mir::prelude as mir;

macro_rules! ret_value {
    ($name:ident, $ty:expr, $llvm_ty:expr, $lit:expr) => {
        #[test]
        fn $name() {
            let h = Harness::new();
            let module = h.build_module(|b| {
                let ty: mir::MirTypeId = $ty.into();
                let mut f = b.start_function("f".into(), ty);
                f.create_block();
                f.ret(Some(mir::Operand::Constant($lit)));
                f.finish_function();
            });
            let ir = h.ir(&module);
            assert!(h.verify(&module), "module invalid: {ir}");
            assert!(
                ir.contains(&format!("define {} @f", $llvm_ty)),
                "expected define {} @f in {ir}",
                $llvm_ty
            );
        }
    };
}

// ── Return-type matrix across primitives ──
ret_value!(ret_i8, mir::MirType::I8, "i8", mir::MirLiteral::I8(1));
ret_value!(ret_i16, mir::MirType::I16, "i16", mir::MirLiteral::I16(1));
ret_value!(ret_i32, mir::MirType::I32, "i32", mir::MirLiteral::I32(1));
ret_value!(ret_i64, mir::MirType::I64, "i64", mir::MirLiteral::I64(1));
ret_value!(ret_u8, mir::MirType::U8, "i8", mir::MirLiteral::U8(1));
ret_value!(ret_u16, mir::MirType::U16, "i16", mir::MirLiteral::U16(1));
ret_value!(ret_u32, mir::MirType::U32, "i32", mir::MirLiteral::U32(1));
ret_value!(ret_u64, mir::MirType::U64, "i64", mir::MirLiteral::U64(1));
ret_value!(ret_bool, mir::MirType::Bool, "i1", mir::MirLiteral::Bool(true));

#[test]
fn ret_f32_declares_float() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let ty: mir::MirTypeId = mir::MirType::F32.into();
        let mut f = b.start_function("f".into(), ty);
        f.create_block();
        f.ret(Some(mir::Operand::Constant(mir::MirLiteral::F32(
            ordered_float::OrderedFloat(1.0),
        ))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("define float @f"), "expected float fn: {ir}");
}

#[test]
fn ret_f64_declares_double() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let ty: mir::MirTypeId = mir::MirType::F64.into();
        let mut f = b.start_function("f".into(), ty);
        f.create_block();
        f.ret(Some(mir::Operand::Constant(mir::MirLiteral::F64(
            ordered_float::OrderedFloat(1.0),
        ))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("define double @f"), "expected double fn: {ir}");
}

#[test]
fn ret_void_for_unit() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::Unit.into());
        f.create_block();
        f.ret(None);
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("define void @f"), "expected void fn: {ir}");
}

// ── Parameter shape matrix ──
macro_rules! param_count {
    ($name:ident, $n:expr) => {
        #[test]
        fn $name() {
            let h = Harness::new();
            let module = h.build_module(move |b| {
                let mut f = b.start_function("f".into(), mir::MirType::I32.into());
                for _ in 0..$n {
                    f.add_param("x".into(), mir::MirType::I32.into(), false);
                }
                f.create_block();
                f.ret(Some(mir::Operand::Constant(mir::MirLiteral::I32(0))));
                f.finish_function();
            });
            let ir = h.ir(&module);
            assert!(h.verify(&module), "module invalid: {ir}");
        }
    };
}

param_count!(params_0, 0);
param_count!(params_1, 1);
param_count!(params_2, 2);
param_count!(params_3, 3);
param_count!(params_4, 4);
param_count!(params_5, 5);
param_count!(params_8, 8);

// ── Mixed parameter types ──
#[test]
fn mixed_parameter_types() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        f.add_param("a".into(), mir::MirType::I8.into(), false);
        f.add_param("b".into(), mir::MirType::I16.into(), false);
        f.add_param("c".into(), mir::MirType::I32.into(), false);
        f.add_param("d".into(), mir::MirType::I64.into(), false);
        f.add_param("e".into(), mir::MirType::F32.into(), false);
        f.add_param("g".into(), mir::MirType::F64.into(), false);
        f.create_block();
        f.ret(Some(mir::Operand::Constant(mir::MirLiteral::I32(0))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("i8"), "missing i8 param: {ir}");
    assert!(ir.contains("i16"), "missing i16 param: {ir}");
    assert!(ir.contains("i64"), "missing i64 param: {ir}");
    assert!(ir.contains("float"), "missing float param: {ir}");
    assert!(ir.contains("double"), "missing double param: {ir}");
}

// ── Call shapes ──
#[test]
fn call_returning_void() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut callee = b.start_function("callee".into(), mir::MirType::Unit.into());
        callee.create_block();
        callee.ret(None);
        callee.finish_function();

        let mut f = b.start_function("caller".into(), mir::MirType::Unit.into());
        let ret_block = f.reserve_block();
        f.create_block();
        f.set_terminator(mir::Terminator::Call {
            callee: mir::Operand::Copy(mir::Place::Static("callee".into())),
            args: thin_vec::ThinVec::new(),
            destination: None,
            target: Some(ret_block),
            target_args: thin_vec::ThinVec::new(),
        });
        f.switch_to_block(ret_block);
        f.ret(None);
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("call void @callee"), "expected void call: {ir}");
}

#[test]
fn divergent_call_emits_unreachable() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let e = b.start_function("never_returns".into(), mir::MirType::Never.into());
        e.finish_function();

        let mut f = b.start_function("caller".into(), mir::MirType::Unit.into());
        f.create_block();
        f.set_terminator(mir::Terminator::Call {
            callee: mir::Operand::Copy(mir::Place::Static("never_returns".into())),
            args: thin_vec::ThinVec::new(),
            destination: None,
            target: None,
            target_args: thin_vec::ThinVec::new(),
        });
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(
        ir.contains("unreachable"),
        "expected unreachable after divergent call: {ir}"
    );
}

#[test]
fn direct_call_matches_declared_signature() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut callee = b.start_function("add".into(), mir::MirType::I64.into());
        callee.add_param("a".into(), mir::MirType::I64.into(), false);
        callee.add_param("b".into(), mir::MirType::I64.into(), false);
        callee.create_block();
        callee.ret(Some(mir::Operand::Constant(mir::MirLiteral::I64(0))));
        callee.finish_function();

        let mut f = b.start_function("caller".into(), mir::MirType::I64.into());
        let dest = f.new_temp(mir::MirType::I64.into(), false);
        let ret_block = f.reserve_block();
        f.create_block();
        f.call_return(
            mir::Operand::Copy(mir::Place::Static("add".into())),
            thin_vec::thin_vec![
                mir::Operand::Constant(mir::MirLiteral::I64(3)),
                mir::Operand::Constant(mir::MirLiteral::I64(4)),
            ],
            mir::Place::Local(dest),
            ret_block,
        );
        f.switch_to_block(ret_block);
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(dest))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("call i64 @add"), "expected i64 call: {ir}");
}

#[test]
fn call_with_target_args_forwards_phis() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut callee = b.start_function("callee".into(), mir::MirType::I32.into());
        callee.create_block();
        callee.ret(Some(mir::Operand::Constant(mir::MirLiteral::I32(42))));
        callee.finish_function();

        let mut f = b.start_function("caller".into(), mir::MirType::I32.into());
        let dest = f.new_temp(mir::MirType::I32.into(), false);
        let merge = f.reserve_block_with_args(&[mir::MirType::I32.into()]);
        f.create_block();
        f.call_return_with_args(
            mir::Operand::Copy(mir::Place::Static("callee".into())),
            thin_vec::ThinVec::new(),
            mir::Place::Local(dest),
            merge.block,
            thin_vec::thin_vec![mir::Operand::Constant(mir::MirLiteral::I32(99))],
        );
        f.switch_to_block(merge.block);
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(merge.arg_locals[0].clone()))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("phi"), "expected phi from call target args: {ir}");
}
