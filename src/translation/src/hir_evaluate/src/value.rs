use crate::error::EvalError;
use crate::evaluator::Evaluator;
use nitrate_hir::prelude::*;
use nitrate_nstring::NString;
use nitrate_tree::SrcPos;
use ordered_float::OrderedFloat;
use std::ops::{Add, BitAnd, BitOr, BitXor, Deref, Div, Mul, Neg, Not, Rem, Shl, Shr, Sub};

/// Evaluate a HIR `Value` to a concrete value.
pub fn evaluate_value(evaluator: &mut Evaluator, value: &Value) -> Result<Value, EvalError> {
    match value {
        Value::Unit { .. }
        | Value::Bool { .. }
        | Value::I8 { .. }
        | Value::I16 { .. }
        | Value::I32 { .. }
        | Value::I64 { .. }
        | Value::I128 { .. }
        | Value::U8 { .. }
        | Value::U16 { .. }
        | Value::U32 { .. }
        | Value::U64 { .. }
        | Value::U128 { .. }
        | Value::F32 { .. }
        | Value::F64 { .. }
        | Value::USize { .. }
        | Value::StringLit { .. }
        | Value::BStringLit { .. }
        | Value::InferredInteger { .. }
        | Value::InferredFloat { .. } => Ok(value.clone()),

        Value::Binary { left, op, right, .. } => eval_binary(evaluator, left, op.clone(), right),
        Value::Unary { operand, op, .. } => eval_unary(evaluator, operand, op.clone()),
        Value::Cast {
            value: expr,
            target_type,
            ..
        } => eval_cast(evaluator, expr, target_type),

        Value::If {
            condition,
            true_branch,
            false_branch,
            ..
        } => eval_if(evaluator, condition, true_branch, false_branch),
        Value::While { condition, body, .. } => eval_while(evaluator, condition, body),
        Value::Loop { body, .. } => eval_infinite_loop(evaluator, body),

        Value::Break { label, .. } => Err(EvalError::Break { label: label.clone() }),
        Value::Continue { label, .. } => Err(EvalError::Continue { label: label.clone() }),
        Value::Return { value: expr, .. } => {
            let result = evaluator.evaluate(&expr.borrow())?;
            Err(EvalError::Return(result))
        }

        Value::Block { block, .. } => evaluator.evaluate_block(&block.borrow()),

        Value::StructObject { struct_def, fields, .. } => eval_struct_object(evaluator, struct_def, fields),
        Value::EnumVariant {
            enum_def,
            variant,
            value: inner,
            ..
        } => eval_enum_variant(evaluator, enum_def, variant, inner),
        Value::List { elements, .. } => eval_list(evaluator, elements),
        Value::Tuple { elements, .. } => eval_tuple(evaluator, elements),

        Value::FieldAccess { expr, field_name, .. } => eval_field_access(evaluator, expr, field_name),
        Value::IndexAccess { collection, index, .. } => eval_index_access(evaluator, collection, index),

        Value::Assign { place, value: rhs, .. } => {
            let rhs_val = evaluator.evaluate(&rhs.borrow())?;
            eval_assign(evaluator, place, rhs_val)
        }

        Value::Borrow { .. } => Err(EvalError::Unsupported("borrow")),
        Value::Deref { .. } => Err(EvalError::Unsupported("deref")),

        Value::Call { callee, args, .. } => eval_call(evaluator, callee, args),
        Value::MethodCall { .. } => Err(EvalError::Unsupported("method call")),

        Value::FunctionSymbol { .. } => Ok(value.clone()),

        Value::GlobalVariableSymbol { .. } => Err(EvalError::Unsupported("global variable")),
        Value::LocalVariableSymbol { id, .. } => {
            let local = id.borrow();
            evaluator.lookup_binding(&local.name).ok_or(EvalError::TypeError)
        }
        Value::ParameterSymbol { id, .. } => {
            let param = id.borrow();
            evaluator.lookup_binding(&param.name).ok_or(EvalError::TypeError)
        }

        Value::Range { .. } => Err(EvalError::Unsupported("range")),
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Helper: convert Value to Lit, normalizing all integer types to
// compatible forms so that mixed solved/unsolved function calls
// produce compatible Lit values regardless of function solve order.
// ═══════════════════════════════════════════════════════════════════════════

/// Extract the raw `u128` value from any integer-like `Value`, so we can
/// normalise it into a single canonical `Lit` that is add-compatible.
fn integer_value_u128(v: &Value) -> u128 {
    match v {
        Value::U8 { value, .. } => *value as u128,
        Value::U16 { value, .. } => *value as u128,
        Value::U32 { value, .. } => *value as u128,
        Value::U64 { value, .. } => *value as u128,
        Value::U128 { value, .. } => **value,
        Value::I8 { value, .. } => *value as u128,
        Value::I16 { value, .. } => *value as u128,
        Value::I32 { value, .. } => *value as u128,
        Value::I64 { value, .. } => *value as u128,
        Value::I128 { value, .. } => **value as u128,
        Value::USize { value, .. } => *value as u128,
        Value::Bool { value, .. } => *value as u128,
        Value::InferredInteger { value, .. } => **value,
        _ => 0,
    }
}

/// Normalize an integer value to a `Lit` variant that is compatible with
/// other normalized integer values.  Small values become `Lit::I32` so
/// that USize, InferredInteger, and other integer types can all be
/// combined (e.g. `foo() + bar()` used as an array size).
fn normalize_integer_to_lit(raw: u128) -> Lit {
    if let Ok(v) = i32::try_from(raw) {
        Lit::I32(v)
    } else if let Ok(v) = i64::try_from(raw) {
        Lit::I64(v)
    } else if let Ok(v) = u64::try_from(raw) {
        Lit::U64(v)
    } else {
        Lit::U128(raw)
    }
}

/// Try to convert a Value to a Lit.  Integer types are normalized so
/// that the evaluator can combine them regardless of which function
/// bodies have already been solved.
fn try_to_lit(value: Value) -> Result<Lit, EvalError> {
    match value {
        Value::InferredInteger { value: v, .. } => Ok(normalize_integer_to_lit(*v)),
        Value::InferredFloat { value: v, .. } => Ok(Lit::F64(v)),
        Value::Unit { .. } => Ok(Lit::Unit),
        Value::F32 { value: v, .. } => Ok(Lit::F32(v)),
        Value::F64 { value: v, .. } => Ok(Lit::F64(v)),
        // Normalize all integer types to I32 (or larger when needed)
        other if other.is_literal() => Ok(normalize_integer_to_lit(integer_value_u128(&other))),
        other => Lit::try_from(other).map_err(|_| EvalError::TypeError),
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Binary operations
// ═══════════════════════════════════════════════════════════════════════════

fn eval_binary(evaluator: &mut Evaluator, left: &ValueId, op: BinaryOp, right: &ValueId) -> Result<Value, EvalError> {
    match op {
        BinaryOp::LogicAnd => {
            let left_val = evaluator.evaluate(&left.borrow())?;
            match &left_val {
                Value::Bool { value: true, .. } => evaluator.evaluate(&right.borrow()),
                Value::Bool { value: false, .. } => Ok(Value::Bool {
                    span: SrcPos::default(),
                    value: false,
                }),
                _ => Err(EvalError::TypeError),
            }
        }
        BinaryOp::LogicOr => {
            let left_val = evaluator.evaluate(&left.borrow())?;
            match &left_val {
                Value::Bool { value: true, .. } => Ok(Value::Bool {
                    span: SrcPos::default(),
                    value: true,
                }),
                Value::Bool { value: false, .. } => evaluator.evaluate(&right.borrow()),
                _ => Err(EvalError::TypeError),
            }
        }
        _ => {
            let left_val = evaluator.evaluate(&left.borrow())?;
            let right_val = evaluator.evaluate(&right.borrow())?;

            let left_lit = try_to_lit(left_val)?;
            let right_lit = try_to_lit(right_val)?;

            match op {
                BinaryOp::Add => left_lit
                    .add(right_lit)
                    .map(Value::from)
                    .map_err(|_| EvalError::TypeError),
                BinaryOp::Sub => left_lit
                    .sub(right_lit)
                    .map(Value::from)
                    .map_err(|_| EvalError::TypeError),
                BinaryOp::Mul => left_lit
                    .mul(right_lit)
                    .map(Value::from)
                    .map_err(|_| EvalError::TypeError),
                BinaryOp::Div => left_lit.div(right_lit).map(Value::from).map_err(|e| match e {
                    LiteralDivError::DivisionByZero => EvalError::DivisionByZero,
                    LiteralDivError::TypeError => EvalError::TypeError,
                }),
                BinaryOp::Mod => left_lit.rem(right_lit).map(Value::from).map_err(|e| match e {
                    LiteralRemError::ModuloByZero => EvalError::ModuloByZero,
                    LiteralRemError::TypeError => EvalError::TypeError,
                }),
                BinaryOp::And => left_lit
                    .bitand(right_lit)
                    .map(Value::from)
                    .map_err(|_| EvalError::TypeError),
                BinaryOp::Or => left_lit
                    .bitor(right_lit)
                    .map(Value::from)
                    .map_err(|_| EvalError::TypeError),
                BinaryOp::Xor => left_lit
                    .bitxor(right_lit)
                    .map(Value::from)
                    .map_err(|_| EvalError::TypeError),
                BinaryOp::Shl => left_lit.shl(right_lit).map(Value::from).map_err(|e| match e {
                    LiteralShlError::ShiftAmountError => EvalError::ShiftAmountError,
                    LiteralShlError::TypeError => EvalError::TypeError,
                }),
                BinaryOp::Shr => left_lit.shr(right_lit).map(Value::from).map_err(|e| match e {
                    LiteralShrError::ShiftAmountError => EvalError::ShiftAmountError,
                    LiteralShrError::TypeError => EvalError::TypeError,
                }),
                BinaryOp::Rol => left_lit
                    .rotate_left(right_lit)
                    .map(Value::from)
                    .map_err(|_| EvalError::TypeError),
                BinaryOp::Ror => left_lit
                    .rotate_right(right_lit)
                    .map(Value::from)
                    .map_err(|_| EvalError::TypeError),
                BinaryOp::Lt => {
                    let val = left_lit.lt(&right_lit).map_err(|_| EvalError::TypeError)?;
                    Ok(Value::Bool {
                        span: SrcPos::default(),
                        value: val,
                    })
                }
                BinaryOp::Gt => {
                    let val = left_lit.gt(&right_lit).map_err(|_| EvalError::TypeError)?;
                    Ok(Value::Bool {
                        span: SrcPos::default(),
                        value: val,
                    })
                }
                BinaryOp::Lte => {
                    let val = left_lit.le(&right_lit).map_err(|_| EvalError::TypeError)?;
                    Ok(Value::Bool {
                        span: SrcPos::default(),
                        value: val,
                    })
                }
                BinaryOp::Gte => {
                    let val = left_lit.ge(&right_lit).map_err(|_| EvalError::TypeError)?;
                    Ok(Value::Bool {
                        span: SrcPos::default(),
                        value: val,
                    })
                }
                BinaryOp::Eq => {
                    let val = left_lit.eq(&right_lit).map_err(|_| EvalError::TypeError)?;
                    Ok(Value::Bool {
                        span: SrcPos::default(),
                        value: val,
                    })
                }
                BinaryOp::Ne => {
                    let val = left_lit.ne(&right_lit).map_err(|_| EvalError::TypeError)?;
                    Ok(Value::Bool {
                        span: SrcPos::default(),
                        value: val,
                    })
                }
                BinaryOp::LogicAnd | BinaryOp::LogicOr => unreachable!(),
            }
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Unary operations
// ═══════════════════════════════════════════════════════════════════════════

fn eval_unary(evaluator: &mut Evaluator, operand: &ValueId, op: UnaryOp) -> Result<Value, EvalError> {
    let operand_val = evaluator.evaluate(&operand.borrow())?;
    let operand_lit = try_to_lit(operand_val)?;

    match op {
        UnaryOp::Add => Ok(Value::from(operand_lit)),
        UnaryOp::Sub => operand_lit.neg().map(Value::from).map_err(|_| EvalError::TypeError),
        UnaryOp::Not => operand_lit.not().map(Value::from).map_err(|_| EvalError::TypeError),
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Type casts
// ═══════════════════════════════════════════════════════════════════════════

enum CastBridge {
    Unit,
    I128(i128),
    U128(u128),
    F128(OrderedFloat<f64>),
}

impl TryFrom<Value> for CastBridge {
    type Error = ();

    fn try_from(v: Value) -> Result<Self, Self::Error> {
        match v {
            Value::Unit { .. } => Ok(CastBridge::Unit),
            Value::Bool { value: b, .. } => Ok(CastBridge::U128(b as u128)),
            Value::I8 { value: i, .. } => Ok(CastBridge::I128(i as i128)),
            Value::I16 { value: i, .. } => Ok(CastBridge::I128(i as i128)),
            Value::I32 { value: i, .. } => Ok(CastBridge::I128(i as i128)),
            Value::I64 { value: i, .. } => Ok(CastBridge::I128(i as i128)),
            Value::I128 { value: i, .. } => Ok(CastBridge::I128(*i)),
            Value::U8 { value: u, .. } => Ok(CastBridge::U128(u as u128)),
            Value::U16 { value: u, .. } => Ok(CastBridge::U128(u as u128)),
            Value::U32 { value: u, .. } => Ok(CastBridge::U128(u as u128)),
            Value::U64 { value: u, .. } => Ok(CastBridge::U128(u as u128)),
            Value::U128 { value: u, .. } => Ok(CastBridge::U128(*u)),
            Value::F32 { value: f, .. } => Ok(CastBridge::F128(OrderedFloat(*f as f64))),
            Value::F64 { value: f, .. } => Ok(CastBridge::F128(f)),
            Value::USize { value: u, .. } => Ok(CastBridge::U128(u as u128)),
            Value::InferredInteger { value: u, .. } => Ok(CastBridge::U128(*u)),
            Value::InferredFloat { value: f, .. } => Ok(CastBridge::F128(f)),
            _ => Err(()),
        }
    }
}

impl CastBridge {
    fn to_u8(self) -> Result<Value, EvalError> {
        let v = match self {
            CastBridge::I128(i) => i as u8,
            CastBridge::U128(u) => u as u8,
            CastBridge::F128(f) => *f as u8,
            _ => return Err(EvalError::TypeError),
        };
        Ok(Value::U8 {
            span: SrcPos::default(),
            value: v,
        })
    }
    fn to_u16(self) -> Result<Value, EvalError> {
        let v = match self {
            CastBridge::I128(i) => i as u16,
            CastBridge::U128(u) => u as u16,
            CastBridge::F128(f) => *f as u16,
            _ => return Err(EvalError::TypeError),
        };
        Ok(Value::U16 {
            span: SrcPos::default(),
            value: v,
        })
    }
    fn to_u32(self) -> Result<Value, EvalError> {
        let v = match self {
            CastBridge::I128(i) => i as u32,
            CastBridge::U128(u) => u as u32,
            CastBridge::F128(f) => *f as u32,
            _ => return Err(EvalError::TypeError),
        };
        Ok(Value::U32 {
            span: SrcPos::default(),
            value: v,
        })
    }
    fn to_u64(self) -> Result<Value, EvalError> {
        let v = match self {
            CastBridge::I128(i) => i as u64,
            CastBridge::U128(u) => u as u64,
            CastBridge::F128(f) => *f as u64,
            _ => return Err(EvalError::TypeError),
        };
        Ok(Value::U64 {
            span: SrcPos::default(),
            value: v,
        })
    }
    fn to_u128(self) -> Result<Value, EvalError> {
        let v = match self {
            CastBridge::I128(i) => i as u128,
            CastBridge::U128(u) => u,
            CastBridge::F128(f) => *f as u128,
            _ => return Err(EvalError::TypeError),
        };
        Ok(Value::U128 {
            span: SrcPos::default(),
            value: Box::new(v),
        })
    }
    fn to_usize(self, ptr_size: PtrSize) -> Result<Value, EvalError> {
        let bits = match ptr_size {
            PtrSize::U32 => 32u8,
            PtrSize::U64 => 64u8,
        };
        let v = match self {
            CastBridge::I128(i) => i as u64,
            CastBridge::U128(u) => u as u64,
            CastBridge::F128(f) => *f as u64,
            _ => return Err(EvalError::TypeError),
        };
        Ok(Value::USize {
            span: SrcPos::default(),
            bits,
            value: v,
        })
    }
    fn to_i8(self) -> Result<Value, EvalError> {
        let v = match self {
            CastBridge::I128(i) => i as i8,
            CastBridge::U128(u) => u as i8,
            CastBridge::F128(f) => *f as i8,
            _ => return Err(EvalError::TypeError),
        };
        Ok(Value::I8 {
            span: SrcPos::default(),
            value: v,
        })
    }
    fn to_i16(self) -> Result<Value, EvalError> {
        let v = match self {
            CastBridge::I128(i) => i as i16,
            CastBridge::U128(u) => u as i16,
            CastBridge::F128(f) => *f as i16,
            _ => return Err(EvalError::TypeError),
        };
        Ok(Value::I16 {
            span: SrcPos::default(),
            value: v,
        })
    }
    fn to_i32(self) -> Result<Value, EvalError> {
        let v = match self {
            CastBridge::I128(i) => i as i32,
            CastBridge::U128(u) => u as i32,
            CastBridge::F128(f) => *f as i32,
            _ => return Err(EvalError::TypeError),
        };
        Ok(Value::I32 {
            span: SrcPos::default(),
            value: v,
        })
    }
    fn to_i64(self) -> Result<Value, EvalError> {
        let v = match self {
            CastBridge::I128(i) => i as i64,
            CastBridge::U128(u) => u as i64,
            CastBridge::F128(f) => *f as i64,
            _ => return Err(EvalError::TypeError),
        };
        Ok(Value::I64 {
            span: SrcPos::default(),
            value: v,
        })
    }
    fn to_i128(self) -> Result<Value, EvalError> {
        let v = match self {
            CastBridge::I128(i) => i,
            CastBridge::U128(u) => u as i128,
            CastBridge::F128(f) => *f as i128,
            _ => return Err(EvalError::TypeError),
        };
        Ok(Value::I128 {
            span: SrcPos::default(),
            value: Box::new(v),
        })
    }
    fn to_f32(self) -> Result<Value, EvalError> {
        let v = match self {
            CastBridge::I128(i) => OrderedFloat(i as f32),
            CastBridge::U128(u) => OrderedFloat(u as f32),
            CastBridge::F128(f) => OrderedFloat(*f as f32),
            _ => return Err(EvalError::TypeError),
        };
        Ok(Value::F32 {
            span: SrcPos::default(),
            value: v,
        })
    }
    fn to_f64(self) -> Result<Value, EvalError> {
        let v = match self {
            CastBridge::I128(i) => OrderedFloat(i as f64),
            CastBridge::U128(u) => OrderedFloat(u as f64),
            CastBridge::F128(f) => f,
            _ => return Err(EvalError::TypeError),
        };
        Ok(Value::F64 {
            span: SrcPos::default(),
            value: v,
        })
    }
}

fn eval_cast(evaluator: &mut Evaluator, expr: &ValueId, target_type: &TypeId) -> Result<Value, EvalError> {
    let expr_val = evaluator.evaluate(&expr.borrow())?;
    let bridge = CastBridge::try_from(expr_val).map_err(|_| EvalError::TypeError)?;
    let target: &Type = target_type.deref();

    match target {
        Type::Unit { .. } => Ok(Value::Unit {
            span: SrcPos::default(),
        }),
        Type::U8 { .. } => bridge.to_u8(),
        Type::U16 { .. } => bridge.to_u16(),
        Type::U32 { .. } => bridge.to_u32(),
        Type::U64 { .. } => bridge.to_u64(),
        Type::U128 { .. } => bridge.to_u128(),
        Type::USize { .. } => bridge.to_usize(evaluator.ptr_size),
        Type::I8 { .. } => bridge.to_i8(),
        Type::I16 { .. } => bridge.to_i16(),
        Type::I32 { .. } => bridge.to_i32(),
        Type::I64 { .. } => bridge.to_i64(),
        Type::I128 { .. } => bridge.to_i128(),
        Type::F32 { .. } => bridge.to_f32(),
        Type::F64 { .. } => bridge.to_f64(),
        _ => Err(EvalError::TypeError),
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Control flow
// ═══════════════════════════════════════════════════════════════════════════

fn eval_if(
    evaluator: &mut Evaluator,
    condition: &ValueId,
    true_branch: &BlockId,
    false_branch: &Option<BlockId>,
) -> Result<Value, EvalError> {
    let cond_val = evaluator.evaluate(&condition.borrow())?;
    match cond_val {
        Value::Bool { value: true, .. } => evaluator.evaluate_block(&true_branch.borrow()),
        Value::Bool { value: false, .. } => match false_branch {
            Some(fb) => evaluator.evaluate_block(&fb.borrow()),
            None => Ok(Value::Unit {
                span: SrcPos::default(),
            }),
        },
        _ => Err(EvalError::TypeError),
    }
}

fn eval_while(evaluator: &mut Evaluator, condition: &ValueId, body: &BlockId) -> Result<Value, EvalError> {
    loop {
        if evaluator.loop_count >= evaluator.loop_limit {
            return Err(EvalError::LoopLimitExceeded);
        }
        let cond_val = evaluator.evaluate(&condition.borrow())?;
        match cond_val {
            Value::Bool { value: true, .. } => {
                evaluator.loop_count += 1;
                match evaluator.evaluate_block(&body.borrow()) {
                    Ok(_) => continue,
                    Err(EvalError::Continue { .. }) => continue,
                    Err(EvalError::Break { .. }) => break,
                    Err(e) => return Err(e),
                }
            }
            Value::Bool { value: false, .. } => break,
            _ => return Err(EvalError::TypeError),
        }
    }
    Ok(Value::Unit {
        span: SrcPos::default(),
    })
}

fn eval_infinite_loop(evaluator: &mut Evaluator, body: &BlockId) -> Result<Value, EvalError> {
    loop {
        if evaluator.loop_count >= evaluator.loop_limit {
            return Err(EvalError::LoopLimitExceeded);
        }
        evaluator.loop_count += 1;
        match evaluator.evaluate_block(&body.borrow()) {
            Ok(_) => continue,
            Err(EvalError::Continue { .. }) => continue,
            Err(EvalError::Break { .. }) => break,
            Err(e) => return Err(e),
        }
    }
    Ok(Value::Unit {
        span: SrcPos::default(),
    })
}

// ═══════════════════════════════════════════════════════════════════════════
// Compound construction
// ═══════════════════════════════════════════════════════════════════════════

fn eval_struct_object(
    evaluator: &mut Evaluator,
    struct_def: &StructDefId,
    fields: &[(NString, ValueId)],
) -> Result<Value, EvalError> {
    let mut evaluated: Vec<(NString, ValueId)> = Vec::with_capacity(fields.len());
    for (name, field_id) in fields.iter() {
        let field_val = evaluator.evaluate(&field_id.borrow())?;
        evaluated.push((name.clone(), field_val.into()));
    }
    Ok(Value::StructObject {
        span: SrcPos::default(),
        struct_def: struct_def.clone(),
        fields: evaluated.into(),
    })
}

fn eval_enum_variant(
    evaluator: &mut Evaluator,
    enum_def: &EnumDefId,
    variant: &NString,
    inner: &ValueId,
) -> Result<Value, EvalError> {
    let inner_val = evaluator.evaluate(&inner.borrow())?;
    Ok(Value::EnumVariant {
        span: SrcPos::default(),
        enum_def: enum_def.clone(),
        variant: variant.clone(),
        value: inner_val.into(),
    })
}

fn eval_list(evaluator: &mut Evaluator, elements: &[ValueId]) -> Result<Value, EvalError> {
    let evaluated: Vec<ValueId> = elements
        .iter()
        .map(|elem| evaluator.evaluate(&elem.borrow()).map(ValueId::from))
        .collect::<Result<_, _>>()?;
    Ok(Value::List {
        span: SrcPos::default(),
        elements: evaluated.into(),
    })
}

fn eval_tuple(evaluator: &mut Evaluator, elements: &[ValueId]) -> Result<Value, EvalError> {
    let evaluated: Vec<ValueId> = elements
        .iter()
        .map(|elem| evaluator.evaluate(&elem.borrow()).map(ValueId::from))
        .collect::<Result<_, _>>()?;
    Ok(Value::Tuple {
        span: SrcPos::default(),
        elements: evaluated.into(),
    })
}

// ═══════════════════════════════════════════════════════════════════════════
// Field and index access
// ═══════════════════════════════════════════════════════════════════════════

fn eval_field_access(evaluator: &mut Evaluator, expr: &ValueId, field_name: &NString) -> Result<Value, EvalError> {
    let obj = evaluator.evaluate(&expr.borrow())?;
    match obj {
        Value::StructObject { fields, .. } => {
            for (name, field_id) in fields.iter() {
                if name == field_name {
                    return evaluator.evaluate(&field_id.borrow());
                }
            }
            Err(EvalError::TypeError)
        }
        _ => Err(EvalError::TypeError),
    }
}

fn eval_index_access(evaluator: &mut Evaluator, collection: &ValueId, index: &ValueId) -> Result<Value, EvalError> {
    let coll = evaluator.evaluate(&collection.borrow())?;
    let idx_val = evaluator.evaluate(&index.borrow())?;

    let idx = match &idx_val {
        Value::U8 { value, .. } => *value as usize,
        Value::U16 { value, .. } => *value as usize,
        Value::U32 { value, .. } => *value as usize,
        Value::U64 { value, .. } => *value as usize,
        Value::I8 { value, .. } if *value >= 0 => *value as usize,
        Value::I16 { value, .. } if *value >= 0 => *value as usize,
        Value::I32 { value, .. } if *value >= 0 => *value as usize,
        Value::I64 { value, .. } if *value >= 0 => *value as usize,
        Value::USize { value, .. } => *value as usize,
        _ => return Err(EvalError::TypeError),
    };

    match coll {
        Value::List { elements, .. } => elements
            .get(idx)
            .map(|elem| evaluator.evaluate(&elem.borrow()))
            .unwrap_or(Err(EvalError::OutOfBoundsAccess)),
        Value::Tuple { elements, .. } => elements
            .get(idx)
            .map(|elem| evaluator.evaluate(&elem.borrow()))
            .unwrap_or(Err(EvalError::OutOfBoundsAccess)),
        _ => Err(EvalError::TypeError),
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Assignment
// ═══════════════════════════════════════════════════════════════════════════

fn eval_assign(evaluator: &mut Evaluator, place: &ValueId, rhs: Value) -> Result<Value, EvalError> {
    let place_val = &*place.borrow();
    match place_val {
        Value::LocalVariableSymbol { id, .. } => {
            let local = id.borrow();
            if let Some(frame) = evaluator.frames.last_mut() {
                frame.set_binding(local.name.clone(), rhs);
                Ok(Value::Unit {
                    span: SrcPos::default(),
                })
            } else {
                Err(EvalError::TypeError)
            }
        }
        _ => Err(EvalError::Unsupported("assignment to non-local")),
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Function calls
// ═══════════════════════════════════════════════════════════════════════════

fn eval_call(evaluator: &mut Evaluator, callee: &ValueId, args: &Arguments<ValueId>) -> Result<Value, EvalError> {
    let callee_val = evaluator.evaluate(&callee.borrow())?;

    let mut eval_args: Vec<Value> = Vec::new();
    for arg in args.positional.iter() {
        eval_args.push(evaluator.evaluate(&arg.borrow())?);
    }
    for (_, arg) in args.named.iter() {
        eval_args.push(evaluator.evaluate(&arg.borrow())?);
    }

    match callee_val {
        Value::FunctionSymbol { id, .. } => evaluator.evaluate_function(id, &eval_args),
        _ => Err(EvalError::TypeError),
    }
}
