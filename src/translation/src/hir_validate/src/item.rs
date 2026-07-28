use core::panic;

use crate::diagnosis::ValidateErr;
use crate::{
    ValidHir, ValidateCtx, ValidateHirItem, ValidateHirType, ValidateHirValue, ValidateTypeOptions, establish_property,
};
use nitrate_hir::prelude::*;
use nitrate_hir_get_type::HirGetType;
use std::ops::Deref;

impl ValidateHirItem for GlobalVariableAttribute {
    fn verify(&self, ctx: &mut ValidateCtx) -> Result<(), ()> {
        if ctx.cyclic_bail(self) {
            return Ok(());
        }

        match self {
            GlobalVariableAttribute::NoMangle => Ok(()),
        }
    }

    fn validate(self, ctx: &mut ValidateCtx) -> Result<ValidHir<Self>, ()> {
        self.verify(ctx)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHirItem for GlobalVariable {
    fn verify(&self, ctx: &mut ValidateCtx) -> Result<(), ()> {
        if ctx.cyclic_bail(self) {
            return Ok(());
        }

        for attr in &self.attributes {
            attr.verify(ctx)?;
        }

        let init_value = self.initializer.borrow();
        init_value.verify(ctx)?;

        establish_property(
            ctx,
            "type_constraint: Sized",
            ValidateErr::TypeNotSized {
                type_repr: format!("{:?}", self.ty),
            },
            |c| self.ty.verify(c, &ValidateTypeOptions::sized()),
        )?;

        establish_property(
            ctx,
            "type_constraint == typeof(initial_value)",
            ValidateErr::TypeMismatch {
                expected: format!("{:?}", self.ty),
                actual: format!("{:?}", init_value.determine_type(ctx.m).map_err(|_| ())?),
            },
            |c| {
                let init_value_ty = init_value.determine_type(c.m).map_err(|_| ())?;

                if !types_refinement_compatible(&self.ty, &init_value_ty) {
                    return Err(());
                }

                Ok(())
            },
        )
    }

    fn validate(self, ctx: &mut ValidateCtx) -> Result<ValidHir<Self>, ()> {
        self.verify(ctx)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHirItem for LocalVariableAttribute {
    fn verify(&self, ctx: &mut ValidateCtx) -> Result<(), ()> {
        if ctx.cyclic_bail(self) {
            return Ok(());
        }

        match self {
            LocalVariableAttribute::Align { alignment } => establish_property(
                ctx,
                "local variable alignment is supported",
                ValidateErr::AlignmentTooLarge {
                    alignment: alignment.get(),
                    max_supported: 4096,
                    context: "local variable".to_string(),
                },
                |_| {
                    const MAX_SUPPORTED_ALIGNMENT: u32 = 4096;

                    if alignment.get() > MAX_SUPPORTED_ALIGNMENT {
                        return Err(());
                    }

                    Ok(())
                },
            ),
        }
    }

    fn validate(self, ctx: &mut ValidateCtx) -> Result<ValidHir<Self>, ()> {
        self.verify(ctx)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHirItem for LocalVariable {
    fn verify(&self, ctx: &mut ValidateCtx) -> Result<(), ()> {
        if ctx.cyclic_bail(self) {
            return Ok(());
        }

        for attr in &self.attributes {
            attr.verify(ctx)?;
        }

        establish_property(
            ctx,
            "type_constraint: Sized",
            ValidateErr::TypeNotSized {
                type_repr: format!("{:?}", self.ty),
            },
            |c| self.ty.verify(c, &ValidateTypeOptions::sized()),
        )?;

        establish_property(
            ctx,
            "type_constraint == typeof(initial_value)",
            ValidateErr::TypeMismatch {
                expected: format!("{:?}", self.ty),
                actual: "?".to_string(),
            },
            |c| {
                let init_value = self.initializer.borrow();
                let init_value_ty = init_value.determine_type(c.m).map_err(|_| ())?;

                if !types_refinement_compatible(&self.ty, &init_value_ty) {
                    return Err(());
                }

                Ok(())
            },
        )
    }

    fn validate(self, ctx: &mut ValidateCtx) -> Result<ValidHir<Self>, ()> {
        self.verify(ctx)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHirItem for ParameterAttribute {
    fn verify(&self, ctx: &mut ValidateCtx) -> Result<(), ()> {
        if ctx.cyclic_bail(self) {
            return Ok(());
        }

        match self {
            ParameterAttribute::Align { alignment } => establish_property(
                ctx,
                "parameter alignment is supported",
                ValidateErr::AlignmentTooLarge {
                    alignment: alignment.get(),
                    max_supported: 4096,
                    context: "parameter".to_string(),
                },
                |_| {
                    const MAX_SUPPORTED_ALIGNMENT: u32 = 4096;

                    if alignment.get() > MAX_SUPPORTED_ALIGNMENT {
                        return Err(());
                    }

                    Ok(())
                },
            ),
        }
    }

    fn validate(self, ctx: &mut ValidateCtx) -> Result<ValidHir<Self>, ()> {
        self.verify(ctx)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHirItem for Parameter {
    fn verify(&self, ctx: &mut ValidateCtx) -> Result<(), ()> {
        if ctx.cyclic_bail(self) {
            return Ok(());
        }

        for attr in &self.attributes {
            attr.verify(ctx)?;
        }

        establish_property(
            ctx,
            "type_constraint: Sized",
            ValidateErr::TypeNotSized {
                type_repr: format!("{:?}", self.ty),
            },
            |c| self.ty.verify(c, &ValidateTypeOptions::sized()),
        )?;

        establish_property(
            ctx,
            "type_constraint == typeof(default_value)",
            ValidateErr::TypeMismatch {
                expected: format!("{:?}", self.ty),
                actual: "?".to_string(),
            },
            |c| {
                if let Some(default_value) = &self.default_value {
                    let default_value = default_value.borrow();
                    let default_value_ty = default_value.determine_type(c.m).map_err(|_| ())?;

                    if !types_refinement_compatible(&self.ty, &default_value_ty) {
                        return Err(());
                    }
                }

                Ok(())
            },
        )
    }

    fn validate(self, ctx: &mut ValidateCtx) -> Result<ValidHir<Self>, ()> {
        self.verify(ctx)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHirItem for BlockElement {
    fn verify(&self, ctx: &mut ValidateCtx) -> Result<(), ()> {
        if ctx.cyclic_bail(self) {
            return Ok(());
        }

        match self {
            BlockElement::Local(stmt) => stmt.borrow().verify(ctx),
            BlockElement::Expr(expr) => expr.borrow().verify(ctx),
        }
    }

    fn validate(self, ctx: &mut ValidateCtx) -> Result<ValidHir<Self>, ()> {
        self.verify(ctx)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHirItem for Function {
    fn verify(&self, ctx: &mut ValidateCtx) -> Result<(), ()> {
        if ctx.cyclic_bail(self) {
            return Ok(());
        }

        for attr in &self.attributes {
            attr.verify(ctx, &ValidateTypeOptions::sized())?;
        }

        for param in &self.params {
            param.borrow().verify(ctx)?;
        }

        establish_property(
            ctx,
            "return_type: Sized",
            ValidateErr::TypeNotSized {
                type_repr: format!("{:?}", self.return_type),
            },
            |c| self.return_type.verify(c, &ValidateTypeOptions::sized()),
        )?;

        if let Some(body) = &self.body {
            for element in body {
                element.verify(ctx)?;
            }

            establish_property(
                ctx,
                "function body ends with return statement",
                ValidateErr::FunctionBodyMissingReturn {
                    function_name: self.name.clone(),
                },
                |_| match body.last() {
                    Some(BlockElement::Expr(expr)) if expr.borrow().is_return() => Ok(()),
                    _ => Err(()),
                },
            )?;

            establish_property(
                ctx,
                "typeof(body) == return_type",
                ValidateErr::ReturnTypeMismatch {
                    function_name: self.name.clone(),
                    expected: format!("{:?}", self.return_type),
                    actual: "?".to_string(),
                },
                |c| {
                    let Value::Return { value, .. } = &*body
                        .last()
                        .expect("Function body should have at least one element")
                        .as_expr()
                        .expect("Last element of function body should be an expression")
                        .borrow()
                    else {
                        panic!("Last element of function body should be a return expression");
                    };

                    let body_ty = value.borrow().determine_type(c.m).map_err(|_| ())?;

                    if *self.return_type != body_ty {
                        return Err(());
                    }

                    Ok(())
                },
            )?;
        }

        Ok(())
    }

    fn validate(self, ctx: &mut ValidateCtx) -> Result<ValidHir<Self>, ()> {
        self.verify(ctx)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHirItem for Trait {
    fn verify(&self, ctx: &mut ValidateCtx) -> Result<(), ()> {
        if ctx.cyclic_bail(self) {
            return Ok(());
        }

        // TODO: verify trait

        for method in &self.methods {
            method.borrow().verify(ctx)?;
        }

        Ok(())
    }

    fn validate(self, ctx: &mut ValidateCtx) -> Result<ValidHir<Self>, ()> {
        self.verify(ctx)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHirItem for ModuleAttribute {
    fn verify(&self, ctx: &mut ValidateCtx) -> Result<(), ()> {
        if ctx.cyclic_bail(self) {
            return Ok(());
        }

        match self {
            ModuleAttribute::Invalid => {
                ctx.report(ValidateErr::InvalidModuleAttribute);
                Err(())
            }
        }
    }

    fn validate(self, ctx: &mut ValidateCtx) -> Result<ValidHir<Self>, ()> {
        self.verify(ctx)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHirItem for Module {
    fn verify(&self, ctx: &mut ValidateCtx) -> Result<(), ()> {
        if ctx.cyclic_bail(self) {
            return Ok(());
        }

        for attr in &self.attributes {
            attr.verify(ctx)?;
        }

        // Push this module onto the current module path for visibility checks
        ctx.current_module_path.push(self.name.clone());

        for item in &self.items {
            item.verify(ctx)?;
        }

        // Pop the module path after processing all items
        ctx.current_module_path.pop();

        Ok(())
    }

    fn validate(self, ctx: &mut ValidateCtx) -> Result<ValidHir<Self>, ()> {
        self.verify(ctx)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHirItem for TypeAliasDef {
    fn verify(&self, ctx: &mut ValidateCtx) -> Result<(), ()> {
        if ctx.cyclic_bail(self) {
            return Ok(());
        }

        self.type_id.verify(ctx, &ValidateTypeOptions::un_sized())
    }

    fn validate(self, ctx: &mut ValidateCtx) -> Result<ValidHir<Self>, ()> {
        self.verify(ctx)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHirItem for StructAttribute {
    fn verify(&self, ctx: &mut ValidateCtx) -> Result<(), ()> {
        if ctx.cyclic_bail(self) {
            return Ok(());
        }

        match self {
            StructAttribute::Packed => Ok(()),
        }
    }

    fn validate(self, ctx: &mut ValidateCtx) -> Result<ValidHir<Self>, ()> {
        self.verify(ctx)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHirItem for StructFieldAttribute {
    fn verify(&self, ctx: &mut ValidateCtx) -> Result<(), ()> {
        if ctx.cyclic_bail(self) {
            return Ok(());
        }

        match self {
            StructFieldAttribute::Align { alignment } => establish_property(
                ctx,
                "struct field alignment is supported",
                ValidateErr::AlignmentTooLarge {
                    alignment: alignment.get(),
                    max_supported: 4096,
                    context: "struct field".to_string(),
                },
                |_| {
                    const MAX_SUPPORTED_ALIGNMENT: u32 = 4096;

                    if alignment.get() > MAX_SUPPORTED_ALIGNMENT {
                        return Err(());
                    }

                    Ok(())
                },
            ),
        }
    }

    fn validate(self, ctx: &mut ValidateCtx) -> Result<ValidHir<Self>, ()> {
        self.verify(ctx)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHirItem for StructField {
    fn verify(&self, ctx: &mut ValidateCtx) -> Result<(), ()> {
        if ctx.cyclic_bail(self) {
            return Ok(());
        }

        for attr in &self.attributes {
            attr.verify(ctx)?;
        }

        establish_property(
            ctx,
            "field_type: Sized",
            ValidateErr::TypeNotSized {
                type_repr: format!("{:?}", self.ty),
            },
            |c| self.ty.verify(c, &ValidateTypeOptions::sized()),
        )?;

        establish_property(
            ctx,
            "type_constraint == typeof(default_value)",
            ValidateErr::TypeMismatch {
                expected: format!("{:?}", self.ty),
                actual: "?".to_string(),
            },
            |c| {
                if let Some(default_value) = &self.default_value {
                    let default_value = default_value.borrow();
                    let default_value_ty = default_value.determine_type(c.m).map_err(|_| ())?;

                    if !types_refinement_compatible(&self.ty, &default_value_ty) {
                        return Err(());
                    }
                }

                Ok(())
            },
        )
    }

    fn validate(self, ctx: &mut ValidateCtx) -> Result<ValidHir<Self>, ()> {
        self.verify(ctx)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHirItem for StructDef {
    fn verify(&self, ctx: &mut ValidateCtx) -> Result<(), ()> {
        if ctx.cyclic_bail(self) {
            return Ok(());
        }

        for attr in &self.attributes {
            attr.verify(ctx)?;
        }

        for field in self.fields.values() {
            field.verify(ctx)?;
        }

        // TODO: verify struct layout

        Ok(())
    }

    fn validate(self, ctx: &mut ValidateCtx) -> Result<ValidHir<Self>, ()> {
        self.verify(ctx)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHirItem for EnumAttribute {
    fn verify(&self, ctx: &mut ValidateCtx) -> Result<(), ()> {
        if ctx.cyclic_bail(self) {
            return Ok(());
        }

        match self {
            EnumAttribute::Invalid => {
                ctx.report(ValidateErr::InvalidEnumAttribute);
                Err(())
            }
        }
    }

    fn validate(self, ctx: &mut ValidateCtx) -> Result<ValidHir<Self>, ()> {
        self.verify(ctx)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHirItem for EnumVariantAttribute {
    fn verify(&self, ctx: &mut ValidateCtx) -> Result<(), ()> {
        if ctx.cyclic_bail(self) {
            return Ok(());
        }

        match self {
            EnumVariantAttribute::Invalid => {
                ctx.report(ValidateErr::InvalidEnumVariantAttribute);
                Err(())
            }
        }
    }

    fn validate(self, ctx: &mut ValidateCtx) -> Result<ValidHir<Self>, ()> {
        self.verify(ctx)?;
        Ok(ValidHir::new(self))
    }
}

/// Check if a declared type (possibly a Refine) is compatible with an actual expression type.
/// A `Refine { base: U8, ... }` is compatible with `U8` (the literal type).
/// Exact equality is also required for non-refinement types.
fn types_refinement_compatible(declared: &Type, actual: &Type) -> bool {
    match declared {
        Type::Refine { base, .. } => base.deref() == actual,
        _ => declared == actual,
    }
}

impl ValidateHirItem for EnumVariant {
    fn verify(&self, ctx: &mut ValidateCtx) -> Result<(), ()> {
        if ctx.cyclic_bail(self) {
            return Ok(());
        }

        for attr in &self.attributes {
            attr.verify(ctx)?;
        }

        establish_property(
            ctx,
            "variant_type: Sized",
            ValidateErr::TypeNotSized {
                type_repr: format!("{:?}", self.ty),
            },
            |c| self.ty.verify(c, &ValidateTypeOptions::sized()),
        )?;

        establish_property(
            ctx,
            "type_constraint == typeof(default_value)",
            ValidateErr::TypeMismatch {
                expected: format!("{:?}", self.ty),
                actual: "?".to_string(),
            },
            |c| {
                if let Some(default_value) = &self.default_value {
                    let default_value = default_value.borrow();
                    let default_value_ty = default_value.determine_type(c.m).map_err(|_| ())?;

                    if !types_refinement_compatible(&self.ty, &default_value_ty) {
                        return Err(());
                    }
                }

                Ok(())
            },
        )
    }

    fn validate(self, ctx: &mut ValidateCtx) -> Result<ValidHir<Self>, ()> {
        self.verify(ctx)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHirItem for EnumDef {
    fn verify(&self, ctx: &mut ValidateCtx) -> Result<(), ()> {
        if ctx.cyclic_bail(self) {
            return Ok(());
        }

        for attr in &self.attributes {
            attr.verify(ctx)?;
        }

        for variant in &self.variants {
            variant.verify(ctx)?;
        }

        Ok(())
    }

    fn validate(self, ctx: &mut ValidateCtx) -> Result<ValidHir<Self>, ()> {
        self.verify(ctx)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHirItem for Item {
    fn verify(&self, ctx: &mut ValidateCtx) -> Result<(), ()> {
        if ctx.cyclic_bail(self) {
            return Ok(());
        }

        match self {
            Item::Module(id) => id.borrow().verify(ctx),
            Item::GlobalVariable(id) => id.borrow().verify(ctx),
            Item::Function(id) => id.borrow().verify(ctx),
            Item::TypeAliasDef(id) => id.borrow().verify(ctx),
            Item::StructDef(id) => id.borrow().verify(ctx),
            Item::EnumDef(id) => id.borrow().verify(ctx),
            Item::Trait(id) => id.borrow().verify(ctx),
        }
    }

    fn validate(self, ctx: &mut ValidateCtx) -> Result<ValidHir<Self>, ()> {
        self.verify(ctx)?;
        Ok(ValidHir::new(self))
    }
}
