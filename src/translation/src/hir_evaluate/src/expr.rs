use crate::{EvalFail, HirEvaluate};
use nitrate_hir::prelude::*;
use std::ops::{Add, Div, Mul, Neg, Not, Sub};

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
            Value::ISize(i) => Ok(Value::ISize(*i)),
            Value::I128(i) => Ok(Value::I128(i.clone())),
            Value::U8(u) => Ok(Value::U8(*u)),
            Value::U16(u) => Ok(Value::U16(*u)),
            Value::U32(u) => Ok(Value::U32(*u)),
            Value::U64(u) => Ok(Value::U64(*u)),
            Value::USize(u) => Ok(Value::USize(*u)),
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

                    BinaryOp::Mod => {
                        // TODO: Impl op
                        todo!()
                    }

                    BinaryOp::And => {
                        // TODO: Impl op
                        todo!()
                    }

                    BinaryOp::Or => {
                        // TODO: Impl op
                        todo!()
                    }

                    BinaryOp::Xor => {
                        // TODO: Impl op
                        todo!()
                    }

                    BinaryOp::Shl => {
                        // TODO: Impl op
                        todo!()
                    }

                    BinaryOp::Shr => {
                        // TODO: Impl op
                        todo!()
                    }

                    BinaryOp::Rol => {
                        // TODO: Impl op
                        todo!()
                    }

                    BinaryOp::Ror => {
                        // TODO: Impl op
                        todo!()
                    }

                    BinaryOp::LogicAnd => {
                        // TODO: Impl op
                        todo!()
                    }

                    BinaryOp::LogicOr => {
                        // TODO: Impl op
                        todo!()
                    }

                    BinaryOp::LogicXor => {
                        // TODO: Impl op
                        todo!()
                    }

                    BinaryOp::Lt => {
                        // TODO: Impl op
                        todo!()
                    }

                    BinaryOp::Gt => {
                        // TODO: Impl op
                        todo!()
                    }

                    BinaryOp::Lte => {
                        // TODO: Impl op
                        todo!()
                    }

                    BinaryOp::Gte => {
                        // TODO: Impl op
                        todo!()
                    }

                    BinaryOp::Eq => {
                        // TODO: Impl op
                        todo!()
                    }

                    BinaryOp::Ne => {
                        // TODO: Impl op
                        todo!()
                    }
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
