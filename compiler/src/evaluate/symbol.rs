use super::abstract_machine::{AbstractMachine, Unwind};
use crate::parsetree::{
    Builder, Expr,
    nodes::{Function, Scope, Variable},
};
use std::rc::Rc;

impl<'a> AbstractMachine<'a> {
    pub(crate) fn evaluate_function(
        &mut self,
        _function: Rc<Function<'a>>,
    ) -> Result<Expr<'a>, Unwind<'a>> {
        // TODO: Implement function evaluation
        unimplemented!()
    }

    pub(crate) fn evaluate_variable(
        &mut self,
        variable: &Variable<'a>,
    ) -> Result<Expr<'a>, Unwind<'a>> {
        if self.current_task().in_function() {
            let initializer = variable.value().ok_or(Unwind::TypeError)?;
            let value = self.evaluate(initializer)?;

            self.current_task_mut()
                .callstack_mut()
                .last_mut()
                .unwrap()
                .set(variable.name(), value);
        } else {
            let initializer = variable.value().ok_or(Unwind::TypeError)?;
            let value = self.evaluate(initializer)?;

            self.current_task_mut()
                .add_task_local(variable.name(), value);
        }

        Ok(Builder::create_unit())
    }

    pub(crate) fn evaluate_identifier(&mut self, _name: &'a str) -> Result<Expr<'a>, Unwind<'a>> {
        // TODO: Implement identifier evaluation
        unimplemented!()
    }

    pub(crate) fn evaluate_scope(&mut self, scope: &Scope<'a>) -> Result<Expr<'a>, Unwind<'a>> {
        for element in scope.elements() {
            self.evaluate(element)?;
        }

        Ok(Builder::create_unit())
    }
}
