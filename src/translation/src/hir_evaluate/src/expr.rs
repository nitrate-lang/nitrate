use crate::{HirEvaluate, Unwind};
use nitrate_hir::prelude::*;
use std::ops::{Add, BitAnd, BitOr, BitXor, Div, Mul, Neg, Not, Rem, Shl, Shr, Sub};

impl HirEvaluate for Lit {
    type Output = Lit;

    fn evaluate(&self, _ctx: &mut crate::HirEvalCtx) -> Result<Self::Output, crate::Unwind> {
        Ok(self.clone())
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
            Value::F8(f) => Ok(Value::F8(*f)),
            Value::F16(f) => Ok(Value::F16(*f)),
            Value::F32(f) => Ok(Value::F32(*f)),
            Value::F64(f) => Ok(Value::F64(*f)),
            Value::F128(f) => Ok(Value::F128(*f)),
            Value::USize32(u) => Ok(Value::USize32(*u)),
            Value::USize64(u) => Ok(Value::USize64(*u)),
            Value::String(s) => Ok(Value::String(s.clone())),
            Value::BString(s) => Ok(Value::BString(s.clone())),

            Value::Struct {
                struct_type,
                fields,
            } => {
                let mut fields = fields.to_owned();
                for field_value in fields.values_mut() {
                    let eval_value = ctx.store[field_value as &ValueId]
                        .evaluate(ctx)?
                        .into_id(ctx.store);

                    *field_value = eval_value;
                }

                Ok(Value::Struct {
                    struct_type: struct_type.clone(),
                    fields,
                })
            }

            Value::Enum {
                enum_type,
                variant,
                value,
            } => {
                let evaluated_value = ctx.store[value].evaluate(ctx)?.into_id(ctx.store);

                Ok(Value::Enum {
                    enum_type: enum_type.clone(),
                    variant: variant.clone(),
                    value: evaluated_value,
                })
            }

            Value::Binary { left, op, right } => {
                let left =
                    Lit::try_from(ctx.store[left].evaluate(ctx)?).map_err(|_| Unwind::TypeError)?;

                let right = Lit::try_from(ctx.store[right].evaluate(ctx)?)
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

                    BinaryOp::LogicXor => match left.logical_xor(right) {
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

            Value::Unary { op, expr } => {
                let operand =
                    Lit::try_from(ctx.store[expr].evaluate(ctx)?).map_err(|_| Unwind::TypeError)?;

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

            Value::Symbol { symbol: _ } => {
                // TODO: Evaluate symbol expressions
                todo!()
            }

            Value::FieldAccess { expr, field } => match ctx.store[expr].evaluate(ctx)? {
                Value::Struct { fields, .. } => {
                    if let Some(field_value) = fields.get(field) {
                        Ok(ctx.store[field_value].evaluate(ctx)?)
                    } else {
                        Err(Unwind::TypeError)
                    }
                }

                _ => Err(Unwind::TypeError),
            },

            Value::ArrayIndex { expr, index } => {
                let index = match ctx.evaluate_to_literal(&ctx.store[index])? {
                    Lit::USize32(i) => i as usize,
                    Lit::USize64(i) => i as usize,
                    _ => return Err(Unwind::TypeError),
                };

                match ctx.store[expr].evaluate(ctx)? {
                    Value::List { elements } => match elements.get(index) {
                        Some(elem) => Ok(ctx.store[elem].evaluate(ctx)?),
                        None => Err(Unwind::IndexOutOfBounds),
                    },

                    _ => Err(Unwind::TypeError),
                }
            }

            Value::Assign { place: _, value: _ } => {
                // TODO: Evaluate assignment expressions
                todo!()
            }

            Value::Deref { place: _ } => {
                // TODO: Evaluate dereference expressions
                todo!()
            }

            Value::GetAddressOf { place: _ } => {
                // TODO: Evaluate address-of expressions
                todo!()
            }

            Value::Cast { expr: _, to: _ } => {
                // TODO: Evaluate cast expressions
                todo!()
            }

            Value::GetTypeOf { expr: _ } => {
                // TODO: Evaluate typeof expressions
                todo!()
            }

            Value::List { elements } => {
                let mut evaluated_elements = Vec::with_capacity(elements.len());
                for element in &**elements {
                    let evaluated_element = ctx.store[element].evaluate(ctx)?.into_id(ctx.store);
                    evaluated_elements.push(evaluated_element);
                }

                Ok(Value::List {
                    elements: Box::new(evaluated_elements),
                })
            }

            Value::If {
                condition,
                true_branch,
                false_branch,
            } => match ctx.store[condition].evaluate(ctx)? {
                Value::Bool(true) => ctx.store[true_branch].evaluate(ctx),

                Value::Bool(false) => {
                    if let Some(false_branch) = false_branch {
                        ctx.store[false_branch].evaluate(ctx)
                    } else {
                        Ok(Value::Unit)
                    }
                }

                _ => Err(Unwind::TypeError),
            },

            Value::While { condition, body } => {
                while let Value::Bool(true) = ctx.store[condition].evaluate(ctx)? {
                    if ctx.loop_iter_count >= ctx.loop_iter_limit {
                        return Err(Unwind::LoopLimitExceeded);
                    }

                    ctx.store[body].evaluate(ctx)?;
                    ctx.loop_iter_count += 1;
                }

                Ok(Value::Unit)
            }

            Value::Loop { body } => loop {
                if ctx.loop_iter_count >= ctx.loop_iter_limit {
                    return Err(Unwind::LoopLimitExceeded);
                }

                ctx.store[body].evaluate(ctx)?;
                ctx.loop_iter_count += 1;
            },

            Value::Break { label } => Err(Unwind::Break {
                label: label.to_owned(),
            }),

            Value::Continue { label } => Err(Unwind::Continue {
                label: label.to_owned(),
            }),

            Value::Return { value } => {
                let value = ctx.store[value].evaluate(ctx)?;
                Err(Unwind::Return(value))
            }

            Value::Block { block } => ctx.store[block].evaluate(ctx),

            Value::Call {
                callee: _,
                arguments: _,
            } => {
                // TODO: Evaluate function call expressions
                todo!()
            }
        }
    }
}
