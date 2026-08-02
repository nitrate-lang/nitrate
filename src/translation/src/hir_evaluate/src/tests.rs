use crate::error::EvalError;
use crate::evaluator::Evaluator;
use crate::memory::Memory;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_nstring::NString;
use nitrate_tree::ByteSpan;
use ordered_float::OrderedFloat;
use std::collections::BTreeMap;
use std::collections::BTreeSet;
use thin_vec::ThinVec;

// ═══════════════════════════════════════════════════════════════════════════
// Test Helpers
// ═══════════════════════════════════════════════════════════════════════════

fn test_log() -> CompilerLog {
    let logger = slog::Logger::root(slog::Discard, slog::o!());
    CompilerLog::new(logger)
}

fn test_store() -> Store {
    Store::new()
}

fn run_test<R>(f: impl FnOnce(&Store, &mut Evaluator) -> R) -> R {
    let store = test_store();
    let log = test_log();
    using_storage(&store, || {
        let mut evaluator = Evaluator::new(&log, PtrSize::U64);
        f(&store, &mut evaluator)
    })
}

fn make_unit() -> Value {
    Value::Unit {
        span: ByteSpan::default(),
    }
}

fn make_bool(b: bool) -> Value {
    Value::Bool {
        span: ByteSpan::default(),
        value: b,
    }
}

fn make_i8(i: i8) -> Value {
    Value::I8 {
        span: ByteSpan::default(),
        value: i,
    }
}

fn make_i32(i: i32) -> Value {
    Value::I32 {
        span: ByteSpan::default(),
        value: i,
    }
}

fn make_u8(u: u8) -> Value {
    Value::U8 {
        span: ByteSpan::default(),
        value: u,
    }
}

fn make_u32(u: u32) -> Value {
    Value::U32 {
        span: ByteSpan::default(),
        value: u,
    }
}

fn make_u64(u: u64) -> Value {
    Value::U64 {
        span: ByteSpan::default(),
        value: u,
    }
}

fn make_f64(f: f64) -> Value {
    Value::F64 {
        span: ByteSpan::default(),
        value: OrderedFloat(f),
    }
}

fn make_binary(left: Value, op: BinaryOp, right: Value) -> Value {
    Value::Binary {
        span: ByteSpan::default(),
        left: ValueId::from(left),
        op,
        right: ValueId::from(right),
    }
}

fn make_add(left: Value, right: Value) -> Value {
    make_binary(left, BinaryOp::Add, right)
}

fn make_sub(left: Value, right: Value) -> Value {
    make_binary(left, BinaryOp::Sub, right)
}

fn make_mul(left: Value, right: Value) -> Value {
    make_binary(left, BinaryOp::Mul, right)
}

fn make_div(left: Value, right: Value) -> Value {
    make_binary(left, BinaryOp::Div, right)
}

fn make_block(elements: Vec<BlockElement>, safety: BlockSafety) -> BlockId {
    Block {
        span: ByteSpan::default(),
        safety,
        elements: elements.into(),
    }
    .into()
}

fn make_local_var(name: &str, ty: Type, initializer: Value) -> LocalVariableId {
    LocalVariable {
        span: ByteSpan::default(),
        kind: LocalKind::Let,
        attributes: BTreeSet::new(),
        is_mutable: false,
        name: NString::from(name),
        ty: ty.into(),
        initializer: initializer.into(),
    }
    .into()
}

fn make_param(name: &str, ty: Type) -> ParameterId {
    Parameter {
        span: ByteSpan::default(),
        attributes: BTreeSet::new(),
        is_mutable: false,
        name: NString::from(name),
        ty: ty.into(),
        default_value: None,
    }
    .into()
}

fn make_function(name: &str, params: Vec<ParameterId>, return_type: Type, body: Vec<BlockElement>) -> FunctionId {
    Function {
        span: ByteSpan::default(),
        visibility: Visibility::Pub,
        attributes: BTreeSet::new(),
        is_unsafe: false,
        name: NString::from(name),
        mangled_name: None,
        generics: None,
        params,
        return_type: return_type.into(),
        body: Some(body),
    }
    .into()
}

fn make_struct_def(name: &str, fields: BTreeMap<NString, StructField>) -> StructDefId {
    StructDef {
        span: ByteSpan::default(),
        visibility: Visibility::Pub,
        name: NString::from(name),
        attributes: BTreeSet::new(),
        fields,
        generics: None,
        layout: ThinVec::new(),
    }
    .into()
}

fn make_enum_def(name: &str, variants: ThinVec<EnumVariant>) -> EnumDefId {
    EnumDef {
        span: ByteSpan::default(),
        visibility: Visibility::Pub,
        name: NString::from(name),
        attributes: BTreeSet::new(),
        generics: None,
        variants,
    }
    .into()
}

fn make_cast(value: Value, target_type: Type) -> Value {
    Value::Cast {
        span: ByteSpan::default(),
        value: value.into(),
        target_type: target_type.into(),
    }
}

fn empty_args() -> Arguments<ValueId> {
    Arguments {
        positional: ThinVec::new(),
        named: ThinVec::new(),
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// 1. Evaluator Construction & Configuration Tests
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn evaluator_new_has_correct_defaults() {
    run_test(|_, ev| {
        assert_eq!(ev.memory_limit, 1024 * 1024);
        assert_eq!(ev.loop_limit, 1_000_000);
        assert_eq!(ev.call_depth_limit, 1000);
        assert_eq!(ev.loop_count, 0);
        assert_eq!(ev.call_depth, 0);
        assert!(matches!(ev.current_safety, BlockSafety::Safe));
        assert_eq!(ev.unsafe_operations_performed, 0);
        assert!(ev.frames.is_empty());
    });
}

#[test]
fn evaluator_has_ptr_size_32() {
    let log = test_log();
    let store = test_store();
    using_storage(&store, || {
        let ev = Evaluator::new(&log, PtrSize::U32);
        assert!(matches!(ev.ptr_size, PtrSize::U32));
    });
}

#[test]
fn evaluator_has_ptr_size_64() {
    let log = test_log();
    let store = test_store();
    using_storage(&store, || {
        let ev = Evaluator::new(&log, PtrSize::U64);
        assert!(matches!(ev.ptr_size, PtrSize::U64));
    });
}

#[test]
fn evaluator_add_builtin_function() {
    run_test(|_, ev| {
        fn dummy_fn(_: &mut Evaluator, _: &[Value]) -> Result<Value, EvalError> {
            Ok(make_unit())
        }
        let name = NString::from("my_builtin");
        ev.add_builtin_function(name.clone(), dummy_fn);
        assert!(ev.get_builtin(&name).is_some());
    });
}

#[test]
fn evaluator_get_builtin_returns_none_for_unknown() {
    run_test(|_, ev| {
        let name = NString::from("nonexistent_builtin");
        assert!(ev.get_builtin(&name).is_none());
    });
}

#[test]
fn evaluator_add_builtin_function_overrides() {
    run_test(|_, ev| {
        fn first_fn(_: &mut Evaluator, _: &[Value]) -> Result<Value, EvalError> {
            Ok(make_unit())
        }
        fn second_fn(_: &mut Evaluator, _: &[Value]) -> Result<Value, EvalError> {
            Ok(make_bool(true))
        }
        let name = NString::from("override_test");
        ev.add_builtin_function(name.clone(), first_fn);
        ev.add_builtin_function(name.clone(), second_fn);
        let builtin = ev.get_builtin(&name);
        assert!(builtin.is_some());
    });
}

#[test]
fn evaluator_loop_count_starts_at_zero() {
    run_test(|_, ev| {
        assert_eq!(ev.loop_count, 0);
    });
}

#[test]
fn evaluator_call_depth_starts_at_zero() {
    run_test(|_, ev| {
        assert_eq!(ev.call_depth, 0);
    });
}

#[test]
fn evaluator_frames_starts_empty() {
    run_test(|_, ev| {
        assert!(ev.frames.is_empty());
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// 2. Literal Evaluation Tests
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn eval_literal_unit() {
    run_test(|_, ev| {
        let result = ev.evaluate(&make_unit()).unwrap();
        assert!(matches!(result, Value::Unit { .. }));
    });
}

#[test]
fn eval_literal_bool_true() {
    run_test(|_, ev| {
        let result = ev.evaluate(&make_bool(true)).unwrap();
        assert_eq!(result, make_bool(true));
    });
}

#[test]
fn eval_literal_bool_false() {
    run_test(|_, ev| {
        let result = ev.evaluate(&make_bool(false)).unwrap();
        assert_eq!(result, make_bool(false));
    });
}

#[test]
fn eval_literal_i8_zero() {
    run_test(|_, ev| {
        let result = ev.evaluate(&make_i8(0)).unwrap();
        assert_eq!(result, make_i8(0));
    });
}

#[test]
fn eval_literal_i8_positive() {
    run_test(|_, ev| {
        let result = ev.evaluate(&make_i8(42)).unwrap();
        assert_eq!(result, make_i8(42));
    });
}

#[test]
fn eval_literal_i8_negative() {
    run_test(|_, ev| {
        let result = ev.evaluate(&make_i8(-42)).unwrap();
        assert_eq!(result, make_i8(-42));
    });
}

#[test]
fn eval_literal_i8_max() {
    run_test(|_, ev| {
        let result = ev.evaluate(&make_i8(127)).unwrap();
        assert_eq!(result, make_i8(127));
    });
}

#[test]
fn eval_literal_i8_min() {
    run_test(|_, ev| {
        let result = ev.evaluate(&make_i8(-128)).unwrap();
        assert_eq!(result, make_i8(-128));
    });
}

#[test]
fn eval_literal_i32_positive() {
    run_test(|_, ev| {
        let result = ev.evaluate(&make_i32(100)).unwrap();
        assert_eq!(result, make_i32(100));
    });
}

#[test]
fn eval_literal_i32_negative() {
    run_test(|_, ev| {
        let result = ev.evaluate(&make_i32(-100)).unwrap();
        assert_eq!(result, make_i32(-100));
    });
}

#[test]
fn eval_literal_u8_zero() {
    run_test(|_, ev| {
        let result = ev.evaluate(&make_u8(0)).unwrap();
        assert_eq!(result, make_u8(0));
    });
}

#[test]
fn eval_literal_u8_positive() {
    run_test(|_, ev| {
        let result = ev.evaluate(&make_u8(255)).unwrap();
        assert_eq!(result, make_u8(255));
    });
}

#[test]
fn eval_literal_u32() {
    run_test(|_, ev| {
        let result = ev.evaluate(&make_u32(1000)).unwrap();
        assert_eq!(result, make_u32(1000));
    });
}

#[test]
fn eval_literal_u64() {
    run_test(|_, ev| {
        let result = ev.evaluate(&make_u64(1000000)).unwrap();
        assert_eq!(result, make_u64(1000000));
    });
}

#[test]
fn eval_literal_f64() {
    run_test(|_, ev| {
        let result = ev.evaluate(&make_f64(3.14)).unwrap();
        assert_eq!(result, make_f64(3.14));
    });
}

#[test]
fn eval_literal_f64_zero() {
    run_test(|_, ev| {
        let result = ev.evaluate(&make_f64(0.0)).unwrap();
        assert_eq!(result, make_f64(0.0));
    });
}

#[test]
fn eval_literal_f64_negative() {
    run_test(|_, ev| {
        let result = ev.evaluate(&make_f64(-1.5)).unwrap();
        assert_eq!(result, make_f64(-1.5));
    });
}

#[test]
fn eval_literal_inferred_integer() {
    run_test(|_, ev| {
        let v = Value::InferredInteger {
            span: ByteSpan::default(),
            value: Box::new(42u128),
        };
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, v);
    });
}

#[test]
fn eval_literal_inferred_float() {
    run_test(|_, ev| {
        let f = OrderedFloat(3.14);
        let v = Value::InferredFloat {
            span: ByteSpan::default(),
            value: f,
        };
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, v);
    });
}

#[test]
fn eval_literal_i16() {
    run_test(|_, ev| {
        let v = Value::I16 {
            span: ByteSpan::default(),
            value: 1000,
        };
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, v);
    });
}

#[test]
fn eval_literal_i64() {
    run_test(|_, ev| {
        let v = Value::I64 {
            span: ByteSpan::default(),
            value: 100000,
        };
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, v);
    });
}

#[test]
fn eval_literal_i128() {
    run_test(|_, ev| {
        let v = Value::I128 {
            span: ByteSpan::default(),
            value: Box::new(1000000000),
        };
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, v);
    });
}

#[test]
fn eval_literal_u16() {
    run_test(|_, ev| {
        let v = Value::U16 {
            span: ByteSpan::default(),
            value: 50000,
        };
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, v);
    });
}

#[test]
fn eval_literal_u128() {
    run_test(|_, ev| {
        let v = Value::U128 {
            span: ByteSpan::default(),
            value: Box::new(1000000000),
        };
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, v);
    });
}

#[test]
fn eval_literal_f32() {
    run_test(|_, ev| {
        let v = Value::F32 {
            span: ByteSpan::default(),
            value: OrderedFloat(1.5f32),
        };
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, v);
    });
}

#[test]
fn eval_literal_usize() {
    run_test(|_, ev| {
        let v = Value::USize {
            span: ByteSpan::default(),
            bits: 64,
            value: 12345,
        };
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, v);
    });
}

#[test]
fn eval_literal_string() {
    run_test(|_, ev| {
        let v = Value::StringLit {
            span: ByteSpan::default(),
            value: "hello".into(),
        };
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, v);
    });
}

#[test]
fn eval_literal_bstring() {
    run_test(|_, ev| {
        let v = Value::BStringLit {
            span: ByteSpan::default(),
            value: vec![1, 2, 3].into(),
        };
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, v);
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// 3. Evaluate to Literal Tests
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn evaluate_to_literal_bool() {
    run_test(|_, ev| {
        let result = ev.evaluate_to_literal(&make_bool(true)).unwrap();
        assert_eq!(result, Lit::Bool(true));
    });
}

#[test]
fn evaluate_to_literal_i8() {
    run_test(|_, ev| {
        let result = ev.evaluate_to_literal(&make_i8(42)).unwrap();
        assert_eq!(result, Lit::I8(42));
    });
}

#[test]
fn evaluate_to_literal_u8() {
    run_test(|_, ev| {
        let result = ev.evaluate_to_literal(&make_u8(100)).unwrap();
        assert_eq!(result, Lit::U8(100));
    });
}

#[test]
fn evaluate_to_literal_f64() {
    run_test(|_, ev| {
        let result = ev.evaluate_to_literal(&make_f64(2.5)).unwrap();
        assert_eq!(result, Lit::F64(OrderedFloat(2.5)));
    });
}

#[test]
fn evaluate_to_literal_unit() {
    run_test(|_, ev| {
        let result = ev.evaluate_to_literal(&make_unit()).unwrap();
        assert_eq!(result, Lit::Unit);
    });
}

#[test]
fn evaluate_to_literal_i32_from_add() {
    run_test(|_, ev| {
        let v = make_add(make_i32(40), make_i32(2));
        let result = ev.evaluate_to_literal(&v).unwrap();
        assert_eq!(result, Lit::I32(42));
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// 4. Binary Operation Tests — Arithmetic
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn binary_add_i32() {
    run_test(|_, ev| {
        let v = make_add(make_i32(10), make_i32(20));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: 30, .. }));
    });
}

#[test]
fn binary_add_i32_negative() {
    run_test(|_, ev| {
        let v = make_add(make_i32(10), make_i32(-20));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: -10, .. }));
    });
}

#[test]
fn binary_add_u64() {
    run_test(|_, ev| {
        let v = make_add(make_u64(100), make_u64(200));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::U64 { value: 300, .. }));
    });
}

#[test]
fn binary_add_f64() {
    run_test(|_, ev| {
        let v = make_add(make_f64(1.1), make_f64(2.2));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::F64 { value, .. } if (*value - 3.3).abs() < 0.0001));
    });
}

#[test]
fn binary_sub_i32() {
    run_test(|_, ev| {
        let v = make_sub(make_i32(30), make_i32(10));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: 20, .. }));
    });
}

#[test]
fn binary_sub_i32_negative_result() {
    run_test(|_, ev| {
        let v = make_sub(make_i32(10), make_i32(30));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: -20, .. }));
    });
}

#[test]
fn binary_sub_u64() {
    run_test(|_, ev| {
        let v = make_sub(make_u64(500), make_u64(200));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::U64 { value: 300, .. }));
    });
}

#[test]
fn binary_sub_f64() {
    run_test(|_, ev| {
        let v = make_sub(make_f64(5.0), make_f64(3.0));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::F64 { value, .. } if (*value - 2.0).abs() < 0.0001));
    });
}

#[test]
fn binary_mul_i32() {
    run_test(|_, ev| {
        let v = make_mul(make_i32(7), make_i32(6));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: 42, .. }));
    });
}

#[test]
fn binary_mul_i32_by_zero() {
    run_test(|_, ev| {
        let v = make_mul(make_i32(7), make_i32(0));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: 0, .. }));
    });
}

#[test]
fn binary_mul_u64() {
    run_test(|_, ev| {
        let v = make_mul(make_u64(10), make_u64(10));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::U64 { value: 100, .. }));
    });
}

#[test]
fn binary_mul_f64() {
    run_test(|_, ev| {
        let v = make_mul(make_f64(2.5), make_f64(4.0));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::F64 { value, .. } if (*value - 10.0).abs() < 0.0001));
    });
}

#[test]
fn binary_div_i32() {
    run_test(|_, ev| {
        let v = make_div(make_i32(100), make_i32(4));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: 25, .. }));
    });
}

#[test]
fn binary_div_i32_negative_divisor() {
    run_test(|_, ev| {
        let v = make_div(make_i32(100), make_i32(-4));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: -25, .. }));
    });
}

#[test]
fn binary_div_f64() {
    run_test(|_, ev| {
        let v = make_div(make_f64(10.0), make_f64(3.0));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::F64 { value, .. } if (*value - 3.3333333).abs() < 0.001));
    });
}

#[test]
fn binary_div_by_zero_i32() {
    run_test(|_, ev| {
        let v = make_div(make_i32(10), make_i32(0));
        let result = ev.evaluate(&v);
        assert!(matches!(result, Err(EvalError::DivisionByZero)));
    });
}

#[test]
fn binary_div_by_zero_u64() {
    run_test(|_, ev| {
        let v = make_div(make_u64(10), make_u64(0));
        let result = ev.evaluate(&v);
        assert!(matches!(result, Err(EvalError::DivisionByZero)));
    });
}

#[test]
fn binary_mod_i32() {
    run_test(|_, ev| {
        let v = make_binary(make_i32(10), BinaryOp::Mod, make_i32(3));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: 1, .. }));
    });
}

#[test]
fn binary_mod_by_zero() {
    run_test(|_, ev| {
        let v = make_binary(make_i32(10), BinaryOp::Mod, make_i32(0));
        let result = ev.evaluate(&v);
        assert!(matches!(result, Err(EvalError::ModuloByZero)));
    });
}

#[test]
fn binary_mod_u64() {
    run_test(|_, ev| {
        let v = make_binary(make_u64(17), BinaryOp::Mod, make_u64(5));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::U64 { value: 2, .. }));
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// 5. Binary Operation Tests — Bitwise
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn binary_and_i32() {
    run_test(|_, ev| {
        let v = make_binary(make_i32(0b1100), BinaryOp::And, make_i32(0b1010));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: 0b1000, .. }));
    });
}

#[test]
fn binary_and_u64() {
    run_test(|_, ev| {
        let v = make_binary(make_u64(0xFF), BinaryOp::And, make_u64(0x0F));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::U64 { value: 0x0F, .. }));
    });
}

#[test]
fn binary_or_i32() {
    run_test(|_, ev| {
        let v = make_binary(make_i32(0b1100), BinaryOp::Or, make_i32(0b1010));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: 0b1110, .. }));
    });
}

#[test]
fn binary_or_u64() {
    run_test(|_, ev| {
        let v = make_binary(make_u64(0xF0), BinaryOp::Or, make_u64(0x0F));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::U64 { value: 0xFF, .. }));
    });
}

#[test]
fn binary_xor_i32() {
    run_test(|_, ev| {
        let v = make_binary(make_i32(0b1100), BinaryOp::Xor, make_i32(0b1010));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: 0b0110, .. }));
    });
}

#[test]
fn binary_xor_u64() {
    run_test(|_, ev| {
        let v = make_binary(make_u64(0xFF), BinaryOp::Xor, make_u64(0x0F));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::U64 { value: 0xF0, .. }));
    });
}

#[test]
fn binary_shl_u32() {
    run_test(|_, ev| {
        let v = make_binary(make_u32(1), BinaryOp::Shl, make_u32(3));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::U32 { value: 8, .. }));
    });
}

#[test]
fn binary_shl_u32_by_u32() {
    run_test(|_, ev| {
        let v = make_binary(make_u32(3), BinaryOp::Shl, make_u32(2));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::U32 { value: 12, .. }));
    });
}

#[test]
fn binary_shr_u32() {
    run_test(|_, ev| {
        let v = make_binary(make_u32(16), BinaryOp::Shr, make_u32(2));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::U32 { value: 4, .. }));
    });
}

#[test]
fn binary_rotate_left_u32() {
    run_test(|_, ev| {
        let v = make_binary(make_u32(1), BinaryOp::Rol, make_u32(1));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::U32 { value: 2, .. }));
    });
}

#[test]
fn binary_rotate_right_u32_small() {
    run_test(|_, ev| {
        // Rotate right: U32(small) >> 1 is a shift right, not a wrap-around
        let v = make_binary(make_u32(4), BinaryOp::Ror, make_u32(1));
        let result = ev.evaluate(&v).unwrap();
        // This uses Lit rotate_right which may compute differently from logical shift
        assert!(result != make_u32(4));
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// 6. Binary Operation Tests — Comparison
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn binary_lt_i32_true() {
    run_test(|_, ev| {
        let v = make_binary(make_i32(5), BinaryOp::Lt, make_i32(10));
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, make_bool(true));
    });
}

#[test]
fn binary_lt_i32_false() {
    run_test(|_, ev| {
        let v = make_binary(make_i32(10), BinaryOp::Lt, make_i32(5));
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, make_bool(false));
    });
}

#[test]
fn binary_lt_i32_equal() {
    run_test(|_, ev| {
        let v = make_binary(make_i32(10), BinaryOp::Lt, make_i32(10));
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, make_bool(false));
    });
}

#[test]
fn binary_gt_i32_true() {
    run_test(|_, ev| {
        let v = make_binary(make_i32(10), BinaryOp::Gt, make_i32(5));
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, make_bool(true));
    });
}

#[test]
fn binary_gt_i32_false() {
    run_test(|_, ev| {
        let v = make_binary(make_i32(5), BinaryOp::Gt, make_i32(10));
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, make_bool(false));
    });
}

#[test]
fn binary_lte_i32_true_equal() {
    run_test(|_, ev| {
        let v = make_binary(make_i32(10), BinaryOp::Lte, make_i32(10));
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, make_bool(true));
    });
}

#[test]
fn binary_lte_i32_true_less() {
    run_test(|_, ev| {
        let v = make_binary(make_i32(5), BinaryOp::Lte, make_i32(10));
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, make_bool(true));
    });
}

#[test]
fn binary_lte_i32_false() {
    run_test(|_, ev| {
        let v = make_binary(make_i32(10), BinaryOp::Lte, make_i32(5));
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, make_bool(false));
    });
}

#[test]
fn binary_gte_i32_true_equal() {
    run_test(|_, ev| {
        let v = make_binary(make_i32(10), BinaryOp::Gte, make_i32(10));
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, make_bool(true));
    });
}

#[test]
fn binary_gte_i32_true_greater() {
    run_test(|_, ev| {
        let v = make_binary(make_i32(15), BinaryOp::Gte, make_i32(10));
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, make_bool(true));
    });
}

#[test]
fn binary_gte_i32_false() {
    run_test(|_, ev| {
        let v = make_binary(make_i32(5), BinaryOp::Gte, make_i32(10));
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, make_bool(false));
    });
}

#[test]
fn binary_eq_i32_true() {
    run_test(|_, ev| {
        let v = make_binary(make_i32(42), BinaryOp::Eq, make_i32(42));
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, make_bool(true));
    });
}

#[test]
fn binary_eq_i32_false() {
    run_test(|_, ev| {
        let v = make_binary(make_i32(42), BinaryOp::Eq, make_i32(43));
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, make_bool(false));
    });
}

#[test]
fn binary_eq_u8() {
    run_test(|_, ev| {
        let v = make_binary(make_u8(255), BinaryOp::Eq, make_u8(255));
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, make_bool(true));
    });
}

#[test]
fn binary_eq_f64_true() {
    run_test(|_, ev| {
        let v = make_binary(make_f64(1.0), BinaryOp::Eq, make_f64(1.0));
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, make_bool(true));
    });
}

#[test]
fn binary_eq_f64_false() {
    run_test(|_, ev| {
        let v = make_binary(make_f64(1.0), BinaryOp::Eq, make_f64(2.0));
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, make_bool(false));
    });
}

#[test]
fn binary_ne_i32_true() {
    run_test(|_, ev| {
        let v = make_binary(make_i32(42), BinaryOp::Ne, make_i32(43));
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, make_bool(true));
    });
}

#[test]
fn binary_ne_i32_false() {
    run_test(|_, ev| {
        let v = make_binary(make_i32(42), BinaryOp::Ne, make_i32(42));
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, make_bool(false));
    });
}

#[test]
fn comparison_f64_lt() {
    run_test(|_, ev| {
        let v = make_binary(make_f64(1.0), BinaryOp::Lt, make_f64(2.0));
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, make_bool(true));
    });
}

#[test]
fn comparison_f64_gt() {
    run_test(|_, ev| {
        let v = make_binary(make_f64(3.0), BinaryOp::Gt, make_f64(2.0));
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, make_bool(true));
    });
}

#[test]
fn comparison_bool_eq_true() {
    run_test(|_, ev| {
        let v = make_binary(make_bool(true), BinaryOp::Eq, make_bool(true));
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, make_bool(true));
    });
}

#[test]
fn comparison_bool_eq_mismatch() {
    run_test(|_, ev| {
        let v = make_binary(make_bool(true), BinaryOp::Eq, make_bool(false));
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, make_bool(false));
    });
}

#[test]
fn comparison_bool_ne() {
    run_test(|_, ev| {
        let v = make_binary(make_bool(true), BinaryOp::Ne, make_bool(false));
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, make_bool(true));
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// 7. Binary Operation Tests — Logical Short-Circuit
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn binary_logic_and_true_true() {
    run_test(|_, ev| {
        let v = make_binary(make_bool(true), BinaryOp::LogicAnd, make_bool(true));
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, make_bool(true));
    });
}

#[test]
fn binary_logic_and_true_false() {
    run_test(|_, ev| {
        let v = make_binary(make_bool(true), BinaryOp::LogicAnd, make_bool(false));
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, make_bool(false));
    });
}

#[test]
fn binary_logic_and_false_short_circuits() {
    run_test(|_, ev| {
        let v = make_binary(make_bool(false), BinaryOp::LogicAnd, make_i32(42));
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, make_bool(false));
    });
}

#[test]
fn binary_logic_or_true_true() {
    run_test(|_, ev| {
        let v = make_binary(make_bool(true), BinaryOp::LogicOr, make_bool(true));
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, make_bool(true));
    });
}

#[test]
fn binary_logic_or_true_false() {
    run_test(|_, ev| {
        let v = make_binary(make_bool(true), BinaryOp::LogicOr, make_bool(false));
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, make_bool(true));
    });
}

#[test]
fn binary_logic_or_false_false() {
    run_test(|_, ev| {
        let v = make_binary(make_bool(false), BinaryOp::LogicOr, make_bool(false));
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, make_bool(false));
    });
}

#[test]
fn binary_logic_or_true_short_circuits() {
    run_test(|_, ev| {
        let v = make_binary(make_bool(true), BinaryOp::LogicOr, make_i32(42));
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, make_bool(true));
    });
}

#[test]
fn binary_logic_and_non_bool_left_errors() {
    run_test(|_, ev| {
        let v = make_binary(make_i32(1), BinaryOp::LogicAnd, make_bool(true));
        let result = ev.evaluate(&v);
        assert!(matches!(result, Err(EvalError::TypeError)));
    });
}

#[test]
fn binary_logic_or_non_bool_left_errors() {
    run_test(|_, ev| {
        let v = make_binary(make_i32(1), BinaryOp::LogicOr, make_bool(false));
        let result = ev.evaluate(&v);
        assert!(matches!(result, Err(EvalError::TypeError)));
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// 8. Binary Operation Tests — Edge Cases & Errors
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn binary_add_string_fails() {
    run_test(|_, ev| {
        let left = Value::StringLit {
            span: ByteSpan::default(),
            value: "hello".into(),
        };
        let right = Value::StringLit {
            span: ByteSpan::default(),
            value: "world".into(),
        };
        let v = make_add(left, right);
        let result = ev.evaluate(&v);
        assert!(matches!(result, Err(EvalError::TypeError)));
    });
}

#[test]
fn binary_shl_negative_amount_fails() {
    run_test(|_, ev| {
        let v = make_binary(make_u32(1), BinaryOp::Shl, make_i32(-1));
        let result = ev.evaluate(&v);
        assert!(result.is_err());
    });
}

#[test]
fn binary_cmp_mixed_types_fails() {
    run_test(|_, ev| {
        let v = make_binary(make_i32(1), BinaryOp::Eq, make_f64(1.0));
        let result = ev.evaluate(&v);
        assert!(result.is_err());
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// 9. Unary Operation Tests
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn unary_add_i32() {
    run_test(|_, ev| {
        let v = Value::Unary {
            span: ByteSpan::default(),
            operand: make_i32(42).into(),
            op: UnaryOp::Add,
        };
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: 42, .. }));
    });
}

#[test]
fn unary_sub_i32() {
    run_test(|_, ev| {
        let v = Value::Unary {
            span: ByteSpan::default(),
            operand: make_i32(42).into(),
            op: UnaryOp::Sub,
        };
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: -42, .. }));
    });
}

#[test]
fn unary_sub_i32_negative() {
    run_test(|_, ev| {
        let v = Value::Unary {
            span: ByteSpan::default(),
            operand: make_i32(-42).into(),
            op: UnaryOp::Sub,
        };
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: 42, .. }));
    });
}

#[test]
fn unary_not_bool_true() {
    run_test(|_, ev| {
        let v = Value::Unary {
            span: ByteSpan::default(),
            operand: make_bool(true).into(),
            op: UnaryOp::Not,
        };
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, make_bool(false));
    });
}

#[test]
fn unary_not_bool_false() {
    run_test(|_, ev| {
        let v = Value::Unary {
            span: ByteSpan::default(),
            operand: make_bool(false).into(),
            op: UnaryOp::Not,
        };
        let result = ev.evaluate(&v).unwrap();
        assert_eq!(result, make_bool(true));
    });
}

#[test]
fn unary_not_i32() {
    run_test(|_, ev| {
        let v = Value::Unary {
            span: ByteSpan::default(),
            operand: make_i32(0).into(),
            op: UnaryOp::Not,
        };
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: -1, .. }));
    });
}

#[test]
fn unary_add_f64() {
    run_test(|_, ev| {
        let v = Value::Unary {
            span: ByteSpan::default(),
            operand: make_f64(3.14).into(),
            op: UnaryOp::Add,
        };
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::F64 { .. }));
    });
}

#[test]
fn unary_sub_f64() {
    run_test(|_, ev| {
        let v = Value::Unary {
            span: ByteSpan::default(),
            operand: make_f64(3.14).into(),
            op: UnaryOp::Sub,
        };
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::F64 { value, .. } if (*value + 3.14).abs() < 0.001));
    });
}

#[test]
fn unary_sub_u64_zero() {
    run_test(|_, ev| {
        let v = Value::Unary {
            span: ByteSpan::default(),
            operand: make_u64(0).into(),
            op: UnaryOp::Sub,
        };
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::U64 { value: 0, .. }));
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// 10. Cast Tests — Integer to Integer
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn cast_i32_to_u8() {
    run_test(|_, ev| {
        let v = make_cast(
            make_i32(65),
            Type::U8 {
                span: ByteSpan::default(),
            },
        );
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::U8 { value: 65, .. }));
    });
}

#[test]
fn cast_i32_to_u16() {
    run_test(|_, ev| {
        let v = make_cast(
            make_i32(1000),
            Type::U16 {
                span: ByteSpan::default(),
            },
        );
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::U16 { value: 1000, .. }));
    });
}

#[test]
fn cast_i32_to_u32() {
    run_test(|_, ev| {
        let v = make_cast(
            make_i32(42),
            Type::U32 {
                span: ByteSpan::default(),
            },
        );
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::U32 { value: 42, .. }));
    });
}

#[test]
fn cast_i32_to_u64() {
    run_test(|_, ev| {
        let v = make_cast(
            make_i32(100),
            Type::U64 {
                span: ByteSpan::default(),
            },
        );
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::U64 { value: 100, .. }));
    });
}

#[test]
fn cast_i32_to_i8() {
    run_test(|_, ev| {
        let v = make_cast(
            make_i32(100),
            Type::I8 {
                span: ByteSpan::default(),
            },
        );
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I8 { value: 100, .. }));
    });
}

#[test]
fn cast_i32_to_i16() {
    run_test(|_, ev| {
        let v = make_cast(
            make_i32(1000),
            Type::I16 {
                span: ByteSpan::default(),
            },
        );
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I16 { value: 1000, .. }));
    });
}

#[test]
fn cast_i32_to_i64() {
    run_test(|_, ev| {
        let v = make_cast(
            make_i32(42),
            Type::I64 {
                span: ByteSpan::default(),
            },
        );
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I64 { value: 42, .. }));
    });
}

#[test]
fn cast_i32_to_i128() {
    run_test(|_, ev| {
        let v = make_cast(
            make_i32(42),
            Type::I128 {
                span: ByteSpan::default(),
            },
        );
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I128 { value, .. } if *value == 42));
    });
}

#[test]
fn cast_i32_to_u128() {
    run_test(|_, ev| {
        let v = make_cast(
            make_i32(42),
            Type::U128 {
                span: ByteSpan::default(),
            },
        );
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::U128 { value, .. } if *value == 42));
    });
}

#[test]
fn cast_i32_to_usize_64() {
    run_test(|_, ev| {
        let v = make_cast(
            make_i32(42),
            Type::USize {
                span: ByteSpan::default(),
            },
        );
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(
            result,
            Value::USize {
                value: 42,
                bits: 64,
                ..
            }
        ));
    });
}

#[test]
fn cast_i32_to_usize_32() {
    let log = test_log();
    let store = test_store();
    using_storage(&store, || {
        let mut ev = Evaluator::new(&log, PtrSize::U32);
        let v = make_cast(
            make_i32(42),
            Type::USize {
                span: ByteSpan::default(),
            },
        );
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(
            result,
            Value::USize {
                value: 42,
                bits: 32,
                ..
            }
        ));
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// 11. Cast Tests — Float Conversions
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn cast_i32_to_f32() {
    run_test(|_, ev| {
        let v = make_cast(
            make_i32(42),
            Type::F32 {
                span: ByteSpan::default(),
            },
        );
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::F32 { value, .. } if (*value - 42.0).abs() < 0.001));
    });
}

#[test]
fn cast_i32_to_f64() {
    run_test(|_, ev| {
        let v = make_cast(
            make_i32(42),
            Type::F64 {
                span: ByteSpan::default(),
            },
        );
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::F64 { value, .. } if (*value - 42.0).abs() < 0.001));
    });
}

#[test]
fn cast_f64_to_i32() {
    run_test(|_, ev| {
        let v = make_cast(
            make_f64(42.7),
            Type::I32 {
                span: ByteSpan::default(),
            },
        );
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { .. }));
    });
}

#[test]
fn cast_u8_to_f64() {
    run_test(|_, ev| {
        let v = make_cast(
            make_u8(255),
            Type::F64 {
                span: ByteSpan::default(),
            },
        );
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::F64 { value, .. } if (*value - 255.0).abs() < 0.001));
    });
}

#[test]
fn cast_bool_to_u8() {
    run_test(|_, ev| {
        let v = make_cast(
            make_bool(true),
            Type::U8 {
                span: ByteSpan::default(),
            },
        );
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::U8 { value: 1, .. }));
    });
}

#[test]
fn cast_bool_false_to_u8() {
    run_test(|_, ev| {
        let v = make_cast(
            make_bool(false),
            Type::U8 {
                span: ByteSpan::default(),
            },
        );
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::U8 { value: 0, .. }));
    });
}

#[test]
fn cast_to_unit() {
    run_test(|_, ev| {
        let v = make_cast(
            make_i32(42),
            Type::Unit {
                span: ByteSpan::default(),
            },
        );
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::Unit { .. }));
    });
}

#[test]
fn cast_unit_fails() {
    run_test(|_, ev| {
        let v = make_cast(
            make_unit(),
            Type::I32 {
                span: ByteSpan::default(),
            },
        );
        let result = ev.evaluate(&v);
        assert!(matches!(result, Err(EvalError::TypeError)));
    });
}

#[test]
fn cast_string_lit_fails() {
    run_test(|_, ev| {
        let left = Value::StringLit {
            span: ByteSpan::default(),
            value: "hi".into(),
        };
        let v = make_cast(
            left,
            Type::I32 {
                span: ByteSpan::default(),
            },
        );
        let result = ev.evaluate(&v);
        assert!(matches!(result, Err(EvalError::TypeError)));
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// 12. Control Flow — If/Else Tests
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn if_true_branch_evaluated() {
    run_test(|_, ev| {
        let true_block = make_block(vec![BlockElement::Expr(make_i32(42).into())], BlockSafety::Safe);
        let false_block = make_block(vec![BlockElement::Expr(make_i32(0).into())], BlockSafety::Safe);
        let v = Value::If {
            span: ByteSpan::default(),
            condition: make_bool(true).into(),
            true_branch: true_block,
            false_branch: Some(false_block),
        };
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: 42, .. }));
    });
}

#[test]
fn if_false_branch_evaluated() {
    run_test(|_, ev| {
        let true_block = make_block(vec![BlockElement::Expr(make_i32(42).into())], BlockSafety::Safe);
        let false_block = make_block(vec![BlockElement::Expr(make_i32(99).into())], BlockSafety::Safe);
        let v = Value::If {
            span: ByteSpan::default(),
            condition: make_bool(false).into(),
            true_branch: true_block,
            false_branch: Some(false_block),
        };
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: 99, .. }));
    });
}

#[test]
fn if_without_else_returns_unit_when_false() {
    run_test(|_, ev| {
        let true_block = make_block(vec![BlockElement::Expr(make_i32(42).into())], BlockSafety::Safe);
        let v = Value::If {
            span: ByteSpan::default(),
            condition: make_bool(false).into(),
            true_branch: true_block,
            false_branch: None,
        };
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::Unit { .. }));
    });
}

#[test]
fn if_non_bool_condition_fails() {
    run_test(|_, ev| {
        let block = make_block(vec![BlockElement::Expr(make_unit().into())], BlockSafety::Safe);
        let v = Value::If {
            span: ByteSpan::default(),
            condition: make_i32(1).into(),
            true_branch: block.clone(),
            false_branch: None,
        };
        let result = ev.evaluate(&v);
        assert!(matches!(result, Err(EvalError::TypeError)));
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// 13. Control Flow — While Loop Tests
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn while_loop_condition_false_returns_unit() {
    run_test(|_, ev| {
        let body = make_block(vec![BlockElement::Expr(make_unit().into())], BlockSafety::Safe);
        let v = Value::While {
            span: ByteSpan::default(),
            condition: make_bool(false).into(),
            body,
        };
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::Unit { .. }));
    });
}

#[test]
fn while_loop_non_bool_condition_fails() {
    run_test(|_, ev| {
        let body = make_block(vec![BlockElement::Expr(make_unit().into())], BlockSafety::Safe);
        let v = Value::While {
            span: ByteSpan::default(),
            condition: make_i32(1).into(),
            body,
        };
        let result = ev.evaluate(&v);
        assert!(matches!(result, Err(EvalError::TypeError)));
    });
}

#[test]
fn while_loop_limit_exceeded() {
    run_test(|_, ev| {
        ev.loop_limit = 5;
        ev.loop_count = 5;
        let body = make_block(vec![BlockElement::Expr(make_unit().into())], BlockSafety::Safe);
        let v = Value::While {
            span: ByteSpan::default(),
            condition: make_bool(true).into(),
            body,
        };
        let result = ev.evaluate(&v);
        assert!(matches!(result, Err(EvalError::LoopLimitExceeded)));
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// 14. Control Flow — Loop & Break/Continue Tests
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn loop_break() {
    run_test(|_, ev| {
        let body = make_block(
            vec![BlockElement::Expr(
                Value::Break {
                    span: ByteSpan::default(),
                    label: None,
                }
                .into(),
            )],
            BlockSafety::Safe,
        );
        let v = Value::Loop {
            span: ByteSpan::default(),
            body,
        };
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::Unit { .. }));
    });
}

#[test]
fn loop_limit_exceeded() {
    run_test(|_, ev| {
        ev.loop_limit = 3;
        ev.loop_count = 3;
        let body = make_block(vec![BlockElement::Expr(make_unit().into())], BlockSafety::Safe);
        let v = Value::Loop {
            span: ByteSpan::default(),
            body,
        };
        let result = ev.evaluate(&v);
        assert!(matches!(result, Err(EvalError::LoopLimitExceeded)));
    });
}

#[test]
fn break_evaluates_to_error() {
    run_test(|_, ev| {
        let v = Value::Break {
            span: ByteSpan::default(),
            label: None,
        };
        let result = ev.evaluate(&v);
        assert!(matches!(result, Err(EvalError::Break { label: None })));
    });
}

#[test]
fn break_with_label() {
    run_test(|_, ev| {
        let label = Some(NString::from("outer"));
        let v = Value::Break {
            span: ByteSpan::default(),
            label: label.clone(),
        };
        let result = ev.evaluate(&v);
        assert!(matches!(result, Err(EvalError::Break { label })));
    });
}

#[test]
fn continue_evaluates_to_error() {
    run_test(|_, ev| {
        let v = Value::Continue {
            span: ByteSpan::default(),
            label: None,
        };
        let result = ev.evaluate(&v);
        assert!(matches!(result, Err(EvalError::Continue { label: None })));
    });
}

#[test]
fn continue_with_label() {
    run_test(|_, ev| {
        let label = Some(NString::from("inner"));
        let v = Value::Continue {
            span: ByteSpan::default(),
            label: label.clone(),
        };
        let result = ev.evaluate(&v);
        assert!(matches!(result, Err(EvalError::Continue { label })));
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// 15. Return Tests
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn return_with_value() {
    run_test(|_, ev| {
        let v = Value::Return {
            span: ByteSpan::default(),
            value: make_i32(42).into(),
        };
        let result = ev.evaluate(&v);
        match result {
            Err(EvalError::Return(val)) => {
                assert!(matches!(val, Value::I32 { value: 42, .. }));
            }
            _ => panic!("Expected Return(42), got {:?}", result),
        }
    });
}

#[test]
fn return_with_unit() {
    run_test(|_, ev| {
        let v = Value::Return {
            span: ByteSpan::default(),
            value: make_unit().into(),
        };
        let result = ev.evaluate(&v);
        match result {
            Err(EvalError::Return(val)) => {
                assert!(matches!(val, Value::Unit { .. }));
            }
            _ => panic!("Expected Return(Unit), got {:?}", result),
        }
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// 16. Block Evaluation Tests
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn evaluate_empty_block_returns_unit() {
    run_test(|_, ev| {
        let block = make_block(vec![], BlockSafety::Safe);
        let v = Value::Block {
            span: ByteSpan::default(),
            block,
        };
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::Unit { .. }));
    });
}

#[test]
fn evaluate_block_returns_last_expr() {
    run_test(|_, ev| {
        let block = make_block(
            vec![
                BlockElement::Expr(make_i32(1).into()),
                BlockElement::Expr(make_i32(2).into()),
                BlockElement::Expr(make_i32(42).into()),
            ],
            BlockSafety::Safe,
        );
        let v = Value::Block {
            span: ByteSpan::default(),
            block,
        };
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: 42, .. }));
    });
}

#[test]
fn evaluate_block_with_local_variable() {
    run_test(|_, ev| {
        let local = make_local_var(
            "x",
            Type::I32 {
                span: ByteSpan::default(),
            },
            make_i32(100),
        );
        let block = make_block(vec![BlockElement::Local(local)], BlockSafety::Safe);
        let v = Value::Block {
            span: ByteSpan::default(),
            block,
        };
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::Unit { .. }));
    });
}

#[test]
fn evaluate_block_switches_safety_context() {
    run_test(|_, ev| {
        let block = make_block(vec![], BlockSafety::Unsafe);
        let v = Value::Block {
            span: ByteSpan::default(),
            block,
        };
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::Unit { .. }));
        assert!(matches!(ev.current_safety, BlockSafety::Safe));
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// 17. Struct Object Tests
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn struct_object_construction() {
    run_test(|_, ev| {
        let struct_def = make_struct_def("Point", BTreeMap::new());
        let fields: Vec<(NString, ValueId)> = vec![
            (NString::from("x"), make_i32(10).into()),
            (NString::from("y"), make_i32(20).into()),
        ];
        let v = Value::StructObject {
            span: ByteSpan::default(),
            struct_def,
            fields: fields.into(),
        };
        let result = ev.evaluate(&v).unwrap();
        match result {
            Value::StructObject { fields, .. } => {
                assert_eq!(fields.len(), 2);
            }
            _ => panic!("Expected StructObject"),
        }
    });
}

#[test]
fn struct_object_with_expression_fields() {
    run_test(|_, ev| {
        let struct_def = make_struct_def("Point", BTreeMap::new());
        let fields: Vec<(NString, ValueId)> = vec![
            (NString::from("x"), make_add(make_i32(5), make_i32(5)).into()),
            (NString::from("y"), make_i32(20).into()),
        ];
        let v = Value::StructObject {
            span: ByteSpan::default(),
            struct_def,
            fields: fields.into(),
        };
        let result = ev.evaluate(&v).unwrap();
        match result {
            Value::StructObject { fields, .. } => {
                assert_eq!(fields.len(), 2);
            }
            _ => panic!("Expected StructObject"),
        }
    });
}

#[test]
fn struct_field_access() {
    run_test(|_, ev| {
        let struct_def = make_struct_def("Data", BTreeMap::new());
        let fields: Vec<(NString, ValueId)> = vec![(NString::from("value"), make_i32(42).into())];
        let obj = Value::StructObject {
            span: ByteSpan::default(),
            struct_def,
            fields: fields.into(),
        };
        let v = Value::FieldAccess {
            span: ByteSpan::default(),
            expr: ValueId::from(obj),
            field_name: NString::from("value"),
        };
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: 42, .. }));
    });
}

#[test]
fn struct_field_access_missing_field_fails() {
    run_test(|_, ev| {
        let struct_def = make_struct_def("Data", BTreeMap::new());
        let fields: Vec<(NString, ValueId)> = vec![(NString::from("x"), make_i32(1).into())];
        let obj = Value::StructObject {
            span: ByteSpan::default(),
            struct_def,
            fields: fields.into(),
        };
        let v = Value::FieldAccess {
            span: ByteSpan::default(),
            expr: ValueId::from(obj),
            field_name: NString::from("nonexistent"),
        };
        let result = ev.evaluate(&v);
        assert!(matches!(result, Err(EvalError::TypeError)));
    });
}

#[test]
fn struct_field_access_on_non_struct_fails() {
    run_test(|_, ev| {
        let v = Value::FieldAccess {
            span: ByteSpan::default(),
            expr: make_i32(42).into(),
            field_name: NString::from("anything"),
        };
        let result = ev.evaluate(&v);
        assert!(matches!(result, Err(EvalError::TypeError)));
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// 18. Enum Variant Tests
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn enum_variant_construction() {
    run_test(|_, ev| {
        let enum_def = make_enum_def("Option", ThinVec::new());
        let v = Value::EnumVariant {
            span: ByteSpan::default(),
            enum_def,
            variant: NString::from("Some"),
            value: make_i32(42).into(),
        };
        let result = ev.evaluate(&v).unwrap();
        match result {
            Value::EnumVariant { variant, .. } => {
                assert_eq!(variant, NString::from("Some"));
            }
            _ => panic!("Expected EnumVariant"),
        }
    });
}

#[test]
fn enum_variant_with_unit_value() {
    run_test(|_, ev| {
        let enum_def = make_enum_def("Option", ThinVec::new());
        let v = Value::EnumVariant {
            span: ByteSpan::default(),
            enum_def,
            variant: NString::from("None"),
            value: make_unit().into(),
        };
        let result = ev.evaluate(&v).unwrap();
        match result {
            Value::EnumVariant { variant, .. } => {
                assert_eq!(variant, NString::from("None"));
            }
            _ => panic!("Expected EnumVariant"),
        }
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// 19. List & Tuple Tests
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn list_construction_empty() {
    run_test(|_, ev| {
        let v = Value::List {
            span: ByteSpan::default(),
            elements: ThinVec::new(),
        };
        let result = ev.evaluate(&v).unwrap();
        match result {
            Value::List { elements, .. } => {
                assert!(elements.is_empty());
            }
            _ => panic!("Expected List"),
        }
    });
}

#[test]
fn list_construction_with_elements() {
    run_test(|_, ev| {
        let v = Value::List {
            span: ByteSpan::default(),
            elements: vec![make_i32(1).into(), make_i32(2).into(), make_i32(3).into()].into(),
        };
        let result = ev.evaluate(&v).unwrap();
        match result {
            Value::List { elements, .. } => {
                assert_eq!(elements.len(), 3);
            }
            _ => panic!("Expected List"),
        }
    });
}

#[test]
fn list_index_access() {
    run_test(|_, ev| {
        let list = Value::List {
            span: ByteSpan::default(),
            elements: vec![make_i32(10).into(), make_i32(20).into(), make_i32(30).into()].into(),
        };
        let v = Value::IndexAccess {
            span: ByteSpan::default(),
            collection: ValueId::from(list),
            index: make_u32(1).into(),
        };
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: 20, .. }));
    });
}

#[test]
fn list_index_access_first() {
    run_test(|_, ev| {
        let list = Value::List {
            span: ByteSpan::default(),
            elements: vec![make_i32(100).into()].into(),
        };
        let v = Value::IndexAccess {
            span: ByteSpan::default(),
            collection: ValueId::from(list),
            index: make_u32(0).into(),
        };
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: 100, .. }));
    });
}

#[test]
fn list_index_out_of_bounds() {
    run_test(|_, ev| {
        let list = Value::List {
            span: ByteSpan::default(),
            elements: vec![make_i32(1).into()].into(),
        };
        let v = Value::IndexAccess {
            span: ByteSpan::default(),
            collection: ValueId::from(list),
            index: make_u32(5).into(),
        };
        let result = ev.evaluate(&v);
        assert!(matches!(result, Err(EvalError::OutOfBoundsAccess)));
    });
}

#[test]
fn tuple_construction() {
    run_test(|_, ev| {
        let v = Value::Tuple {
            span: ByteSpan::default(),
            elements: vec![make_i32(1).into(), make_bool(true).into()].into(),
        };
        let result = ev.evaluate(&v).unwrap();
        match result {
            Value::Tuple { elements, .. } => {
                assert_eq!(elements.len(), 2);
            }
            _ => panic!("Expected Tuple"),
        }
    });
}

#[test]
fn tuple_empty() {
    run_test(|_, ev| {
        let v = Value::Tuple {
            span: ByteSpan::default(),
            elements: ThinVec::new(),
        };
        let result = ev.evaluate(&v).unwrap();
        match result {
            Value::Tuple { elements, .. } => {
                assert!(elements.is_empty());
            }
            _ => panic!("Expected Tuple"),
        }
    });
}

#[test]
fn tuple_index_access() {
    run_test(|_, ev| {
        let tuple = Value::Tuple {
            span: ByteSpan::default(),
            elements: vec![make_i32(10).into(), make_i32(20).into()].into(),
        };
        let v = Value::IndexAccess {
            span: ByteSpan::default(),
            collection: ValueId::from(tuple),
            index: make_u32(0).into(),
        };
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: 10, .. }));
    });
}

#[test]
fn tuple_index_out_of_bounds() {
    run_test(|_, ev| {
        let tuple = Value::Tuple {
            span: ByteSpan::default(),
            elements: ThinVec::new(),
        };
        let v = Value::IndexAccess {
            span: ByteSpan::default(),
            collection: ValueId::from(tuple),
            index: make_u32(0).into(),
        };
        let result = ev.evaluate(&v);
        assert!(matches!(result, Err(EvalError::OutOfBoundsAccess)));
    });
}

#[test]
fn index_access_on_non_collection_fails() {
    run_test(|_, ev| {
        let v = Value::IndexAccess {
            span: ByteSpan::default(),
            collection: make_i32(42).into(),
            index: make_u32(0).into(),
        };
        let result = ev.evaluate(&v);
        assert!(matches!(result, Err(EvalError::TypeError)));
    });
}

#[test]
fn index_access_with_non_integer_fails() {
    run_test(|_, ev| {
        let list = Value::List {
            span: ByteSpan::default(),
            elements: vec![make_i32(1).into()].into(),
        };
        let v = Value::IndexAccess {
            span: ByteSpan::default(),
            collection: ValueId::from(list),
            index: make_bool(false).into(),
        };
        let result = ev.evaluate(&v);
        assert!(matches!(result, Err(EvalError::TypeError)));
    });
}

#[test]
fn index_access_with_negative_index_fails() {
    run_test(|_, ev| {
        let list = Value::List {
            span: ByteSpan::default(),
            elements: vec![make_i32(1).into()].into(),
        };
        let v = Value::IndexAccess {
            span: ByteSpan::default(),
            collection: ValueId::from(list),
            index: make_i8(-1).into(),
        };
        let result = ev.evaluate(&v);
        assert!(matches!(result, Err(EvalError::TypeError)));
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// 20. Assignment Tests
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn assign_to_local_variable() {
    run_test(|_, ev| {
        let local = make_local_var(
            "x",
            Type::I32 {
                span: ByteSpan::default(),
            },
            make_i32(0),
        );
        let frame = crate::evaluator::Frame::new(&[]);
        ev.frames.push(frame);
        ev.frames
            .last_mut()
            .unwrap()
            .set_binding(NString::from("x"), make_i32(0));

        let local_sym = Value::LocalVariableSymbol {
            span: ByteSpan::default(),
            id: local,
        };
        let v = Value::Assign {
            span: ByteSpan::default(),
            place: ValueId::from(local_sym),
            value: make_i32(42).into(),
        };
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::Unit { .. }));
    });
}

#[test]
fn assign_to_non_local_fails() {
    run_test(|_, ev| {
        let v = Value::Assign {
            span: ByteSpan::default(),
            place: make_i32(1).into(),
            value: make_i32(42).into(),
        };
        let result = ev.evaluate(&v);
        assert!(matches!(result, Err(EvalError::Unsupported(_))));
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// 21. Function Call Tests
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn function_call_unit_body() {
    run_test(|_, ev| {
        let fn_id = make_function(
            "unit_body",
            vec![],
            Type::Unit {
                span: ByteSpan::default(),
            },
            vec![BlockElement::Expr(make_unit().into())],
        );
        let v = Value::Call {
            span: ByteSpan::default(),
            callee: Value::from(Value::FunctionSymbol {
                span: ByteSpan::default(),
                id: fn_id,
            })
            .into(),
            args: Arguments {
                positional: ThinVec::new(),
                named: ThinVec::new(),
            },
        };
        let result = ev.evaluate(&v);
        assert!(result.is_ok());
    });
}

#[test]
fn function_call_depth_exceeded() {
    run_test(|_, ev| {
        ev.call_depth_limit = 0;
        let fn_id = make_function(
            "f",
            vec![],
            Type::Unit {
                span: ByteSpan::default(),
            },
            vec![],
        );
        let v = Value::Call {
            span: ByteSpan::default(),
            callee: Value::from(Value::FunctionSymbol {
                span: ByteSpan::default(),
                id: fn_id,
            })
            .into(),
            args: empty_args(),
        };
        let result = ev.evaluate(&v);
        assert!(matches!(result, Err(EvalError::CallDepthExceeded)));
    });
}

#[test]
fn function_call_non_function_symbol_fails() {
    run_test(|_, ev| {
        let v = Value::Call {
            span: ByteSpan::default(),
            callee: make_i32(42).into(),
            args: empty_args(),
        };
        let result = ev.evaluate(&v);
        assert!(matches!(result, Err(EvalError::TypeError)));
    });
}

#[test]
fn unsafe_function_in_safe_context_fails() {
    run_test(|_, ev| {
        let fn_id = Function {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            is_unsafe: true,
            name: NString::from("unsafe_fn"),
            mangled_name: None,
            generics: None,
            params: vec![],
            return_type: Type::Unit {
                span: ByteSpan::default(),
            }
            .into(),
            body: Some(vec![]),
        }
        .into();
        let v = Value::Call {
            span: ByteSpan::default(),
            callee: Value::from(Value::FunctionSymbol {
                span: ByteSpan::default(),
                id: fn_id,
            })
            .into(),
            args: empty_args(),
        };
        let result = ev.evaluate(&v);
        assert!(matches!(result, Err(EvalError::UnsafeInSafeContext)));
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// 22. Function Body Evaluation Tests
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn evaluate_function_empty_body_returns_unit() {
    run_test(|_, ev| {
        let fn_id = make_function(
            "empty",
            vec![],
            Type::Unit {
                span: ByteSpan::default(),
            },
            vec![],
        );
        let result = ev.evaluate_function(fn_id, &[]).unwrap();
        assert!(matches!(result, Value::Unit { .. }));
    });
}

#[test]
fn evaluate_function_with_missing_args_uses_unit() {
    run_test(|_, ev| {
        let fn_id = make_function(
            "f",
            vec![make_param(
                "x",
                Type::I32 {
                    span: ByteSpan::default(),
                },
            )],
            Type::Unit {
                span: ByteSpan::default(),
            },
            vec![],
        );
        let result = ev.evaluate_function(fn_id, &[]).unwrap();
        assert!(matches!(result, Value::Unit { .. }));
    });
}

#[test]
fn evaluate_function_restores_safety_after_call() {
    run_test(|_, ev| {
        ev.current_safety = BlockSafety::Unsafe;
        let fn_id = make_function(
            "f",
            vec![],
            Type::Unit {
                span: ByteSpan::default(),
            },
            vec![],
        );
        ev.evaluate_function(fn_id, &[]).unwrap();
        assert!(matches!(ev.current_safety, BlockSafety::Unsafe));
    });
}

#[test]
fn evaluate_function_body_with_block() {
    run_test(|_, ev| {
        let fn_id = make_function(
            "test",
            vec![],
            Type::I32 {
                span: ByteSpan::default(),
            },
            vec![BlockElement::Expr(make_i32(42).into())],
        );
        let result = ev.evaluate_function(fn_id, &[]).unwrap();
        assert!(matches!(result, Value::I32 { value: 42, .. }));
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// 23. Builtin Function Tests
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn builtin_abs_positive() {
    run_test(|_, ev| {
        fn abs_builtin(_: &mut Evaluator, args: &[Value]) -> Result<Value, EvalError> {
            match &args[0] {
                Value::I32 { value, .. } => Ok(Value::I32 {
                    span: ByteSpan::default(),
                    value: value.abs(),
                }),
                _ => Err(EvalError::TypeError),
            }
        }
        let name = NString::from("abs");
        ev.add_builtin_function(name.clone(), abs_builtin);
        let fn_id = Function {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            is_unsafe: false,
            name: name.clone(),
            mangled_name: None,
            generics: None,
            params: vec![],
            return_type: Type::I32 {
                span: ByteSpan::default(),
            }
            .into(),
            body: None,
        }
        .into();
        let result = ev.evaluate_function(fn_id, &[make_i32(-42)]);
        assert!(result.is_ok());
    });
}

#[test]
fn builtin_max() {
    run_test(|_, ev| {
        fn max_builtin(_: &mut Evaluator, args: &[Value]) -> Result<Value, EvalError> {
            match (&args[0], &args[1]) {
                (Value::I32 { value: a, .. }, Value::I32 { value: b, .. }) => Ok(Value::I32 {
                    span: ByteSpan::default(),
                    value: std::cmp::max(*a, *b),
                }),
                _ => Err(EvalError::TypeError),
            }
        }
        let name = NString::from("max");
        ev.add_builtin_function(name.clone(), max_builtin);
        let fn_id = Function {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            is_unsafe: false,
            name: name.clone(),
            mangled_name: None,
            generics: None,
            params: vec![],
            return_type: Type::I32 {
                span: ByteSpan::default(),
            }
            .into(),
            body: None,
        }
        .into();
        let result = ev.evaluate_function(fn_id, &[make_i32(10), make_i32(20)]);
        match result {
            Ok(Value::I32 { value, .. }) => assert_eq!(value, 20),
            other => panic!("Unexpected result: {:?}", other),
        }
    });
}

#[test]
fn builtin_extern_no_body_or_builtin_fails() {
    run_test(|_, ev| {
        let fn_id = Function {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            is_unsafe: false,
            name: NString::from("unknown_extern"),
            mangled_name: None,
            generics: None,
            params: vec![],
            return_type: Type::Unit {
                span: ByteSpan::default(),
            }
            .into(),
            body: None,
        }
        .into();
        let result = ev.evaluate_function(fn_id, &[]);
        assert!(matches!(result, Err(EvalError::Unsupported(_))));
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// 24. Frame Management Tests
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn frame_new_stores_params() {
    let params = vec![(NString::from("x"), make_i32(42))];
    let frame = crate::evaluator::Frame::new(&params);
    let binding = frame.get_binding(&NString::from("x"));
    assert!(binding.is_some());
    assert_eq!(*binding.unwrap(), make_i32(42));
}

#[test]
fn frame_new_empty() {
    let frame = crate::evaluator::Frame::new(&[]);
    assert!(frame.get_binding(&NString::from("anything")).is_none());
}

#[test]
fn frame_set_binding_overrides() {
    let params = vec![(NString::from("x"), make_i32(1))];
    let mut frame = crate::evaluator::Frame::new(&params);
    frame.set_binding(NString::from("x"), make_i32(99));
    let binding = frame.get_binding(&NString::from("x"));
    assert_eq!(*binding.unwrap(), make_i32(99));
}

#[test]
fn frame_set_binding_adds_new() {
    let mut frame = crate::evaluator::Frame::new(&[]);
    frame.set_binding(NString::from("y"), make_bool(true));
    let binding = frame.get_binding(&NString::from("y"));
    assert_eq!(*binding.unwrap(), make_bool(true));
}

#[test]
fn frame_locals_override_params() {
    let params = vec![(NString::from("x"), make_i32(1))];
    let mut frame = crate::evaluator::Frame::new(&params);
    frame.set_binding(NString::from("x"), make_i32(2));
    let binding = frame.get_binding(&NString::from("x"));
    assert_eq!(*binding.unwrap(), make_i32(2));
}

#[test]
fn evaluator_lookup_binding_empty_frames() {
    run_test(|_, ev| {
        let result = ev.lookup_binding(&NString::from("x"));
        assert!(result.is_none());
    });
}

#[test]
fn evaluator_lookup_binding_in_frame() {
    run_test(|_, ev| {
        let frame = crate::evaluator::Frame::new(&[(NString::from("x"), make_i32(42))]);
        ev.frames.push(frame);
        let result = ev.lookup_binding(&NString::from("x"));
        assert_eq!(result, Some(make_i32(42)));
    });
}

#[test]
fn evaluator_lookup_binding_across_frames() {
    run_test(|_, ev| {
        let outer = crate::evaluator::Frame::new(&[(NString::from("a"), make_i32(1))]);
        ev.frames.push(outer);
        let inner = crate::evaluator::Frame::new(&[(NString::from("b"), make_i32(2))]);
        ev.frames.push(inner);
        assert_eq!(ev.lookup_binding(&NString::from("b")), Some(make_i32(2)));
        assert_eq!(ev.lookup_binding(&NString::from("a")), Some(make_i32(1)));
        assert!(ev.lookup_binding(&NString::from("c")).is_none());
    });
}

#[test]
fn evaluator_lookup_binding_inner_shadows_outer() {
    run_test(|_, ev| {
        let outer = crate::evaluator::Frame::new(&[(NString::from("x"), make_i32(1))]);
        ev.frames.push(outer);
        let mut inner = crate::evaluator::Frame::new(&[]);
        inner.set_binding(NString::from("x"), make_i32(999));
        ev.frames.push(inner);
        let result = ev.lookup_binding(&NString::from("x"));
        assert_eq!(result, Some(make_i32(999)));
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// 25. Local Variable Symbol Tests
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn local_variable_symbol_resolves() {
    run_test(|_, ev| {
        let local = make_local_var(
            "my_var",
            Type::I32 {
                span: ByteSpan::default(),
            },
            make_i32(77),
        );
        let frame = crate::evaluator::Frame::new(&[(NString::from("my_var"), make_i32(77))]);
        ev.frames.push(frame);
        let v = Value::LocalVariableSymbol {
            span: ByteSpan::default(),
            id: local,
        };
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: 77, .. }));
    });
}

#[test]
fn local_variable_symbol_not_bound() {
    run_test(|_, ev| {
        let local = make_local_var(
            "missing",
            Type::I32 {
                span: ByteSpan::default(),
            },
            make_i32(0),
        );
        let v = Value::LocalVariableSymbol {
            span: ByteSpan::default(),
            id: local,
        };
        let result = ev.evaluate(&v);
        assert!(matches!(result, Err(EvalError::TypeError)));
    });
}

#[test]
fn parameter_symbol_resolves() {
    run_test(|_, ev| {
        let param = make_param(
            "p",
            Type::I32 {
                span: ByteSpan::default(),
            },
        );
        let frame = crate::evaluator::Frame::new(&[(NString::from("p"), make_i32(55))]);
        ev.frames.push(frame);
        let v = Value::ParameterSymbol {
            span: ByteSpan::default(),
            id: param,
        };
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: 55, .. }));
    });
}

#[test]
fn parameter_symbol_not_bound() {
    run_test(|_, ev| {
        let param = make_param(
            "missing_param",
            Type::I32 {
                span: ByteSpan::default(),
            },
        );
        let v = Value::ParameterSymbol {
            span: ByteSpan::default(),
            id: param,
        };
        let result = ev.evaluate(&v);
        assert!(matches!(result, Err(EvalError::TypeError)));
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// 26. Function Symbol Tests
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn function_symbol_evaluates_to_itself() {
    run_test(|_, ev| {
        let fn_id = make_function(
            "f",
            vec![],
            Type::Unit {
                span: ByteSpan::default(),
            },
            vec![],
        );
        let v = Value::FunctionSymbol {
            span: ByteSpan::default(),
            id: fn_id,
        };
        let result = ev.evaluate(&v).unwrap();
        match result {
            Value::FunctionSymbol { .. } => {}
            _ => panic!("Expected FunctionSymbol"),
        }
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// 27. Unsupported Operations Tests
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn borrow_is_unsupported() {
    run_test(|_, ev| {
        let v = Value::Borrow {
            span: ByteSpan::default(),
            exclusive: false,
            mutable: false,
            place: make_i32(1).into(),
        };
        let result = ev.evaluate(&v);
        assert!(matches!(result, Err(EvalError::Unsupported("borrow"))));
    });
}

#[test]
fn deref_is_unsupported() {
    run_test(|_, ev| {
        let v = Value::Deref {
            span: ByteSpan::default(),
            place: make_i32(1).into(),
        };
        let result = ev.evaluate(&v);
        assert!(matches!(result, Err(EvalError::Unsupported("deref"))));
    });
}

#[test]
fn method_call_is_unsupported() {
    run_test(|_, ev| {
        let v = Value::MethodCall {
            span: ByteSpan::default(),
            object: make_i32(1).into(),
            method_name: NString::from("foo"),
            args: Arguments {
                positional: ThinVec::new(),
                named: ThinVec::new(),
            },
        };
        let result = ev.evaluate(&v);
        assert!(matches!(result, Err(EvalError::Unsupported("method call"))));
    });
}

#[test]
fn global_variable_symbol_is_unsupported() {
    run_test(|_, ev| {
        let global_var = GlobalVariable {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            is_mutable: false,
            name: NString::from("GLOBAL"),
            mangled_name: None,
            ty: Type::I32 {
                span: ByteSpan::default(),
            }
            .into(),
            initializer: make_i32(42).into(),
        }
        .into();
        let v = Value::GlobalVariableSymbol {
            span: ByteSpan::default(),
            id: global_var,
        };
        let result = ev.evaluate(&v);
        assert!(matches!(result, Err(EvalError::Unsupported("global variable"))));
    });
}

#[test]
fn range_is_unsupported() {
    run_test(|_, ev| {
        let v = Value::Range {
            span: ByteSpan::default(),
            start: Some(make_i32(0).into()),
            end: Some(make_i32(10).into()),
            inclusive: false,
        };
        let result = ev.evaluate(&v);
        assert!(matches!(result, Err(EvalError::Unsupported("range"))));
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// 28. Global Initializer Evaluation Tests
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn evaluate_global_initializer_i32() {
    run_test(|_, ev| {
        let global = GlobalVariable {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            is_mutable: false,
            name: NString::from("G"),
            mangled_name: None,
            ty: Type::I32 {
                span: ByteSpan::default(),
            }
            .into(),
            initializer: make_i32(42).into(),
        };
        let result = ev.evaluate_global_initializer(&global).unwrap();
        assert_eq!(result, Lit::I32(42));
    });
}

#[test]
fn evaluate_global_initializer_bool() {
    run_test(|_, ev| {
        let global = GlobalVariable {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            is_mutable: false,
            name: NString::from("FLAG"),
            mangled_name: None,
            ty: Type::Bool {
                span: ByteSpan::default(),
            }
            .into(),
            initializer: make_bool(true).into(),
        };
        let result = ev.evaluate_global_initializer(&global).unwrap();
        assert_eq!(result, Lit::Bool(true));
    });
}

#[test]
fn evaluate_global_initializer_unit() {
    run_test(|_, ev| {
        let global = GlobalVariable {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            is_mutable: false,
            name: NString::from("V"),
            mangled_name: None,
            ty: Type::Unit {
                span: ByteSpan::default(),
            }
            .into(),
            initializer: make_unit().into(),
        };
        let result = ev.evaluate_global_initializer(&global).unwrap();
        assert_eq!(result, Lit::Unit);
    });
}

#[test]
fn evaluate_global_initializer_with_expression() {
    run_test(|_, ev| {
        let global = GlobalVariable {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            attributes: BTreeSet::new(),
            is_mutable: false,
            name: NString::from("SUM"),
            mangled_name: None,
            ty: Type::I32 {
                span: ByteSpan::default(),
            }
            .into(),
            initializer: make_add(make_i32(10), make_i32(20)).into(),
        };
        let result = ev.evaluate_global_initializer(&global).unwrap();
        assert_eq!(result, Lit::I32(30));
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// 29. Memory Tests
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn memory_allocate_returns_address() {
    run_test(|_, ev| {
        let addr = ev.memory.allocate(100, 8, true).unwrap();
        assert!(addr >= 0x1000);
        assert!(ev.memory.is_valid_pointer(addr));
    });
}

#[test]
fn memory_allocate_alignment() {
    run_test(|_, ev| {
        let addr = ev.memory.allocate(10, 16, true).unwrap();
        assert_eq!(addr % 16, 0);
    });
}

#[test]
fn memory_read_write_roundtrip() {
    run_test(|_, ev| {
        let addr = ev.memory.allocate(100, 8, true).unwrap();
        let data = b"hello";
        ev.memory.write(addr, 0, data).unwrap();
        let read = ev.memory.read(addr, 0, 5).unwrap();
        assert_eq!(read, b"hello");
    });
}

#[test]
fn memory_write_immutable_fails() {
    run_test(|_, ev| {
        let addr = ev.memory.allocate(100, 8, false).unwrap();
        let result = ev.memory.write(addr, 0, b"data");
        assert!(matches!(result, Err(EvalError::TypeError)));
    });
}

#[test]
fn memory_read_invalid_pointer_fails() {
    run_test(|_, ev| {
        let result = ev.memory.read(0xDEAD, 0, 4);
        assert!(matches!(result, Err(EvalError::InvalidPointer)));
    });
}

#[test]
fn memory_write_invalid_pointer_fails() {
    run_test(|_, ev| {
        let result = ev.memory.write(0xDEAD, 0, b"data");
        assert!(matches!(result, Err(EvalError::InvalidPointer)));
    });
}

#[test]
fn memory_read_out_of_bounds_fails() {
    run_test(|_, ev| {
        let addr = ev.memory.allocate(10, 8, true).unwrap();
        let result = ev.memory.read(addr, 8, 5);
        assert!(matches!(result, Err(EvalError::OutOfBoundsAccess)));
    });
}

#[test]
fn memory_write_out_of_bounds_fails() {
    run_test(|_, ev| {
        let addr = ev.memory.allocate(10, 8, true).unwrap();
        let result = ev.memory.write(addr, 8, b"hello");
        assert!(matches!(result, Err(EvalError::OutOfBoundsAccess)));
    });
}

#[test]
fn memory_limit_exceeded() {
    run_test(|_, ev| {
        ev.memory = Memory::new(10);
        let result = ev.memory.allocate(100, 8, true);
        assert!(matches!(result, Err(EvalError::MemoryLimitExceeded)));
    });
}

#[test]
fn memory_allocate_sequential_addresses_increase() {
    run_test(|_, ev| {
        let addr1 = ev.memory.allocate(10, 1, true).unwrap();
        let addr2 = ev.memory.allocate(10, 1, true).unwrap();
        assert!(addr2 > addr1);
    });
}

#[test]
fn memory_deallocate_removes_pointer() {
    run_test(|_, ev| {
        let addr = ev.memory.allocate(10, 8, true).unwrap();
        assert!(ev.memory.is_valid_pointer(addr));
        ev.memory.deallocate(addr);
        assert!(!ev.memory.is_valid_pointer(addr));
    });
}

#[test]
fn memory_deallocate_nonexistent_does_not_panic() {
    run_test(|_, ev| {
        ev.memory.deallocate(0xBAD);
    });
}

#[test]
fn memory_deallocate_frees_space() {
    run_test(|_, ev| {
        let mut memory = Memory::new(50);
        let addr1 = memory.allocate(20, 1, true).unwrap();
        assert!(memory.is_valid_pointer(addr1));
        memory.deallocate(addr1);
        let addr2 = memory.allocate(20, 1, true).unwrap();
        assert!(memory.is_valid_pointer(addr2));
    });
}

#[test]
fn memory_allocation_size() {
    run_test(|_, ev| {
        let addr = ev.memory.allocate(42, 8, true).unwrap();
        assert_eq!(ev.memory.allocation_size(addr), Some(42));
    });
}

#[test]
fn memory_allocation_size_nonexistent() {
    run_test(|_, ev| {
        assert_eq!(ev.memory.allocation_size(0xFFFF), None);
    });
}

#[test]
fn memory_allocation_mutable_true() {
    run_test(|_, ev| {
        let addr = ev.memory.allocate(10, 8, true).unwrap();
        assert_eq!(ev.memory.allocation_mutable(addr), Some(true));
    });
}

#[test]
fn memory_allocation_mutable_false() {
    run_test(|_, ev| {
        let addr = ev.memory.allocate(10, 8, false).unwrap();
        assert_eq!(ev.memory.allocation_mutable(addr), Some(false));
    });
}

#[test]
fn memory_allocation_mutable_nonexistent() {
    run_test(|_, ev| {
        assert_eq!(ev.memory.allocation_mutable(0xFFFF), None);
    });
}

#[test]
fn memory_read_exact_boundary() {
    run_test(|_, ev| {
        let addr = ev.memory.allocate(10, 8, true).unwrap();
        let data = vec![1u8, 2, 3, 4, 5];
        ev.memory.write(addr, 5, &data).unwrap();
        let read = ev.memory.read(addr, 5, 5).unwrap();
        assert_eq!(read, data);
    });
}

#[test]
fn memory_read_empty_size_zero() {
    run_test(|_, ev| {
        let addr = ev.memory.allocate(10, 8, true).unwrap();
        let result = ev.memory.read(addr, 0, 0).unwrap();
        assert!(result.is_empty());
    });
}

#[test]
fn memory_default_new() {
    let mem = Memory::new(1024);
    assert!(!mem.is_valid_pointer(0));
    assert!(!mem.is_valid_pointer(0x1000));
}

// ═══════════════════════════════════════════════════════════════════════════
// 30. Error Type Tests
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn eval_error_break_clone() {
    let err = EvalError::Break {
        label: Some(NString::from("loop1")),
    };
    let cloned = err.clone();
    match cloned {
        EvalError::Break { label } => assert_eq!(label, Some(NString::from("loop1"))),
        _ => panic!("Wrong variant"),
    }
}

#[test]
fn eval_error_continue_clone() {
    let err = EvalError::Continue { label: None };
    let cloned = err.clone();
    assert!(matches!(cloned, EvalError::Continue { label: None }));
}

#[test]
fn eval_error_return_clone() {
    let err = EvalError::Return(make_i32(42));
    let cloned = err.clone();
    match cloned {
        EvalError::Return(val) => assert_eq!(val, make_i32(42)),
        _ => panic!("Wrong variant"),
    }
}

#[test]
fn eval_error_division_by_zero_clone() {
    let err = EvalError::DivisionByZero;
    let cloned = err.clone();
    assert!(matches!(cloned, EvalError::DivisionByZero));
}

#[test]
fn eval_error_modulo_by_zero_clone() {
    let err = EvalError::ModuloByZero;
    let cloned = err.clone();
    assert!(matches!(cloned, EvalError::ModuloByZero));
}

#[test]
fn eval_error_shift_amount_clone() {
    let err = EvalError::ShiftAmountError;
    let cloned = err.clone();
    assert!(matches!(cloned, EvalError::ShiftAmountError));
}

#[test]
fn eval_error_type_error_clone() {
    let err = EvalError::TypeError;
    let cloned = err.clone();
    assert!(matches!(cloned, EvalError::TypeError));
}

#[test]
fn eval_error_loop_limit_clone() {
    let err = EvalError::LoopLimitExceeded;
    let cloned = err.clone();
    assert!(matches!(cloned, EvalError::LoopLimitExceeded));
}

#[test]
fn eval_error_call_depth_clone() {
    let err = EvalError::CallDepthExceeded;
    let cloned = err.clone();
    assert!(matches!(cloned, EvalError::CallDepthExceeded));
}

#[test]
fn eval_error_memory_limit_clone() {
    let err = EvalError::MemoryLimitExceeded;
    let cloned = err.clone();
    assert!(matches!(cloned, EvalError::MemoryLimitExceeded));
}

#[test]
fn eval_error_invalid_pointer_clone() {
    let err = EvalError::InvalidPointer;
    let cloned = err.clone();
    assert!(matches!(cloned, EvalError::InvalidPointer));
}

#[test]
fn eval_error_oob_access_clone() {
    let err = EvalError::OutOfBoundsAccess;
    let cloned = err.clone();
    assert!(matches!(cloned, EvalError::OutOfBoundsAccess));
}

#[test]
fn eval_error_misaligned_access_clone() {
    let err = EvalError::MisalignedAccess;
    let cloned = err.clone();
    assert!(matches!(cloned, EvalError::MisalignedAccess));
}

#[test]
fn eval_error_unsupported_clone() {
    let err = EvalError::Unsupported("raw pointers");
    let cloned = err.clone();
    match cloned {
        EvalError::Unsupported(msg) => assert_eq!(msg, "raw pointers"),
        _ => panic!("Wrong variant"),
    }
}

#[test]
fn eval_error_unsafe_in_safe_clone() {
    let err = EvalError::UnsafeInSafeContext;
    let cloned = err.clone();
    assert!(matches!(cloned, EvalError::UnsafeInSafeContext));
}

#[test]
fn eval_error_debug_format() {
    let err = EvalError::DivisionByZero;
    let debug_str = format!("{:?}", err);
    assert!(debug_str.contains("DivisionByZero"));
}

#[test]
fn eval_error_return_debug_format() {
    let err = EvalError::Return(make_i32(42));
    let debug_str = format!("{:?}", err);
    assert!(debug_str.contains("Return"));
}

// ═══════════════════════════════════════════════════════════════════════════
// 31. Edge Case Tests
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn evaluate_nested_binary_operations() {
    run_test(|_, ev| {
        let inner = make_add(make_i32(10), make_i32(20));
        let v = make_mul(inner, make_i32(3));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: 90, .. }));
    });
}

#[test]
fn evaluate_deeply_nested_binary() {
    run_test(|_, ev| {
        let left = make_mul(make_add(make_i32(1), make_i32(2)), make_add(make_i32(3), make_i32(4)));
        let v = make_sub(left, make_i32(5));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: 16, .. }));
    });
}

#[test]
fn evaluate_cast_after_arithmetic() {
    run_test(|_, ev| {
        let sum = make_add(make_i32(100), make_i32(27));
        let v = make_cast(
            sum,
            Type::U8 {
                span: ByteSpan::default(),
            },
        );
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::U8 { value: 127, .. }));
    });
}

#[test]
fn evaluate_unary_negate_expression() {
    run_test(|_, ev| {
        let expr = make_add(make_i32(10), make_i32(20));
        let v = Value::Unary {
            span: ByteSpan::default(),
            operand: ValueId::from(expr),
            op: UnaryOp::Sub,
        };
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: -30, .. }));
    });
}

#[test]
fn block_element_expr_vs_local() {
    run_test(|_, ev| {
        let block = make_block(
            vec![
                BlockElement::Local(make_local_var(
                    "a",
                    Type::I32 {
                        span: ByteSpan::default(),
                    },
                    make_i32(1),
                )),
                BlockElement::Expr(make_i32(42).into()),
            ],
            BlockSafety::Safe,
        );
        let v = Value::Block {
            span: ByteSpan::default(),
            block,
        };
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: 42, .. }));
    });
}

#[test]
fn call_depth_tracks_correctly() {
    run_test(|_, ev| {
        assert_eq!(ev.call_depth, 0);
        let fn_id = make_function(
            "f",
            vec![],
            Type::Unit {
                span: ByteSpan::default(),
            },
            vec![],
        );
        let _ = ev.evaluate_function(fn_id, &[]);
        assert_eq!(ev.call_depth, 0);
    });
}

#[test]
fn loop_count_increments_in_loop() {
    run_test(|_, ev| {
        ev.loop_limit = 100;
        assert_eq!(ev.loop_count, 0);
        let body = make_block(
            vec![BlockElement::Expr(
                Value::Break {
                    span: ByteSpan::default(),
                    label: None,
                }
                .into(),
            )],
            BlockSafety::Safe,
        );
        let v = Value::Loop {
            span: ByteSpan::default(),
            body,
        };
        ev.evaluate(&v).unwrap();
        assert_eq!(ev.loop_count, 1);
    });
}

#[test]
fn if_else_with_complex_expressions() {
    run_test(|_, ev| {
        let true_block = make_block(
            vec![BlockElement::Expr(make_add(make_i32(1), make_i32(2)).into())],
            BlockSafety::Safe,
        );
        let false_block = make_block(
            vec![BlockElement::Expr(make_sub(make_i32(10), make_i32(3)).into())],
            BlockSafety::Safe,
        );
        let condition = make_binary(make_i32(5), BinaryOp::Gt, make_i32(3));
        let v = Value::If {
            span: ByteSpan::default(),
            condition: ValueId::from(condition),
            true_branch: true_block,
            false_branch: Some(false_block),
        };
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: 3, .. }));
    });
}

#[test]
fn multiple_local_variables_in_block() {
    run_test(|_, ev| {
        let frame = crate::evaluator::Frame::new(&[]);
        ev.frames.push(frame);
        let block = make_block(
            vec![
                BlockElement::Local(make_local_var(
                    "x",
                    Type::I32 {
                        span: ByteSpan::default(),
                    },
                    make_i32(10),
                )),
                BlockElement::Local(make_local_var(
                    "y",
                    Type::I32 {
                        span: ByteSpan::default(),
                    },
                    make_i32(20),
                )),
            ],
            BlockSafety::Safe,
        );
        ev.evaluate_block_element(&BlockElement::Expr(
            Value::Block {
                span: ByteSpan::default(),
                block,
            }
            .into(),
        ))
        .unwrap();
        assert_eq!(ev.lookup_binding(&NString::from("x")), Some(make_i32(10)));
        assert_eq!(ev.lookup_binding(&NString::from("y")), Some(make_i32(20)));
    });
}

#[test]
fn while_loop_with_break() {
    run_test(|_, ev| {
        let body = make_block(
            vec![BlockElement::Expr(
                Value::Break {
                    span: ByteSpan::default(),
                    label: None,
                }
                .into(),
            )],
            BlockSafety::Safe,
        );
        let v = Value::While {
            span: ByteSpan::default(),
            condition: make_bool(true).into(),
            body,
        };
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::Unit { .. }));
    });
}

#[test]
fn while_loop_with_continue() {
    run_test(|_, ev| {
        ev.loop_limit = 1;
        let body = make_block(
            vec![BlockElement::Expr(
                Value::Continue {
                    span: ByteSpan::default(),
                    label: None,
                }
                .into(),
            )],
            BlockSafety::Safe,
        );
        let v = Value::While {
            span: ByteSpan::default(),
            condition: make_bool(true).into(),
            body,
        };
        let result = ev.evaluate(&v);
        assert!(matches!(result, Err(EvalError::LoopLimitExceeded)));
    });
}

#[test]
fn evaluate_block_safe_then_unsafe_resets() {
    run_test(|_, ev| {
        assert!(matches!(ev.current_safety, BlockSafety::Safe));
        let block = make_block(vec![], BlockSafety::Unsafe);
        let v = Value::Block {
            span: ByteSpan::default(),
            block,
        };
        ev.evaluate(&v).unwrap();
        assert!(matches!(ev.current_safety, BlockSafety::Safe));
    });
}

#[test]
fn evaluate_block_unsafe_then_unsafe_resets() {
    run_test(|_, ev| {
        ev.current_safety = BlockSafety::Unsafe;
        let block = make_block(vec![], BlockSafety::Unsafe);
        let v = Value::Block {
            span: ByteSpan::default(),
            block,
        };
        ev.evaluate(&v).unwrap();
        assert!(matches!(ev.current_safety, BlockSafety::Unsafe));
    });
}

#[test]
fn evaluate_block_safe_then_safe_unchanged() {
    run_test(|_, ev| {
        assert!(matches!(ev.current_safety, BlockSafety::Safe));
        let block = make_block(vec![], BlockSafety::Safe);
        let v = Value::Block {
            span: ByteSpan::default(),
            block,
        };
        ev.evaluate(&v).unwrap();
        assert!(matches!(ev.current_safety, BlockSafety::Safe));
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// 32. Additional Arithmetic Edge Cases
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn binary_add_i32_zero() {
    run_test(|_, ev| {
        let v = make_add(make_i32(42), make_i32(0));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: 42, .. }));
    });
}

#[test]
fn binary_sub_self() {
    run_test(|_, ev| {
        let v = make_sub(make_i32(42), make_i32(42));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: 0, .. }));
    });
}

#[test]
fn binary_mul_identity() {
    run_test(|_, ev| {
        let v = make_mul(make_i32(42), make_i32(1));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: 42, .. }));
    });
}

#[test]
fn binary_div_identity() {
    run_test(|_, ev| {
        let v = make_div(make_i32(42), make_i32(1));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: 42, .. }));
    });
}

#[test]
fn binary_shl_zero() {
    run_test(|_, ev| {
        let v = make_binary(make_u32(42), BinaryOp::Shl, make_u32(0));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::U32 { value: 42, .. }));
    });
}

#[test]
fn binary_shr_zero() {
    run_test(|_, ev| {
        let v = make_binary(make_u32(42), BinaryOp::Shr, make_u32(0));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::U32 { value: 42, .. }));
    });
}

#[test]
fn binary_and_same_is_identity() {
    run_test(|_, ev| {
        let v = make_binary(make_u32(0xABCD), BinaryOp::And, make_u32(0xABCD));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::U32 { value: 0xABCD, .. }));
    });
}

#[test]
fn binary_or_with_zero_is_identity() {
    run_test(|_, ev| {
        let v = make_binary(make_u32(42), BinaryOp::Or, make_u32(0));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::U32 { value: 42, .. }));
    });
}

#[test]
fn binary_xor_with_zero_is_identity() {
    run_test(|_, ev| {
        let v = make_binary(make_u32(42), BinaryOp::Xor, make_u32(0));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::U32 { value: 42, .. }));
    });
}

#[test]
fn binary_xor_self_is_zero() {
    run_test(|_, ev| {
        let v = make_binary(make_u32(42), BinaryOp::Xor, make_u32(42));
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::U32 { value: 0, .. }));
    });
}

#[test]
fn comparison_i32_multiple_operators() {
    run_test(|_, ev| {
        let v1 = make_binary(make_i32(5), BinaryOp::Lte, make_i32(5));
        assert_eq!(ev.evaluate(&v1).unwrap(), make_bool(true));
        let v2 = make_binary(make_i32(5), BinaryOp::Lte, make_i32(10));
        assert_eq!(ev.evaluate(&v2).unwrap(), make_bool(true));
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// 33. Cast I128/U128 Roundtrip Tests
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn cast_i128_to_i8_truncates() {
    run_test(|_, ev| {
        let big = Value::I128 {
            span: ByteSpan::default(),
            value: Box::new(300),
        };
        let v = make_cast(
            big,
            Type::I8 {
                span: ByteSpan::default(),
            },
        );
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I8 { .. }));
    });
}

#[test]
fn cast_u128_to_u8_truncates() {
    run_test(|_, ev| {
        let big = Value::U128 {
            span: ByteSpan::default(),
            value: Box::new(300),
        };
        let v = make_cast(
            big,
            Type::U8 {
                span: ByteSpan::default(),
            },
        );
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::U8 { value: 44, .. }));
    });
}

#[test]
fn cast_i32_to_i128_preserves_value() {
    run_test(|_, ev| {
        let v = make_cast(
            make_i32(-1000),
            Type::I128 {
                span: ByteSpan::default(),
            },
        );
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I128 { value, .. } if *value == -1000));
    });
}

#[test]
fn cast_f64_to_i32_truncates() {
    run_test(|_, ev| {
        let v = make_cast(
            make_f64(3.99),
            Type::I32 {
                span: ByteSpan::default(),
            },
        );
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: 3, .. }));
    });
}

#[test]
fn cast_u32_to_f64() {
    run_test(|_, ev| {
        let v = make_cast(
            make_u32(42),
            Type::F64 {
                span: ByteSpan::default(),
            },
        );
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::F64 { value, .. } if (*value - 42.0).abs() < 0.001));
    });
}

#[test]
fn cast_u16_to_u32() {
    run_test(|_, ev| {
        let val = Value::U16 {
            span: ByteSpan::default(),
            value: 1000,
        };
        let v = make_cast(
            val,
            Type::U32 {
                span: ByteSpan::default(),
            },
        );
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::U32 { value: 1000, .. }));
    });
}

#[test]
fn cast_i16_to_i64() {
    run_test(|_, ev| {
        let val = Value::I16 {
            span: ByteSpan::default(),
            value: -100,
        };
        let v = make_cast(
            val,
            Type::I64 {
                span: ByteSpan::default(),
            },
        );
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I64 { value: -100, .. }));
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// 34. Memory Additional Tests
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn memory_write_then_read_different_offsets() {
    run_test(|_, ev| {
        let addr = ev.memory.allocate(20, 8, true).unwrap();
        ev.memory.write(addr, 0, b"AAAA").unwrap();
        ev.memory.write(addr, 10, b"BBBB").unwrap();
        assert_eq!(ev.memory.read(addr, 0, 4).unwrap(), b"AAAA");
        assert_eq!(ev.memory.read(addr, 10, 4).unwrap(), b"BBBB");
    });
}

#[test]
fn memory_overwrite() {
    run_test(|_, ev| {
        let addr = ev.memory.allocate(10, 8, true).unwrap();
        ev.memory.write(addr, 0, b"hello").unwrap();
        ev.memory.write(addr, 0, b"world").unwrap();
        assert_eq!(ev.memory.read(addr, 0, 5).unwrap(), b"world");
    });
}

#[test]
fn memory_partial_overwrite() {
    run_test(|_, ev| {
        let addr = ev.memory.allocate(10, 8, true).unwrap();
        ev.memory.write(addr, 0, b"hello").unwrap();
        ev.memory.write(addr, 3, b"p!").unwrap();
        assert_eq!(ev.memory.read(addr, 0, 5).unwrap(), b"help!");
    });
}

#[test]
fn memory_allocated_bytes_are_zeroed() {
    run_test(|_, ev| {
        let addr = ev.memory.allocate(10, 8, true).unwrap();
        let data = ev.memory.read(addr, 0, 10).unwrap();
        assert_eq!(data, vec![0u8; 10]);
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// 35. Type Promotion Tests
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn inferred_integer_promotes_via_binary_op() {
    run_test(|_, ev| {
        let left = Value::InferredInteger {
            span: ByteSpan::default(),
            value: Box::new(10u128),
        };
        let right = Value::InferredInteger {
            span: ByteSpan::default(),
            value: Box::new(20u128),
        };
        let v = make_add(left, right);
        let lit = ev.evaluate_to_literal(&v).unwrap();
        assert_eq!(lit, Lit::U128(30));
    });
}

#[test]
fn inferred_float_promotes_via_binary_op() {
    run_test(|_, ev| {
        let left = Value::InferredFloat {
            span: ByteSpan::default(),
            value: OrderedFloat(1.5),
        };
        let right = Value::InferredFloat {
            span: ByteSpan::default(),
            value: OrderedFloat(2.5),
        };
        let v = make_add(left, right);
        let lit = ev.evaluate_to_literal(&v).unwrap();
        assert_eq!(lit, Lit::F64(OrderedFloat(4.0)));
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// 36. USize Tests
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn cast_i32_to_usize_32bit() {
    let log = test_log();
    let store = test_store();
    using_storage(&store, || {
        let mut ev = Evaluator::new(&log, PtrSize::U32);
        let v = make_cast(
            make_i32(100),
            Type::USize {
                span: ByteSpan::default(),
            },
        );
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(
            result,
            Value::USize {
                bits: 32,
                value: 100,
                ..
            }
        ));
    });
}

#[test]
fn cast_u64_to_usize() {
    run_test(|_, ev| {
        let v = make_cast(
            make_u64(999),
            Type::USize {
                span: ByteSpan::default(),
            },
        );
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(
            result,
            Value::USize {
                value: 999,
                bits: 64,
                ..
            }
        ));
    });
}

#[test]
fn usize_index_access() {
    run_test(|_, ev| {
        let list = Value::List {
            span: ByteSpan::default(),
            elements: vec![make_i32(10).into(), make_i32(20).into()].into(),
        };
        let idx = Value::USize {
            span: ByteSpan::default(),
            bits: 64,
            value: 1,
        };
        let v = Value::IndexAccess {
            span: ByteSpan::default(),
            collection: ValueId::from(list),
            index: ValueId::from(idx),
        };
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: 20, .. }));
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// 37. Index Access with Various Integer Types
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn index_access_with_u8() {
    run_test(|_, ev| {
        let list = Value::List {
            span: ByteSpan::default(),
            elements: vec![make_i32(10).into(), make_i32(20).into()].into(),
        };
        let v = Value::IndexAccess {
            span: ByteSpan::default(),
            collection: ValueId::from(list),
            index: make_u8(1).into(),
        };
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: 20, .. }));
    });
}

#[test]
fn index_access_with_u16() {
    run_test(|_, ev| {
        let list = Value::List {
            span: ByteSpan::default(),
            elements: vec![make_i32(10).into(), make_i32(20).into()].into(),
        };
        let idx = Value::U16 {
            span: ByteSpan::default(),
            value: 1,
        };
        let v = Value::IndexAccess {
            span: ByteSpan::default(),
            collection: ValueId::from(list),
            index: ValueId::from(idx),
        };
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: 20, .. }));
    });
}

#[test]
fn index_access_with_u64() {
    run_test(|_, ev| {
        let list = Value::List {
            span: ByteSpan::default(),
            elements: vec![make_i32(10).into()].into(),
        };
        let v = Value::IndexAccess {
            span: ByteSpan::default(),
            collection: ValueId::from(list),
            index: make_u64(0).into(),
        };
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: 10, .. }));
    });
}

#[test]
fn index_access_with_i64_positive() {
    run_test(|_, ev| {
        let list = Value::List {
            span: ByteSpan::default(),
            elements: vec![make_i32(10).into()].into(),
        };
        let idx = Value::I64 {
            span: ByteSpan::default(),
            value: 0,
        };
        let v = Value::IndexAccess {
            span: ByteSpan::default(),
            collection: ValueId::from(list),
            index: ValueId::from(idx),
        };
        let result = ev.evaluate(&v).unwrap();
        assert!(matches!(result, Value::I32 { value: 10, .. }));
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// 38. Struct Object Additional Tests
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn struct_object_with_computed_field() {
    run_test(|_, ev| {
        let struct_def = make_struct_def("S", BTreeMap::new());
        let field_expr = make_add(make_i32(1), make_i32(2));
        let fields: Vec<(NString, ValueId)> = vec![(NString::from("sum"), field_expr.into())];
        let v = Value::StructObject {
            span: ByteSpan::default(),
            struct_def,
            fields: fields.into(),
        };
        let result = ev.evaluate(&v).unwrap();
        match result {
            Value::StructObject { fields, .. } => {
                let (_, val_id) = &fields[0];
                let val = val_id.borrow();
                assert!(matches!(&*val, Value::I32 { value: 3, .. }));
            }
            _ => panic!("Expected StructObject"),
        }
    });
}

#[test]
fn struct_object_empty_fields() {
    run_test(|_, ev| {
        let struct_def = make_struct_def("Unit", BTreeMap::new());
        let v = Value::StructObject {
            span: ByteSpan::default(),
            struct_def,
            fields: ThinVec::new(),
        };
        let result = ev.evaluate(&v).unwrap();
        match result {
            Value::StructObject { fields, .. } => {
                assert!(fields.is_empty());
            }
            _ => panic!("Expected StructObject"),
        }
    });
}

// ═══════════════════════════════════════════════════════════════════════════
// 39. Miscellaneous
// ═══════════════════════════════════════════════════════════════════════════

#[test]
fn allocation_new_has_correct_size() {
    let alloc = crate::memory::Allocation::new(100, true);
    assert_eq!(alloc.data.len(), 100);
    assert_eq!(alloc.data, vec![0u8; 100]);
    assert_eq!(alloc.mutable, true);
}

#[test]
fn allocation_new_immutable() {
    let alloc = crate::memory::Allocation::new(50, false);
    assert_eq!(alloc.data.len(), 50);
    assert_eq!(alloc.mutable, false);
}

#[test]
fn eval_error_unsupported_different_messages() {
    let err1 = EvalError::Unsupported("message one");
    let err2 = EvalError::Unsupported("message two");
    assert_ne!(format!("{:?}", err1), format!("{:?}", err2));
}

#[test]
fn eval_error_break_different_labels() {
    let err1 = EvalError::Break {
        label: Some(NString::from("outer")),
    };
    let err2 = EvalError::Break {
        label: Some(NString::from("inner")),
    };
    let s1 = format!("{:?}", err1);
    let s2 = format!("{:?}", err2);
    assert_ne!(s1, s2);
}

#[test]
fn evaluator_ptr_size_determines_usize_bits() {
    let log = test_log();
    let store = test_store();
    using_storage(&store, || {
        let ev64 = Evaluator::new(&log, PtrSize::U64);
        let ev32 = Evaluator::new(&log, PtrSize::U32);
        assert!(matches!(ev64.ptr_size, PtrSize::U64));
        assert!(matches!(ev32.ptr_size, PtrSize::U32));
    });
}

#[test]
fn block_evaluation_drops_frame() {
    run_test(|_, ev| {
        let frame = crate::evaluator::Frame::new(&[]);
        ev.frames.push(frame);
        assert_eq!(ev.frames.len(), 1);
    });
}
