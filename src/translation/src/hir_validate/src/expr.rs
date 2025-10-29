use crate::{ValidHir, ValidateHir};
use nitrate_hir::{SymbolTab, prelude::*};

impl ValidateHir for Block {
    fn verify(&self, store: &Store, symtab: &SymbolTab) -> Result<(), ()> {
        // TODO: Ensure unconditional branches are only at the end of the block

        for (i, elem) in self.elements.iter().enumerate() {
            let is_last = i == self.elements.len() - 1;
            if matches!(elem, BlockElement::Expr(_)) && !is_last {
                return Err(());
            }

            match elem {
                BlockElement::Expr(expr) => store[expr].borrow().verify(store, symtab)?,
                BlockElement::Stmt(expr) => store[expr].borrow().verify(store, symtab)?,
                BlockElement::Local(local) => store[local].borrow().verify(store, symtab)?,
            }
        }

        Ok(())
    }

    fn validate(self, store: &Store, symtab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, symtab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for Value {
    fn verify(&self, store: &Store, symtab: &SymbolTab) -> Result<(), ()> {
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
                struct_path,
                fields,
            } => {
                // TODO: verify struct object
                Ok(())
            }

            Value::EnumVariant {
                enum_path,
                variant,
                value,
            } => {
                // TODO: verify enum variant
                Ok(())
            }

            Value::Binary { left, op, right } => {
                // TODO: verify binary expression
                Ok(())
            }

            Value::Unary { op, operand } => {
                // TODO: verify unary expression
                Ok(())
            }

            Value::FieldAccess { expr, field } => {
                // TODO: verify field access
                Ok(())
            }

            Value::IndexAccess { collection, index } => {
                // TODO: verify index access
                Ok(())
            }

            Value::Assign { place, value } => {
                // TODO: verify assignment
                Ok(())
            }

            Value::Deref { place } => {
                // TODO: verify dereference
                Ok(())
            }

            Value::Cast { expr, to } => {
                // TODO: verify cast
                Ok(())
            }

            Value::Borrow {
                exclusive,
                mutable,
                place,
            } => {
                // TODO: verify borrow
                Ok(())
            }

            Value::List { elements } => {
                // TODO: verify list
                Ok(())
            }

            Value::Tuple { elements } => {
                // TODO: verify tuple
                Ok(())
            }

            Value::If {
                condition,
                true_branch,
                false_branch,
            } => {
                // TODO: verify if expression
                Ok(())
            }

            Value::While { condition, body } => {
                // TODO: verify while expression
                Ok(())
            }

            Value::Loop { body } => {
                // TODO: verify loop expression
                Ok(())
            }

            Value::Break { label } => {
                // TODO: verify break expression
                Ok(())
            }

            Value::Continue { label } => {
                // TODO: verify continue expression
                Ok(())
            }

            Value::Return { value } => {
                // TODO: verify return expression
                Ok(())
            }

            Value::Block { block } => {
                // TODO: verify block expression
                Ok(())
            }

            Value::Closure { captures, callee } => {
                // TODO: verify closure expression
                Ok(())
            }

            Value::Call {
                callee,
                positional,
                named,
            } => {
                // TODO: verify call expression
                Ok(())
            }

            Value::MethodCall {
                object,
                method_name,
                positional,
                named,
            } => {
                // TODO: verify method call expression
                Ok(())
            }

            Value::Symbol { path } => {
                // TODO: verify symbol
                Ok(())
            }
        }
    }

    fn validate(self, store: &Store, symtab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, symtab)?;
        Ok(ValidHir::new(self))
    }
}
