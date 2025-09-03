use super::abstract_machine::{AbstractMachine, Unwind};
use crate::parsetree::{
    Expr,
    nodes::{Function, Scope, Variable},
};

impl<'a> AbstractMachine<'a> {
    pub(crate) fn evaluate_function(
        &mut self,
        _function: &Function<'a>,
    ) -> Result<Expr<'a>, Unwind<'a>> {
        // TODO: Implement function evaluation
        unimplemented!()
    }

    pub(crate) fn evaluate_variable(
        &mut self,
        _variable: &Variable<'a>,
    ) -> Result<Expr<'a>, Unwind<'a>> {
        // TODO: Implement variable evaluation
        unimplemented!()
    }

    pub(crate) fn evaluate_identifier(
        &mut self,
        _name: &'a str,
    ) -> Result<Expr<'a>, Unwind<'a>> {
        // TODO: Implement identifier evaluation
        unimplemented!()
    }

    pub(crate) fn evaluate_scope(
        &mut self,
        _scope: &Scope<'a>,
    ) -> Result<Expr<'a>, Unwind<'a>> {
        // TODO: Implement scope evaluation
        unimplemented!()
    }
}
