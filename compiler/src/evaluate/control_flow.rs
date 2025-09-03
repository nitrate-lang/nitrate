use crate::{
    evaluate::abstract_machine::{AbstractMachine, CallFrame, Unwind},
    parsetree::{
        Builder, Expr,
        nodes::{
            Assert, Await, Break, Call, Continue, DoWhileLoop, ForEach, If, Return, Switch,
            WhileLoop,
        },
    },
};

impl<'a> AbstractMachine<'a> {
    pub(crate) fn evaluate_if(&mut self, if_expr: &If<'a>) -> Result<Expr<'a>, Unwind<'a>> {
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
    ) -> Result<Expr<'a>, Unwind<'a>> {
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
    ) -> Result<Expr<'a>, Unwind<'a>> {
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
    ) -> Result<Expr<'a>, Unwind<'a>> {
        // TODO: Evaluate switch
        unimplemented!()
    }

    pub(crate) fn evaluate_break(&mut self, _break: &Break<'a>) -> Result<Expr<'a>, Unwind<'a>> {
        // TODO: Evaluate break
        unimplemented!()
    }

    pub(crate) fn evaluate_continue(
        &mut self,
        _continue: &Continue<'a>,
    ) -> Result<Expr<'a>, Unwind<'a>> {
        // TODO: Evaluate continue
        unimplemented!()
    }

    pub(crate) fn evaluate_return(&mut self, return_: &Return<'a>) -> Result<Expr<'a>, Unwind<'a>> {
        let return_value = return_
            .value()
            .map_or(Ok(Builder::create_unit()), |v| self.evaluate(v))?;

        // Not actually an error, this just propagates the return value
        Err(Unwind::FunctionReturn(return_value))
    }

    pub(crate) fn evaluate_for_each(
        &mut self,
        _for_each: &ForEach<'a>,
    ) -> Result<Expr<'a>, Unwind<'a>> {
        // TODO: Evaluate for_each
        unimplemented!()
    }

    pub(crate) fn evaluate_await(&mut self, _await: &Await<'a>) -> Result<Expr<'a>, Unwind<'a>> {
        // TODO: Evaluate await
        unimplemented!()
    }

    pub(crate) fn evaluate_assert(&mut self, assert: &Assert<'a>) -> Result<Expr<'a>, Unwind<'a>> {
        let condition = self.evaluate(assert.condition())?;
        let message = self.evaluate(assert.message())?;

        match condition {
            Expr::BooleanLit(true) => Ok(Builder::create_unit()),

            _ => {
                let message_string = match message {
                    Expr::StringLit(s) => s.get().to_string(),
                    _ => return Err(Unwind::TypeError),
                };

                Err(Unwind::ProgramaticAssertionFailed(message_string))
            }
        }
    }

    pub(crate) fn evaluate_call(&mut self, call: &Call<'a>) -> Result<Expr<'a>, Unwind<'a>> {
        let mut callframe = CallFrame::default();

        for (provided_name, value) in call.arguments() {
            let name = provided_name.ok_or(Unwind::MissingArgument)?;
            callframe.set(name, self.evaluate(value)?);
        }

        self.tasks[self.current_task].call_stack.push(callframe);

        let result = match call.callee() {
            Expr::Function(function) if function.is_definition() => {
                self.evaluate(function.definition().unwrap())
            }

            Expr::Identifier(callee_name) => {
                if let Some(intrinsic) = self.resolve_intrinsic(callee_name) {
                    intrinsic(self)
                } else {
                    Err(Unwind::UnknownCallee(callee_name))
                }
            }

            _ => {
                return Err(Unwind::UnknownCallee("?"));
            }
        };

        self.tasks[self.current_task].call_stack.pop();

        match result {
            Err(Unwind::FunctionReturn(value)) => Ok(value),
            Ok(value) => Ok(value),
            Err(e) => Err(e),
        }
    }
}
