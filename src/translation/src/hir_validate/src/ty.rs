use std::ops::Deref;

use crate::{ValidHir, ValidateHir};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::{SymbolTab, prelude::*};

impl ValidateHir for FunctionAttribute {
    fn verify(&self, _tab: &SymbolTab, _log: &CompilerLog) -> Result<(), ()> {
        match self {
            FunctionAttribute::CVariadic => Ok(()),
            FunctionAttribute::NoMangle => Ok(()),
        }
    }

    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for FunctionType {
    fn verify(&self, tab: &SymbolTab, log: &CompilerLog) -> Result<(), ()> {
        for attr in &self.attributes {
            attr.verify(tab, log)?;
        }

        self.return_type.verify(tab, log)?;

        for param in &self.params {
            param.1.verify(tab, log)?;
        }

        Ok(())
    }

    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for Type {
    fn verify(&self, tab: &SymbolTab, log: &CompilerLog) -> Result<(), ()> {
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

            Type::Array { element_type, .. } => element_type.verify(tab, log),

            Type::Tuple { element_types } => {
                for elem_type in element_types {
                    elem_type.verify(tab, log)?;
                }

                Ok(())
            }

            Type::Struct { def } => def.borrow().verify(tab, log),

            Type::Enum { def } => def.borrow().verify(tab, log),

            Type::TypeAlias { def } => def.borrow().verify(tab, log),

            Type::Refine { base, min, max } => {
                base.verify(tab, log)?;

                let min = min.deref();
                let max = max.deref();

                if min > max {
                    return Err(());
                }

                Ok(())
            }

            Type::Function { function_type } => function_type.verify(tab, log),

            Type::Reference { lifetime, to, .. } => {
                match lifetime {
                    Lifetime::Static
                    | Lifetime::Gc
                    | Lifetime::ThreadLocal
                    | Lifetime::TaskLocal => {}

                    Lifetime::Inferred => return Err(()),
                }

                // FIXME: Infinite recursion for self-referential types
                to.verify(tab, log)
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
                element_type.verify(tab, log)
            }

            Type::Pointer { to, .. } => {
                // FIXME: Infinite recursion for self-referential types
                to.verify(tab, log)
            }

            Type::InferredFloat | Type::InferredInteger | Type::Inferred { .. } => Err(()),
        }
    }

    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log)?;
        Ok(ValidHir::new(self))
    }
}
