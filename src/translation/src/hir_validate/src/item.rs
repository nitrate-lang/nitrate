use crate::{ValidHir, ValidateHir};
use nitrate_hir::{SymbolTab, prelude::*};
use nitrate_hir_get_type::HirGetType;

impl ValidateHir for GlobalVariableAttribute {
    fn verify(&self, _store: &Store, _tab: &SymbolTab) -> Result<(), ()> {
        match self {
            GlobalVariableAttribute::NoMangle => Ok(()),
        }
    }

    fn validate(self, store: &Store, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for GlobalVariable {
    fn verify(&self, store: &Store, tab: &SymbolTab) -> Result<(), ()> {
        for attr in &self.attributes {
            attr.verify(store, tab)?;
        }

        let ty = &store[&self.ty];
        let init = store[&self.init].borrow();

        ty.verify(store, tab)?;
        init.verify(store, tab)?;

        let init_ty = init.get_type(store, tab).map_err(|_| ())?;
        if *ty != init_ty {
            return Err(());
        }

        Ok(())
    }

    fn validate(self, store: &Store, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for LocalVariableAttribute {
    fn verify(&self, _store: &Store, _tab: &SymbolTab) -> Result<(), ()> {
        match self {
            LocalVariableAttribute::Invalid => return Err(()),
        }
    }

    fn validate(self, store: &Store, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for LocalVariable {
    fn verify(&self, store: &Store, tab: &SymbolTab) -> Result<(), ()> {
        for attr in &self.attributes {
            attr.verify(store, tab)?;
        }

        let ty = &store[&self.ty];
        ty.verify(store, tab)?;

        if let Some(init_expr) = &self.init {
            let init = store[init_expr].borrow();
            init.verify(store, tab)?;

            let init_ty = init.get_type(store, tab).map_err(|_| ())?;
            if *ty != init_ty {
                return Err(());
            }
        }

        Ok(())
    }

    fn validate(self, store: &Store, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for ParameterAttribute {
    fn verify(&self, _store: &Store, _tab: &SymbolTab) -> Result<(), ()> {
        match self {
            ParameterAttribute::Invalid => return Err(()),
        }
    }

    fn validate(self, store: &Store, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for Parameter {
    fn verify(&self, store: &Store, tab: &SymbolTab) -> Result<(), ()> {
        for attr in &self.attributes {
            attr.verify(store, tab)?;
        }

        let ty = &store[&self.ty];
        ty.verify(store, tab)?;

        if let Some(default_value) = &self.default_value {
            let init = store[default_value].borrow();
            init.verify(store, tab)?;

            let init_ty = init.get_type(store, tab).map_err(|_| ())?;
            if *ty != init_ty {
                return Err(());
            }
        }

        Ok(())
    }

    fn validate(self, store: &Store, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for Function {
    fn verify(&self, store: &Store, tab: &SymbolTab) -> Result<(), ()> {
        for attr in &self.attributes {
            attr.verify(store, tab)?;
        }

        for param in &self.params {
            store[param].borrow().verify(store, tab)?;
        }

        store[&self.return_type].verify(store, tab)?;

        if let Some(body) = &self.body {
            store[body].borrow().verify(store, tab)?;
        }

        Ok(())
    }

    fn validate(self, store: &Store, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for Trait {
    fn verify(&self, _store: &Store, _tab: &SymbolTab) -> Result<(), ()> {
        // TODO: verify trait
        unimplemented!()
    }

    fn validate(self, store: &Store, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for ModuleAttribute {
    fn verify(&self, _store: &Store, _tab: &SymbolTab) -> Result<(), ()> {
        match self {
            ModuleAttribute::Invalid => return Err(()),
        }
    }

    fn validate(self, store: &Store, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for Module {
    fn verify(&self, store: &Store, tab: &SymbolTab) -> Result<(), ()> {
        for attr in &self.attributes {
            attr.verify(store, tab)?;
        }

        for item in &self.items {
            item.verify(store, tab)?;
        }

        Ok(())
    }

    fn validate(self, store: &Store, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for TypeAliasDef {
    fn verify(&self, store: &Store, tab: &SymbolTab) -> Result<(), ()> {
        store[&self.type_id].verify(store, tab)
    }

    fn validate(self, store: &Store, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for StructDef {
    fn verify(&self, store: &Store, tab: &SymbolTab) -> Result<(), ()> {
        for (_, default_value) in &self.field_extras {
            if let Some(expr) = default_value {
                store[expr].borrow().verify(store, tab)?;
            }
        }

        store[&self.struct_id].verify(store, tab)?;

        Ok(())
    }

    fn validate(self, store: &Store, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for EnumDef {
    fn verify(&self, store: &Store, tab: &SymbolTab) -> Result<(), ()> {
        for value in &self.variant_extras {
            if let Some(expr) = value {
                store[expr].borrow().verify(store, tab)?;
            }
        }

        store[&self.enum_id].verify(store, tab)?;

        Ok(())
    }

    fn validate(self, store: &Store, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for Item {
    fn verify(&self, store: &Store, tab: &SymbolTab) -> Result<(), ()> {
        match self {
            Item::Module(id) => store[id].borrow().verify(store, tab),
            Item::GlobalVariable(id) => store[id].borrow().verify(store, tab),
            Item::Function(id) => store[id].borrow().verify(store, tab),
            Item::TypeAliasDef(id) => store[id].borrow().verify(store, tab),
            Item::StructDef(id) => store[id].borrow().verify(store, tab),
            Item::EnumDef(id) => store[id].borrow().verify(store, tab),
        }
    }

    fn validate(self, store: &Store, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, tab)?;
        Ok(ValidHir::new(self))
    }
}
