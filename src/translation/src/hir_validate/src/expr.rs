use crate::{ValidHir, ValidateCtx, ValidateHirItem, ValidateHirValue, establish_property};
use nitrate_hir::prelude::*;

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

                    establish_property("divergent statements have no successors", || {
                        if !is_last {
                            return Err(());
                        }
                        Ok(())
                    })?;
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
            | Value::USize32(_)
            | Value::USize64(_)
            | Value::StringLit(_)
            | Value::BStringLit(_) => Ok(()),

            Value::InferredInteger(_) | Value::InferredFloat(_) => Err(()),

            Value::StructObject {
                struct_def: _,
                fields: _,
            } => {
                // TODO: verify struct object
                Ok(())
            }

            Value::EnumVariant {
                enum_def: _,
                variant: _,
                value: _,
            } => {
                // TODO: verify enum variant
                Ok(())
            }

            Value::Binary {
                left: _,
                op: _,
                right: _,
            } => {
                // TODO: verify binary expression
                Ok(())
            }

            Value::Unary { op: _, operand: _ } => {
                // TODO: verify unary expression
                Ok(())
            }

            Value::FieldAccess {
                expr: _,
                field_name: _,
            } => {
                // TODO: verify field access
                Ok(())
            }

            Value::Assign { place: _, value: _ } => {
                // TODO: verify assignment
                Ok(())
            }

            Value::Deref { place: _ } => {
                // TODO: verify dereference
                Ok(())
            }

            Value::Cast {
                value: _,
                target_type: _,
            } => {
                // TODO: verify cast
                Ok(())
            }

            Value::Borrow {
                exclusive: _,
                mutable: _,
                place: _,
            } => {
                // TODO: verify borrow
                Ok(())
            }

            Value::List { elements: _ } => {
                // TODO: verify list
                Ok(())
            }

            Value::Tuple { elements: _ } => {
                // TODO: verify tuple
                Ok(())
            }

            Value::If {
                condition: _,
                true_branch: _,
                false_branch: _,
            } => {
                // TODO: verify if expression
                Ok(())
            }

            Value::While {
                condition: _,
                body: _,
            } => {
                // TODO: verify while expression
                Ok(())
            }

            Value::Loop { body: _ } => {
                // TODO: verify loop expression
                Ok(())
            }

            Value::Break { label: _ } => {
                // TODO: verify break expression
                Ok(())
            }

            Value::Continue { label: _ } => {
                // TODO: verify continue expression
                Ok(())
            }

            Value::Return { value: _ } => {
                // TODO: verify return expression
                Ok(())
            }

            Value::Block { block: _ } => {
                // TODO: verify block expression
                Ok(())
            }

            Value::Call {
                callee: _,
                positional: _,
                named: _,
            } => {
                // TODO: verify call expression
                Ok(())
            }

            Value::MethodCall {
                object: _,
                method_name: _,
                positional: _,
                named: _,
            } => {
                // TODO: verify method call expression
                Ok(())
            }

            Value::FunctionSymbol { .. } => Ok(()),
            Value::GlobalVariableSymbol { .. } => Ok(()),
            Value::LocalVariableSymbol { .. } => Ok(()),
            Value::ParameterSymbol { .. } => Ok(()),
        }
    }

    fn validate(self, ctx: &mut ValidateCtx) -> Result<ValidHir<Self>, ()> {
        self.verify(ctx)?;
        Ok(ValidHir::new(self))
    }
}
