use super::abstract_machine::*;
use crate::parsetree::{nodes::*, *};

impl<'a> AbstractMachine<'a> {
    pub(crate) fn evaluate_if(&mut self, if_expr: &If<'a>) -> Result<Expr<'a>, EvalError> {
        let Expr::BooleanLit(b) = self.evaluate(&if_expr.condition())? else {
            return Err(EvalError::TypeError);
        };

        if b {
            self.evaluate(&if_expr.then_branch())
        } else if let Some(else_branch) = if_expr.else_branch() {
            self.evaluate(else_branch)
        } else {
            Ok(Builder::get_unit().into())
        }
    }

    pub(crate) fn evaluate_while(
        &mut self,
        while_loop: &WhileLoop<'a>,
    ) -> Result<Expr<'a>, EvalError> {
        loop {
            match self.evaluate(&while_loop.condition())? {
                Expr::BooleanLit(true) => {
                    self.evaluate(&while_loop.body())?;
                }

                Expr::BooleanLit(false) => break,

                _ => return Err(EvalError::TypeError),
            }
        }

        Ok(Builder::get_unit().into())
    }

    pub(crate) fn evaluate_do_while(
        &mut self,
        do_while_loop: &DoWhileLoop<'a>,
    ) -> Result<Expr<'a>, EvalError> {
        self.evaluate(&do_while_loop.body())?;

        loop {
            match self.evaluate(&do_while_loop.condition())? {
                Expr::BooleanLit(true) => {
                    self.evaluate(&do_while_loop.body())?;
                }

                Expr::BooleanLit(false) => break,
                _ => return Err(EvalError::TypeError),
            }
        }

        Ok(Builder::get_unit().into())
    }
}
