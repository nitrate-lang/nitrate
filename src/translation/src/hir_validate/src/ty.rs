use std::ops::Deref;

use crate::{ValidHir, ValidateHirItem, ValidateHirType, ValidateTypeOptions};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::{SymbolTab, prelude::*};

impl ValidateHirType for FunctionAttribute {
    fn verify(
        &self,
        _tab: &SymbolTab,
        _log: &CompilerLog,
        _options: &ValidateTypeOptions,
    ) -> Result<(), ()> {
        // TODO: verify

        match self {
            FunctionAttribute::CVariadic => Ok(()),
            FunctionAttribute::NoMangle => Ok(()),
        }
    }

    fn validate(
        self,
        tab: &SymbolTab,
        log: &CompilerLog,
        options: &ValidateTypeOptions,
    ) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log, options)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHirType for FunctionType {
    fn verify(
        &self,
        tab: &SymbolTab,
        log: &CompilerLog,
        options: &ValidateTypeOptions,
    ) -> Result<(), ()> {
        // TODO: verify

        for attr in &self.attributes {
            attr.verify(tab, log, options)?;
        }

        self.return_type.verify(tab, log, options)?;

        for param in &self.params {
            param.1.verify(tab, log, options)?;
        }

        Ok(())
    }

    fn validate(
        self,
        tab: &SymbolTab,
        log: &CompilerLog,
        options: &ValidateTypeOptions,
    ) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log, options)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHirType for Type {
    fn verify(
        &self,
        tab: &SymbolTab,
        log: &CompilerLog,
        options: &ValidateTypeOptions,
    ) -> Result<(), ()> {
        // TODO: verify

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

            Type::Array { element_type, .. } => element_type.verify(tab, log, options),

            Type::Tuple { element_types } => {
                for elem_type in element_types {
                    elem_type.verify(tab, log, options)?;
                }

                Ok(())
            }

            Type::Struct { def } => def.borrow().verify(tab, log),

            Type::Enum { def } => def.borrow().verify(tab, log),

            Type::TypeAlias { def } => def.borrow().verify(tab, log),

            Type::Refine { base, min, max } => {
                base.verify(tab, log, options)?;

                let min = min.deref();
                let max = max.deref();

                if min > max {
                    return Err(());
                }

                Ok(())
            }

            Type::Function { function_type } => function_type.verify(tab, log, options),

            Type::Reference { lifetime, to, .. } => {
                match lifetime {
                    Lifetime::Static
                    | Lifetime::Gc
                    | Lifetime::ThreadLocal
                    | Lifetime::TaskLocal => {}

                    Lifetime::Inferred => return Err(()),
                }

                // FIXME: Infinite recursion for self-referential types
                to.verify(tab, log, options)
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
                element_type.verify(tab, log, options)
            }

            Type::Pointer { to, .. } => {
                // FIXME: Infinite recursion for self-referential types
                to.verify(tab, log, options)
            }

            Type::InferredFloat | Type::InferredInteger | Type::Inferred { .. } => Err(()),
        }
    }

    fn validate(
        self,
        tab: &SymbolTab,
        log: &CompilerLog,
        options: &ValidateTypeOptions,
    ) -> Result<ValidHir<Self>, ()> {
        self.verify(tab, log, options)?;
        Ok(ValidHir::new(self))
    }
}
