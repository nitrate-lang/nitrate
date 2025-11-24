use crate::{
    ValidHir, ValidateHirItem, ValidateHirType, ValidateHirValue, ValidateTypeOptions,
    diagnosis::Issue, establish_property,
};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::{SymbolTab, prelude::*};
use nitrate_hir_get_type::HirGetType;
use std::ops::Deref;

impl ValidateHirItem for GlobalVariableAttribute {
    fn verify(&self, _tab: &SymbolTab, _log: &CompilerLog) -> Result<(), ()> {
        match self {
            GlobalVariableAttribute::NoMangle => Ok(()),
        }
    }

    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHirItem for GlobalVariable {
    fn verify(&self, tab: &SymbolTab, log: &CompilerLog) -> Result<(), ()> {
        establish_property("attributes are valid", || -> Result<(), ()> {
            for attr in &self.attributes {
                attr.verify(tab, log)?;
            }

            Ok(())
        })?;

        establish_property("init value is acceptable", || -> Result<(), ()> {
            let init_value = self.init.borrow();
            init_value.verify(tab, log)?;
            Ok(())
        })?;

        establish_property("type is Sized", || -> Result<(), ()> {
            self.ty.verify(tab, log, &ValidateTypeOptions::storable())
        })?;

        establish_property("value type satisfies constraint", || -> Result<(), ()> {
            let init_value = self.init.borrow();
            let init_value_ty = init_value.determine_type(tab).map_err(|_| ())?;

            if *self.ty != init_value_ty {
                log.report(&Issue::TypeMismatch {
                    expected: self.ty,
                    found: init_value_ty.into(),
                });

                return Err(());
            }

            Ok(())
        })?;

        Ok(())
    }

    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHirItem for LocalVariableAttribute {
    fn verify(&self, _tab: &SymbolTab, _log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify local variable attribute

        match self {
            LocalVariableAttribute::Invalid => Err(()),
        }
    }

    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHirItem for LocalVariable {
    fn verify(&self, tab: &SymbolTab, log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify local variable

        for attr in &self.attributes {
            attr.verify(tab, log)?;
        }

        let ty = self.ty.deref();
        ty.verify(tab, log, &ValidateTypeOptions::storable())?;

        if let Some(init_expr) = &self.init {
            let init = init_expr.borrow();
            init.verify(tab, log)?;

            let init_ty = init.determine_type(tab).map_err(|_| ())?;
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

impl ValidateHirItem for ParameterAttribute {
    fn verify(&self, _tab: &SymbolTab, _log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify parameter attribute

        match self {
            ParameterAttribute::Invalid => Err(()),
        }
    }

    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHirItem for Parameter {
    fn verify(&self, tab: &SymbolTab, log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify parameter

        for attr in &self.attributes {
            attr.verify(tab, log)?;
        }

        let ty = self.ty.deref();
        ty.verify(tab, log, &ValidateTypeOptions::storable())?;

        if let Some(default_value) = &self.default_value {
            let init = default_value.borrow();
            init.verify(tab, log)?;

            let init_ty = init.determine_type(tab).map_err(|_| ())?;
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

impl ValidateHirItem for Function {
    fn verify(&self, tab: &SymbolTab, log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify function

        for attr in &self.attributes {
            attr.verify(tab, log, &ValidateTypeOptions::storable())?;
        }

        for param in &self.params {
            param.borrow().verify(tab, log)?;
        }

        self.return_type
            .verify(tab, log, &ValidateTypeOptions::storable())?;

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

impl ValidateHirItem for Trait {
    fn verify(&self, _tab: &SymbolTab, _log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify trait
        unimplemented!()
    }

    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHirItem for ModuleAttribute {
    fn verify(&self, _tab: &SymbolTab, _log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify module attribute

        match self {
            ModuleAttribute::Invalid => Err(()),
        }
    }

    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHirItem for Module {
    fn verify(&self, tab: &SymbolTab, log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify module

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

impl ValidateHirItem for TypeAliasDef {
    fn verify(&self, tab: &SymbolTab, log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify type alias

        self.type_id
            .verify(tab, log, &ValidateTypeOptions::storable())
    }

    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHirItem for StructAttribute {
    fn verify(&self, _tab: &SymbolTab, _log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify struct attribute

        match self {
            StructAttribute::Packed => Ok(()),
        }
    }

    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHirItem for StructFieldAttribute {
    fn verify(&self, _tab: &SymbolTab, _log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify struct field attribute

        match self {
            StructFieldAttribute::Invalid => Err(()),
        }
    }

    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHirItem for StructField {
    fn verify(&self, tab: &SymbolTab, log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify struct field

        for attr in &self.attributes {
            attr.verify(tab, log)?;
        }

        self.ty.verify(tab, log, &ValidateTypeOptions::storable())?;
        if let Some(default_value) = &self.default_value {
            let init = default_value.borrow();
            init.verify(tab, log)?;

            let init_ty = init.determine_type(tab).map_err(|_| ())?;
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

impl ValidateHirItem for StructDef {
    fn verify(&self, tab: &SymbolTab, log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify struct

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

impl ValidateHirItem for EnumAttribute {
    fn verify(&self, _tab: &SymbolTab, _log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify enum attribute

        match self {
            EnumAttribute::Invalid => Err(()),
        }
    }

    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHirItem for EnumVariantAttribute {
    fn verify(&self, _tab: &SymbolTab, _log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify enum variant attribute

        match self {
            EnumVariantAttribute::Invalid => Err(()),
        }
    }

    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHirItem for EnumVariant {
    fn verify(&self, tab: &SymbolTab, log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify enum variant

        for attr in &self.attributes {
            attr.verify(tab, log)?;
        }

        self.ty.verify(tab, log, &ValidateTypeOptions::storable())
    }

    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHirItem for EnumDef {
    fn verify(&self, tab: &SymbolTab, log: &CompilerLog) -> Result<(), ()> {
        // TODO: verify enum

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

impl ValidateHirItem for Item {
    fn verify(&self, tab: &SymbolTab, log: &CompilerLog) -> Result<(), ()> {
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
