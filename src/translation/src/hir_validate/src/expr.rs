use crate::{ValidHir, ValidateHir};
use nitrate_hir::{SymbolTab, prelude::*};

impl ValidateHir for BlockElement {
    fn verify(&self, store: &Store, symtab: &SymbolTab) -> Result<(), ()> {
        match self {
            BlockElement::Expr(expr) => store[expr].borrow().verify(store, symtab),
            BlockElement::Stmt(expr) => store[expr].borrow().verify(store, symtab),
            BlockElement::Local(local) => store[local].borrow().verify(store, symtab),
        }
    }

    fn validate(self, store: &Store, symtab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, symtab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for Block {
    fn verify(&self, store: &Store, symtab: &SymbolTab) -> Result<(), ()> {
        for (i, elem) in self.elements.iter().enumerate() {
            let is_last = i == self.elements.len() - 1;
            if matches!(elem, BlockElement::Expr(_)) && !is_last {
                return Err(());
            }

            elem.verify(store, symtab)?;
        }

        Ok(())
    }

    fn validate(self, store: &Store, symtab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, symtab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for Value {
    fn verify(&self, _store: &Store, _symtab: &SymbolTab) -> Result<(), ()> {
        // TODO: Validate expressions
        unimplemented!()
    }

    fn validate(self, store: &Store, symtab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, symtab)?;
        Ok(ValidHir::new(self))
    }
}
