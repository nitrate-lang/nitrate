use super::abstract_machine::{AbstractMachine, Unwind};
use interned_string::Intern;
use nitrate_structure::{
    Builder,
    kind::{BinExpr, Block, Expr, List, Object, Statement, Type, UnaryExpr},
};
use std::collections::BTreeMap;

impl AbstractMachine {
    pub(crate) fn evaluate_type_envelop(&mut self, content: &Type) -> Result<Expr, Unwind> {
        let evaluated_type = self.evaluate_type(content)?;

        Ok(Builder::create_object()
            .add_field("inner".intern(), evaluated_type.into())
            .build())
    }

    pub(crate) fn evaluate_list(&mut self, list: &List) -> Result<Expr, Unwind> {
        // TODO: Verify and write tests

        let mut elements = Vec::new();
        elements.reserve(list.elements().len());

        for element in list.elements() {
            elements.push(self.evaluate(element)?);
        }

        Ok(Builder::create_list().add_elements(elements).build())
    }

    pub(crate) fn evaluate_object(&mut self, object: &Object) -> Result<Expr, Unwind> {
        // TODO: Verify and write tests

        let mut fields = BTreeMap::new();

        for (key, value) in object.fields() {
            let evaluated_value = self.evaluate(value)?;
            fields.insert(key.to_owned(), evaluated_value);
        }

        Ok(Builder::create_object().add_fields(fields).build())
    }

    pub(crate) fn evaluate_unaryexpr(&mut self, _uexpr: &UnaryExpr) -> Result<Expr, Unwind> {
        // TODO: Verify and write tests

        // TODO: Evaluate unary expression
        unimplemented!()
    }

    pub(crate) fn evaluate_binexpr(&mut self, _bexpr: &BinExpr) -> Result<Expr, Unwind> {
        // TODO: Verify and write tests

        // TODO: Evaluate binary expression
        unimplemented!()
    }

    pub(crate) fn evaluate_statement(&mut self, statement: &Statement) -> Result<Expr, Unwind> {
        // TODO: Verify and write tests

        self.evaluate(&statement.get())?;
        Ok(Builder::create_unit())
    }

    pub(crate) fn evaluate_block(&mut self, block: &Block) -> Result<Expr, Unwind> {
        // TODO: Verify and write tests

        let mut result = None;
        for element in block.elements() {
            result = Some(self.evaluate(element)?);
        }

        Ok(result.unwrap_or_else(Builder::create_unit))
    }
}
