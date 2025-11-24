use std::ops::Deref;

use crate::{ValidHir, ValidateHir};
use nitrate_hir::{SymbolTab, prelude::*};

impl ValidateHir for FunctionAttribute {
    fn verify(&self, _tab: &SymbolTab) -> Result<(), ()> {
        match self {
            FunctionAttribute::CVariadic => Ok(()),
            FunctionAttribute::NoMangle => Ok(()),
        }
    }

    fn validate(self, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for FunctionType {
    fn verify(&self, tab: &SymbolTab) -> Result<(), ()> {
        for attr in &self.attributes {
            attr.verify(tab)?;
        }

        self.return_type.verify(tab)?;

        for param in &self.params {
            param.1.verify(tab)?;
        }
        Ok(())
    }

    fn validate(self, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(tab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for Type {
    fn verify(&self, tab: &SymbolTab) -> Result<(), ()> {
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

            Type::Array { element_type, .. } => element_type.verify(tab),

            Type::Tuple { element_types } => {
                for elem_type in element_types {
                    elem_type.verify(tab)?;
                }

                Ok(())
            }

            Type::Struct { def } => def.borrow().verify(tab),
            Type::Enum { def } => def.borrow().verify(tab),
            Type::TypeAlias { def } => def.borrow().verify(tab),

            Type::Refine { base, min, max } => {
                base.verify(tab)?;

                let min = min.deref();
                let max = max.deref();

                if min > max {
                    return Err(());
                }

                Ok(())
            }

            Type::Function { function_type } => function_type.verify(tab),

            Type::Reference { lifetime, to, .. } => {
                match lifetime {
                    Lifetime::Static
                    | Lifetime::Gc
                    | Lifetime::ThreadLocal
                    | Lifetime::TaskLocal => {}

                    Lifetime::Inferred => return Err(()),
                }

                // FIXME: Infinite recursion for self-referential types
                to.verify(tab)
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
                element_type.verify(tab)
            }

            Type::Pointer { to, .. } => {
                // FIXME: Infinite recursion for self-referential types
                to.verify(tab)
            }

            Type::InferredFloat | Type::InferredInteger | Type::Inferred { .. } => Err(()),
        }
    }

    fn validate(self, tab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(tab)?;
        Ok(ValidHir::new(self))
    }
}
