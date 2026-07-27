use std::{collections::HashSet, ops::Deref};

use crate::{ValidHir, ValidateCtx, ValidateHirItem, ValidateHirType, ValidateTypeOptions, establish_property};
use nitrate_hir::prelude::*;

fn verify_array(
    ctx: &mut ValidateCtx,
    element_type: &Type,
    _len: u32,
    _options: &ValidateTypeOptions,
) -> Result<(), ()> {
    establish_property("element_type: Sized", || {
        element_type.verify(ctx, &ValidateTypeOptions::sized())
    })
}

fn verify_tuple(ctx: &mut ValidateCtx, element_types: &[TypeId], _options: &ValidateTypeOptions) -> Result<(), ()> {
    establish_property("all tuple element types: Sized", || {
        for elem_type in element_types {
            establish_property("element_type: Sized", || {
                elem_type.verify(ctx, &ValidateTypeOptions::sized())
            })?;
        }

        Ok(())
    })
}

fn verify_refinement_type(
    ctx: &mut ValidateCtx,
    base: &Type,
    min: &LiteralId,
    max: &LiteralId,
    options: &ValidateTypeOptions,
) -> Result<(), ()> {
    base.verify(ctx, options)?;

    establish_property("refinement bounds: max >= min", || {
        if max.deref() >= min.deref() { Ok(()) } else { Err(()) }
    })
}

impl ValidateHirType for FunctionAttribute {
    fn verify(&self, ctx: &mut ValidateCtx, _options: &ValidateTypeOptions) -> Result<(), ()> {
        if ctx.cyclic_bail(self) {
            return Ok(());
        }

        match self {
            FunctionAttribute::CVariadic => Ok(()),
            FunctionAttribute::NoMangle => Ok(()),
        }
    }

    fn validate(self, ctx: &mut ValidateCtx, options: &ValidateTypeOptions) -> Result<ValidHir<Self>, ()> {
        self.verify(ctx, options)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHirType for FunctionType {
    fn verify(&self, ctx: &mut ValidateCtx, options: &ValidateTypeOptions) -> Result<(), ()> {
        if ctx.cyclic_bail(self) {
            return Ok(());
        }

        for attr in &self.attributes {
            attr.verify(ctx, options)?;
        }

        for param in &self.params {
            establish_property("parameter type: Sized", || {
                param.1.verify(ctx, &ValidateTypeOptions::sized())
            })?;
        }

        establish_property("parameter name uniqueness", || {
            let mut names = HashSet::new();

            for param in &self.params {
                if !names.insert(&param.0) {
                    return Err(());
                }
            }

            Ok(())
        })?;

        establish_property("return_type: Sized", || {
            self.return_type.verify(ctx, &ValidateTypeOptions::sized())
        })?;

        Ok(())
    }

    fn validate(self, ctx: &mut ValidateCtx, options: &ValidateTypeOptions) -> Result<ValidHir<Self>, ()> {
        self.verify(ctx, options)?;
        Ok(ValidHir::new(self))
    }
}

fn verify_reference_type(
    ctx: &mut ValidateCtx,
    lifetime: &Lifetime,
    _exclusive: bool,
    _mutable: bool,
    to: &Type,
    _options: &ValidateTypeOptions,
) -> Result<(), ()> {
    match lifetime {
        Lifetime::Static | Lifetime::Gc | Lifetime::ThreadLocal | Lifetime::TaskLocal | Lifetime::Inferred => {}
    }

    to.verify(ctx, &ValidateTypeOptions::un_sized())
}

fn verify_slice_reference_type(
    ctx: &mut ValidateCtx,
    lifetime: &Lifetime,
    _exclusive: bool,
    _mutable: bool,
    element_type: &Type,
    _options: &ValidateTypeOptions,
) -> Result<(), ()> {
    match lifetime {
        Lifetime::Static | Lifetime::Gc | Lifetime::ThreadLocal | Lifetime::TaskLocal | Lifetime::Inferred => {}
    }

    element_type.verify(ctx, &ValidateTypeOptions::sized())
}

fn verify_pointer_type(
    ctx: &mut ValidateCtx,
    to: &Type,
    _exclusive: bool,
    _mutable: bool,
    _options: &ValidateTypeOptions,
) -> Result<(), ()> {
    to.verify(ctx, &ValidateTypeOptions::un_sized())
}

fn verify_slice_pointer_type(
    ctx: &mut ValidateCtx,
    _exclusive: bool,
    _mutable: bool,
    element_type: &Type,
    _options: &ValidateTypeOptions,
) -> Result<(), ()> {
    element_type.verify(ctx, &ValidateTypeOptions::sized())
}

impl ValidateHirType for Type {
    fn verify(&self, ctx: &mut ValidateCtx, options: &ValidateTypeOptions) -> Result<(), ()> {
        if ctx.cyclic_bail(self) {
            return Ok(());
        }

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

            Type::Array { element_type, len } => verify_array(ctx, element_type, *len, options),

            Type::Tuple { element_types } => verify_tuple(ctx, element_types, options),

            Type::Struct { def } => def.borrow().verify(ctx),

            Type::Enum { def } => def.borrow().verify(ctx),

            Type::TypeAlias { def } => def.borrow().verify(ctx),

            Type::Refine { base, min, max } => verify_refinement_type(ctx, base, min, max, options),

            Type::Function { function_type } => function_type.verify(ctx, options),

            Type::Reference {
                lifetime,
                exclusive,
                mutable,
                to,
            } => verify_reference_type(ctx, lifetime, *exclusive, *mutable, to, options),

            Type::SliceRef {
                lifetime,
                exclusive,
                mutable,
                element_type,
            } => verify_slice_reference_type(ctx, lifetime, *exclusive, *mutable, element_type, options),

            Type::Pointer {
                to, exclusive, mutable, ..
            } => verify_pointer_type(ctx, to, *exclusive, *mutable, options),

            Type::SlicePtr {
                exclusive,
                mutable,
                element_type,
                ..
            } => verify_slice_pointer_type(ctx, *exclusive, *mutable, element_type, options),

            Type::TraitObject { .. } => Ok(()),

            Type::Parameterized { base, args: _ } => {
                base.verify(ctx, options)
                // TODO: Verify that the type arguments satisfy the generic constraints.
            }

            Type::InferredFloat | Type::InferredInteger | Type::Inferred { .. } => Err(()),

            Type::GenericParam { .. } => {
                // Generic parameters are valid in uninstantiated contexts (before monomorphization)
                // They will be replaced with concrete types during monomorphization.
                Ok(())
            }
        }
    }
    fn validate(self, ctx: &mut ValidateCtx, options: &ValidateTypeOptions) -> Result<ValidHir<Self>, ()> {
        self.verify(ctx, options)?;
        Ok(ValidHir::new(self))
    }
}
