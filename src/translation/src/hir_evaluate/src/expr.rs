use crate::{HirEvaluate, Unwind};
use nitrate_hir::prelude::*;
use nitrate_nstring::NString;
use nitrate_tree::ByteSpan;
use ordered_float::OrderedFloat;
use std::ops::{Add, BitAnd, BitOr, BitXor, Deref, Div, Mul, Neg, Not, Rem, Shl, Shr, Sub};

enum CastLitBridge {
    Unit,
    I128(i128),
    U128(u128),
    F128(OrderedFloat<f64>),
}

impl TryFrom<Value> for CastLitBridge {
    type Error = ();

    fn try_from(lit: Value) -> Result<Self, Self::Error> {
        match lit {
            Value::Unit { .. } => Ok(CastLitBridge::Unit),
            Value::Bool { value: b, .. } => Ok(CastLitBridge::U128(b as u128)),
            Value::I8 { value: i, .. } => Ok(CastLitBridge::I128(i as i128)),
            Value::I16 { value: i, .. } => Ok(CastLitBridge::I128(i as i128)),
            Value::I32 { value: i, .. } => Ok(CastLitBridge::I128(i as i128)),
            Value::I64 { value: i, .. } => Ok(CastLitBridge::I128(i as i128)),
            Value::I128 { value: i, .. } => Ok(CastLitBridge::I128(*i)),
            Value::U8 { value: u, .. } => Ok(CastLitBridge::U128(u as u128)),
            Value::U16 { value: u, .. } => Ok(CastLitBridge::U128(u as u128)),
            Value::U32 { value: u, .. } => Ok(CastLitBridge::U128(u as u128)),
            Value::U64 { value: u, .. } => Ok(CastLitBridge::U128(u as u128)),
            Value::U128 { value: u, .. } => Ok(CastLitBridge::U128(*u)),
            Value::F32 { value: f, .. } => Ok(CastLitBridge::F128(OrderedFloat::from(*f as f64))),
            Value::F64 { value: f, .. } => Ok(CastLitBridge::F128(OrderedFloat::from(*f))),
            Value::USize { value: u, .. } => Ok(CastLitBridge::U128(u as u128)),
            Value::InferredInteger { value: u, .. } => Ok(CastLitBridge::U128(*u)),
            Value::InferredFloat { value: f, .. } => Ok(CastLitBridge::F128(f)),
            _ => Err(()),
        }
    }
}

impl CastLitBridge {
    fn to_unit(self) -> Result<Value, Unwind> {
        match self {
            CastLitBridge::Unit => Ok(Value::Unit {
                span: ByteSpan::default(),
            }),
            _ => Err(Unwind::TypeError),
        }
    }
    fn to_u8(self) -> Result<Value, Unwind> {
        match self {
            CastLitBridge::I128(i) => Ok(Value::U8 {
                span: ByteSpan::default(),
                value: i as u8,
            }),
            CastLitBridge::U128(u) => Ok(Value::U8 {
                span: ByteSpan::default(),
                value: u as u8,
            }),
            CastLitBridge::F128(f) => Ok(Value::U8 {
                span: ByteSpan::default(),
                value: *f as u8,
            }),
            _ => Err(Unwind::TypeError),
        }
    }
    fn to_u16(self) -> Result<Value, Unwind> {
        match self {
            CastLitBridge::I128(i) => Ok(Value::U16 {
                span: ByteSpan::default(),
                value: i as u16,
            }),
            CastLitBridge::U128(u) => Ok(Value::U16 {
                span: ByteSpan::default(),
                value: u as u16,
            }),
            CastLitBridge::F128(f) => Ok(Value::U16 {
                span: ByteSpan::default(),
                value: *f as u16,
            }),
            _ => Err(Unwind::TypeError),
        }
    }
    fn to_u32(self) -> Result<Value, Unwind> {
        match self {
            CastLitBridge::I128(i) => Ok(Value::U32 {
                span: ByteSpan::default(),
                value: i as u32,
            }),
            CastLitBridge::U128(u) => Ok(Value::U32 {
                span: ByteSpan::default(),
                value: u as u32,
            }),
            CastLitBridge::F128(f) => Ok(Value::U32 {
                span: ByteSpan::default(),
                value: *f as u32,
            }),
            _ => Err(Unwind::TypeError),
        }
    }
    fn to_u64(self) -> Result<Value, Unwind> {
        match self {
            CastLitBridge::I128(i) => Ok(Value::U64 {
                span: ByteSpan::default(),
                value: i as u64,
            }),
            CastLitBridge::U128(u) => Ok(Value::U64 {
                span: ByteSpan::default(),
                value: u as u64,
            }),
            CastLitBridge::F128(f) => Ok(Value::U64 {
                span: ByteSpan::default(),
                value: *f as u64,
            }),
            _ => Err(Unwind::TypeError),
        }
    }
    fn to_u128(self) -> Result<Value, Unwind> {
        match self {
            CastLitBridge::I128(i) => Ok(Value::U128 {
                span: ByteSpan::default(),
                value: Box::new(i as u128),
            }),
            CastLitBridge::U128(u) => Ok(Value::U128 {
                span: ByteSpan::default(),
                value: Box::new(u),
            }),
            CastLitBridge::F128(f) => Ok(Value::U128 {
                span: ByteSpan::default(),
                value: Box::new(*f as u128),
            }),
            _ => Err(Unwind::TypeError),
        }
    }
    fn to_usize(self, ptr_size: PtrSize) -> Result<Value, Unwind> {
        match ptr_size {
            PtrSize::U32 => match self {
                CastLitBridge::I128(i) => Ok(Value::USize {
                    span: ByteSpan::default(),
                    bits: 32,
                    value: i as u64,
                }),
                CastLitBridge::U128(u) => Ok(Value::USize {
                    span: ByteSpan::default(),
                    bits: 32,
                    value: u as u64,
                }),
                CastLitBridge::F128(f) => Ok(Value::USize {
                    span: ByteSpan::default(),
                    bits: 32,
                    value: *f as u64,
                }),
                _ => Err(Unwind::TypeError),
            },
            PtrSize::U64 => match self {
                CastLitBridge::I128(i) => Ok(Value::USize {
                    span: ByteSpan::default(),
                    bits: 64,
                    value: i as u64,
                }),
                CastLitBridge::U128(u) => Ok(Value::USize {
                    span: ByteSpan::default(),
                    bits: 64,
                    value: u as u64,
                }),
                CastLitBridge::F128(f) => Ok(Value::USize {
                    span: ByteSpan::default(),
                    bits: 64,
                    value: *f as u64,
                }),
                _ => Err(Unwind::TypeError),
            },
        }
    }
    fn to_i8(self) -> Result<Value, Unwind> {
        match self {
            CastLitBridge::I128(i) => Ok(Value::I8 {
                span: ByteSpan::default(),
                value: i as i8,
            }),
            CastLitBridge::U128(u) => Ok(Value::I8 {
                span: ByteSpan::default(),
                value: u as i8,
            }),
            CastLitBridge::F128(f) => Ok(Value::I8 {
                span: ByteSpan::default(),
                value: *f as i8,
            }),
            _ => Err(Unwind::TypeError),
        }
    }
    fn to_i16(self) -> Result<Value, Unwind> {
        match self {
            CastLitBridge::I128(i) => Ok(Value::I16 {
                span: ByteSpan::default(),
                value: i as i16,
            }),
            CastLitBridge::U128(u) => Ok(Value::I16 {
                span: ByteSpan::default(),
                value: u as i16,
            }),
            CastLitBridge::F128(f) => Ok(Value::I16 {
                span: ByteSpan::default(),
                value: *f as i16,
            }),
            _ => Err(Unwind::TypeError),
        }
    }
    fn to_i32(self) -> Result<Value, Unwind> {
        match self {
            CastLitBridge::I128(i) => Ok(Value::I32 {
                span: ByteSpan::default(),
                value: i as i32,
            }),
            CastLitBridge::U128(u) => Ok(Value::I32 {
                span: ByteSpan::default(),
                value: u as i32,
            }),
            CastLitBridge::F128(f) => Ok(Value::I32 {
                span: ByteSpan::default(),
                value: *f as i32,
            }),
            _ => Err(Unwind::TypeError),
        }
    }
    fn to_i64(self) -> Result<Value, Unwind> {
        match self {
            CastLitBridge::I128(i) => Ok(Value::I64 {
                span: ByteSpan::default(),
                value: i as i64,
            }),
            CastLitBridge::U128(u) => Ok(Value::I64 {
                span: ByteSpan::default(),
                value: u as i64,
            }),
            CastLitBridge::F128(f) => Ok(Value::I64 {
                span: ByteSpan::default(),
                value: *f as i64,
            }),
            _ => Err(Unwind::TypeError),
        }
    }
    fn to_i128(self) -> Result<Value, Unwind> {
        match self {
            CastLitBridge::I128(i) => Ok(Value::I128 {
                span: ByteSpan::default(),
                value: Box::new(i),
            }),
            CastLitBridge::U128(u) => Ok(Value::I128 {
                span: ByteSpan::default(),
                value: Box::new(u as i128),
            }),
            CastLitBridge::F128(f) => Ok(Value::I128 {
                span: ByteSpan::default(),
                value: Box::new(*f as i128),
            }),
            _ => Err(Unwind::TypeError),
        }
    }
    fn to_f32(self) -> Result<Value, Unwind> {
        match self {
            CastLitBridge::I128(i) => Ok(Value::F32 {
                span: ByteSpan::default(),
                value: OrderedFloat::from(i as f32),
            }),
            CastLitBridge::U128(u) => Ok(Value::F32 {
                span: ByteSpan::default(),
                value: OrderedFloat::from(u as f32),
            }),
            CastLitBridge::F128(f) => Ok(Value::F32 {
                span: ByteSpan::default(),
                value: OrderedFloat::from(*f as f32),
            }),
            _ => Err(Unwind::TypeError),
        }
    }
    fn to_f64(self) -> Result<Value, Unwind> {
        match self {
            CastLitBridge::I128(i) => Ok(Value::F64 {
                span: ByteSpan::default(),
                value: OrderedFloat::from(i as f64),
            }),
            CastLitBridge::U128(u) => Ok(Value::F64 {
                span: ByteSpan::default(),
                value: OrderedFloat::from(u as f64),
            }),
            CastLitBridge::F128(f) => Ok(Value::F64 {
                span: ByteSpan::default(),
                value: OrderedFloat::from(*f),
            }),
            _ => Err(Unwind::TypeError),
        }
    }
}

impl HirEvaluate for Lit {
    type Output = Lit;
    fn evaluate(&self, _ctx: &mut crate::HirEvalCtx) -> Result<Self::Output, crate::Unwind> {
        Ok(*self)
    }
}

impl HirEvaluate for BlockElement {
    type Output = Value;
    fn evaluate(&self, ctx: &mut crate::HirEvalCtx) -> Result<Self::Output, crate::Unwind> {
        match self {
            BlockElement::Expr(expr) => expr.borrow().evaluate(ctx),
            BlockElement::Local(_) => unimplemented!(),
        }
    }
}

impl HirEvaluate for Block {
    type Output = Value;
    fn evaluate(&self, ctx: &mut crate::HirEvalCtx) -> Result<Self::Output, crate::Unwind> {
        if ctx.current_safety != BlockSafety::Safe {
            ctx.unsafe_operations_performed += 1;
        }
        let before_safety = ctx.current_safety.clone();
        ctx.current_safety = self.safety.clone();
        let mut last_value = Value::Unit {
            span: ByteSpan::default(),
        };
        for expr in &self.elements {
            last_value = expr.evaluate(ctx)?;
        }
        ctx.current_safety = before_safety;
        Ok(last_value)
    }
}

impl HirEvaluate for Value {
    type Output = Value;
    fn evaluate(&self, ctx: &mut crate::HirEvalCtx) -> Result<Self::Output, crate::Unwind> {
        if ctx.current_safety != BlockSafety::Safe {
            ctx.unsafe_operations_performed += 1;
        }
        match self {
            Value::Unit { .. } => Ok(self.clone()),
            Value::Bool { .. } => Ok(self.clone()),
            Value::I8 { .. } => Ok(self.clone()),
            Value::I16 { .. } => Ok(self.clone()),
            Value::I32 { .. } => Ok(self.clone()),
            Value::I64 { .. } => Ok(self.clone()),
            Value::I128 { .. } => Ok(self.clone()),
            Value::U8 { .. } => Ok(self.clone()),
            Value::U16 { .. } => Ok(self.clone()),
            Value::U32 { .. } => Ok(self.clone()),
            Value::U64 { .. } => Ok(self.clone()),
            Value::U128 { .. } => Ok(self.clone()),
            Value::F32 { .. } => Ok(self.clone()),
            Value::F64 { .. } => Ok(self.clone()),
            Value::USize { .. } => Ok(self.clone()),
            Value::StringLit { .. } => Ok(self.clone()),
            Value::BStringLit { .. } => Ok(self.clone()),
            Value::InferredInteger { .. } => Ok(self.clone()),
            Value::InferredFloat { .. } => Ok(self.clone()),

            Value::StructObject { struct_def, fields, .. } => {
                let mut new_fields: Vec<(NString, ValueId)> = Vec::with_capacity(fields.len());
                for (name, field_id) in fields.iter() {
                    let evaluated = field_id.borrow().evaluate(ctx)?.into();
                    new_fields.push((name.clone(), evaluated));
                }
                Ok(Value::StructObject {
                    span: self.span(),
                    struct_def: struct_def.clone(),
                    fields: new_fields.into(),
                })
            }

            Value::EnumVariant {
                enum_def,
                variant,
                value,
                ..
            } => {
                let evaluated_value = value.borrow().evaluate(ctx)?.into();
                Ok(Value::EnumVariant {
                    span: self.span(),
                    enum_def: enum_def.clone(),
                    variant: variant.clone(),
                    value: evaluated_value,
                })
            }

            Value::Binary { left, op, right, .. } => {
                let left = Lit::try_from(left.borrow().evaluate(ctx)?).map_err(|_| Unwind::TypeError)?;
                let right = Lit::try_from(right.borrow().evaluate(ctx)?).map_err(|_| Unwind::TypeError)?;
                match op {
                    BinaryOp::Add => match left.add(right) {
                        Ok(lit) => Ok(lit.into()),
                        Err(LiteralAddError::TypeError) => Err(Unwind::TypeError),
                    },
                    BinaryOp::Sub => match left.sub(right) {
                        Ok(lit) => Ok(lit.into()),
                        Err(LiteralSubError::TypeError) => Err(Unwind::TypeError),
                    },
                    BinaryOp::Mul => match left.mul(right) {
                        Ok(lit) => Ok(lit.into()),
                        Err(LiteralMulError::TypeError) => Err(Unwind::TypeError),
                    },
                    BinaryOp::Div => match left.div(right) {
                        Ok(lit) => Ok(lit.into()),
                        Err(LiteralDivError::TypeError) => Err(Unwind::TypeError),
                        Err(LiteralDivError::DivisionByZero) => Err(Unwind::DivisionByZero),
                    },
                    BinaryOp::Mod => match left.rem(right) {
                        Ok(lit) => Ok(lit.into()),
                        Err(LiteralRemError::TypeError) => Err(Unwind::TypeError),
                        Err(LiteralRemError::ModuloByZero) => Err(Unwind::ModuloByZero),
                    },
                    BinaryOp::And => match left.bitand(right) {
                        Ok(lit) => Ok(lit.into()),
                        Err(LiteralBitAndError::TypeError) => Err(Unwind::TypeError),
                    },
                    BinaryOp::Or => match left.bitor(right) {
                        Ok(lit) => Ok(lit.into()),
                        Err(LiteralBitOrError::TypeError) => Err(Unwind::TypeError),
                    },
                    BinaryOp::Xor => match left.bitxor(right) {
                        Ok(lit) => Ok(lit.into()),
                        Err(LiteralBitXorError::TypeError) => Err(Unwind::TypeError),
                    },
                    BinaryOp::Shl => match left.shl(right) {
                        Ok(lit) => Ok(lit.into()),
                        Err(LiteralShlError::TypeError) => Err(Unwind::TypeError),
                        Err(LiteralShlError::ShiftAmountError) => Err(Unwind::ShiftAmountError),
                    },
                    BinaryOp::Shr => match left.shr(right) {
                        Ok(lit) => Ok(lit.into()),
                        Err(LiteralShrError::TypeError) => Err(Unwind::TypeError),
                        Err(LiteralShrError::ShiftAmountError) => Err(Unwind::ShiftAmountError),
                    },
                    BinaryOp::Rol => match left.rotate_left(right) {
                        Ok(lit) => Ok(lit.into()),
                        Err(LiteralRolError::TypeError) => Err(Unwind::TypeError),
                    },
                    BinaryOp::Ror => match left.rotate_right(right) {
                        Ok(lit) => Ok(lit.into()),
                        Err(LiteralRorError::TypeError) => Err(Unwind::TypeError),
                    },
                    BinaryOp::LogicAnd => match left.logical_and(right) {
                        Ok(lit) => Ok(Value::Bool {
                            span: ByteSpan::default(),
                            value: lit,
                        }),
                        Err(LiteralCmpError::TypeError) => Err(Unwind::TypeError),
                    },
                    BinaryOp::LogicOr => match left.logical_or(right) {
                        Ok(lit) => Ok(Value::Bool {
                            span: ByteSpan::default(),
                            value: lit,
                        }),
                        Err(LiteralCmpError::TypeError) => Err(Unwind::TypeError),
                    },
                    BinaryOp::Lt => match left.lt(&right) {
                        Ok(lit) => Ok(Value::Bool {
                            span: ByteSpan::default(),
                            value: lit,
                        }),
                        Err(LiteralCmpError::TypeError) => Err(Unwind::TypeError),
                    },
                    BinaryOp::Gt => match left.lt(&right) {
                        Ok(lit) => Ok(Value::Bool {
                            span: ByteSpan::default(),
                            value: !lit,
                        }),
                        Err(LiteralCmpError::TypeError) => Err(Unwind::TypeError),
                    },
                    BinaryOp::Lte => match left.le(&right) {
                        Ok(lit) => Ok(Value::Bool {
                            span: ByteSpan::default(),
                            value: lit,
                        }),
                        Err(LiteralCmpError::TypeError) => Err(Unwind::TypeError),
                    },
                    BinaryOp::Gte => match left.ge(&right) {
                        Ok(lit) => Ok(Value::Bool {
                            span: ByteSpan::default(),
                            value: lit,
                        }),
                        Err(LiteralCmpError::TypeError) => Err(Unwind::TypeError),
                    },
                    BinaryOp::Eq => match left.eq(&right) {
                        Ok(lit) => Ok(Value::Bool {
                            span: ByteSpan::default(),
                            value: lit,
                        }),
                        Err(LiteralCmpError::TypeError) => Err(Unwind::TypeError),
                    },
                    BinaryOp::Ne => match left.ne(&right) {
                        Ok(lit) => Ok(Value::Bool {
                            span: ByteSpan::default(),
                            value: lit,
                        }),
                        Err(LiteralCmpError::TypeError) => Err(Unwind::TypeError),
                    },
                }
            }

            Value::Unary { operand: expr, op, .. } => {
                let operand = Lit::try_from(expr.borrow().evaluate(ctx)?).map_err(|_| Unwind::TypeError)?;
                match op {
                    UnaryOp::Add => Ok(operand.into()),
                    UnaryOp::Sub => match operand.neg() {
                        Ok(lit) => Ok(lit.into()),
                        Err(LiteralNegError::TypeError) => Err(Unwind::TypeError),
                    },
                    UnaryOp::Not => match operand.not() {
                        Ok(lit) => Ok(lit.into()),
                        Err(LiteralNotError::TypeError) => Err(Unwind::TypeError),
                    },
                }
            }

            Value::IndexAccess { .. } => unimplemented!(),
            Value::FieldAccess {
                expr,
                field_name: field,
                ..
            } => match expr.borrow().evaluate(ctx)? {
                Value::StructObject { fields, .. } => {
                    if let Some((_, field_value)) = fields.iter().find(|x| &x.0 == field) {
                        Ok(field_value.borrow().evaluate(ctx)?)
                    } else {
                        Err(Unwind::TypeError)
                    }
                }
                _ => Err(Unwind::TypeError),
            },

            Value::Assign { .. } => unimplemented!(),
            Value::Deref { .. } => unimplemented!(),
            Value::Borrow { .. } => unimplemented!(),

            Value::Cast {
                value: expr,
                target_type: to,
                ..
            } => {
                let expr = expr.borrow().evaluate(ctx)?;
                if expr.is_literal() {
                    let bridge = CastLitBridge::try_from(expr).expect("into cast bridge");
                    let result = match to.deref() {
                        Type::Unit { .. } => bridge.to_unit(),
                        Type::U8 { .. } => bridge.to_u8(),
                        Type::U16 { .. } => bridge.to_u16(),
                        Type::U32 { .. } => bridge.to_u32(),
                        Type::U64 { .. } => bridge.to_u64(),
                        Type::U128 { .. } => bridge.to_u128(),
                        Type::USize { .. } => bridge.to_usize(ctx.ptr_size),
                        Type::I8 { .. } => bridge.to_i8(),
                        Type::I16 { .. } => bridge.to_i16(),
                        Type::I32 { .. } => bridge.to_i32(),
                        Type::I64 { .. } => bridge.to_i64(),
                        Type::I128 { .. } => bridge.to_i128(),
                        Type::F32 { .. } => bridge.to_f32(),
                        Type::F64 { .. } => bridge.to_f64(),
                        _ => Err(Unwind::TypeError),
                    };
                    return result;
                }
                Err(Unwind::TypeError)
            }

            Value::List { elements, .. } => {
                let mut evaluated = Vec::with_capacity(elements.len());
                for element in elements.iter() {
                    evaluated.push(element.borrow().evaluate(ctx)?.into());
                }
                Ok(Value::List {
                    span: self.span(),
                    elements: evaluated.into(),
                })
            }

            Value::Tuple { elements, .. } => {
                let mut evaluated = Vec::with_capacity(elements.len());
                for element in elements.iter() {
                    evaluated.push(element.borrow().evaluate(ctx)?.into());
                }
                Ok(Value::Tuple {
                    span: self.span(),
                    elements: evaluated.into(),
                })
            }

            Value::If {
                condition,
                true_branch,
                false_branch,
                ..
            } => match condition.borrow().evaluate(ctx)? {
                Value::Bool { value: true, .. } => true_branch.borrow().evaluate(ctx),
                Value::Bool { value: false, .. } => {
                    if let Some(false_branch) = false_branch {
                        false_branch.borrow().evaluate(ctx)
                    } else {
                        Ok(Value::Unit {
                            span: ByteSpan::default(),
                        })
                    }
                }
                _ => Err(Unwind::TypeError),
            },

            Value::While { condition, body, .. } => {
                while let Value::Bool { value: true, .. } = condition.borrow().evaluate(ctx)? {
                    if ctx.loop_iter_count >= ctx.loop_iter_limit {
                        return Err(Unwind::LoopLimitExceeded);
                    }
                    body.borrow().evaluate(ctx)?;
                    ctx.loop_iter_count += 1;
                }
                Ok(Value::Unit {
                    span: ByteSpan::default(),
                })
            }

            Value::Loop { body, .. } => loop {
                if ctx.loop_iter_count >= ctx.loop_iter_limit {
                    return Err(Unwind::LoopLimitExceeded);
                }
                body.borrow().evaluate(ctx)?;
                ctx.loop_iter_count += 1;
            },

            Value::Break { label, .. } => Err(Unwind::Break {
                label: label.to_owned(),
            }),
            Value::Continue { label, .. } => Err(Unwind::Continue {
                label: label.to_owned(),
            }),
            Value::Return { value, .. } => Err(Unwind::Return(value.borrow().evaluate(ctx)?)),

            Value::Block { block, .. } => block.borrow().evaluate(ctx),

            Value::Call { .. } => unimplemented!(),
            Value::MethodCall { .. } => unimplemented!(),
            Value::FunctionSymbol { .. } => unimplemented!(),
            Value::GlobalVariableSymbol { .. } => unimplemented!(),
            Value::LocalVariableSymbol { .. } => unimplemented!(),
            Value::ParameterSymbol { .. } => unimplemented!(),
        }
    }
}
