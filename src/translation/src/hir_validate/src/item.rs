use crate::{
    ValidHir, ValidateCtx, ValidateHirItem, ValidateHirType, ValidateHirValue, ValidateTypeOptions,
    diagnosis::Issue, establish_property,
};
use nitrate_hir::prelude::*;
use nitrate_hir_get_type::HirGetType;

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

        let init_value = self.init.borrow();
        init_value.verify(ctx)?;

        establish_property("type_constraint: Sized", || {
            self.ty.verify(ctx, &ValidateTypeOptions::sized())
        })?;

        establish_property("type_constraint == typeof(initial_value)", || {
            let init_value = self.init.borrow();
            let init_value_ty = init_value.determine_type().map_err(|_| ())?;

            if *self.ty != init_value_ty {
                ctx.log.report(&Issue::TypeMismatch {
                    expected: self.ty,
                    found: init_value_ty.into(),
                });

                return Err(());
            }

            Ok(())
        })
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
            LocalVariableAttribute::Align { alignment } => {
                establish_property("local variable alignment is supported", || {
                    const MAX_SUPPORTED_ALIGNMENT: u32 = 4096;

                    if alignment.get() > MAX_SUPPORTED_ALIGNMENT {
                        ctx.log.report(&Issue::UnsupportedAlignment {
                            alignment: alignment.get(),
                            max_supported: MAX_SUPPORTED_ALIGNMENT,
                        });

                        return Err(());
                    }

                    Ok(())
                })
            }
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

        establish_property("type_constraint: Sized", || {
            self.ty.verify(ctx, &ValidateTypeOptions::sized())
        })?;

        establish_property("type_constraint == typeof(initial_value)", || {
            if let Some(init_value) = &self.init {
                let init_value = init_value.borrow();
                let init_value_ty = init_value.determine_type().map_err(|_| ())?;

                if *self.ty != init_value_ty {
                    ctx.log.report(&Issue::TypeMismatch {
                        expected: self.ty,
                        found: init_value_ty.into(),
                    });

                    return Err(());
                }
            }

            Ok(())
        })
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
            ParameterAttribute::Align { alignment } => {
                establish_property("parameter alignment is supported", || {
                    const MAX_SUPPORTED_ALIGNMENT: u32 = 4096;

                    if alignment.get() > MAX_SUPPORTED_ALIGNMENT {
                        ctx.log.report(&Issue::UnsupportedAlignment {
                            alignment: alignment.get(),
                            max_supported: MAX_SUPPORTED_ALIGNMENT,
                        });

                        return Err(());
                    }

                    Ok(())
                })
            }
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

        establish_property("type_constraint: Sized", || {
            self.ty.verify(ctx, &ValidateTypeOptions::sized())
        })?;

        establish_property("type_constraint == typeof(default_value)", || {
            if let Some(default_value) = &self.default_value {
                let default_value = default_value.borrow();
                let default_value_ty = default_value.determine_type().map_err(|_| ())?;

                if *self.ty != default_value_ty {
                    ctx.log.report(&Issue::TypeMismatch {
                        expected: self.ty,
                        found: default_value_ty.into(),
                    });

                    return Err(());
                }
            }

            Ok(())
        })
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

        establish_property("return_type: Sized", || {
            self.return_type.verify(ctx, &ValidateTypeOptions::sized())
        })?;

        if let Some(body) = &self.body {
            for element in body {
                element.verify(ctx)?;
            }

            establish_property("function body ends with return statement", || {
                match body.last() {
                    Some(BlockElement::Expr(expr)) if expr.borrow().is_return() => Ok(()),
                    _ => {
                        let issue = Issue::MissingReturnStatementInFunctionBody {
                            function_name: self.name.clone(),
                        };
                        ctx.log.report(&issue);
                        Err(())
                    }
                }
            })?;

            establish_property("typeof(body) == return_type", || {
                let Value::Return { value } = &*body
                    .last()
                    .expect("Function body should have at least one element")
                    .as_expr()
                    .expect("Last element of function body should be an expression")
                    .borrow()
                else {
                    panic!("Last element of function body should be a return expression");
                };

                let body_ty = value.borrow().determine_type().map_err(|_| ())?;

                if *self.return_type != body_ty {
                    ctx.log.report(&Issue::TypeMismatch {
                        expected: self.return_type,
                        found: body_ty.into(),
                    });

                    return Err(());
                }

                Ok(())
            })?;
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
        unimplemented!()
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
            ModuleAttribute::Invalid => Err(()),
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

        for item in &self.items {
            item.verify(ctx)?;
        }

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
            StructFieldAttribute::Align { alignment } => {
                establish_property("struct field alignment is supported", || {
                    const MAX_SUPPORTED_ALIGNMENT: u32 = 4096;

                    if alignment.get() > MAX_SUPPORTED_ALIGNMENT {
                        ctx.log.report(&Issue::UnsupportedAlignment {
                            alignment: alignment.get(),
                            max_supported: MAX_SUPPORTED_ALIGNMENT,
                        });

                        return Err(());
                    }

                    Ok(())
                })
            }
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

        establish_property("field_type: Sized", || {
            self.ty.verify(ctx, &ValidateTypeOptions::sized())
        })?;

        establish_property("type_constraint == typeof(default_value)", || {
            if let Some(default_value) = &self.default_value {
                let default_value = default_value.borrow();
                let default_value_ty = default_value.determine_type().map_err(|_| ())?;

                if *self.ty != default_value_ty {
                    ctx.log.report(&Issue::TypeMismatch {
                        expected: self.ty,
                        found: default_value_ty.into(),
                    });

                    return Err(());
                }
            }

            Ok(())
        })
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
            EnumAttribute::Invalid => Err(()),
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
            EnumVariantAttribute::Invalid => Err(()),
        }
    }

    fn validate(self, ctx: &mut ValidateCtx) -> Result<ValidHir<Self>, ()> {
        self.verify(ctx)?;
        Ok(ValidHir::new(self))
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

        establish_property("variant_type: Sized", || {
            self.ty.verify(ctx, &ValidateTypeOptions::sized())
        })?;

        establish_property("type_constraint == typeof(default_value)", || {
            if let Some(default_value) = &self.default_value {
                let default_value = default_value.borrow();
                let default_value_ty = default_value.determine_type().map_err(|_| ())?;

                if *self.ty != default_value_ty {
                    ctx.log.report(&Issue::TypeMismatch {
                        expected: self.ty,
                        found: default_value_ty.into(),
                    });

                    return Err(());
                }
            }

            Ok(())
        })
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
        }
    }

    fn validate(self, ctx: &mut ValidateCtx) -> Result<ValidHir<Self>, ()> {
        self.verify(ctx)?;
        Ok(ValidHir::new(self))
    }
}
