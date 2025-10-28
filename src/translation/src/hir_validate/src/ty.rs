use crate::{ValidHir, ValidateHir};
use nitrate_hir::prelude::*;

impl ValidateHir for StructAttribute {
    fn verify(&self, _store: &Store) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store) -> Result<ValidHir<Self>, ()> {
        self.verify(store)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for StructFieldAttribute {
    fn verify(&self, _store: &Store) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store) -> Result<ValidHir<Self>, ()> {
        self.verify(store)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for StructField {
    fn verify(&self, _store: &Store) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store) -> Result<ValidHir<Self>, ()> {
        self.verify(store)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for StructType {
    fn verify(&self, _store: &Store) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store) -> Result<ValidHir<Self>, ()> {
        self.verify(store)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for EnumAttribute {
    fn verify(&self, _store: &Store) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store) -> Result<ValidHir<Self>, ()> {
        self.verify(store)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for EnumVariantAttribute {
    fn verify(&self, _store: &Store) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store) -> Result<ValidHir<Self>, ()> {
        self.verify(store)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for EnumVariant {
    fn verify(&self, _store: &Store) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store) -> Result<ValidHir<Self>, ()> {
        self.verify(store)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for EnumType {
    fn verify(&self, _store: &Store) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store) -> Result<ValidHir<Self>, ()> {
        self.verify(store)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for FunctionAttribute {
    fn verify(&self, _store: &Store) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store) -> Result<ValidHir<Self>, ()> {
        self.verify(store)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for FunctionType {
    fn verify(&self, _store: &Store) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store) -> Result<ValidHir<Self>, ()> {
        self.verify(store)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for Type {
    fn verify(&self, _store: &Store) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store) -> Result<ValidHir<Self>, ()> {
        self.verify(store)?;
        Ok(ValidHir::new(self))
    }
}
