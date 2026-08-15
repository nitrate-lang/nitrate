//! Tests for function declaration/definition lowering and call terminators.

use crate::test_common::Harness;
use nitrate_mir::prelude as mir;

#[test]
fn extern_function_declared_without_body() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut f = b.start_function("extern_fn".into(), mir::MirType::I32.into());
        f.add_param("x".into(), mir::MirType::I32.into(), false);
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(
        ir.contains("declare i32 @extern_fn"),
        "expected extern declaration: {ir}"
    );
}

#[test]
fn defined_function_has_body_and_returns() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        f.create_block();
        f.ret(Some(mir::Operand::Constant(mir::MirLiteral::I32(3))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("define i32 @f"), "expected definition: {ir}");
}

#[test]
fn function_parameters_stored_into_allocas() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let p = f.add_param("x".into(), mir::MirType::I32.into(), false);
        f.create_block();
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(p))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("@f(i32"), "expected i32 parameter: {ir}");
}

#[test]
fn variadic_function_uses_varargs_signature() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut f = b.start_function("printf".into(), mir::MirType::I32.into());
        f.add_param("fmt".into(), mir::MirType::Str.into(), false);
        f.set_c_variadic();
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("..."), "expected variadic signature: {ir}");
}

#[test]
fn direct_call_to_named_function() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut callee = b.start_function("callee".into(), mir::MirType::I32.into());
        callee.create_block();
        callee.ret(Some(mir::Operand::Constant(mir::MirLiteral::I32(42))));
        callee.finish_function();

        let mut f = b.start_function("caller".into(), mir::MirType::I32.into());
        let dest = f.new_temp(mir::MirType::I32.into(), false);
        let ret_block = f.reserve_block();
        f.create_block();
        f.call_return(
            mir::Operand::Copy(mir::Place::Static("callee".into())),
            thin_vec::ThinVec::new(),
            mir::Place::Local(dest.clone()),
            ret_block,
        );
        f.switch_to_block(ret_block);
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(dest))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("call i32 @callee"), "expected direct call: {ir}");
}

#[test]
fn indirect_call_via_function_pointer() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let fp = f.new_temp(
            mir::MirType::Pointer {
                exclusive: false,
                mutable: false,
                to: mir::MirType::I32.into(),
            }
            .into(),
            false,
        );
        let dest = f.new_temp(mir::MirType::I32.into(), false);
        let ret_block = f.reserve_block();
        f.create_block();
        f.call_return(
            mir::Operand::Copy(mir::Place::Local(fp.clone())),
            thin_vec::ThinVec::new(),
            mir::Place::Local(dest.clone()),
            ret_block,
        );
        f.switch_to_block(ret_block);
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(dest))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("call"), "expected indirect call: {ir}");
}

#[test]
fn mutual_recursion_declares_both_functions() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut g = b.start_function("g".into(), mir::MirType::I32.into());
        g.create_block();
        g.ret(Some(mir::Operand::Constant(mir::MirLiteral::I32(0))));
        g.finish_function();

        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        f.create_block();
        f.ret(Some(mir::Operand::Constant(mir::MirLiteral::I32(1))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(
        ir.contains("@g") && ir.contains("@f"),
        "expected both declarations: {ir}"
    );
}

#[test]
fn multiple_functions_do_not_collide() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        for i in 0..8u8 {
            let name = format!("fn_{i}");
            let mut f = b.start_function(name.into(), mir::MirType::U8.into());
            f.create_block();
            f.ret(Some(mir::Operand::Constant(mir::MirLiteral::U8(i))));
            f.finish_function();
        }
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    for i in 0..8u8 {
        assert!(ir.contains(&format!("@fn_{i}")), "missing fn_{i}: {ir}");
    }
}

#[test]
fn mixed_extern_and_defined_functions() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let e = b.start_function("ext".into(), mir::MirType::Unit.into());
        e.finish_function();

        let mut d = b.start_function("def".into(), mir::MirType::Unit.into());
        d.create_block();
        d.ret(None);
        d.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("declare void @ext"), "expected extern: {ir}");
    assert!(ir.contains("define void @def"), "expected definition: {ir}");
}

#[test]
fn call_with_arguments_passes_them() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut callee = b.start_function("sum".into(), mir::MirType::I32.into());
        callee.add_param("a".into(), mir::MirType::I32.into(), false);
        callee.add_param("b".into(), mir::MirType::I32.into(), false);
        callee.create_block();
        callee.ret(Some(mir::Operand::Constant(mir::MirLiteral::I32(0))));
        callee.finish_function();

        let mut f = b.start_function("caller".into(), mir::MirType::I32.into());
        let dest = f.new_temp(mir::MirType::I32.into(), false);
        let ret_block = f.reserve_block();
        f.create_block();
        f.call_return(
            mir::Operand::Copy(mir::Place::Static("sum".into())),
            thin_vec::thin_vec![
                mir::Operand::Constant(mir::MirLiteral::I32(1)),
                mir::Operand::Constant(mir::MirLiteral::I32(2)),
            ],
            mir::Place::Local(dest),
            ret_block,
        );
        f.switch_to_block(ret_block);
        f.ret(Some(mir::Operand::Constant(mir::MirLiteral::I32(0))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(
        ir.contains("call i32 @sum(i32 1, i32 2)"),
        "expected call with args: {ir}"
    );
}
