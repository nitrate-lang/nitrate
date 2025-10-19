use crate::{HirEvaluate, Unwind};
use nitrate_hir::prelude::*;
use ordered_float::OrderedFloat;
use std::ops::{Add, BitAnd, BitOr, BitXor, Div, Mul, Neg, Not, Rem, Shl, Shr, Sub};

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
            Value::Unit => Ok(CastLitBridge::Unit),
            Value::Bool(b) => Ok(CastLitBridge::U128(b as u128)),
            Value::I8(i) => Ok(CastLitBridge::I128(i as i128)),
            Value::I16(i) => Ok(CastLitBridge::I128(i as i128)),
            Value::I32(i) => Ok(CastLitBridge::I128(i as i128)),
            Value::I64(i) => Ok(CastLitBridge::I128(i as i128)),
            Value::I128(i) => Ok(CastLitBridge::I128(*i)),
            Value::U8(u) => Ok(CastLitBridge::U128(u as u128)),
            Value::U16(u) => Ok(CastLitBridge::U128(u as u128)),
            Value::U32(u) => Ok(CastLitBridge::U128(u as u128)),
            Value::U64(u) => Ok(CastLitBridge::U128(u as u128)),
            Value::U128(u) => Ok(CastLitBridge::U128(*u)),
            Value::F32(f) => Ok(CastLitBridge::F128(OrderedFloat::from(*f as f64))),
            Value::F64(f) => Ok(CastLitBridge::F128(OrderedFloat::from(*f as f64))),
            Value::USize32(u) => Ok(CastLitBridge::U128(u as u128)),
            Value::USize64(u) => Ok(CastLitBridge::U128(u as u128)),
            Value::InferredInteger(u) => Ok(CastLitBridge::U128(*u)),
            Value::InferredFloat(f) => Ok(CastLitBridge::F128(f)),
            _ => Err(()),
        }
    }
}

impl CastLitBridge {
    fn to_unit(self) -> Result<Value, Unwind> {
        match self {
            CastLitBridge::Unit => Ok(Value::Unit),
            CastLitBridge::I128(_) | CastLitBridge::U128(_) | CastLitBridge::F128(_) => {
                Err(Unwind::TypeError)
            }
        }
    }

    fn to_u8(self) -> Result<Value, Unwind> {
        match self {
            CastLitBridge::I128(i) => Ok(Value::U8(i as u8)),
            CastLitBridge::U128(u) => Ok(Value::U8(u as u8)),
            CastLitBridge::F128(f) => Ok(Value::U8(*f as u8)),
            CastLitBridge::Unit => Err(Unwind::TypeError),
        }
    }

    fn to_u16(self) -> Result<Value, Unwind> {
        match self {
            CastLitBridge::I128(i) => Ok(Value::U16(i as u16)),
            CastLitBridge::U128(u) => Ok(Value::U16(u as u16)),
            CastLitBridge::F128(f) => Ok(Value::U16(*f as u16)),
            CastLitBridge::Unit => Err(Unwind::TypeError),
        }
    }

    fn to_u32(self) -> Result<Value, Unwind> {
        match self {
            CastLitBridge::I128(i) => Ok(Value::U32(i as u32)),
            CastLitBridge::U128(u) => Ok(Value::U32(u as u32)),
            CastLitBridge::F128(f) => Ok(Value::U32(*f as u32)),
            CastLitBridge::Unit => Err(Unwind::TypeError),
        }
    }

    fn to_u64(self) -> Result<Value, Unwind> {
        match self {
            CastLitBridge::I128(i) => Ok(Value::U64(i as u64)),
            CastLitBridge::U128(u) => Ok(Value::U64(u as u64)),
            CastLitBridge::F128(f) => Ok(Value::U64(*f as u64)),
            CastLitBridge::Unit => Err(Unwind::TypeError),
        }
    }

    fn to_u128(self) -> Result<Value, Unwind> {
        match self {
            CastLitBridge::I128(i) => Ok(Value::U128(Box::new(i as u128))),
            CastLitBridge::U128(u) => Ok(Value::U128(Box::new(u))),
            CastLitBridge::F128(f) => Ok(Value::U128(Box::new(*f as u128))),
            CastLitBridge::Unit => Err(Unwind::TypeError),
        }
    }

    fn to_usize(self, ptr_size: PtrSize) -> Result<Value, Unwind> {
        match ptr_size {
            PtrSize::U32 => match self {
                CastLitBridge::I128(i) => Ok(Value::USize32(i as u32)),
                CastLitBridge::U128(u) => Ok(Value::USize32(u as u32)),
                CastLitBridge::F128(f) => Ok(Value::USize32(*f as u32)),
                CastLitBridge::Unit => Err(Unwind::TypeError),
            },
            PtrSize::U64 => match self {
                CastLitBridge::I128(i) => Ok(Value::USize64(i as u64)),
                CastLitBridge::U128(u) => Ok(Value::USize64(u as u64)),
                CastLitBridge::F128(f) => Ok(Value::USize64(*f as u64)),
                CastLitBridge::Unit => Err(Unwind::TypeError),
            },
        }
    }

    fn to_i8(self) -> Result<Value, Unwind> {
        match self {
            CastLitBridge::I128(i) => Ok(Value::I8(i as i8)),
            CastLitBridge::U128(u) => Ok(Value::I8(u as i8)),
            CastLitBridge::F128(f) => Ok(Value::I8(*f as i8)),
            CastLitBridge::Unit => Err(Unwind::TypeError),
        }
    }

    fn to_i16(self) -> Result<Value, Unwind> {
        match self {
            CastLitBridge::I128(i) => Ok(Value::I16(i as i16)),
            CastLitBridge::U128(u) => Ok(Value::I16(u as i16)),
            CastLitBridge::F128(f) => Ok(Value::I16(*f as i16)),
            CastLitBridge::Unit => Err(Unwind::TypeError),
        }
    }

    fn to_i32(self) -> Result<Value, Unwind> {
        match self {
            CastLitBridge::I128(i) => Ok(Value::I32(i as i32)),
            CastLitBridge::U128(u) => Ok(Value::I32(u as i32)),
            CastLitBridge::F128(f) => Ok(Value::I32(*f as i32)),
            CastLitBridge::Unit => Err(Unwind::TypeError),
        }
    }

    fn to_i64(self) -> Result<Value, Unwind> {
        match self {
            CastLitBridge::I128(i) => Ok(Value::I64(i as i64)),
            CastLitBridge::U128(u) => Ok(Value::I64(u as i64)),
            CastLitBridge::F128(f) => Ok(Value::I64(*f as i64)),
            CastLitBridge::Unit => Err(Unwind::TypeError),
        }
    }

    fn to_i128(self) -> Result<Value, Unwind> {
        match self {
            CastLitBridge::I128(i) => Ok(Value::I128(Box::new(i))),
            CastLitBridge::U128(u) => Ok(Value::I128(Box::new(u as i128))),
            CastLitBridge::F128(f) => Ok(Value::I128(Box::new(*f as i128))),
            CastLitBridge::Unit => Err(Unwind::TypeError),
        }
    }

    fn to_f32(self) -> Result<Value, Unwind> {
        match self {
            CastLitBridge::I128(i) => Ok(Value::F32(OrderedFloat::from(i as f32))),
            CastLitBridge::U128(u) => Ok(Value::F32(OrderedFloat::from(u as f32))),
            CastLitBridge::F128(f) => Ok(Value::F32(OrderedFloat::from(*f as f32))),
            CastLitBridge::Unit => Err(Unwind::TypeError),
        }
    }

    fn to_f64(self) -> Result<Value, Unwind> {
        match self {
            CastLitBridge::I128(i) => Ok(Value::F64(OrderedFloat::from(i as f64))),
            CastLitBridge::U128(u) => Ok(Value::F64(OrderedFloat::from(u as f64))),
            CastLitBridge::F128(f) => Ok(Value::F64(OrderedFloat::from(*f as f64))),
            CastLitBridge::Unit => Err(Unwind::TypeError),
        }
    }
}

impl HirEvaluate for Lit {
    type Output = Lit;

    fn evaluate(&self, _ctx: &mut crate::HirEvalCtx) -> Result<Self::Output, crate::Unwind> {
        Ok(self.clone())
    }
}

impl HirEvaluate for BlockElement {
    type Output = Value;

    fn evaluate(&self, ctx: &mut crate::HirEvalCtx) -> Result<Self::Output, crate::Unwind> {
        match self {
            BlockElement::Expr(expr) => ctx.store[expr].borrow().evaluate(ctx),

            BlockElement::Stmt(expr) => {
                ctx.store[expr].borrow().evaluate(ctx)?;
                Ok(Value::Unit)
            }

            BlockElement::Local(_local) => {
                // TODO: handle local variable declarations
                unimplemented!()
            }
        }
    }
}

impl HirEvaluate for Block {
    type Output = Value;

    fn evaluate(&self, ctx: &mut crate::HirEvalCtx) -> Result<Self::Output, crate::Unwind> {
        if ctx.current_safety != BlockSafety::Safe {
            ctx.unsafe_operations_performed += 1;
        }

        let before_safety = ctx.current_safety;
        ctx.current_safety = self.safety;

        let mut last_value = Value::Unit;
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
            Value::Unit => Ok(Value::Unit),
            Value::Bool(b) => Ok(Value::Bool(*b)),
            Value::I8(i) => Ok(Value::I8(*i)),
            Value::I16(i) => Ok(Value::I16(*i)),
            Value::I32(i) => Ok(Value::I32(*i)),
            Value::I64(i) => Ok(Value::I64(*i)),
            Value::I128(i) => Ok(Value::I128(i.clone())),
            Value::U8(u) => Ok(Value::U8(*u)),
            Value::U16(u) => Ok(Value::U16(*u)),
            Value::U32(u) => Ok(Value::U32(*u)),
            Value::U64(u) => Ok(Value::U64(*u)),
            Value::U128(u) => Ok(Value::U128(u.clone())),
            Value::F32(f) => Ok(Value::F32(*f)),
            Value::F64(f) => Ok(Value::F64(*f)),
            Value::USize32(u) => Ok(Value::USize32(*u)),
            Value::USize64(u) => Ok(Value::USize64(*u)),
            Value::StringLit(s) => Ok(Value::StringLit(s.clone())),
            Value::BStringLit(s) => Ok(Value::BStringLit(s.clone())),
            Value::InferredInteger(i) => Ok(Value::InferredInteger(i.clone())),
            Value::InferredFloat(f) => Ok(Value::InferredFloat(*f)),

            Value::StructObject {
                struct_type,
                fields,
            } => {
                let mut fields = fields.to_owned();
                for (_, field_value) in &mut fields {
                    let eval_value = ctx.store[field_value as &ValueId]
                        .borrow()
                        .evaluate(ctx)?
                        .into_id(ctx.store);

                    *field_value = eval_value;
                }

                Ok(Value::StructObject {
                    struct_type: struct_type.clone(),
                    fields,
                })
            }

            Value::EnumVariant {
                enum_type,
                variant,
                value,
            } => {
                let evaluated_value = ctx.store[value].borrow().evaluate(ctx)?.into_id(ctx.store);

                Ok(Value::EnumVariant {
                    enum_type: enum_type.clone(),
                    variant: variant.clone(),
                    value: evaluated_value,
                })
            }

            Value::Binary { left, op, right } => {
                let left = Lit::try_from(ctx.store[left].borrow().evaluate(ctx)?)
                    .map_err(|_| Unwind::TypeError)?;

                let right = Lit::try_from(ctx.store[right].borrow().evaluate(ctx)?)
                    .map_err(|_| Unwind::TypeError)?;

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
                        Ok(lit) => Ok(Value::Bool(lit)),
                        Err(LiteralCmpError::TypeError) => Err(Unwind::TypeError),
                    },

                    BinaryOp::LogicOr => match left.logical_or(right) {
                        Ok(lit) => Ok(Value::Bool(lit)),
                        Err(LiteralCmpError::TypeError) => Err(Unwind::TypeError),
                    },

                    BinaryOp::Lt => match left.lt(&right) {
                        Ok(lit) => Ok(Value::Bool(lit)),
                        Err(LiteralCmpError::TypeError) => Err(Unwind::TypeError),
                    },

                    BinaryOp::Gt => match left.lt(&right) {
                        Ok(lit) => Ok(Value::Bool(!lit)),
                        Err(LiteralCmpError::TypeError) => Err(Unwind::TypeError),
                    },

                    BinaryOp::Lte => match left.le(&right) {
                        Ok(lit) => Ok(Value::Bool(lit)),
                        Err(LiteralCmpError::TypeError) => Err(Unwind::TypeError),
                    },

                    BinaryOp::Gte => match left.ge(&right) {
                        Ok(lit) => Ok(Value::Bool(lit)),
                        Err(LiteralCmpError::TypeError) => Err(Unwind::TypeError),
                    },

                    BinaryOp::Eq => match left.eq(&right) {
                        Ok(lit) => Ok(Value::Bool(lit)),
                        Err(LiteralCmpError::TypeError) => Err(Unwind::TypeError),
                    },

                    BinaryOp::Ne => match left.ne(&right) {
                        Ok(lit) => Ok(Value::Bool(lit)),
                        Err(LiteralCmpError::TypeError) => Err(Unwind::TypeError),
                    },
                }
            }

            Value::Unary { op, operand: expr } => {
                let operand = Lit::try_from(ctx.store[expr].borrow().evaluate(ctx)?)
                    .map_err(|_| Unwind::TypeError)?;

                match op {
                    UnaryOp::Add => Ok(operand.into()),

                    UnaryOp::Sub => match operand.neg() {
                        Ok(lit) => Ok(lit.into()),
                        Err(LiteralNegError::TypeError) => Err(Unwind::TypeError),
                    },

                    UnaryOp::BitNot | UnaryOp::LogicNot => match operand.not() {
                        Ok(lit) => Ok(lit.into()),
                        Err(LiteralNotError::TypeError) => Err(Unwind::TypeError),
                    },
                }
            }

            Value::Symbol { name: _, link: _ } => {
                // TODO: evaluate symbol expressions
                unimplemented!()
            }

            Value::FieldAccess { expr, field } => match ctx.store[expr].borrow().evaluate(ctx)? {
                Value::StructObject { fields, .. } => {
                    if let Some((_, field_value)) = fields.iter().find(|x| &x.0 == field) {
                        Ok(ctx.store[field_value].borrow().evaluate(ctx)?)
                    } else {
                        Err(Unwind::TypeError)
                    }
                }

                _ => Err(Unwind::TypeError),
            },

            Value::IndexAccess {
                collection: expr,
                index,
            } => {
                let index = match ctx.evaluate_to_literal(&ctx.store[index].borrow())? {
                    Lit::USize32(i) => i as usize,
                    Lit::USize64(i) => i as usize,
                    _ => return Err(Unwind::TypeError),
                };

                match ctx.store[expr].borrow().evaluate(ctx)? {
                    Value::List { elements } => match elements.get(index) {
                        Some(elem) => Ok(elem.evaluate(ctx)?),
                        None => Err(Unwind::IndexOutOfBounds),
                    },

                    _ => Err(Unwind::TypeError),
                }
            }

            Value::Assign { place: _, value: _ } => {
                // TODO: evaluate assignment expressions
                unimplemented!()
            }

            Value::Deref { place: _ } => {
                // TODO: evaluate dereference expressions
                unimplemented!()
            }

            Value::Borrow {
                mutable: _,
                place: _,
            } => {
                // TODO: evaluate address-of expressions
                unimplemented!()
            }

            Value::Cast { expr, to } => {
                let expr = ctx.store[expr].borrow().evaluate(ctx)?;

                if expr.is_literal() {
                    let bridge = CastLitBridge::try_from(expr).expect("into cast bridge");
                    let result = match &ctx.store[to] {
                        Type::Unit => bridge.to_unit(),
                        Type::U8 => bridge.to_u8(),
                        Type::U16 => bridge.to_u16(),
                        Type::U32 => bridge.to_u32(),
                        Type::U64 => bridge.to_u64(),
                        Type::U128 => bridge.to_u128(),
                        Type::USize => bridge.to_usize(ctx.ptr_size),
                        Type::I8 => bridge.to_i8(),
                        Type::I16 => bridge.to_i16(),
                        Type::I32 => bridge.to_i32(),
                        Type::I64 => bridge.to_i64(),
                        Type::I128 => bridge.to_i128(),
                        Type::F32 => bridge.to_f32(),
                        Type::F64 => bridge.to_f64(),
                        _ => Err(Unwind::TypeError),
                    };

                    return result;
                }

                // TODO: handle non-literal type casts

                return Err(Unwind::TypeError);
            }

            Value::List { elements } => {
                let mut evaluated_elements = Vec::with_capacity(elements.len());
                for element in &**elements {
                    let evaluated_element = element.evaluate(ctx)?;
                    evaluated_elements.push(evaluated_element);
                }

                Ok(Value::List {
                    elements: evaluated_elements.into(),
                })
            }

            Value::Tuple { elements } => {
                let mut evaluated_elements = Vec::with_capacity(elements.len());
                for element in &**elements {
                    let evaluated_element = element.evaluate(ctx)?;
                    evaluated_elements.push(evaluated_element);
                }

                Ok(Value::Tuple {
                    elements: evaluated_elements.into(),
                })
            }

            Value::If {
                condition,
                true_branch,
                false_branch,
            } => match ctx.store[condition].borrow().evaluate(ctx)? {
                Value::Bool(true) => ctx.store[true_branch].borrow().evaluate(ctx),

                Value::Bool(false) => {
                    if let Some(false_branch) = false_branch {
                        ctx.store[false_branch].borrow().evaluate(ctx)
                    } else {
                        Ok(Value::Unit)
                    }
                }

                _ => Err(Unwind::TypeError),
            },

            Value::While { condition, body } => {
                while let Value::Bool(true) = ctx.store[condition].borrow().evaluate(ctx)? {
                    if ctx.loop_iter_count >= ctx.loop_iter_limit {
                        return Err(Unwind::LoopLimitExceeded);
                    }

                    ctx.store[body].borrow().evaluate(ctx)?;
                    ctx.loop_iter_count += 1;
                }

                Ok(Value::Unit)
            }

            Value::Loop { body } => loop {
                if ctx.loop_iter_count >= ctx.loop_iter_limit {
                    return Err(Unwind::LoopLimitExceeded);
                }

                ctx.store[body].borrow().evaluate(ctx)?;
                ctx.loop_iter_count += 1;
            },

            Value::Break { label } => Err(Unwind::Break {
                label: label.to_owned(),
            }),

            Value::Continue { label } => Err(Unwind::Continue {
                label: label.to_owned(),
            }),

            Value::Return { value } => {
                let value = ctx.store[value].borrow().evaluate(ctx)?;
                Err(Unwind::Return(value))
            }

            Value::Block { block } => ctx.store[block].borrow().evaluate(ctx),

            Value::Closure {
                captures: _,
                callee: _,
            } => {
                // TODO: evaluate closure expressions
                unimplemented!()
            }

            Value::Call {
                callee: _,
                arguments: _,
            } => {
                // TODO: evaluate function call expressions
                unimplemented!()
            }
        }
    }
}
