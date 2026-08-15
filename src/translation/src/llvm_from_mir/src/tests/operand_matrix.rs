//! Matrix of operand-level tests: Copy vs Move equivalence, mixed
//! constant/place operands in binary operations, and logical operators on
//! non-boolean integer widths.

use crate::test_common::{Harness, fn_binary};
use nitrate_mir::prelude as mir;

// Mixed constant/place combinations are expressed directly via the inline
// test bodies below (no sentinel place is needed since the builder provides
// real `LocalId`s from `add_param`).

#[test]
fn copy_and_move_operands_both_load() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let i32: mir::MirTypeId = mir::MirType::I32.into();
        let mut f = b.start_function("f".into(), i32);
        let x = f.add_param("x".into(), i32, false);
        let tmp = f.new_temp(i32, false);
        f.create_block();
        f.push_assign(
            mir::Place::Local(tmp.clone()),
            mir::Rvalue::Use(mir::Operand::Copy(mir::Place::Local(x))),
        );
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(tmp))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("load i32"), "copy should load: {ir}");
}

#[test]
fn move_operand_loads() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let i32: mir::MirTypeId = mir::MirType::I32.into();
        let mut f = b.start_function("f".into(), i32);
        let x = f.add_param("x".into(), i32, false);
        let tmp = f.new_temp(i32, false);
        f.create_block();
        f.push_assign(
            mir::Place::Local(tmp.clone()),
            mir::Rvalue::Use(mir::Operand::Move(mir::Place::Local(x))),
        );
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(tmp))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("load i32"), "move should load: {ir}");
}

/// Logical operators must work on any integer width (not just bool). They
/// compare each operand against zero and combine the resulting booleans.
macro_rules! logic_op {
    ($name:ident, $ty:expr, $op:ident, $instr:expr) => {
        #[test]
        fn $name() {
            let h = Harness::new();
            let module = h.build_module(move |b| {
                let ty: mir::MirTypeId = $ty.into();
                let bool: mir::MirTypeId = mir::MirType::Bool.into();
                fn_binary(b, "f", ty.clone(), ty, bool, mir::MirBinaryOp::$op);
            });
            let ir = h.ir(&module);
            assert!(h.verify(&module), "module invalid: {ir}");
            assert!(ir.contains($instr), "expected `{}` in {ir}", $instr);
        }
    };
}

logic_op!(logic_and_i8, mir::MirType::I8, LogicAnd, "and i1");
logic_op!(logic_and_i16, mir::MirType::I16, LogicAnd, "and i1");
logic_op!(logic_and_i32, mir::MirType::I32, LogicAnd, "and i1");
logic_op!(logic_and_i64, mir::MirType::I64, LogicAnd, "and i1");
logic_op!(logic_and_u8, mir::MirType::U8, LogicAnd, "and i1");
logic_op!(logic_and_u16, mir::MirType::U16, LogicAnd, "and i1");
logic_op!(logic_and_u32, mir::MirType::U32, LogicAnd, "and i1");
logic_op!(logic_and_u64, mir::MirType::U64, LogicAnd, "and i1");
logic_op!(logic_or_i8, mir::MirType::I8, LogicOr, "or i1");
logic_op!(logic_or_i16, mir::MirType::I16, LogicOr, "or i1");
logic_op!(logic_or_i32, mir::MirType::I32, LogicOr, "or i1");
logic_op!(logic_or_i64, mir::MirType::I64, LogicOr, "or i1");
logic_op!(logic_or_u8, mir::MirType::U8, LogicOr, "or i1");
logic_op!(logic_or_u16, mir::MirType::U16, LogicOr, "or i1");
logic_op!(logic_or_u32, mir::MirType::U32, LogicOr, "or i1");
logic_op!(logic_or_u64, mir::MirType::U64, LogicOr, "or i1");

/// Mixed constant-place operand combinations: for each combination of
/// (constant, place) left/right operands, the correct instruction is emitted.
macro_rules! mixed_op {
    ($name:ident, $op:ident, $ty:expr, $ret:expr, $instr:expr) => {
        #[test]
        fn $name() {
            let h = Harness::new();
            let module = h.build_module(move |b| {
                let ty: mir::MirTypeId = $ty.into();
                let ret: mir::MirTypeId = $ret.into();
                let mut f = b.start_function("f".into(), ret);
                let a = f.add_param("a".into(), ty.clone(), false);
                let tmp = f.new_temp(ret, false);
                f.create_block();
                f.push_assign(
                    mir::Place::Local(tmp.clone()),
                    mir::Rvalue::BinaryOp {
                        op: mir::MirBinaryOp::$op,
                        lhs: mir::Operand::Constant(mir::MirLiteral::I32(1)),
                        rhs: mir::Operand::Copy(mir::Place::Local(a)),
                    },
                );
                f.ret(Some(mir::Operand::Copy(mir::Place::Local(tmp))));
                f.finish_function();
            });
            let ir = h.ir(&module);
            assert!(h.verify(&module), "module invalid: {ir}");
            assert!(ir.contains($instr), "expected `{}` in {ir}", $instr);
        }
    };
}

mixed_op!(
    mixed_add_const_lhs,
    Add,
    mir::MirType::I32,
    mir::MirType::I32,
    "add i32"
);
mixed_op!(
    mixed_sub_const_lhs,
    Sub,
    mir::MirType::I32,
    mir::MirType::I32,
    "sub i32"
);
mixed_op!(
    mixed_mul_const_lhs,
    Mul,
    mir::MirType::I32,
    mir::MirType::I32,
    "mul i32"
);
mixed_op!(
    mixed_div_const_lhs,
    Div,
    mir::MirType::I32,
    mir::MirType::I32,
    "sdiv i32"
);
mixed_op!(
    mixed_mod_const_lhs,
    Mod,
    mir::MirType::I32,
    mir::MirType::I32,
    "srem i32"
);
mixed_op!(
    mixed_and_const_lhs,
    And,
    mir::MirType::I32,
    mir::MirType::I32,
    "and i32"
);
mixed_op!(mixed_or_const_lhs, Or, mir::MirType::I32, mir::MirType::I32, "or i32");
mixed_op!(
    mixed_xor_const_lhs,
    Xor,
    mir::MirType::I32,
    mir::MirType::I32,
    "xor i32"
);
mixed_op!(
    mixed_shl_const_lhs,
    Shl,
    mir::MirType::I32,
    mir::MirType::I32,
    "shl i32"
);
mixed_op!(
    mixed_shr_const_lhs,
    Shr,
    mir::MirType::I32,
    mir::MirType::I32,
    "ashr i32"
);
mixed_op!(
    mixed_lt_const_lhs,
    Lt,
    mir::MirType::I32,
    mir::MirType::Bool,
    "icmp slt"
);
mixed_op!(
    mixed_gt_const_lhs,
    Gt,
    mir::MirType::I32,
    mir::MirType::Bool,
    "icmp sgt"
);
mixed_op!(mixed_eq_const_lhs, Eq, mir::MirType::I32, mir::MirType::Bool, "icmp eq");
mixed_op!(mixed_ne_const_lhs, Ne, mir::MirType::I32, mir::MirType::Bool, "icmp ne");

/// Dynamic lhs + constant rhs via different operand ordering.
macro_rules! mixed_op_rhs {
    ($name:ident, $op:ident, $ret:expr, $instr:expr) => {
        #[test]
        fn $name() {
            let h = Harness::new();
            let module = h.build_module(move |b| {
                let i32: mir::MirTypeId = mir::MirType::I32.into();
                let ret: mir::MirTypeId = $ret.into();
                let mut f = b.start_function("f".into(), ret);
                let a = f.add_param("a".into(), i32, false);
                let tmp = f.new_temp(ret, false);
                f.create_block();
                f.push_assign(
                    mir::Place::Local(tmp.clone()),
                    mir::Rvalue::BinaryOp {
                        op: mir::MirBinaryOp::$op,
                        lhs: mir::Operand::Copy(mir::Place::Local(a)),
                        rhs: mir::Operand::Constant(mir::MirLiteral::I32(1)),
                    },
                );
                f.ret(Some(mir::Operand::Copy(mir::Place::Local(tmp))));
                f.finish_function();
            });
            let ir = h.ir(&module);
            assert!(h.verify(&module), "module invalid: {ir}");
            assert!(ir.contains($instr), "expected `{}` in {ir}", $instr);
        }
    };
}

mixed_op_rhs!(mixed_add_const_rhs, Add, mir::MirType::I32, "add i32");
mixed_op_rhs!(mixed_sub_const_rhs, Sub, mir::MirType::I32, "sub i32");
mixed_op_rhs!(mixed_div_const_rhs, Div, mir::MirType::I32, "sdiv i32");
mixed_op_rhs!(mixed_lt_const_rhs, Lt, mir::MirType::Bool, "icmp slt");
mixed_op_rhs!(mixed_eq_const_rhs, Eq, mir::MirType::Bool, "icmp eq");

/// Copy and Move are equivalent at the LLVM level for both Use and BinaryOp.
#[test]
fn move_operand_in_binary_op() {
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
            mir::Rvalue::BinaryOp {
                op: mir::MirBinaryOp::Add,
                lhs: mir::Operand::Move(mir::Place::Local(a)),
                rhs: mir::Operand::Move(mir::Place::Local(b)),
            },
        );
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(tmp))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("add i32"), "expected add: {ir}");
}

/// A function whose return is a constant string operand (not a place) loads
/// the pointer directly.
#[test]
fn constant_str_operand_returns_pointer() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::Str.into());
        f.create_block();
        f.ret(Some(mir::Operand::Constant(mir::MirLiteral::Str("x".into()))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("ptr"), "expected pointer: {ir}");
}

/// Boolean operand used directly as a terminator condition (not copied from a
/// local).
#[test]
fn bool_constant_operand_as_condition() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let a = f.reserve_block();
        let c = f.reserve_block();
        f.create_block();
        f.set_terminator(mir::Terminator::If {
            condition: mir::Operand::Constant(mir::MirLiteral::Bool(true)),
            true_target: a,
            true_args: thin_vec::ThinVec::new(),
            false_target: c,
            false_args: thin_vec::ThinVec::new(),
        });
        f.switch_to_block(a);
        f.ret(Some(mir::Operand::Constant(mir::MirLiteral::I32(1))));
        f.switch_to_block(c);
        f.ret(Some(mir::Operand::Constant(mir::MirLiteral::I32(0))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("br i1"), "expected br i1: {ir}");
}

/// Bool copy operand used as terminator condition (loads i1).
#[test]
fn bool_place_operand_as_condition() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let bool: mir::MirTypeId = mir::MirType::Bool.into();
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let cond = f.add_param("cond".into(), bool, false);
        let a = f.reserve_block();
        let c = f.reserve_block();
        f.create_block();
        f.set_terminator(mir::Terminator::If {
            condition: mir::Operand::Copy(mir::Place::Local(cond)),
            true_target: a,
            true_args: thin_vec::ThinVec::new(),
            false_target: c,
            false_args: thin_vec::ThinVec::new(),
        });
        f.switch_to_block(a);
        f.ret(Some(mir::Operand::Constant(mir::MirLiteral::I32(1))));
        f.switch_to_block(c);
        f.ret(Some(mir::Operand::Constant(mir::MirLiteral::I32(0))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("load i1"), "expected bool load: {ir}");
}
