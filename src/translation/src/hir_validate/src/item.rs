use crate::{ValidHir, ValidateHir};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::{SymbolTab, prelude::*};
use nitrate_hir_get_type::HirGetType;
use std::ops::Deref;

impl ValidateHir for GlobalVariableAttribute {
    fn verify(&self, _tab: &SymbolTab, _log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify

        match self {
            GlobalVariableAttribute::NoMangle => Ok(()),
        }
    }

    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for GlobalVariable {
    fn verify(&self, tab: &SymbolTab, log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify

        for attr in &self.attributes {
            attr.verify(tab, log)?;
        }

        let ty = self.ty.deref();
        let init = self.init.borrow();

        ty.verify(tab, log)?;
        init.verify(tab, log)?;

        let init_ty = init.get_type(tab).map_err(|_| ())?;
        if *ty != init_ty {
            return Err(());
        }

        Ok(())
    }

    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for LocalVariableAttribute {
    fn verify(&self, _tab: &SymbolTab, _log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify

        match self {
            LocalVariableAttribute::Invalid => Err(()),
        }
    }

    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for LocalVariable {
    fn verify(&self, tab: &SymbolTab, log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify

        for attr in &self.attributes {
            attr.verify(tab, log)?;
        }

        let ty = self.ty.deref();
        ty.verify(tab, log)?;

        if let Some(init_expr) = &self.init {
            let init = init_expr.borrow();
            init.verify(tab, log)?;

            let init_ty = init.get_type(tab).map_err(|_| ())?;
            if *ty != init_ty {
                return Err(());
            }
        }

        Ok(())
    }

    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for ParameterAttribute {
    fn verify(&self, _tab: &SymbolTab, _log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify

        match self {
            ParameterAttribute::Invalid => Err(()),
        }
    }

    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for Parameter {
    fn verify(&self, tab: &SymbolTab, log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify

        for attr in &self.attributes {
            attr.verify(tab, log)?;
        }

        let ty = self.ty.deref();
        ty.verify(tab, log)?;

        if let Some(default_value) = &self.default_value {
            let init = default_value.borrow();
            init.verify(tab, log)?;

            let init_ty = init.get_type(tab).map_err(|_| ())?;
            if *ty != init_ty {
                return Err(());
            }
        }

        Ok(())
    }

    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for Function {
    fn verify(&self, tab: &SymbolTab, log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify

        for attr in &self.attributes {
            attr.verify(tab, log)?;
        }

        for param in &self.params {
            param.borrow().verify(tab, log)?;
        }

        self.return_type.verify(tab, log)?;

        if let Some(body) = &self.body {
            body.borrow().verify(tab, log)?;
        }

        Ok(())
    }

    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for Trait {
    fn verify(&self, _tab: &SymbolTab, _log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify

        // TODO: verify trait
        unimplemented!()
    }

    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for ModuleAttribute {
    fn verify(&self, _tab: &SymbolTab, _log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify

        match self {
            ModuleAttribute::Invalid => Err(()),
        }
    }

    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for Module {
    fn verify(&self, tab: &SymbolTab, log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify

        for attr in &self.attributes {
            attr.verify(tab, log)?;
        }

        for item in &self.items {
            item.verify(tab, log)?;
        }

        Ok(())
    }

    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for TypeAliasDef {
    fn verify(&self, tab: &SymbolTab, log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify

        self.type_id.verify(tab, log)
    }

    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for StructAttribute {
    fn verify(&self, _tab: &SymbolTab, _log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify

        match self {
            StructAttribute::Packed => Ok(()),
        }
    }

    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for StructFieldAttribute {
    fn verify(&self, _tab: &SymbolTab, _log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify

        match self {
            StructFieldAttribute::Invalid => Err(()),
        }
    }

    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for StructField {
    fn verify(&self, tab: &SymbolTab, log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify

        for attr in &self.attributes {
            attr.verify(tab, log)?;
        }

        self.ty.verify(tab, log)?;
        if let Some(default_value) = &self.default_value {
            let init = default_value.borrow();
            init.verify(tab, log)?;

            let init_ty = init.get_type(tab).map_err(|_| ())?;
            let field_ty = self.ty.deref();
            if *field_ty != init_ty {
                return Err(());
            }
        }

        Ok(())
    }

    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for StructDef {
    fn verify(&self, tab: &SymbolTab, log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify

        for attr in &self.attributes {
            attr.verify(tab, log)?;
        }

        for field in self.fields.values() {
            field.verify(tab, log)?;
        }

        Ok(())
    }

    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for EnumAttribute {
    fn verify(&self, _tab: &SymbolTab, _log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify

        match self {
            EnumAttribute::Invalid => Err(()),
        }
    }

    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for EnumVariantAttribute {
    fn verify(&self, _tab: &SymbolTab, _log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify

        match self {
            EnumVariantAttribute::Invalid => Err(()),
        }
    }

    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for EnumVariant {
    fn verify(&self, tab: &SymbolTab, log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify

        for attr in &self.attributes {
            attr.verify(tab, log)?;
        }

        self.ty.verify(tab, log)
    }

    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for EnumDef {
    fn verify(&self, tab: &SymbolTab, log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify

        for expr in self.variant_extras.iter().flatten() {
            expr.borrow().verify(tab, log)?;
        }

        for attr in &self.attributes {
            attr.verify(tab, log)?;
        }

        for variant in &self.variants {
            variant.verify(tab, log)?;
        }

        Ok(())
    }

    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for Item {
    fn verify(&self, tab: &SymbolTab, log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify

        match self {
            Item::Module(id) => id.borrow().verify(tab, log),
            Item::GlobalVariable(id) => id.borrow().verify(tab, log),
            Item::Function(id) => id.borrow().verify(tab, log),
            Item::TypeAliasDef(id) => id.borrow().verify(tab, log),
            Item::StructDef(id) => id.borrow().verify(tab, log),
            Item::EnumDef(id) => id.borrow().verify(tab, log),
        }
    }

    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log)?;
        Ok(ValidHir::new(self))
    }
}
