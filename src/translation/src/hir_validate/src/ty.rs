use crate::{ValidHir, ValidateHir};
use nitrate_hir::{SymbolTab, prelude::*};

impl ValidateHir for StructAttribute {
    fn verify(&self, _store: &Store, _tab: &SymbolTab) -> Result<(), ()> {
        match self {
            StructAttribute::Packed => Ok(()),
        }
    }

    fn validate(self, store: &Store, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for StructFieldAttribute {
    fn verify(&self, _store: &Store, _tab: &SymbolTab) -> Result<(), ()> {
        match self {
            StructFieldAttribute::Invalid => Err(()),
        }
    }

    fn validate(self, store: &Store, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for StructField {
    fn verify(&self, store: &Store, tab: &SymbolTab) -> Result<(), ()> {
        for attr in &self.attributes {
            attr.verify(store, tab)?;
        }

        store[&self.ty].verify(store, tab)?;

        Ok(())
    }

    fn validate(self, store: &Store, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for StructType {
    fn verify(&self, store: &Store, tab: &SymbolTab) -> Result<(), ()> {
        for attr in &self.attributes {
            attr.verify(store, tab)?;
        }

        for field in &self.fields {
            field.verify(store, tab)?;
        }

        Ok(())
    }

    fn validate(self, store: &Store, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for EnumAttribute {
    fn verify(&self, _store: &Store, _tab: &SymbolTab) -> Result<(), ()> {
        match self {
            EnumAttribute::Invalid => Err(()),
        }
    }

    fn validate(self, store: &Store, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for EnumVariantAttribute {
    fn verify(&self, _store: &Store, _tab: &SymbolTab) -> Result<(), ()> {
        match self {
            EnumVariantAttribute::Invalid => Err(()),
        }
    }

    fn validate(self, store: &Store, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for EnumVariant {
    fn verify(&self, store: &Store, tab: &SymbolTab) -> Result<(), ()> {
        for attr in &self.attributes {
            attr.verify(store, tab)?;
        }

        store[&self.ty].verify(store, tab)
    }

    fn validate(self, store: &Store, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for EnumType {
    fn verify(&self, store: &Store, tab: &SymbolTab) -> Result<(), ()> {
        for attr in &self.attributes {
            attr.verify(store, tab)?;
        }

        for variant in &self.variants {
            variant.verify(store, tab)?;
        }

        Ok(())
    }

    fn validate(self, store: &Store, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for FunctionAttribute {
    fn verify(&self, _store: &Store, _tab: &SymbolTab) -> Result<(), ()> {
        match self {
            FunctionAttribute::CVariadic => Ok(()),
            FunctionAttribute::NoMangle => Ok(()),
        }
    }

    fn validate(self, store: &Store, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for FunctionType {
    fn verify(&self, store: &Store, tab: &SymbolTab) -> Result<(), ()> {
        for attr in &self.attributes {
            attr.verify(store, tab)?;
        }

        store[&self.return_type].verify(store, tab)?;

        for param in &self.params {
            store[&param.1].verify(store, tab)?;
        }
        Ok(())
    }

    fn validate(self, store: &Store, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for Type {
    fn verify(&self, store: &Store, tab: &SymbolTab) -> Result<(), ()> {
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

            Type::Array { element_type, .. } => store[element_type].verify(store, tab),

            Type::Tuple { element_types } => {
                for elem_type in element_types {
                    store[elem_type].verify(store, tab)?;
                }

                Ok(())
            }

            Type::Struct { struct_type } => store[struct_type].verify(store, tab),

            Type::Enum { enum_type } => store[enum_type].verify(store, tab),

            Type::Refine { base, min, max } => {
                store[base].verify(store, tab)?;

                let min = store[min];
                let max = store[max];

                if min > max {
                    return Err(());
                }

                Ok(())
            }

            Type::Function { function_type } => store[function_type].verify(store, tab),

            Type::Reference { lifetime, to, .. } => {
                match lifetime {
                    Lifetime::Static
                    | Lifetime::Gc
                    | Lifetime::ThreadLocal
                    | Lifetime::TaskLocal => {}

                    Lifetime::Inferred => return Err(()),
                }

                // FIXME: Infinite recursion for self-referential types
                store[to].verify(store, tab)
            }

            Type::SliceRef {
                lifetime,
                element_type,
                ..
            } => {
                match lifetime {
                    Lifetime::Static
                    | Lifetime::Gc
                    | Lifetime::ThreadLocal
                    | Lifetime::TaskLocal => {}

                    Lifetime::Inferred => return Err(()),
                }

                // FIXME: Infinite recursion for self-referential types
                store[element_type].verify(store, tab)
            }

            Type::Pointer { to, .. } => {
                // FIXME: Infinite recursion for self-referential types
                store[to].verify(store, tab)
            }

            Type::InferredFloat | Type::InferredInteger | Type::Inferred { .. } => Err(()),
        }
    }

    fn validate(self, store: &Store, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, tab)?;
        Ok(ValidHir::new(self))
    }
}
