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
    fn verify(&self, _store: &Store) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
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
    fn verify(&self, _store: &Store) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store) -> Result<ValidHir<Self>, ()> {
        self.verify(store)?;
        Ok(ValidHir::new(self))
    }
}
