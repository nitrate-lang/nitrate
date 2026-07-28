use std::{collections::HashSet, ops::Deref};

use crate::diagnosis::ValidateErr;
use crate::{ValidHir, ValidateCtx, ValidateHirItem, ValidateHirType, ValidateTypeOptions, establish_property};
use nitrate_hir::prelude::*;

fn verify_array(
    ctx: &mut ValidateCtx,
    element_type: &Type,
    _len: u32,
    _options: &ValidateTypeOptions,
) -> Result<(), ()> {
    establish_property(
        ctx,
        "element_type: Sized",
        ValidateErr::TypeNotSized {
            type_repr: format!("{:?}", element_type),
        },
        |c| element_type.verify(c, &ValidateTypeOptions::sized()),
    )
}

fn verify_tuple(ctx: &mut ValidateCtx, element_types: &[TypeId], _options: &ValidateTypeOptions) -> Result<(), ()> {
    for elem_type in element_types {
        establish_property(
            ctx,
            "element_type: Sized",
            ValidateErr::TypeNotSized {
                type_repr: format!("{:?}", elem_type),
            },
            |c| elem_type.verify(c, &ValidateTypeOptions::sized()),
        )?;
    }

    Ok(())
}

fn verify_refinement_type(
    ctx: &mut ValidateCtx,
    base: &Type,
    min: &LiteralId,
    max: &LiteralId,
    options: &ValidateTypeOptions,
) -> Result<(), ()> {
    base.verify(ctx, options)?;

    establish_property(
        ctx,
        "refinement bounds: max >= min",
        ValidateErr::RefinementBoundsInvalid {
            min: min.deref().to_string().parse().unwrap_or(0),
            max: max.deref().to_string().parse().unwrap_or(0),
        },
        |_| {
            if max.deref() >= min.deref() { Ok(()) } else { Err(()) }
        },
    )
}

/// Valid extern ABI names that LLVM supports.
/// Maps to the calling conventions in llvm/src/symbol.rs
const VALID_ABI_NAMES: &[&str] = &[
    // Standard
    "C",
    "cdecl",
    "system",
    "rust-intrinsic",
    "platform-intrinsic",
    "rust-call",
    "unadjusted",
    // x86
    "fastcall",
    "x86-fastcall",
    "stdcall",
    "x86-stdcall",
    "thiscall",
    "x86-thiscall",
    "vectorcall",
    "x86-vectorcall",
    "regcall",
    "x86-regcall",
    "x86-intr",
    // x86-64
    "win64",
    "x86-64-win64",
    "sysv64",
    "x86-64-sysv",
    // ARM
    "aapcs",
    "arm-aapcs",
    "aapcs-vfp",
    "arm-aapcs-vfp",
    "arm-apcs",
    // GPU
    "ptx-kernel",
    "ptx-device",
    "amdgpu-kernel",
    "amdgpu-vs",
    "amdgpu-gs",
    "amdgpu-ps",
    "amdgpu-cs",
    "amdgpu-hs",
    "amdgpu-es",
    "amdgpu-ls",
    "amdgpu-call",
    // SPIR
    "spir-func",
    "spir-function",
    "spir-kernel",
    "intel-ocl-bicc",
    // Special
    "cold",
    "fast",
    "swift",
    "swift-tail",
    "preserve-most",
    "preserve-all",
    "tail",
    "cxx-fast-tls",
    "ghc",
    "ghcc",
    "hipcc",
    "anyreg",
    "webkit-js",
    // HHVM
    "hhvm",
    "hhvmc",
    "hhvm-c",
    // AVR
    "avr-intr",
    "avr-signal",
    "avr-builtin",
    // MSP430
    "msp430-intr",
];

impl ValidateHirType for FunctionAttribute {
    fn verify(&self, ctx: &mut ValidateCtx, _options: &ValidateTypeOptions) -> Result<(), ()> {
        if ctx.cyclic_bail(self) {
            return Ok(());
        }

        match self {
            FunctionAttribute::CVariadic => Ok(()),
            FunctionAttribute::NoMangle => Ok(()),
            FunctionAttribute::ExternAbi(abi) => {
                if VALID_ABI_NAMES.iter().any(|&name| name == &*abi.name) {
                    Ok(())
                } else {
                    ctx.report(ValidateErr::InvalidExternAbi {
                        abi_name: abi.name.to_string(),
                    });
                    Err(())
                }
            }
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
            establish_property(
                ctx,
                "parameter type: Sized",
                ValidateErr::TypeNotSized {
                    type_repr: format!("{:?}", param.1),
                },
                |c| param.1.verify(c, &ValidateTypeOptions::sized()),
            )?;
        }

        establish_property(
            ctx,
            "parameter name uniqueness",
            ValidateErr::DuplicateParameterName { name: "".into() },
            |_| {
                let mut names = HashSet::new();

                for param in &self.params {
                    if !names.insert(&param.0) {
                        return Err(());
                    }
                }

                Ok(())
            },
        )?;

        establish_property(
            ctx,
            "return_type: Sized",
            ValidateErr::TypeNotSized {
                type_repr: format!("{:?}", self.return_type),
            },
            |c| self.return_type.verify(c, &ValidateTypeOptions::sized()),
        )?;

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
            Type::Never { .. }
            | Type::Unit { .. }
            | Type::Bool { .. }
            | Type::U8 { .. }
            | Type::U16 { .. }
            | Type::U32 { .. }
            | Type::U64 { .. }
            | Type::U128 { .. }
            | Type::USize { .. }
            | Type::I8 { .. }
            | Type::I16 { .. }
            | Type::I32 { .. }
            | Type::I64 { .. }
            | Type::I128 { .. }
            | Type::F32 { .. }
            | Type::F64 { .. } => Ok(()),

            Type::Array { element_type, len, .. } => verify_array(ctx, element_type, *len, options),

            Type::Tuple { element_types, .. } => verify_tuple(ctx, element_types, options),

            Type::Struct { def, .. } => def.borrow().verify(ctx),

            Type::Enum { def, .. } => def.borrow().verify(ctx),

            Type::TypeAlias { def, .. } => def.borrow().verify(ctx),

            Type::Refine { base, min, max, .. } => verify_refinement_type(ctx, base, min, max, options),

            Type::Function { function_type, .. } => function_type.verify(ctx, options),

            Type::Reference {
                lifetime,
                exclusive,
                mutable,
                to,
                ..
            } => verify_reference_type(ctx, lifetime, *exclusive, *mutable, to, options),

            Type::SliceRef {
                lifetime,
                exclusive,
                mutable,
                element_type,
                ..
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

            Type::Parameterized { base, args: _, .. } => {
                base.verify(ctx, options)
                // TODO: Verify that the type arguments satisfy the generic constraints.
            }

            Type::InferredFloat { .. } | Type::InferredInteger { .. } | Type::Inferred { .. } => {
                ctx.report(ValidateErr::InferredTypeNotAllowed {
                    type_repr: format!("{:?}", self),
                });
                Err(())
            }

            Type::GenericParam { .. } => {
                // Generic parameters are valid in uninstantiated contexts (before monomorphization).
                // They will be replaced with concrete types during monomorphization.
                // The codegen layer will panic if any GenericParam survives to LLVM IR generation.
                Ok(())
            }
        }
    }
    fn validate(self, ctx: &mut ValidateCtx, options: &ValidateTypeOptions) -> Result<ValidHir<Self>, ()> {
        self.verify(ctx, options)?;
        Ok(ValidHir::new(self))
    }
}
