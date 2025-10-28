use crate::{ValidHir, ValidateHir};
use nitrate_hir::{SymbolTab, prelude::*};

impl ValidateHir for StructAttribute {
    fn verify(&self, _store: &Store, _symtab: &SymbolTab) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store, symtab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, symtab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for StructFieldAttribute {
    fn verify(&self, _store: &Store, _symtab: &SymbolTab) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store, symtab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, symtab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for StructField {
    fn verify(&self, _store: &Store, _symtab: &SymbolTab) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store, symtab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, symtab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for StructType {
    fn verify(&self, _store: &Store, _symtab: &SymbolTab) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store, symtab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, symtab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for EnumAttribute {
    fn verify(&self, _store: &Store, _symtab: &SymbolTab) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store, symtab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, symtab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for EnumVariantAttribute {
    fn verify(&self, _store: &Store, _symtab: &SymbolTab) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store, symtab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, symtab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for EnumVariant {
    fn verify(&self, store: &Store, symtab: &SymbolTab) -> Result<(), ()> {
        for attr in &self.attributes {
            attr.verify(store, symtab)?;
        }

        store[&self.ty].verify(store, symtab)
    }

    fn validate(self, store: &Store, symtab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, symtab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for EnumType {
    fn verify(&self, store: &Store, symtab: &SymbolTab) -> Result<(), ()> {
        for attr in &self.attributes {
            attr.verify(store, symtab)?;
        }

        for variant in &self.variants {
            variant.verify(store, symtab)?;
        }

        Ok(())
    }

    fn validate(self, store: &Store, symtab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, symtab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for FunctionAttribute {
    fn verify(&self, _store: &Store, _symtab: &SymbolTab) -> Result<(), ()> {
        // TODO: implement
        unimplemented!()
    }

    fn validate(self, store: &Store, symtab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, symtab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for FunctionType {
    fn verify(&self, store: &Store, symtab: &SymbolTab) -> Result<(), ()> {
        for attr in &self.attributes {
            attr.verify(store, symtab)?;
        }

        store[&self.return_type].verify(store, symtab)?;

        for param in &self.params {
            store[&param.1].verify(store, symtab)?;
        }
        Ok(())
    }

    fn validate(self, store: &Store, symtab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, symtab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for Type {
    fn verify(&self, store: &Store, symtab: &SymbolTab) -> Result<(), ()> {
        match self {
            Type::Never
            | Type::Unit
            | Type::Bool
            | Type::U8
            | Type::U16
            | Type::U32
            | Type::U64
            | Type::U128
            | Type::USize
            | Type::I8
            | Type::I16
            | Type::I32
            | Type::I64
            | Type::I128
            | Type::F32
            | Type::F64 => Ok(()),

            Type::Array { element_type, .. } => store[element_type].verify(store, symtab),

            Type::Tuple { element_types } => {
                for elem_type in element_types {
                    store[elem_type].verify(store, symtab)?;
                }

                Ok(())
            }

            Type::Slice { element_type } => store[element_type].verify(store, symtab),

            Type::Struct { struct_type } => store[struct_type].verify(store, symtab),

            Type::Enum { enum_type } => store[enum_type].verify(store, symtab),

            Type::Refine { base, min, max } => {
                store[base].verify(store, symtab)?;

                let min = store[min];
                let max = store[max];

                if min > max {
                    return Err(());
                }

                Ok(())
            }

            Type::Function { function_type } => store[function_type].verify(store, symtab),

            Type::Reference { lifetime, to, .. } => {
                match lifetime {
                    Lifetime::Static
                    | Lifetime::Gc
                    | Lifetime::ThreadLocal
                    | Lifetime::TaskLocal => {}

                    Lifetime::Inferred => return Err(()),
                }

                store[to].verify(store, symtab)
            }

            Type::Pointer { to, .. } => store[to].verify(store, symtab),

            Type::Symbol { path: _ } => {
                // TODO: cyclic analysis of types
                Ok(())
            }

            Type::InferredFloat | Type::InferredInteger | Type::Inferred { .. } => Err(()),
        }
    }

    fn validate(self, store: &Store, symtab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, symtab)?;
        Ok(ValidHir::new(self))
    }
}
