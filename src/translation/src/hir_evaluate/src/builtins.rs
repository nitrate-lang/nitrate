use crate::error::EvalError;
use crate::evaluator::{BuiltinFn, Evaluator};
use nitrate_hir::prelude::*;
use nitrate_nstring::NString;
use nitrate_tree::ByteSpan;
use ordered_float::OrderedFloat;
use std::collections::HashMap;
use std::sync::LazyLock;

/// Default built-in functions available during evaluation.
///
/// These are lazily initialized and merged with user-registered builtins
/// at evaluation time.
pub static DEFAULT_BUILTIN_FUNCTIONS: LazyLock<HashMap<NString, BuiltinFn>> = LazyLock::new(|| {
    let mut m: HashMap<NString, BuiltinFn> = HashMap::new();

    // ── std::math::abs — absolute value ──
    m.insert(
        "std::math::abs".into(),
        (|_eval, args| {
            if args.len() != 1 {
                return Err(EvalError::TypeError);
            }
            match &args[0] {
                Value::I8 { value, .. } => Ok(Value::I8 {
                    span: ByteSpan::default(),
                    value: value.abs(),
                }),
                Value::I16 { value, .. } => Ok(Value::I16 {
                    span: ByteSpan::default(),
                    value: value.abs(),
                }),
                Value::I32 { value, .. } => Ok(Value::I32 {
                    span: ByteSpan::default(),
                    value: value.abs(),
                }),
                Value::I64 { value, .. } => Ok(Value::I64 {
                    span: ByteSpan::default(),
                    value: value.abs(),
                }),
                Value::I128 { value, .. } => Ok(Value::I128 {
                    span: ByteSpan::default(),
                    value: Box::new(value.abs()),
                }),
                Value::F32 { value, .. } => Ok(Value::F32 {
                    span: ByteSpan::default(),
                    value: OrderedFloat(value.abs()),
                }),
                Value::F64 { value, .. } => Ok(Value::F64 {
                    span: ByteSpan::default(),
                    value: OrderedFloat(value.abs()),
                }),
                _ => Err(EvalError::TypeError),
            }
        }) as BuiltinFn,
    );

    // ── std::math::max — maximum of two values ──
    m.insert(
        "std::math::max".into(),
        (|_eval, args| {
            if args.len() != 2 {
                return Err(EvalError::TypeError);
            }
            let left = Lit::try_from(args[0].clone()).map_err(|_| EvalError::TypeError)?;
            let right = Lit::try_from(args[1].clone()).map_err(|_| EvalError::TypeError)?;
            match left.lt(&right) {
                Ok(true) => Ok(Value::from(right)),
                Ok(false) => Ok(Value::from(left)),
                Err(_) => Err(EvalError::TypeError),
            }
        }) as BuiltinFn,
    );

    // ── std::math::min — minimum of two values ──
    m.insert(
        "std::math::min".into(),
        (|_eval, args| {
            if args.len() != 2 {
                return Err(EvalError::TypeError);
            }
            let left = Lit::try_from(args[0].clone()).map_err(|_| EvalError::TypeError)?;
            let right = Lit::try_from(args[1].clone()).map_err(|_| EvalError::TypeError)?;
            match left.lt(&right) {
                Ok(true) => Ok(Value::from(left)),
                Ok(false) => Ok(Value::from(right)),
                Err(_) => Err(EvalError::TypeError),
            }
        }) as BuiltinFn,
    );

    // ── std::mem::size_of — size of a type (returns usize) ──
    // NOTE: This is a stub — actual size_of needs type information from the type system.
    // For now it returns 0.
    m.insert(
        "std::mem::size_of".into(),
        (|eval, _args| {
            Ok(Value::USize {
                span: ByteSpan::default(),
                bits: match eval.ptr_size {
                    PtrSize::U32 => 32,
                    PtrSize::U64 => 64,
                },
                value: 0,
            })
        }) as BuiltinFn,
    );

    m
});
