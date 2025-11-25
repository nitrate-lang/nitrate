use crate::prelude::*;
use std::ops::ControlFlow;

impl BlockIterMut<'_> {
    pub(crate) fn try_for_each_mut<T>(
        &mut self,

        vcb: &mut dyn FnMut(&mut Value) -> ControlFlow<T>,
    ) -> ControlFlow<T> {
        for element in &self.node.elements {
            match element {
                BlockElement::Expr(id) => {
                    id.borrow_mut().iter_mut().try_for_each_mut(vcb)?;
                }

                BlockElement::Local(id) => {
                    let local_variable = &id.borrow_mut();

                    local_variable
                        .initializer
                        .borrow_mut()
                        .iter_mut()
                        .try_for_each_mut(vcb)?;
                }
            }
        }

        ControlFlow::Continue(())
    }
}

impl ValueIterMut<'_> {
    pub(crate) fn try_for_each_mut<T>(
        &mut self,

        vcb: &mut dyn FnMut(&mut Value) -> ControlFlow<T>,
    ) -> ControlFlow<T> {
        vcb(self.node)?;

        match self.node {
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
            | Value::BStringLit(_)
            | Value::InferredInteger(_)
            | Value::InferredFloat(_) => {}

            Value::StructObject {
                struct_def: _,
                fields,
            } => {
                for (_field_name, field_value) in fields {
                    field_value.borrow_mut().iter_mut().try_for_each_mut(vcb)?;
                }
            }

            Value::EnumVariant {
                enum_def: _,
                variant: _,
                value,
            } => {
                value.borrow_mut().iter_mut().try_for_each_mut(vcb)?;
            }

            Value::Binary { left, op: _, right } => {
                left.borrow_mut().iter_mut().try_for_each_mut(vcb)?;

                right.borrow_mut().iter_mut().try_for_each_mut(vcb)?;
            }

            Value::Unary { op: _, operand } => {
                operand.borrow_mut().iter_mut().try_for_each_mut(vcb)?;
            }

            Value::FieldAccess {
                expr,
                field_name: _,
            } => {
                expr.borrow_mut().iter_mut().try_for_each_mut(vcb)?;
            }

            Value::Assign { place, value } => {
                place.borrow_mut().iter_mut().try_for_each_mut(vcb)?;
                value.borrow_mut().iter_mut().try_for_each_mut(vcb)?;
            }

            Value::Deref { place } => {
                place.borrow_mut().iter_mut().try_for_each_mut(vcb)?;
            }

            Value::Cast {
                value: expr,
                target_type: _,
            } => {
                expr.borrow_mut().iter_mut().try_for_each_mut(vcb)?;
            }

            Value::Borrow {
                exclusive: _,
                mutable: _,
                place,
            } => {
                place.borrow_mut().iter_mut().try_for_each_mut(vcb)?;
            }

            Value::List { elements } => {
                for element in elements {
                    element.borrow_mut().iter_mut().try_for_each_mut(vcb)?;
                }
            }

            Value::Tuple { elements } => {
                for element in elements {
                    element.borrow_mut().iter_mut().try_for_each_mut(vcb)?;
                }
            }

            Value::If {
                condition,
                true_branch,
                false_branch,
            } => {
                condition.borrow_mut().iter_mut().try_for_each_mut(vcb)?;

                true_branch.borrow_mut().iter_mut().try_for_each_mut(vcb)?;

                if let Some(false_branch) = false_branch {
                    false_branch.borrow_mut().iter_mut().try_for_each_mut(vcb)?;
                }
            }

            Value::While { condition, body } => {
                condition.borrow_mut().iter_mut().try_for_each_mut(vcb)?;

                body.borrow_mut().iter_mut().try_for_each_mut(vcb)?;
            }

            Value::Loop { body } => {
                body.borrow_mut().iter_mut().try_for_each_mut(vcb)?;
            }

            Value::Break { label: _ } => {}

            Value::Continue { label: _ } => {}

            Value::Return { value } => {
                value.borrow_mut().iter_mut().try_for_each_mut(vcb)?;
            }

            Value::Block { block } => {
                block.borrow_mut().iter_mut().try_for_each_mut(vcb)?;
            }

            Value::Call {
                callee,
                positional,
                named,
            } => {
                callee.borrow_mut().iter_mut().try_for_each_mut(vcb)?;

                for argument in positional {
                    argument.borrow_mut().iter_mut().try_for_each_mut(vcb)?;
                }

                for (_name, argument) in named {
                    argument.borrow_mut().iter_mut().try_for_each_mut(vcb)?;
                }
            }

            Value::MethodCall {
                object,
                method_name: _,
                positional,
                named,
            } => {
                object.borrow_mut().iter_mut().try_for_each_mut(vcb)?;

                for argument in positional {
                    argument.borrow_mut().iter_mut().try_for_each_mut(vcb)?;
                }

                for (_name, argument) in named {
                    argument.borrow_mut().iter_mut().try_for_each_mut(vcb)?;
                }
            }

            Value::FunctionSymbol { .. }
            | Value::GlobalVariableSymbol { .. }
            | Value::LocalVariableSymbol { .. }
            | Value::ParameterSymbol { .. } => {}
        }

        ControlFlow::Continue(())
    }
}

impl GlobalVariableIterMut<'_> {
    pub(crate) fn try_for_each_mut<T>(
        &mut self,

        vcb: &mut dyn FnMut(&mut Value) -> ControlFlow<T>,
    ) -> ControlFlow<T> {
        self.node
            .initializer
            .borrow_mut()
            .iter_mut()
            .try_for_each_mut(vcb)?;

        ControlFlow::Continue(())
    }
}

impl ModuleIterMut<'_> {
    pub(crate) fn try_for_each_mut<T>(
        &mut self,

        vcb: &mut dyn FnMut(&mut Value) -> ControlFlow<T>,
    ) -> ControlFlow<T> {
        for item in &self.node.items {
            match item {
                Item::Module(id) => {
                    id.borrow_mut().iter_mut().try_for_each_mut(vcb)?;
                }

                Item::GlobalVariable(id) => {
                    id.borrow_mut().iter_mut().try_for_each_mut(vcb)?;
                }

                Item::Function(id) => {
                    id.borrow_mut().iter_mut().try_for_each_mut(vcb)?;
                }

                Item::TypeAliasDef(_) => {}

                Item::StructDef(id) => {
                    id.borrow_mut().iter_mut().try_for_each_mut(vcb)?;
                }

                Item::EnumDef(id) => {
                    id.borrow_mut().iter_mut().try_for_each_mut(vcb)?;
                }
            }
        }

        ControlFlow::Continue(())
    }
}

impl StructDefIterMut<'_> {
    pub(crate) fn try_for_each_mut<T>(
        &mut self,

        vcb: &mut dyn FnMut(&mut Value) -> ControlFlow<T>,
    ) -> ControlFlow<T> {
        for field in self.node.fields.values() {
            if let Some(default_value) = &field.default_value {
                default_value
                    .borrow_mut()
                    .iter_mut()
                    .try_for_each_mut(vcb)?;
            }
        }

        ControlFlow::Continue(())
    }
}

impl EnumDefIterMut<'_> {
    pub(crate) fn try_for_each_mut<T>(
        &mut self,
        vcb: &mut dyn FnMut(&mut Value) -> ControlFlow<T>,
    ) -> ControlFlow<T> {
        for variant in self.node.variants.iter_mut() {
            if let Some(default_value) = &variant.default_value {
                default_value
                    .borrow_mut()
                    .iter_mut()
                    .try_for_each_mut(vcb)?;
            }
        }

        ControlFlow::Continue(())
    }
}

impl FunctionIterMut<'_> {
    pub(crate) fn try_for_each_mut<T>(
        &mut self,
        vcb: &mut dyn FnMut(&mut Value) -> ControlFlow<T>,
    ) -> ControlFlow<T> {
        for param in &self.node.params {
            let parameter = param.borrow_mut();

            if let Some(default_value) = &parameter.default_value {
                default_value
                    .borrow_mut()
                    .iter_mut()
                    .try_for_each_mut(vcb)?;
            }
        }

        if let Some(body) = &self.node.body {
            for element in body {
                match element {
                    BlockElement::Expr(id) => id.borrow_mut().iter_mut().try_for_each_mut(vcb)?,
                    BlockElement::Local(id) => {
                        let local_variable = &id.borrow_mut();
                        local_variable
                            .initializer
                            .borrow_mut()
                            .iter_mut()
                            .try_for_each_mut(vcb)?;
                    }
                }
            }
        }

        ControlFlow::Continue(())
    }
}
