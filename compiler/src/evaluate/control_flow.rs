use super::abstract_machine::{AbstractMachine, EvalError};
use crate::parsetree::{
    Builder, Expr,
    nodes::{Assert, Await, Break, Continue, DoWhileLoop, ForEach, If, Return, Switch, WhileLoop},
};

impl<'a> AbstractMachine<'a> {
    pub(crate) fn evaluate_if(&mut self, if_expr: &If<'a>) -> Result<Expr<'a>, EvalError> {
        if let Expr::BooleanLit(true) = self.evaluate(if_expr.condition())? {
            self.evaluate(if_expr.then_branch())
        } else if let Some(else_branch) = if_expr.else_branch() {
            self.evaluate(else_branch)
        } else {
            Ok(Builder::create_unit())
        }
    }

    pub(crate) fn evaluate_while(
        &mut self,
        while_loop: &WhileLoop<'a>,
    ) -> Result<Expr<'a>, EvalError> {
        loop {
            match self.evaluate(while_loop.condition())? {
                Expr::BooleanLit(true) => {
                    self.evaluate(while_loop.body())?;
                }

                _ => break,
            }
        }

        Ok(Builder::create_unit())
    }

    pub(crate) fn evaluate_do_while(
        &mut self,
        do_while_loop: &DoWhileLoop<'a>,
    ) -> Result<Expr<'a>, EvalError> {
        self.evaluate(do_while_loop.body())?;

        loop {
            match self.evaluate(do_while_loop.condition())? {
                Expr::BooleanLit(true) => {
                    self.evaluate(do_while_loop.body())?;
                }

                _ => break,
            }
        }

        Ok(Builder::create_unit())
    }

    pub(crate) fn evaluate_switch(
        &mut self,
        _switch_expr: &Switch<'a>,
    ) -> Result<Expr<'a>, EvalError> {
        // TODO: Evaluate switch
        unimplemented!()
    }

    pub(crate) fn evaluate_break(&mut self, _break: &Break<'a>) -> Result<Expr<'a>, EvalError> {
        // TODO: Evaluate break
        unimplemented!()
    }

    pub(crate) fn evaluate_continue(
        &mut self,
        _continue: &Continue<'a>,
    ) -> Result<Expr<'a>, EvalError> {
        // TODO: Evaluate continue
        unimplemented!()
    }

    pub(crate) fn evaluate_return(&mut self, _return: &Return<'a>) -> Result<Expr<'a>, EvalError> {
        // TODO: Evaluate return
        unimplemented!()
    }

    pub(crate) fn evaluate_for_each(
        &mut self,
        _for_each: &ForEach<'a>,
    ) -> Result<Expr<'a>, EvalError> {
        // TODO: Evaluate for_each
        unimplemented!()
    }

    pub(crate) fn evaluate_await(&mut self, _await: &Await<'a>) -> Result<Expr<'a>, EvalError> {
        // TODO: Evaluate await
        unimplemented!()
    }

    pub(crate) fn evaluate_assert(&mut self, assert: &Assert<'a>) -> Result<Expr<'a>, EvalError> {
        let condition = self.evaluate(assert.condition())?;
        let message = self.evaluate(assert.message())?;

        match condition {
            Expr::BooleanLit(true) => Ok(Builder::create_unit()),

            _ => {
                let message_string = match message {
                    Expr::StringLit(s) => s.get().to_string(),
                    _ => return Err(EvalError::TypeError),
                };

                Err(EvalError::ProgramaticAssertionFailed(message_string))
            }
        }
    }
}
