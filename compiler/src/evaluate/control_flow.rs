use super::abstract_machine::{AbstractMachine, EvalCancel};
use crate::parsetree::{
    Builder, Expr,
    nodes::{
        Assert, Await, Break, Continue, DirectCall, DoWhileLoop, ForEach, If, IndirectCall, Return,
        Switch, WhileLoop,
    },
};

impl<'a> AbstractMachine<'a> {
    pub(crate) fn evaluate_if(&mut self, if_expr: &If<'a>) -> Result<Expr<'a>, EvalCancel<'a>> {
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
    ) -> Result<Expr<'a>, EvalCancel<'a>> {
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
    ) -> Result<Expr<'a>, EvalCancel<'a>> {
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
    ) -> Result<Expr<'a>, EvalCancel<'a>> {
        // TODO: Evaluate switch
        unimplemented!()
    }

    pub(crate) fn evaluate_break(
        &mut self,
        _break: &Break<'a>,
    ) -> Result<Expr<'a>, EvalCancel<'a>> {
        // TODO: Evaluate break
        unimplemented!()
    }

    pub(crate) fn evaluate_continue(
        &mut self,
        _continue: &Continue<'a>,
    ) -> Result<Expr<'a>, EvalCancel<'a>> {
        // TODO: Evaluate continue
        unimplemented!()
    }

    pub(crate) fn evaluate_return(
        &mut self,
        return_: &Return<'a>,
    ) -> Result<Expr<'a>, EvalCancel<'a>> {
        let return_value = return_
            .value()
            .map_or(Ok(Builder::create_unit()), |v| self.evaluate(v))?;

        // Not actually an error, this just propagates the return value
        Err(EvalCancel::FunctionReturn(return_value))
    }

    pub(crate) fn evaluate_for_each(
        &mut self,
        _for_each: &ForEach<'a>,
    ) -> Result<Expr<'a>, EvalCancel<'a>> {
        // TODO: Evaluate for_each
        unimplemented!()
    }

    pub(crate) fn evaluate_await(
        &mut self,
        _await: &Await<'a>,
    ) -> Result<Expr<'a>, EvalCancel<'a>> {
        // TODO: Evaluate await
        unimplemented!()
    }

    pub(crate) fn evaluate_assert(
        &mut self,
        assert: &Assert<'a>,
    ) -> Result<Expr<'a>, EvalCancel<'a>> {
        let condition = self.evaluate(assert.condition())?;
        let message = self.evaluate(assert.message())?;

        match condition {
            Expr::BooleanLit(true) => Ok(Builder::create_unit()),

            _ => {
                let message_string = match message {
                    Expr::StringLit(s) => s.get().to_string(),
                    _ => return Err(EvalCancel::TypeError),
                };

                Err(EvalCancel::ProgramaticAssertionFailed(message_string))
            }
        }
    }

    pub(crate) fn evaluate_direct_call(
        &mut self,
        _direct_call: &DirectCall<'a>,
    ) -> Result<Expr<'a>, EvalCancel<'a>> {
        // TODO: Evaluate direct call
        unimplemented!()
    }

    pub(crate) fn evaluate_indirect_call(
        &mut self,
        _indirect_call: &IndirectCall<'a>,
    ) -> Result<Expr<'a>, EvalCancel<'a>> {
        // TODO: Evaluate indirect call
        unimplemented!()
    }
}
