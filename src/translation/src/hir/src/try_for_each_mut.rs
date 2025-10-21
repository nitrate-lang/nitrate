use crate::prelude::*;
use std::ops::ControlFlow;

impl BlockIterMut<'_> {
    pub(crate) fn try_for_each_mut<T>(
        &mut self,
        store: &Store,
        vcb: &mut dyn FnMut(&mut Value) -> ControlFlow<T>,
    ) -> ControlFlow<T> {
        for element in &self.node.elements {
            match element {
                BlockElement::Expr(id) => {
                    store[id]
                        .borrow_mut()
                        .iter_mut()
                        .try_for_each_mut(store, vcb)?;
                }

                BlockElement::Stmt(id) => {
                    store[id]
                        .borrow_mut()
                        .iter_mut()
                        .try_for_each_mut(store, vcb)?;
                }

                BlockElement::Local(id) => {
                    let local_variable = &store[id].borrow_mut();

                    if let Some(init) = &local_variable.init {
                        store[init]
                            .borrow_mut()
                            .iter_mut()
                            .try_for_each_mut(store, vcb)?;
                    }
                }
            }
        }

        ControlFlow::Continue(())
    }
}

impl ValueIterMut<'_> {
    pub(crate) fn try_for_each_mut<T>(
        &mut self,
        store: &Store,
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
                struct_path: _,
                fields,
            } => {
                for (_field_name, field_value) in fields {
                    store[field_value as &ValueId]
                        .borrow_mut()
                        .iter_mut()
                        .try_for_each_mut(store, vcb)?;
                }
            }

            Value::EnumVariant {
                enum_path: _,
                variant: _,
                value,
            } => {
                store[value as &ValueId]
                    .borrow_mut()
                    .iter_mut()
                    .try_for_each_mut(store, vcb)?;
            }

            Value::Binary { left, op: _, right } => {
                store[left as &ValueId]
                    .borrow_mut()
                    .iter_mut()
                    .try_for_each_mut(store, vcb)?;

                store[right as &ValueId]
                    .borrow_mut()
                    .iter_mut()
                    .try_for_each_mut(store, vcb)?;
            }

            Value::Unary { op: _, operand } => {
                store[operand as &ValueId]
                    .borrow_mut()
                    .iter_mut()
                    .try_for_each_mut(store, vcb)?;
            }

            Value::FieldAccess { expr, field: _ } => {
                store[expr as &ValueId]
                    .borrow_mut()
                    .iter_mut()
                    .try_for_each_mut(store, vcb)?;
            }

            Value::IndexAccess { collection, index } => {
                store[collection as &ValueId]
                    .borrow_mut()
                    .iter_mut()
                    .try_for_each_mut(store, vcb)?;

                store[index as &ValueId]
                    .borrow_mut()
                    .iter_mut()
                    .try_for_each_mut(store, vcb)?;
            }

            Value::Assign { place, value } => {
                store[place as &ValueId]
                    .borrow_mut()
                    .iter_mut()
                    .try_for_each_mut(store, vcb)?;

                store[value as &ValueId]
                    .borrow_mut()
                    .iter_mut()
                    .try_for_each_mut(store, vcb)?;
            }

            Value::Deref { place } => {
                store[place as &ValueId]
                    .borrow_mut()
                    .iter_mut()
                    .try_for_each_mut(store, vcb)?;
            }

            Value::Cast { expr, to: _ } => {
                store[expr as &ValueId]
                    .borrow_mut()
                    .iter_mut()
                    .try_for_each_mut(store, vcb)?;
            }

            Value::Borrow { mutable: _, place } => {
                store[place as &ValueId]
                    .borrow_mut()
                    .iter_mut()
                    .try_for_each_mut(store, vcb)?;
            }

            Value::List { elements } => {
                for element in elements {
                    element.iter_mut().try_for_each_mut(store, vcb)?;
                }
            }

            Value::Tuple { elements } => {
                for element in elements {
                    element.iter_mut().try_for_each_mut(store, vcb)?;
                }
            }

            Value::If {
                condition,
                true_branch,
                false_branch,
            } => {
                store[condition as &ValueId]
                    .borrow_mut()
                    .iter_mut()
                    .try_for_each_mut(store, vcb)?;

                store[true_branch as &BlockId]
                    .borrow_mut()
                    .iter_mut()
                    .try_for_each_mut(store, vcb)?;

                if let Some(false_branch) = false_branch {
                    store[false_branch as &BlockId]
                        .borrow_mut()
                        .iter_mut()
                        .try_for_each_mut(store, vcb)?;
                }
            }

            Value::While { condition, body } => {
                store[condition as &ValueId]
                    .borrow_mut()
                    .iter_mut()
                    .try_for_each_mut(store, vcb)?;

                store[body as &BlockId]
                    .borrow_mut()
                    .iter_mut()
                    .try_for_each_mut(store, vcb)?;
            }

            Value::Loop { body } => {
                store[body as &BlockId]
                    .borrow_mut()
                    .iter_mut()
                    .try_for_each_mut(store, vcb)?;
            }

            Value::Break { label: _ } => {}

            Value::Continue { label: _ } => {}

            Value::Return { value } => {
                store[value as &ValueId]
                    .borrow_mut()
                    .iter_mut()
                    .try_for_each_mut(store, vcb)?;
            }

            Value::Block { block } => {
                store[block as &BlockId]
                    .borrow_mut()
                    .iter_mut()
                    .try_for_each_mut(store, vcb)?;
            }

            Value::Closure {
                captures: _,
                callee,
            } => {
                store[callee as &FunctionId]
                    .borrow_mut()
                    .iter_mut()
                    .try_for_each_mut(store, vcb)?;
            }

            Value::Call {
                callee,
                positional,
                named,
            } => {
                store[callee as &ValueId]
                    .borrow_mut()
                    .iter_mut()
                    .try_for_each_mut(store, vcb)?;

                for argument in positional {
                    store[argument as &ValueId]
                        .borrow_mut()
                        .iter_mut()
                        .try_for_each_mut(store, vcb)?;
                }

                for (_name, argument) in named {
                    store[argument as &ValueId]
                        .borrow_mut()
                        .iter_mut()
                        .try_for_each_mut(store, vcb)?;
                }
            }

            Value::MethodCall {
                object,
                method: _,
                positional,
                named,
            } => {
                store[object as &ValueId]
                    .borrow_mut()
                    .iter_mut()
                    .try_for_each_mut(store, vcb)?;

                for argument in positional {
                    store[argument as &ValueId]
                        .borrow_mut()
                        .iter_mut()
                        .try_for_each_mut(store, vcb)?;
                }

                for (_name, argument) in named {
                    store[argument as &ValueId]
                        .borrow_mut()
                        .iter_mut()
                        .try_for_each_mut(store, vcb)?;
                }
            }

            Value::Symbol { path: _ } => {}
        }

        ControlFlow::Continue(())
    }
}

impl GlobalVariableIterMut<'_> {
    pub(crate) fn try_for_each_mut<T>(
        &mut self,
        store: &Store,
        vcb: &mut dyn FnMut(&mut Value) -> ControlFlow<T>,
    ) -> ControlFlow<T> {
        if let Some(init) = &self.node.init {
            store[init]
                .borrow_mut()
                .iter_mut()
                .try_for_each_mut(store, vcb)?;
        }

        ControlFlow::Continue(())
    }
}

impl ModuleIterMut<'_> {
    pub(crate) fn try_for_each_mut<T>(
        &mut self,
        store: &Store,
        vcb: &mut dyn FnMut(&mut Value) -> ControlFlow<T>,
    ) -> ControlFlow<T> {
        for item in &self.node.items {
            match item {
                Item::Module(id) => {
                    store[id]
                        .borrow_mut()
                        .iter_mut()
                        .try_for_each_mut(store, vcb)?;
                }

                Item::GlobalVariable(id) => {
                    store[id]
                        .borrow_mut()
                        .iter_mut()
                        .try_for_each_mut(store, vcb)?;
                }

                Item::Function(id) => {
                    store[id]
                        .borrow_mut()
                        .iter_mut()
                        .try_for_each_mut(store, vcb)?;
                }

                Item::TypeAliasDef(_) => {}

                Item::StructDef(id) => {
                    store[id]
                        .borrow_mut()
                        .iter_mut()
                        .try_for_each_mut(store, vcb)?;
                }

                Item::EnumDef(id) => {
                    store[id]
                        .borrow_mut()
                        .iter_mut()
                        .try_for_each_mut(store, vcb)?;
                }
            }
        }

        ControlFlow::Continue(())
    }
}

impl StructDefIterMut<'_> {
    pub(crate) fn try_for_each_mut<T>(
        &mut self,
        store: &Store,
        vcb: &mut dyn FnMut(&mut Value) -> ControlFlow<T>,
    ) -> ControlFlow<T> {
        for (_vis, default) in &self.node.field_extras {
            if let Some(default_value) = default {
                store[default_value]
                    .borrow_mut()
                    .iter_mut()
                    .try_for_each_mut(store, vcb)?;
            }
        }

        ControlFlow::Continue(())
    }
}

impl EnumDefIterMut<'_> {
    pub(crate) fn try_for_each_mut<T>(
        &mut self,
        store: &Store,
        vcb: &mut dyn FnMut(&mut Value) -> ControlFlow<T>,
    ) -> ControlFlow<T> {
        for default in &self.node.variant_extras {
            if let Some(default_value) = default {
                store[default_value]
                    .borrow_mut()
                    .iter_mut()
                    .try_for_each_mut(store, vcb)?;
            }
        }

        ControlFlow::Continue(())
    }
}

impl FunctionIterMut<'_> {
    pub(crate) fn try_for_each_mut<T>(
        &mut self,
        store: &Store,
        vcb: &mut dyn FnMut(&mut Value) -> ControlFlow<T>,
    ) -> ControlFlow<T> {
        for param in &self.node.params {
            let parameter = store[param].borrow_mut();

            if let Some(default_value) = &parameter.default_value {
                store[default_value]
                    .borrow_mut()
                    .iter_mut()
                    .try_for_each_mut(store, vcb)?;
            }
        }

        if let Some(body) = &self.node.body {
            store[body]
                .borrow_mut()
                .iter_mut()
                .try_for_each_mut(store, vcb)?;
        }

        ControlFlow::Continue(())
    }
}
