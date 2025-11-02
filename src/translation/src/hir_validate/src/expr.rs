use crate::{ValidHir, ValidateHir};
use nitrate_hir::{SymbolTab, prelude::*};

impl ValidateHir for Block {
    fn verify(&self, store: &Store, symtab: &SymbolTab) -> Result<(), ()> {
        // TODO: Ensure unconditional branches are only at the end of the block

        for elem in &self.elements {
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
    fn verify(&self, _store: &Store, _symtab: &SymbolTab) -> Result<(), ()> {
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
                struct_path: _,
                fields: _,
            } => {
                // TODO: verify struct object
                Ok(())
            }

            Value::EnumVariant {
                enum_path: _,
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

            Value::FieldAccess { expr: _, field: _ } => {
                // TODO: verify field access
                Ok(())
            }

            Value::IndexAccess {
                collection: _,
                index: _,
            } => {
                // TODO: verify index access
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

            Value::Cast { expr: _, to: _ } => {
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

            Value::Closure {
                captures: _,
                callee: _,
            } => {
                // TODO: verify closure expression
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

            Value::Symbol { path: _ } => {
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
