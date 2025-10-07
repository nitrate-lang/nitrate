use crate::{EvalFail, HirEvaluate};
use nitrate_hir::prelude::*;
use std::ops::{Add, BitAnd, BitOr, BitXor, Div, Mul, Neg, Not, Rem, Shl, Shr, Sub};

impl HirEvaluate for Literal {
    type Output = Literal;

    fn evaluate(&self, _ctx: &mut crate::HirEvalCtx) -> Result<Self::Output, crate::EvalFail> {
        Ok(self.clone())
    }
}

impl HirEvaluate for Block {
    type Output = Value;

    fn evaluate(&self, ctx: &mut crate::HirEvalCtx) -> Result<Self::Output, crate::EvalFail> {
        if ctx.current_safety != BlockSafety::Safe {
            ctx.unsafe_operations_performed += 1;
        }

        let before_safety = ctx.current_safety;
        ctx.current_safety = self.safety;

        let mut last_value = Value::Unit;
        for expr in &self.exprs {
            last_value = ctx.store[expr].evaluate(ctx)?;
        }

        ctx.current_safety = before_safety;

        Ok(last_value)
    }
}

impl HirEvaluate for Value {
    type Output = Value;

    fn evaluate(&self, ctx: &mut crate::HirEvalCtx) -> Result<Self::Output, crate::EvalFail> {
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
            Value::String(s) => Ok(Value::String(s.clone())),
            Value::BString(s) => Ok(Value::BString(s.clone())),

            Value::Binary { left, op, right } => {
                let left = Literal::try_from(ctx.store[left].evaluate(ctx)?)
                    .map_err(|_| EvalFail::TypeError)?;

                let right = Literal::try_from(ctx.store[right].evaluate(ctx)?)
                    .map_err(|_| EvalFail::TypeError)?;

                match op {
                    BinaryOp::Add => match left.add(right) {
                        Ok(lit) => Ok(lit.into()),
                        Err(LiteralAddError::TypeError) => Err(EvalFail::TypeError),
                    },

                    BinaryOp::Sub => match left.sub(right) {
                        Ok(lit) => Ok(lit.into()),
                        Err(LiteralSubError::TypeError) => Err(EvalFail::TypeError),
                    },

                    BinaryOp::Mul => match left.mul(right) {
                        Ok(lit) => Ok(lit.into()),
                        Err(LiteralMulError::TypeError) => Err(EvalFail::TypeError),
                    },

                    BinaryOp::Div => match left.div(right) {
                        Ok(lit) => Ok(lit.into()),
                        Err(LiteralDivError::TypeError) => Err(EvalFail::TypeError),
                        Err(LiteralDivError::DivisionByZero) => Err(EvalFail::DivisionByZero),
                    },

                    BinaryOp::Mod => match left.rem(right) {
                        Ok(lit) => Ok(lit.into()),
                        Err(LiteralRemError::TypeError) => Err(EvalFail::TypeError),
                        Err(LiteralRemError::ModuloByZero) => Err(EvalFail::ModuloByZero),
                    },

                    BinaryOp::And => match left.bitand(right) {
                        Ok(lit) => Ok(lit.into()),
                        Err(LiteralBitAndError::TypeError) => Err(EvalFail::TypeError),
                    },

                    BinaryOp::Or => match left.bitor(right) {
                        Ok(lit) => Ok(lit.into()),
                        Err(LiteralBitOrError::TypeError) => Err(EvalFail::TypeError),
                    },

                    BinaryOp::Xor => match left.bitxor(right) {
                        Ok(lit) => Ok(lit.into()),
                        Err(LiteralBitXorError::TypeError) => Err(EvalFail::TypeError),
                    },

                    BinaryOp::Shl => match left.shl(right) {
                        Ok(lit) => Ok(lit.into()),
                        Err(LiteralShlError::TypeError) => Err(EvalFail::TypeError),
                        Err(LiteralShlError::ShiftAmountError) => Err(EvalFail::ShiftAmountError),
                    },

                    BinaryOp::Shr => match left.shr(right) {
                        Ok(lit) => Ok(lit.into()),
                        Err(LiteralShrError::TypeError) => Err(EvalFail::TypeError),
                        Err(LiteralShrError::ShiftAmountError) => Err(EvalFail::ShiftAmountError),
                    },

                    BinaryOp::Rol => match left.rotate_left(right) {
                        Ok(lit) => Ok(lit.into()),
                        Err(LiteralRolError::TypeError) => Err(EvalFail::TypeError),
                    },

                    BinaryOp::Ror => match left.rotate_right(right) {
                        Ok(lit) => Ok(lit.into()),
                        Err(LiteralRorError::TypeError) => Err(EvalFail::TypeError),
                    },

                    BinaryOp::LogicAnd => match left.logical_and(right) {
                        Ok(lit) => Ok(Value::Bool(lit)),
                        Err(LiteralCmpError::TypeError) => Err(EvalFail::TypeError),
                    },

                    BinaryOp::LogicOr => match left.logical_or(right) {
                        Ok(lit) => Ok(Value::Bool(lit)),
                        Err(LiteralCmpError::TypeError) => Err(EvalFail::TypeError),
                    },

                    BinaryOp::LogicXor => match left.logical_xor(right) {
                        Ok(lit) => Ok(Value::Bool(lit)),
                        Err(LiteralCmpError::TypeError) => Err(EvalFail::TypeError),
                    },

                    BinaryOp::Lt => match left.lt(&right) {
                        Ok(lit) => Ok(Value::Bool(lit)),
                        Err(LiteralCmpError::TypeError) => Err(EvalFail::TypeError),
                    },

                    BinaryOp::Gt => match left.lt(&right) {
                        Ok(lit) => Ok(Value::Bool(!lit)),
                        Err(LiteralCmpError::TypeError) => Err(EvalFail::TypeError),
                    },

                    BinaryOp::Lte => match left.le(&right) {
                        Ok(lit) => Ok(Value::Bool(lit)),
                        Err(LiteralCmpError::TypeError) => Err(EvalFail::TypeError),
                    },

                    BinaryOp::Gte => match left.ge(&right) {
                        Ok(lit) => Ok(Value::Bool(lit)),
                        Err(LiteralCmpError::TypeError) => Err(EvalFail::TypeError),
                    },

                    BinaryOp::Eq => match left.eq(&right) {
                        Ok(lit) => Ok(Value::Bool(lit)),
                        Err(LiteralCmpError::TypeError) => Err(EvalFail::TypeError),
                    },

                    BinaryOp::Ne => match left.ne(&right) {
                        Ok(lit) => Ok(Value::Bool(lit)),
                        Err(LiteralCmpError::TypeError) => Err(EvalFail::TypeError),
                    },
                }
            }

            Value::Unary { op, expr } => {
                let operand = Literal::try_from(ctx.store[expr].evaluate(ctx)?)
                    .map_err(|_| EvalFail::TypeError)?;

                match op {
                    UnaryOp::Add => Ok(operand.into()),

                    UnaryOp::Sub => match operand.neg() {
                        Ok(lit) => Ok(lit.into()),
                        Err(LiteralNegError::TypeError) => Err(EvalFail::TypeError),
                    },

                    UnaryOp::BitNot | UnaryOp::LogicNot => match operand.not() {
                        Ok(lit) => Ok(lit.into()),
                        Err(LiteralNotError::TypeError) => Err(EvalFail::TypeError),
                    },
                }
            }

            Value::Symbol { symbol } => {
                // TODO: Evaluate symbol expressions
                todo!()
            }

            Value::FieldAccess { expr, field } => {
                // TODO: Evaluate field access expressions
                todo!()
            }

            Value::ArrayIndex { expr, index } => {
                // TODO: Evaluate array index expressions
                todo!()
            }

            Value::Assign { place, value } => {
                // TODO: Evaluate assignment expressions
                todo!()
            }

            Value::Deref { place } => {
                // TODO: Evaluate dereference expressions
                todo!()
            }

            Value::GetAddressOf { place } => {
                // TODO: Evaluate address-of expressions
                todo!()
            }

            Value::Cast { expr, to } => {
                // TODO: Evaluate cast expressions
                todo!()
            }

            Value::GetTypeOf { expr } => {
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

                _ => Err(EvalFail::TypeError),
            },

            Value::While { condition, body } => {
                while let Value::Bool(true) = ctx.store[condition].evaluate(ctx)? {
                    if ctx.loop_iter_count >= ctx.loop_iter_limit {
                        return Err(EvalFail::LoopLimitExceeded);
                    }

                    ctx.store[body].evaluate(ctx)?;
                    ctx.loop_iter_count += 1;
                }

                Ok(Value::Unit)
            }

            Value::Loop { body } => loop {
                if ctx.loop_iter_count >= ctx.loop_iter_limit {
                    return Err(EvalFail::LoopLimitExceeded);
                }

                ctx.store[body].evaluate(ctx)?;
                ctx.loop_iter_count += 1;
            },

            Value::Break { label } => {
                // TODO: Evaluate break expressions
                todo!()
            }

            Value::Continue { label } => {
                // TODO: Evaluate continue expressions
                todo!()
            }

            Value::Return { value } => {
                // TODO: Evaluate return expressions
                todo!()
            }

            Value::Call { callee, arguments } => {
                // TODO: Evaluate function call expressions
                todo!()
            }
        }
    }
}
