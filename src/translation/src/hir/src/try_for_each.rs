use crate::prelude::*;
use std::ops::ControlFlow;

impl StructTypeIter<'_> {
    pub(crate) fn try_for_each<T>(
        &self,
        store: &Store,
        vcb: &mut dyn FnMut(&Value) -> ControlFlow<T>,
        tcb: &mut dyn FnMut(&Type) -> ControlFlow<T>,
    ) -> ControlFlow<T> {
        for field in &self.node.fields {
            store[&field.ty].iter().try_for_each(store, vcb, tcb)?;
        }

        ControlFlow::Continue(())
    }
}

impl EnumTypeIter<'_> {
    pub(crate) fn try_for_each<T>(
        &self,
        store: &Store,
        vcb: &mut dyn FnMut(&Value) -> ControlFlow<T>,
        tcb: &mut dyn FnMut(&Type) -> ControlFlow<T>,
    ) -> ControlFlow<T> {
        for variant in &self.node.variants {
            store[&variant.ty].iter().try_for_each(store, vcb, tcb)?;
        }

        ControlFlow::Continue(())
    }
}

impl FunctionTypeIter<'_> {
    pub(crate) fn try_for_each<T>(
        &self,
        store: &Store,
        vcb: &mut dyn FnMut(&Value) -> ControlFlow<T>,
        tcb: &mut dyn FnMut(&Type) -> ControlFlow<T>,
    ) -> ControlFlow<T> {
        for param in &self.node.params {
            let parameter = store[param].borrow();

            store[&parameter.ty].iter().try_for_each(store, vcb, tcb)?;

            if let Some(default_value) = &parameter.default_value {
                store[default_value]
                    .borrow()
                    .iter()
                    .try_for_each(store, vcb, tcb)?;
            }
        }

        store[&self.node.return_type]
            .iter()
            .try_for_each(store, vcb, tcb)?;

        ControlFlow::Continue(())
    }
}

impl TypeIter<'_> {
    pub(crate) fn try_for_each<T>(
        &self,
        store: &Store,
        vcb: &mut dyn FnMut(&Value) -> ControlFlow<T>,
        tcb: &mut dyn FnMut(&Type) -> ControlFlow<T>,
    ) -> ControlFlow<T> {
        tcb(self.node)?;

        match self.node {
            Type::Never
            | Type::Unit
            | Type::Bool
            | Type::U8
            | Type::U16
            | Type::U32
            | Type::U64
            | Type::U128
            | Type::USize
            | Type::I8
            | Type::I16
            | Type::I32
            | Type::I64
            | Type::I128
            | Type::F32
            | Type::F64
            | Type::Opaque { name: _ } => {}

            Type::Array {
                element_type,
                len: _,
            } => {
                store[element_type].iter().try_for_each(store, vcb, tcb)?;
            }

            Type::Tuple { element_types } => {
                for element_type in element_types {
                    store[element_type].iter().try_for_each(store, vcb, tcb)?;
                }
            }

            Type::Slice { element_type } => {
                store[element_type].iter().try_for_each(store, vcb, tcb)?;
            }

            Type::Struct { struct_type } => {
                for field in &store[struct_type].fields {
                    store[&field.ty].iter().try_for_each(store, vcb, tcb)?;
                }
            }

            Type::Enum { enum_type } => {
                for variant in &store[enum_type].variants {
                    store[&variant.ty].iter().try_for_each(store, vcb, tcb)?;
                }
            }

            Type::Refine {
                base,
                min: _,
                max: _,
            } => {
                store[base].iter().try_for_each(store, vcb, tcb)?;
            }

            Type::Bitfield { base, bits: _ } => {
                store[base].iter().try_for_each(store, vcb, tcb)?;
            }

            Type::Function { function_type } => {
                let function = &store[function_type];
                for param in &function.params {
                    let parameter = store[param].borrow();

                    store[&parameter.ty].iter().try_for_each(store, vcb, tcb)?;

                    if let Some(default_value) = &parameter.default_value {
                        store[default_value]
                            .borrow()
                            .iter()
                            .try_for_each(store, vcb, tcb)?;
                    }
                }

                store[&function.return_type]
                    .iter()
                    .try_for_each(store, vcb, tcb)?;
            }

            Type::Reference {
                lifetime: _,
                exclusive: _,
                mutable: _,
                to,
            } => {
                store[to].iter().try_for_each(store, vcb, tcb)?;
            }

            Type::Pointer {
                exclusive: _,
                mutable: _,
                to,
            } => {
                store[to].iter().try_for_each(store, vcb, tcb)?;
            }

            Type::Symbol { path: _ } => {}
            Type::InferredFloat => {}
            Type::InferredInteger => {}
            Type::Inferred { id: _ } => {}
        }

        ControlFlow::Continue(())
    }
}

impl BlockIter<'_> {
    pub(crate) fn try_for_each<T>(
        &self,
        store: &Store,
        vcb: &mut dyn FnMut(&Value) -> ControlFlow<T>,
        tcb: &mut dyn FnMut(&Type) -> ControlFlow<T>,
    ) -> ControlFlow<T> {
        for element in &self.node.elements {
            match element {
                BlockElement::Expr(id) => {
                    store[id].borrow().iter().try_for_each(store, vcb, tcb)?;
                }

                BlockElement::Stmt(id) => {
                    store[id].borrow().iter().try_for_each(store, vcb, tcb)?;
                }

                BlockElement::Local(id) => {
                    let local_variable = &store[id].borrow();

                    store[&local_variable.ty]
                        .iter()
                        .try_for_each(store, vcb, tcb)?;

                    if let Some(init) = &local_variable.init {
                        store[init].borrow().iter().try_for_each(store, vcb, tcb)?;
                    }
                }
            }
        }

        ControlFlow::Continue(())
    }
}

impl ValueIter<'_> {
    pub(crate) fn try_for_each<T>(
        &self,
        store: &Store,
        vcb: &mut dyn FnMut(&Value) -> ControlFlow<T>,
        tcb: &mut dyn FnMut(&Type) -> ControlFlow<T>,
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
                    store[field_value]
                        .borrow()
                        .iter()
                        .try_for_each(store, vcb, tcb)?;
                }
            }

            Value::EnumVariant {
                enum_path: _,
                variant: _,
                value,
            } => {
                store[value].borrow().iter().try_for_each(store, vcb, tcb)?;
            }

            Value::Binary { left, op: _, right } => {
                store[left].borrow().iter().try_for_each(store, vcb, tcb)?;

                store[right].borrow().iter().try_for_each(store, vcb, tcb)?;
            }

            Value::Unary { op: _, operand } => {
                store[operand]
                    .borrow()
                    .iter()
                    .try_for_each(store, vcb, tcb)?;
            }

            Value::FieldAccess { expr, field: _ } => {
                store[expr].borrow().iter().try_for_each(store, vcb, tcb)?;
            }

            Value::IndexAccess { collection, index } => {
                store[collection]
                    .borrow()
                    .iter()
                    .try_for_each(store, vcb, tcb)?;

                store[index].borrow().iter().try_for_each(store, vcb, tcb)?;
            }

            Value::Assign { place, value } => {
                store[place].borrow().iter().try_for_each(store, vcb, tcb)?;

                store[value].borrow().iter().try_for_each(store, vcb, tcb)?;
            }

            Value::Deref { place } => {
                store[place].borrow().iter().try_for_each(store, vcb, tcb)?;
            }

            Value::Cast { expr, to } => {
                store[expr].borrow().iter().try_for_each(store, vcb, tcb)?;

                store[to].iter().try_for_each(store, vcb, tcb)?;
            }

            Value::Borrow { mutable: _, place } => {
                store[place].borrow().iter().try_for_each(store, vcb, tcb)?;
            }

            Value::List { elements } => {
                for element in elements {
                    element.iter().try_for_each(store, vcb, tcb)?;
                }
            }

            Value::Tuple { elements } => {
                for element in elements {
                    element.iter().try_for_each(store, vcb, tcb)?;
                }
            }

            Value::If {
                condition,
                true_branch,
                false_branch,
            } => {
                store[condition]
                    .borrow()
                    .iter()
                    .try_for_each(store, vcb, tcb)?;

                store[true_branch]
                    .borrow()
                    .iter()
                    .try_for_each(store, vcb, tcb)?;

                if let Some(false_branch) = false_branch {
                    store[false_branch]
                        .borrow()
                        .iter()
                        .try_for_each(store, vcb, tcb)?;
                }
            }

            Value::While { condition, body } => {
                store[condition]
                    .borrow()
                    .iter()
                    .try_for_each(store, vcb, tcb)?;

                store[body].borrow().iter().try_for_each(store, vcb, tcb)?;
            }

            Value::Loop { body } => {
                store[body].borrow().iter().try_for_each(store, vcb, tcb)?;
            }

            Value::Break { label: _ } => {}

            Value::Continue { label: _ } => {}

            Value::Return { value } => {
                store[value].borrow().iter().try_for_each(store, vcb, tcb)?;
            }

            Value::Block { block } => {
                store[block].borrow().iter().try_for_each(store, vcb, tcb)?;
            }

            Value::Closure {
                captures: _,
                callee,
            } => {
                store[callee]
                    .borrow()
                    .iter()
                    .try_for_each(store, vcb, tcb)?;
            }

            Value::Call { callee, arguments } => {
                store[callee]
                    .borrow()
                    .iter()
                    .try_for_each(store, vcb, tcb)?;

                for (_name, argument) in arguments {
                    store[argument]
                        .borrow()
                        .iter()
                        .try_for_each(store, vcb, tcb)?;
                }
            }

            Value::MethodCall {
                object,
                method: _,
                arguments,
            } => {
                store[object]
                    .borrow()
                    .iter()
                    .try_for_each(store, vcb, tcb)?;

                for (_name, argument) in arguments {
                    store[argument]
                        .borrow()
                        .iter()
                        .try_for_each(store, vcb, tcb)?;
                }
            }

            Value::Symbol { path: _ } => {}
        }

        ControlFlow::Continue(())
    }
}

impl GlobalVariableIter<'_> {
    pub(crate) fn try_for_each<T>(
        &self,
        store: &Store,
        vcb: &mut dyn FnMut(&Value) -> ControlFlow<T>,
        tcb: &mut dyn FnMut(&Type) -> ControlFlow<T>,
    ) -> ControlFlow<T> {
        store[&self.node.ty].iter().try_for_each(store, vcb, tcb)?;

        if let Some(init) = &self.node.init {
            store[init].borrow().iter().try_for_each(store, vcb, tcb)?;
        }

        ControlFlow::Continue(())
    }
}

impl ModuleIter<'_> {
    pub(crate) fn try_for_each<T>(
        &self,
        store: &Store,
        vcb: &mut dyn FnMut(&Value) -> ControlFlow<T>,
        tcb: &mut dyn FnMut(&Type) -> ControlFlow<T>,
    ) -> ControlFlow<T> {
        for item in &self.node.items {
            match item {
                Item::Module(id) => {
                    store[id].borrow().iter().try_for_each(store, vcb, tcb)?;
                }

                Item::GlobalVariable(id) => {
                    store[id].borrow().iter().try_for_each(store, vcb, tcb)?;
                }

                Item::Function(id) => {
                    store[id].borrow().iter().try_for_each(store, vcb, tcb)?;
                }

                Item::TypeAliasDef(id) => {
                    store[id].borrow().iter().try_for_each(store, vcb, tcb)?;
                }

                Item::StructDef(id) => {
                    store[id].borrow().iter().try_for_each(store, vcb, tcb)?;
                }

                Item::EnumDef(id) => {
                    store[id].borrow().iter().try_for_each(store, vcb, tcb)?;
                }
            }
        }

        ControlFlow::Continue(())
    }
}

impl TypeAliasDefIter<'_> {
    pub(crate) fn try_for_each<T>(
        &self,
        store: &Store,
        vcb: &mut dyn FnMut(&Value) -> ControlFlow<T>,
        tcb: &mut dyn FnMut(&Type) -> ControlFlow<T>,
    ) -> ControlFlow<T> {
        store[&self.node.type_id]
            .iter()
            .try_for_each(store, vcb, tcb)?;

        ControlFlow::Continue(())
    }
}

impl StructDefIter<'_> {
    pub(crate) fn try_for_each<T>(
        &self,
        store: &Store,
        vcb: &mut dyn FnMut(&Value) -> ControlFlow<T>,
        tcb: &mut dyn FnMut(&Type) -> ControlFlow<T>,
    ) -> ControlFlow<T> {
        for (_vis, default) in &self.node.field_extras {
            if let Some(default_value) = default {
                store[default_value]
                    .borrow()
                    .iter()
                    .try_for_each(store, vcb, tcb)?;
            }
        }

        store[&self.node.struct_id]
            .iter()
            .try_for_each(store, vcb, tcb)?;

        ControlFlow::Continue(())
    }
}

impl EnumDefIter<'_> {
    pub(crate) fn try_for_each<T>(
        &self,
        store: &Store,
        vcb: &mut dyn FnMut(&Value) -> ControlFlow<T>,
        tcb: &mut dyn FnMut(&Type) -> ControlFlow<T>,
    ) -> ControlFlow<T> {
        for default in &self.node.variant_extras {
            if let Some(default_value) = default {
                store[default_value]
                    .borrow()
                    .iter()
                    .try_for_each(store, vcb, tcb)?;
            }
        }

        store[&self.node.enum_id]
            .iter()
            .try_for_each(store, vcb, tcb)?;

        ControlFlow::Continue(())
    }
}

impl FunctionIter<'_> {
    pub(crate) fn try_for_each<T>(
        &self,
        store: &Store,
        vcb: &mut dyn FnMut(&Value) -> ControlFlow<T>,
        tcb: &mut dyn FnMut(&Type) -> ControlFlow<T>,
    ) -> ControlFlow<T> {
        for param in &self.node.params {
            let parameter = store[param].borrow();

            store[&parameter.ty].iter().try_for_each(store, vcb, tcb)?;

            if let Some(default_value) = &parameter.default_value {
                store[default_value]
                    .borrow()
                    .iter()
                    .try_for_each(store, vcb, tcb)?;
            }
        }

        store[&self.node.return_type]
            .iter()
            .try_for_each(store, vcb, tcb)?;

        if let Some(body) = &self.node.body {
            store[body].borrow().iter().try_for_each(store, vcb, tcb)?;
        }

        ControlFlow::Continue(())
    }
}
