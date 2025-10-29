use crate::{ValidHir, ValidateHir};
use nitrate_hir::{SymbolTab, prelude::*};

impl ValidateHir for BlockElement {
    fn verify(&self, store: &Store, symtab: &SymbolTab) -> Result<(), ()> {
        match self {
            BlockElement::Expr(expr) => store[expr].borrow().verify(store, symtab),
            BlockElement::Stmt(expr) => store[expr].borrow().verify(store, symtab),
            BlockElement::Local(local) => store[local].borrow().verify(store, symtab),
        }
    }

    fn validate(self, store: &Store, symtab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, symtab)?;
        Ok(ValidHir::new(self))
    }
}

impl ValidateHir for Block {
    fn verify(&self, store: &Store, symtab: &SymbolTab) -> Result<(), ()> {
        for (i, elem) in self.elements.iter().enumerate() {
            let is_last = i == self.elements.len() - 1;
            if matches!(elem, BlockElement::Expr(_)) && !is_last {
                return Err(());
            }

            elem.verify(store, symtab)?;
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
                unimplemented!()
            }

            Value::EnumVariant {
                enum_path,
                variant,
                value,
            } => {
                // TODO: verify enum variant
                unimplemented!()
            }

            Value::Binary { left, op, right } => {
                // TODO: verify binary expression
                unimplemented!()
            }

            Value::Unary { op, operand } => {
                // TODO: verify unary expression
                unimplemented!()
            }

            Value::FieldAccess { expr, field } => {
                // TODO: verify field access
                unimplemented!()
            }

            Value::IndexAccess { collection, index } => {
                // TODO: verify index access
                unimplemented!()
            }

            Value::Assign { place, value } => {
                // TODO: verify assignment
                unimplemented!()
            }

            Value::Deref { place } => {
                // TODO: verify dereference
                unimplemented!()
            }

            Value::Cast { expr, to } => {
                // TODO: verify cast
                unimplemented!()
            }

            Value::Borrow {
                exclusive,
                mutable,
                place,
            } => {
                // TODO: verify borrow
                unimplemented!()
            }

            Value::List { elements } => {
                // TODO: verify list
                unimplemented!()
            }

            Value::Tuple { elements } => {
                // TODO: verify tuple
                unimplemented!()
            }

            Value::If {
                condition,
                true_branch,
                false_branch,
            } => {
                // TODO: verify if expression
                unimplemented!()
            }

            Value::While { condition, body } => {
                // TODO: verify while expression
                unimplemented!()
            }

            Value::Loop { body } => {
                // TODO: verify loop expression
                unimplemented!()
            }

            Value::Break { label } => {
                // TODO: verify break expression
                unimplemented!()
            }

            Value::Continue { label } => {
                // TODO: verify continue expression
                unimplemented!()
            }

            Value::Return { value } => {
                // TODO: verify return expression
                unimplemented!()
            }

            Value::Block { block } => {
                // TODO: verify block expression
                unimplemented!()
            }

            Value::Closure { captures, callee } => {
                // TODO: verify closure expression
                unimplemented!()
            }

            Value::Call {
                callee,
                positional,
                named,
            } => {
                // TODO: verify call expression
                unimplemented!()
            }

            Value::MethodCall {
                object,
                method_name,
                positional,
                named,
            } => {
                // TODO: verify method call expression
                unimplemented!()
            }

            Value::Symbol { path } => {
                // TODO: verify symbol
                unimplemented!()
            }
        }
    }

    fn validate(self, store: &Store, symtab: &SymbolTab) -> Result<ValidHir<Self>, ()> {
        self.verify(store, symtab)?;
        Ok(ValidHir::new(self))
    }
}
