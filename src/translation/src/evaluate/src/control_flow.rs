use super::abstract_machine::{AbstractMachine, CallFrame, IntrinsicFunction, Unwind};
use nitrate_structure::{
    Builder,
    kind::{
        Await, Break, Call, Continue, DoWhileLoop, Expr, ForEach, Function, If, Return, Switch,
        WhileLoop,
    },
};
use std::sync::{Arc, RwLock};

impl AbstractMachine {
    pub(crate) fn evaluate_if(&mut self, if_expr: &If) -> Result<Expr, Unwind> {
        // TODO: Verify and write tests

        if let Expr::Boolean(true) = self.evaluate(if_expr.condition())? {
            self.evaluate(if_expr.then_branch())
        } else if let Some(else_branch) = if_expr.else_branch() {
            self.evaluate(else_branch)
        } else {
            Ok(Builder::create_unit())
        }
    }

    pub(crate) fn evaluate_while(&mut self, while_loop: &WhileLoop) -> Result<Expr, Unwind> {
        // TODO: Verify and write tests

        loop {
            match self.evaluate(while_loop.condition())? {
                Expr::Boolean(true) => {
                    self.evaluate(while_loop.body())?;
                }

                _ => break,
            }
        }

        Ok(Builder::create_unit())
    }

    pub(crate) fn evaluate_do_while(
        &mut self,
        do_while_loop: &DoWhileLoop,
    ) -> Result<Expr, Unwind> {
        // TODO: Verify and write tests

        self.evaluate(do_while_loop.body())?;

        loop {
            match self.evaluate(do_while_loop.condition())? {
                Expr::Boolean(true) => {
                    self.evaluate(do_while_loop.body())?;
                }

                _ => break,
            }
        }

        Ok(Builder::create_unit())
    }

    pub(crate) fn evaluate_switch(&mut self, _switch_expr: &Switch) -> Result<Expr, Unwind> {
        // TODO: Verify and write tests

        // TODO: Evaluate switch
        unimplemented!()
    }

    pub(crate) fn evaluate_break(&mut self, _break: &Break) -> Result<Expr, Unwind> {
        // TODO: Verify and write tests

        // TODO: Evaluate break
        unimplemented!()
    }

    pub(crate) fn evaluate_continue(&mut self, _continue: &Continue) -> Result<Expr, Unwind> {
        // TODO: Verify and write tests

        // TODO: Evaluate continue
        unimplemented!()
    }

    pub(crate) fn evaluate_return(&mut self, return_: &Return) -> Result<Expr, Unwind> {
        // TODO: Verify and write tests

        let return_value = return_
            .value()
            .map_or(Ok(Builder::create_unit()), |v| self.evaluate(v))?;

        // Not actually an error, this just propagates the return value
        Err(Unwind::FunctionReturn(return_value))
    }

    pub(crate) fn evaluate_for_each(&mut self, _for_each: &ForEach) -> Result<Expr, Unwind> {
        // TODO: Verify and write tests

        // TODO: Evaluate for_each
        unimplemented!()
    }

    pub(crate) fn evaluate_await(&mut self, _await: &Await) -> Result<Expr, Unwind> {
        // TODO: Verify and write tests

        // TODO: Evaluate await
        unimplemented!()
    }

    pub(crate) fn evaluate_call(&mut self, call: &Call) -> Result<Expr, Unwind> {
        // TODO: Verify and write tests

        enum Callee {
            FunctionCode(Arc<RwLock<Function>>),
            Intrinsic(IntrinsicFunction),
        }

        let callee = match self.evaluate(call.callee()) {
            Ok(Expr::Function(function)) => Ok(Callee::FunctionCode(function)),

            Err(Unwind::UnresolvedIdentifier(callee_name)) => {
                if let Some(intrinsic) = self.resolve_intrinsic(&callee_name) {
                    Ok(Callee::Intrinsic(intrinsic.to_owned()))
                } else {
                    Err(Unwind::UnknownCallee(callee_name))
                }
            }

            Err(e) => Err(e),
            _ => Err(Unwind::TypeError),
        }?;

        let mut callframe = CallFrame::default();

        for (provided_name, value) in call.arguments() {
            let name = provided_name.to_owned().ok_or(Unwind::MissingArgument)?;
            callframe.set(name, self.evaluate(value)?);
        }

        self.current_task_mut().callstack_mut().push(callframe);

        let result = match callee {
            Callee::FunctionCode(function) => self.evaluate(
                function
                    .read()
                    .unwrap()
                    .definition()
                    .ok_or(Unwind::TypeError)?,
            ),
            Callee::Intrinsic(intrinsic) => intrinsic(self),
        };

        self.current_task_mut().callstack_mut().pop();

        match result {
            Err(Unwind::FunctionReturn(value)) => Ok(value),
            Ok(value) => Ok(value),
            Err(e) => Err(e),
        }
    }
}
