use crate::{ValidHir, ValidateHir};
use nitrate_hir::prelude::*;

impl ValidateHir for BlockElement {
    fn verify(&self, store: &Store) -> Result<(), ()> {
        match self {
            BlockElement::Expr(expr) => store[expr].borrow().verify(store),
            BlockElement::Stmt(expr) => store[expr].borrow().verify(store),
            BlockElement::Local(local) => store[local].borrow().verify(store),
        }
    }

    fn validate(self, store: &Store) -> Result<ValidHir<Self>, ()> {
        self.verify(store)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for Block {
    fn verify(&self, store: &Store) -> Result<(), ()> {
        for (i, elem) in self.elements.iter().enumerate() {
            let is_last = i == self.elements.len() - 1;
            if matches!(elem, BlockElement::Expr(_)) && !is_last {
                return Err(());
            }

            elem.verify(store)?;
        }

        Ok(())
    }

    fn validate(self, store: &Store) -> Result<ValidHir<Self>, ()> {
        self.verify(store)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for Value {
    fn verify(&self, _store: &Store) -> Result<(), ()> {
        // TODO: Validate expressions
        unimplemented!()
    }

    fn validate(self, store: &Store) -> Result<ValidHir<Self>, ()> {
        self.verify(store)?;
        Ok(ValidHir::new(self))
    }
}
