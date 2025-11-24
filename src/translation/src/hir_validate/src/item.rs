use crate::{ValidHir, ValidateHir};
use nitrate_hir::{SymbolTab, prelude::*};
use nitrate_hir_get_type::HirGetType;
use std::ops::Deref;

impl ValidateHir for GlobalVariableAttribute {
    fn verify(&self, _tab: &SymbolTab) -> Result<(), ()> {
        match self {
            GlobalVariableAttribute::NoMangle => Ok(()),
        }
    }

    fn validate(self, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for GlobalVariable {
    fn verify(&self, tab: &SymbolTab) -> Result<(), ()> {
        for attr in &self.attributes {
            attr.verify(tab)?;
        }

        let ty = self.ty.deref();
        let init = self.init.borrow();

        ty.verify(tab)?;
        init.verify(tab)?;

        let init_ty = init.get_type(tab).map_err(|_| ())?;
        if *ty != init_ty {
            return Err(());
        }

        Ok(())
    }

    fn validate(self, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for LocalVariableAttribute {
    fn verify(&self, _tab: &SymbolTab) -> Result<(), ()> {
        match self {
            LocalVariableAttribute::Invalid => return Err(()),
        }
    }

    fn validate(self, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for LocalVariable {
    fn verify(&self, tab: &SymbolTab) -> Result<(), ()> {
        for attr in &self.attributes {
            attr.verify(tab)?;
        }

        let ty = self.ty.deref();
        ty.verify(tab)?;

        if let Some(init_expr) = &self.init {
            let init = init_expr.borrow();
            init.verify(tab)?;

            let init_ty = init.get_type(tab).map_err(|_| ())?;
            if *ty != init_ty {
                return Err(());
            }
        }

        Ok(())
    }

    fn validate(self, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for ParameterAttribute {
    fn verify(&self, _tab: &SymbolTab) -> Result<(), ()> {
        match self {
            ParameterAttribute::Invalid => return Err(()),
        }
    }

    fn validate(self, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for Parameter {
    fn verify(&self, tab: &SymbolTab) -> Result<(), ()> {
        for attr in &self.attributes {
            attr.verify(tab)?;
        }

        let ty = self.ty.deref();
        ty.verify(tab)?;

        if let Some(default_value) = &self.default_value {
            let init = default_value.borrow();
            init.verify(tab)?;

            let init_ty = init.get_type(tab).map_err(|_| ())?;
            if *ty != init_ty {
                return Err(());
            }
        }

        Ok(())
    }

    fn validate(self, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for Function {
    fn verify(&self, tab: &SymbolTab) -> Result<(), ()> {
        for attr in &self.attributes {
            attr.verify(tab)?;
        }

        for param in &self.params {
            param.borrow().verify(tab)?;
        }

        self.return_type.verify(tab)?;

        if let Some(body) = &self.body {
            body.borrow().verify(tab)?;
        }

        Ok(())
    }

    fn validate(self, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for Trait {
    fn verify(&self, _tab: &SymbolTab) -> Result<(), ()> {
        // TODO: verify trait
        unimplemented!()
    }

    fn validate(self, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for ModuleAttribute {
    fn verify(&self, _tab: &SymbolTab) -> Result<(), ()> {
        match self {
            ModuleAttribute::Invalid => return Err(()),
        }
    }

    fn validate(self, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for Module {
    fn verify(&self, tab: &SymbolTab) -> Result<(), ()> {
        for attr in &self.attributes {
            attr.verify(tab)?;
        }

        for item in &self.items {
            item.verify(tab)?;
        }

        Ok(())
    }

    fn validate(self, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for TypeAliasDef {
    fn verify(&self, tab: &SymbolTab) -> Result<(), ()> {
        self.type_id.verify(tab)
    }

    fn validate(self, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for StructAttribute {
    fn verify(&self, _tab: &SymbolTab) -> Result<(), ()> {
        match self {
            StructAttribute::Packed => Ok(()),
        }
    }

    fn validate(self, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for StructFieldAttribute {
    fn verify(&self, _tab: &SymbolTab) -> Result<(), ()> {
        match self {
            StructFieldAttribute::Invalid => Err(()),
        }
    }

    fn validate(self, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for StructField {
    fn verify(&self, tab: &SymbolTab) -> Result<(), ()> {
        for attr in &self.attributes {
            attr.verify(tab)?;
        }

        self.ty.verify(tab)?;
        if let Some(default_value) = &self.default_value {
            let init = default_value.borrow();
            init.verify(tab)?;

            let init_ty = init.get_type(tab).map_err(|_| ())?;
            let field_ty = self.ty.deref();
            if *field_ty != init_ty {
                return Err(());
            }
        }

        Ok(())
    }

    fn validate(self, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for StructDef {
    fn verify(&self, tab: &SymbolTab) -> Result<(), ()> {
        for attr in &self.attributes {
            attr.verify(tab)?;
        }

        for (_, field) in &self.fields {
            field.verify(tab)?;
        }

        Ok(())
    }

    fn validate(self, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for EnumAttribute {
    fn verify(&self, _tab: &SymbolTab) -> Result<(), ()> {
        match self {
            EnumAttribute::Invalid => Err(()),
        }
    }

    fn validate(self, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for EnumVariantAttribute {
    fn verify(&self, _tab: &SymbolTab) -> Result<(), ()> {
        match self {
            EnumVariantAttribute::Invalid => Err(()),
        }
    }

    fn validate(self, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for EnumVariant {
    fn verify(&self, tab: &SymbolTab) -> Result<(), ()> {
        for attr in &self.attributes {
            attr.verify(tab)?;
        }

        self.ty.verify(tab)
    }

    fn validate(self, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for EnumDef {
    fn verify(&self, tab: &SymbolTab) -> Result<(), ()> {
        for value in &self.variant_extras {
            if let Some(expr) = value {
                expr.borrow().verify(tab)?;
            }
        }

        for attr in &self.attributes {
            attr.verify(tab)?;
        }

        for variant in &self.variants {
            variant.verify(tab)?;
        }

        Ok(())
    }

    fn validate(self, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for Item {
    fn verify(&self, tab: &SymbolTab) -> Result<(), ()> {
        match self {
            Item::Module(id) => id.borrow().verify(tab),
            Item::GlobalVariable(id) => id.borrow().verify(tab),
            Item::Function(id) => id.borrow().verify(tab),
            Item::TypeAliasDef(id) => id.borrow().verify(tab),
            Item::StructDef(id) => id.borrow().verify(tab),
            Item::EnumDef(id) => id.borrow().verify(tab),
        }
    }

    fn validate(self, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(tab)?;
        Ok(ValidHir::new(self))
    }
}
