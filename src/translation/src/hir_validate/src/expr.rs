use crate::diagnosis::ValidateErr;
use crate::{
    ValidHir, ValidateCtx, ValidateHirItem, ValidateHirType, ValidateHirValue, ValidateTypeOptions, establish_property,
};
use nitrate_hir::prelude::*;
use nitrate_hir_get_type::HirGetType;

impl ValidateHirValue for Block {
    fn verify(&self, ctx: &mut ValidateCtx) -> Result<(), ()> {
        if ctx.cyclic_bail(self) {
            return Ok(());
        }

        for (i, elem) in self.elements.iter().enumerate() {
            let is_last = i == self.elements.len() - 1;

            match elem {
                BlockElement::Expr(expr)
                    if matches!(
                        &*expr.borrow(),
                        Value::Break { .. } | Value::Continue { .. } | Value::Return { .. }
                    ) =>
                {
                    expr.borrow().verify(ctx)?;

                    establish_property(
                        ctx,
                        "divergent statements have no successors",
                        ValidateErr::DivergentStatementHasSuccessors,
                        |_| {
                            if !is_last {
                                return Err(());
                            }
                            Ok(())
                        },
                    )?;
                }

                BlockElement::Expr(expr) => expr.borrow().verify(ctx)?,
                BlockElement::Local(local) => local.borrow().verify(ctx)?,
            }
        }

        Ok(())
    }

    fn validate(self, ctx: &mut ValidateCtx) -> Result<ValidHir<Self>, ()> {
        self.verify(ctx)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHirValue for Value {
    fn verify(&self, ctx: &mut ValidateCtx) -> Result<(), ()> {
        if ctx.cyclic_bail(self) {
            return Ok(());
        }

        match self {
            Value::Unit
            | Value::Bool(_)
            | Value::I8(_)
            | Value::I16(_)
            | Value::I32(_)
            | Value::I64(_)
            | Value::I128(_)
            | Value::U8(_)
            | Value::U16(_)
            | Value::U32(_)
            | Value::U64(_)
            | Value::U128(_)
            | Value::F32(_)
            | Value::F64(_)
            | Value::USize(..)
            | Value::StringLit(_)
            | Value::BStringLit(_) => Ok(()),

            Value::InferredInteger(_) | Value::InferredFloat(_) => {
                ctx.report(ValidateErr::InferredTypeNotAllowed {
                    type_repr: format!("{:?}", self),
                });
                Err(())
            }

            Value::StructObject { struct_def, fields } => {
                // Verify that all fields exist on the struct and are accessible
                let struct_def = struct_def.borrow();
                for (field_name, field_value) in fields {
                    match struct_def.fields.get(field_name) {
                        Some(field) => {
                            // Check field visibility
                            establish_property(
                                ctx,
                                "struct field visibility",
                                ValidateErr::FieldAccessVisibility {
                                    field_name: field_name.clone(),
                                },
                                |c| {
                                    if !c.check_visibility(&field.visibility, field_name) {
                                        return Err(());
                                    }
                                    Ok(())
                                },
                            )?;
                            field_value.borrow().verify(ctx)?;
                        }
                        None => {
                            // Field doesn't exist on struct
                            ctx.report(ValidateErr::StructFieldDoesNotExist {
                                struct_name: struct_def.name.clone(),
                                field_name: field_name.clone(),
                            });
                            return Err(());
                        }
                    }
                }
                Ok(())
            }

            Value::EnumVariant {
                enum_def,
                variant,
                value,
            } => {
                // Verify that the enum variant exists
                let enum_def = enum_def.borrow();
                if !enum_def.variants.iter().any(|v| &v.name == variant) {
                    ctx.report(ValidateErr::EnumVariantDoesNotExist {
                        enum_name: enum_def.name.clone(),
                        variant_name: variant.clone(),
                    });
                    return Err(());
                }
                value.borrow().verify(ctx)?;
                Ok(())
            }

            Value::Binary { left, op: _, right } => {
                left.borrow().verify(ctx)?;
                right.borrow().verify(ctx)?;
                Ok(())
            }

            Value::Unary { op: _, operand } => operand.borrow().verify(ctx),

            Value::FieldAccess { expr, field_name } => {
                // Verify field access visibility
                let expr_value = expr.borrow();
                expr_value.verify(ctx)?;

                // Check that we can access the field based on struct field visibility
                if let Ok(ty) = expr_value.determine_type(ctx.m)
                    && let Type::Struct { def } = ty {
                        let struct_def = def.borrow();
                        if let Some(field) = struct_def.fields.get(field_name) {
                            establish_property(
                                ctx,
                                "field access visibility",
                                ValidateErr::FieldAccessVisibility {
                                    field_name: field_name.clone(),
                                },
                                |c| {
                                    let qualified_name = format!("{}::{}", struct_def.name, field_name).into();
                                    if !c.check_visibility(&field.visibility, &qualified_name) {
                                        return Err(());
                                    }
                                    Ok(())
                                },
                            )?;
                        } else {
                            // Field doesn't exist
                            ctx.report(ValidateErr::StructFieldDoesNotExist {
                                struct_name: struct_def.name.clone(),
                                field_name: field_name.clone(),
                            });
                            return Err(());
                        }
                    }
                Ok(())
            }

            Value::Assign { place, value } => {
                place.borrow().verify(ctx)?;
                value.borrow().verify(ctx)?;

                // Mutability enforcement: assignment target must be mutable
                establish_property(
                    ctx,
                    "assignment target is mutable",
                    ValidateErr::AssignmentTargetNotMutable,
                    |c| {
                        if !c.is_place_mutable(&place.borrow()) {
                            return Err(());
                        }
                        Ok(())
                    },
                )
            }

            Value::Deref { place } => place.borrow().verify(ctx),

            Value::Cast { value, target_type } => {
                value.borrow().verify(ctx)?;
                target_type.verify(ctx, &ValidateTypeOptions::un_sized())?;
                Ok(())
            }

            Value::Borrow {
                exclusive: _,
                mutable,
                place,
            } => {
                place.borrow().verify(ctx)?;

                // Mutability enforcement: if borrowing as mutable, the place must be mutable
                if *mutable {
                    establish_property(
                        ctx,
                        "mutable borrow target is mutable",
                        ValidateErr::MutableBorrowTargetNotMutable,
                        |c| {
                            if !c.is_place_mutable(&place.borrow()) {
                                return Err(());
                            }
                            Ok(())
                        },
                    )?;
                }
                Ok(())
            }

            Value::List { elements } => {
                for elem in elements {
                    elem.borrow().verify(ctx)?;
                }
                Ok(())
            }

            Value::Tuple { elements } => {
                for elem in elements {
                    elem.borrow().verify(ctx)?;
                }
                Ok(())
            }

            Value::If {
                condition,
                true_branch,
                false_branch,
            } => {
                condition.borrow().verify(ctx)?;
                true_branch.borrow().verify(ctx)?;
                if let Some(false_branch) = false_branch {
                    false_branch.borrow().verify(ctx)?;
                }
                Ok(())
            }

            Value::While { condition, body } => {
                condition.borrow().verify(ctx)?;
                body.borrow().verify(ctx)
            }

            Value::Loop { body } => body.borrow().verify(ctx),

            Value::Break { label: _ } => Ok(()),

            Value::Continue { label: _ } => Ok(()),

            Value::Return { value } => value.borrow().verify(ctx),

            Value::Block { block } => block.borrow().verify(ctx),

            Value::Call { callee, args } => {
                callee.borrow().verify(ctx)?;
                for arg in args.clone().into_iter() {
                    arg.borrow().verify(ctx)?;
                }
                Ok(())
            }

            Value::MethodCall {
                object,
                method_name: _,
                args,
            } => {
                object.borrow().verify(ctx)?;
                for arg in args.clone().into_iter() {
                    arg.borrow().verify(ctx)?;
                }
                Ok(())
            }

            Value::IndexAccess { collection, index } => {
                collection.borrow().verify(ctx)?;
                index.borrow().verify(ctx)
            }

            Value::FunctionSymbol { id } => {
                // Visibility enforcement: check that the function is accessible
                let func = id.borrow();
                let qualified_name = &func.name;
                establish_property(
                    ctx,
                    "function visibility",
                    ValidateErr::FunctionNotAccessible {
                        function_name: qualified_name.clone(),
                    },
                    |c| {
                        if !c.check_visibility(&func.visibility, qualified_name) {
                            return Err(());
                        }
                        Ok(())
                    },
                )
            }

            Value::GlobalVariableSymbol { id } => {
                // Visibility enforcement: check that the global variable is accessible
                let glb = id.borrow();
                let qualified_name = &glb.name;
                establish_property(
                    ctx,
                    "global variable visibility",
                    ValidateErr::GlobalVariableNotAccessible {
                        variable_name: qualified_name.clone(),
                    },
                    |c| {
                        if !c.check_visibility(&glb.visibility, qualified_name) {
                            return Err(());
                        }
                        Ok(())
                    },
                )
            }

            Value::LocalVariableSymbol { .. } => Ok(()),

            Value::ParameterSymbol { .. } => Ok(()),
        }
    }

    fn validate(self, ctx: &mut ValidateCtx) -> Result<ValidHir<Self>, ()> {
        self.verify(ctx)?;
        Ok(ValidHir::new(self))
    }
}
