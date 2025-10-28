use crate::{ValidHir, ValidateHir};
use nitrate_hir::{SymbolTab, prelude::*};

impl ValidateHir for GlobalVariableAttribute {
    fn verify(&self, _store: &Store, _symtab: &SymbolTab) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store, symtab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, symtab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for GlobalVariable {
    fn verify(&self, _store: &Store, _symtab: &SymbolTab) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store, symtab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, symtab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for LocalVariableAttribute {
    fn verify(&self, _store: &Store, _symtab: &SymbolTab) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store, symtab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, symtab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for LocalVariable {
    fn verify(&self, _store: &Store, _symtab: &SymbolTab) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store, symtab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, symtab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for ParameterAttribute {
    fn verify(&self, _store: &Store, _symtab: &SymbolTab) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store, symtab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, symtab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for Parameter {
    fn verify(&self, _store: &Store, _symtab: &SymbolTab) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store, symtab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, symtab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for Function {
    fn verify(&self, store: &Store, symtab: &SymbolTab) -> Result<(), ()> {
        store[&self.return_type].verify(store, symtab)?;

        for param in &self.params {
            store[param].borrow().verify(store, symtab)?;
        }

        if let Some(body) = &self.body {
            store[body].borrow().verify(store, symtab)?;
        }

        Ok(())
    }

    fn validate(self, store: &Store, symtab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, symtab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for Trait {
    fn verify(&self, _store: &Store, _symtab: &SymbolTab) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store, symtab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, symtab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for ModuleAttribute {
    fn verify(&self, _store: &Store, _symtab: &SymbolTab) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store, symtab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, symtab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for Module {
    fn verify(&self, store: &Store, symtab: &SymbolTab) -> Result<(), ()> {
        for attr in &self.attributes {
            attr.verify(store, symtab)?;
        }

        for item in &self.items {
            item.verify(store, symtab)?;
        }

        Ok(())
    }

    fn validate(self, store: &Store, symtab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, symtab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for TypeAliasDef {
    fn verify(&self, store: &Store, symtab: &SymbolTab) -> Result<(), ()> {
        store[&self.type_id].verify(store, symtab)
    }

    fn validate(self, store: &Store, symtab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, symtab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for StructDef {
    fn verify(&self, _store: &Store, _symtab: &SymbolTab) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store, symtab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, symtab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for EnumDef {
    fn verify(&self, _store: &Store, _symtab: &SymbolTab) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store, symtab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, symtab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for TypeDefinition {
    fn verify(&self, _store: &Store, _symtab: &SymbolTab) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store, symtab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, symtab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for SymbolId {
    fn verify(&self, _store: &Store, _symtab: &SymbolTab) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store, symtab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, symtab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for Item {
    fn verify(&self, store: &Store, symtab: &SymbolTab) -> Result<(), ()> {
        match self {
            Item::Module(id) => store[id].borrow().verify(store, symtab),
            Item::GlobalVariable(id) => store[id].borrow().verify(store, symtab),
            Item::Function(id) => store[id].borrow().verify(store, symtab),
            Item::TypeAliasDef(id) => store[id].borrow().verify(store, symtab),
            Item::StructDef(id) => store[id].borrow().verify(store, symtab),
            Item::EnumDef(id) => store[id].borrow().verify(store, symtab),
        }
    }

    fn validate(self, store: &Store, symtab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, symtab)?;
        Ok(ValidHir::new(self))
    }
}
