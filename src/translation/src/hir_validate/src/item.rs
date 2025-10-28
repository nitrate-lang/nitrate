use crate::{ValidHir, ValidateHir};
use nitrate_hir::prelude::*;

impl ValidateHir for GlobalVariableAttribute {
    fn verify(&self, _store: &Store) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store) -> Result<ValidHir<Self>, ()> {
        self.verify(store)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for GlobalVariable {
    fn verify(&self, _store: &Store) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store) -> Result<ValidHir<Self>, ()> {
        self.verify(store)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for LocalVariableAttribute {
    fn verify(&self, _store: &Store) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store) -> Result<ValidHir<Self>, ()> {
        self.verify(store)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for LocalVariable {
    fn verify(&self, _store: &Store) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store) -> Result<ValidHir<Self>, ()> {
        self.verify(store)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for ParameterAttribute {
    fn verify(&self, _store: &Store) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store) -> Result<ValidHir<Self>, ()> {
        self.verify(store)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for Parameter {
    fn verify(&self, _store: &Store) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store) -> Result<ValidHir<Self>, ()> {
        self.verify(store)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for Function {
    fn verify(&self, _store: &Store) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store) -> Result<ValidHir<Self>, ()> {
        self.verify(store)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for Trait {
    fn verify(&self, _store: &Store) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store) -> Result<ValidHir<Self>, ()> {
        self.verify(store)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for ModuleAttribute {
    fn verify(&self, _store: &Store) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store) -> Result<ValidHir<Self>, ()> {
        self.verify(store)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for Module {
    fn verify(&self, store: &Store) -> Result<(), ()> {
        for attr in &self.attributes {
            attr.verify(store)?;
        }

        for item in &self.items {
            item.verify(store)?;
        }

        Ok(())
    }

    fn validate(self, store: &Store) -> Result<ValidHir<Self>, ()> {
        self.verify(store)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for TypeAliasDef {
    fn verify(&self, _store: &Store) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store) -> Result<ValidHir<Self>, ()> {
        self.verify(store)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for StructDef {
    fn verify(&self, _store: &Store) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store) -> Result<ValidHir<Self>, ()> {
        self.verify(store)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for EnumDef {
    fn verify(&self, _store: &Store) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store) -> Result<ValidHir<Self>, ()> {
        self.verify(store)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for TypeDefinition {
    fn verify(&self, _store: &Store) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store) -> Result<ValidHir<Self>, ()> {
        self.verify(store)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for SymbolId {
    fn verify(&self, _store: &Store) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store) -> Result<ValidHir<Self>, ()> {
        self.verify(store)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for Item {
    fn verify(&self, store: &Store) -> Result<(), ()> {
        match self {
            Item::Module(id) => store[id].borrow().verify(store),
            Item::GlobalVariable(id) => store[id].borrow().verify(store),
            Item::Function(id) => store[id].borrow().verify(store),
            Item::TypeAliasDef(id) => store[id].borrow().verify(store),
            Item::StructDef(id) => store[id].borrow().verify(store),
            Item::EnumDef(id) => store[id].borrow().verify(store),
        }
    }

    fn validate(self, store: &Store) -> Result<ValidHir<Self>, ()> {
        self.verify(store)?;
        Ok(ValidHir::new(self))
    }
}
