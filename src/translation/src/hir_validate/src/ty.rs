use std::{collections::HashSet, ops::Deref};

use crate::{
    ValidHir, ValidateHirItem, ValidateHirType, ValidateTypeOptions, diagnosis::Issue,
    establish_property,
};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::{SymbolTab, prelude::*};

fn verify_array(
    element_type: &Type,
    _len: u32,
    tab: &SymbolTab,
    log: &CompilerLog,
    _options: &ValidateTypeOptions,
) -> Result<(), ()> {
    establish_property("element_type: Sized", || {
        element_type.verify(tab, log, &ValidateTypeOptions::sized())
    })
}

fn verify_tuple(
    element_types: &[TypeId],
    tab: &SymbolTab,
    log: &CompilerLog,
    _options: &ValidateTypeOptions,
) -> Result<(), ()> {
    establish_property("all tuple element types: Sized", || {
        for elem_type in element_types {
            establish_property("element_type: Sized", || {
                elem_type.verify(tab, log, &ValidateTypeOptions::sized())
            })?;
        }

        Ok(())
    })
}

fn verify_refinement_type(
    base: &Type,
    min: &LiteralId,
    max: &LiteralId,
    tab: &SymbolTab,
    log: &CompilerLog,
    options: &ValidateTypeOptions,
) -> Result<(), ()> {
    base.verify(tab, log, options)?;

    establish_property("refinement bounds: max >= min", || {
        if max.deref() >= min.deref() {
            Ok(())
        } else {
            log.report(&Issue::RefinementMinimumGreaterThanMaximum {
                min: min.clone(),
                max: max.clone(),
                type_id: base.clone().into(),
            });
            Err(())
        }
    })
}

impl ValidateHirType for FunctionAttribute {
    fn verify(
        &self,
        _tab: &SymbolTab,
        _log: &CompilerLog,
        _options: &ValidateTypeOptions,
    ) -> Result<(), ()> {
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
        for attr in &self.attributes {
            attr.verify(tab, log, options)?;
        }

        for param in &self.params {
            establish_property("parameter type: Sized", || {
                param.1.verify(tab, log, &ValidateTypeOptions::sized())
            })?;
        }

        establish_property("parameter name uniqueness", || {
            let mut names = HashSet::new();

            for param in &self.params {
                if !names.insert(&param.0) {
                    let function = Type::Function {
                        function_type: Box::new(self.clone()),
                    };

                    log.report(&Issue::FunctionTypeDuplicateParameterName {
                        name: param.0.clone(),
                        function: function.into(),
                    });

                    return Err(());
                }
            }

            Ok(())
        })?;

        establish_property("return_type: Sized", || {
            self.return_type
                .verify(tab, log, &ValidateTypeOptions::sized())
        })?;

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

fn verify_reference_type(
    lifetime: &Lifetime,
    _exclusive: bool,
    _mutable: bool,
    to: &Type,
    tab: &SymbolTab,
    log: &CompilerLog,
    _options: &ValidateTypeOptions,
) -> Result<(), ()> {
    match lifetime {
        Lifetime::Static | Lifetime::Gc | Lifetime::ThreadLocal | Lifetime::TaskLocal => {}
        Lifetime::Inferred => {
            log.report(&Issue::UninferredTypeResidue);
            return Err(());
        }
    }

    // FIXME: Infinite recursion for self-referential types
    to.verify(tab, log, &ValidateTypeOptions::un_sized())
}

fn verify_slice_reference_type(
    lifetime: &Lifetime,
    _exclusive: bool,
    _mutable: bool,
    element_type: &Type,
    tab: &SymbolTab,
    log: &CompilerLog,
    _options: &ValidateTypeOptions,
) -> Result<(), ()> {
    match lifetime {
        Lifetime::Static | Lifetime::Gc | Lifetime::ThreadLocal | Lifetime::TaskLocal => {}

        Lifetime::Inferred => {
            log.report(&Issue::UninferredTypeResidue);
            return Err(());
        }
    }

    // FIXME: Infinite recursion for self-referential types
    element_type.verify(tab, log, &ValidateTypeOptions::sized())
}

fn verify_pointer_type(
    to: &Type,
    _exclusive: bool,
    _mutable: bool,
    tab: &SymbolTab,
    log: &CompilerLog,
    _options: &ValidateTypeOptions,
) -> Result<(), ()> {
    // FIXME: Infinite recursion for self-referential types
    to.verify(tab, log, &ValidateTypeOptions::un_sized())
}

impl ValidateHirType for Type {
    fn verify(
        &self,
        tab: &SymbolTab,
        log: &CompilerLog,
        options: &ValidateTypeOptions,
    ) -> Result<(), ()> {
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

            Type::Array { element_type, len } => {
                verify_array(element_type, *len, tab, log, options)
            }

            Type::Tuple { element_types } => verify_tuple(element_types, tab, log, options),

            Type::Struct { def } => def.borrow().verify(tab, log),

            Type::Enum { def } => def.borrow().verify(tab, log),

            Type::TypeAlias { def } => def.borrow().verify(tab, log),

            Type::Refine { base, min, max } => {
                verify_refinement_type(base, min, max, tab, log, options)
            }

            Type::Function { function_type } => function_type.verify(tab, log, options),

            Type::Reference {
                lifetime,
                exclusive,
                mutable,
                to,
            } => verify_reference_type(lifetime, *exclusive, *mutable, to, tab, log, options),

            Type::SliceRef {
                lifetime,
                exclusive,
                mutable,
                element_type,
            } => verify_slice_reference_type(
                lifetime,
                *exclusive,
                *mutable,
                element_type,
                tab,
                log,
                options,
            ),

            Type::Pointer {
                to,
                exclusive,
                mutable,
            } => verify_pointer_type(to, *exclusive, *mutable, tab, log, options),

            Type::InferredFloat | Type::InferredInteger | Type::Inferred { .. } => {
                log.report(&Issue::UninferredTypeResidue);
                Err(())
            }
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
