use crate::ValidateHir;
use nitrate_hir::prelude::*;

impl ValidateHir for BlockElement {
    fn validate(&self, store: &Store) -> Result<(), ()> {
        match self {
            BlockElement::Expr(expr) => store[expr].borrow().validate(store),
            BlockElement::Stmt(expr) => store[expr].borrow().validate(store),
            BlockElement::Local(local) => store[local].borrow().validate(store),
        }
    }
}

impl ValidateHir for Block {
    fn validate(&self, store: &Store) -> Result<(), ()> {
        for (i, elem) in self.elements.iter().enumerate() {
            let is_last = i == self.elements.len() - 1;
            if matches!(elem, BlockElement::Expr(_)) && !is_last {
                return Err(());
            }

            elem.validate(store)?;
        }

        Ok(())
    }
}

impl ValidateHir for Value {
    fn validate(&self, _store: &Store) -> Result<(), ()> {
        // TODO: Validate expressions
        unimplemented!()
    }
}
