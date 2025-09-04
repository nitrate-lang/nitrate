use super::abstract_machine::{AbstractMachine, Unwind};
use nitrate_parsetree::{
    kind::{Expr, Identifier, Scope, Variable},
    Builder,
};

impl<'a> AbstractMachine<'a> {
    pub(crate) fn evaluate_variable(
        &mut self,
        variable: &Variable<'a>,
    ) -> Result<Expr<'a>, Unwind<'a>> {
        // TODO: Write tests
        // TODO: Verify logic

        // FIXME: Properly handle variable storage class attributes
        // TODO: Analyze and implement interaction with closures

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

    pub(crate) fn evaluate_identifier(
        &mut self,
        identifier: &Identifier<'a>,
    ) -> Result<Expr<'a>, Unwind<'a>> {
        // TODO: Write tests
        // TODO: Verify logic

        // FIXME: What about lvalue vs rvalue?

        self.resolve(identifier.full_name())
            .cloned()
            .ok_or(Unwind::UnresolvedIdentifier(
                identifier.full_name().to_string(),
            ))
    }

    pub(crate) fn evaluate_scope(&mut self, scope: &Scope<'a>) -> Result<Expr<'a>, Unwind<'a>> {
        // TODO: Write tests
        // TODO: Verify logic

        for element in scope.elements() {
            self.evaluate(element)?;
        }

        Ok(Builder::create_unit())
    }
}
