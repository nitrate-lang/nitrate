use crate::{EvalFail, HirEvaluate};
use nitrate_hir::prelude::*;

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

        // TODO:

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
                todo!()
            }

            Value::Unary { op, expr } => {
                todo!()
            }

            Value::Symbol { symbol } => {
                todo!()
            }

            Value::FieldAccess { expr, field } => {
                todo!()
            }

            Value::ArrayIndex { expr, index } => {
                todo!()
            }

            Value::Assign { place, value } => {
                todo!()
            }

            Value::Deref { place } => {
                todo!()
            }

            Value::GetAddressOf { place } => {
                todo!()
            }

            Value::Cast { expr, to } => {
                todo!()
            }

            Value::GetTypeOf { expr } => {
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

            Value::Loop { body } => {
                todo!()
            }

            Value::Break { label } => {
                todo!()
            }

            Value::Continue { label } => {
                todo!()
            }

            Value::Return { value } => {
                todo!()
            }

            Value::Call { callee, arguments } => {
                todo!()
            }
        }
    }
}
