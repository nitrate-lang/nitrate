use crate::prelude::*;
use std::{collections::HashSet, ops::ControlFlow};

impl StructTypeIter<'_> {
    pub(crate) fn try_for_each<T>(
        &self,
        store: &Store,
        vcb: &mut dyn FnMut(&Value) -> ControlFlow<T>,
        tcb: &mut dyn FnMut(&Type) -> ControlFlow<T>,
        visited: &mut HashSet<*const ()>,
    ) -> ControlFlow<T> {
        for field in &self.node.fields {
            store[&field.ty]
                .iter()
                .try_for_each(store, vcb, tcb, visited)?;
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
        visited: &mut HashSet<*const ()>,
    ) -> ControlFlow<T> {
        for variant in &self.node.variants {
            store[&variant.ty]
                .iter()
                .try_for_each(store, vcb, tcb, visited)?;
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
        visited: &mut HashSet<*const ()>,
    ) -> ControlFlow<T> {
        for param in &self.node.params {
            store[&param.1]
                .iter()
                .try_for_each(store, vcb, tcb, visited)?;
        }

        store[&self.node.return_type]
            .iter()
            .try_for_each(store, vcb, tcb, visited)?;

        ControlFlow::Continue(())
    }
}

impl TypeIter<'_> {
    pub(crate) fn try_for_each<T>(
        &self,
        store: &Store,
        vcb: &mut dyn FnMut(&Value) -> ControlFlow<T>,
        tcb: &mut dyn FnMut(&Type) -> ControlFlow<T>,
        visited: &mut HashSet<*const ()>,
    ) -> ControlFlow<T> {
        if !visited.insert(self.node as *const Type as *const ()) {
            return ControlFlow::Continue(());
        }

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
            | Type::F64 => {}

            Type::Array {
                element_type,
                len: _,
            } => {
                store[element_type]
                    .iter()
                    .try_for_each(store, vcb, tcb, visited)?;
            }

            Type::Tuple { element_types } => {
                for element_type in element_types {
                    store[element_type]
                        .iter()
                        .try_for_each(store, vcb, tcb, visited)?;
                }
            }

            Type::Struct { struct_type } => {
                for field in &store[struct_type].fields {
                    store[&field.ty]
                        .iter()
                        .try_for_each(store, vcb, tcb, visited)?;
                }
            }

            Type::Enum { enum_type } => {
                for variant in &store[enum_type].variants {
                    store[&variant.ty]
                        .iter()
                        .try_for_each(store, vcb, tcb, visited)?;
                }
            }

            Type::Refine {
                base,
                min: _,
                max: _,
            } => {
                store[base].iter().try_for_each(store, vcb, tcb, visited)?;
            }

            Type::Function { function_type } => {
                let function = &store[function_type];
                for param in &function.params {
                    store[&param.1]
                        .iter()
                        .try_for_each(store, vcb, tcb, visited)?;
                }

                store[&function.return_type]
                    .iter()
                    .try_for_each(store, vcb, tcb, visited)?;
            }

            Type::Reference { to, .. } => {
                store[to].iter().try_for_each(store, vcb, tcb, visited)?;
            }

            Type::SliceRef { element_type, .. } => {
                store[element_type]
                    .iter()
                    .try_for_each(store, vcb, tcb, visited)?;
            }

            Type::Pointer { to, .. } => {
                store[to].iter().try_for_each(store, vcb, tcb, visited)?;
            }

            Type::InferredFloat => {}
            Type::InferredInteger => {}
            Type::Inferred { id: _ } => {}
        }

        visited.remove(&(self.node as *const Type as *const ()));

        ControlFlow::Continue(())
    }
}

impl BlockIter<'_> {
    pub(crate) fn try_for_each<T>(
        &self,
        store: &Store,
        vcb: &mut dyn FnMut(&Value) -> ControlFlow<T>,
        tcb: &mut dyn FnMut(&Type) -> ControlFlow<T>,
        visited: &mut HashSet<*const ()>,
    ) -> ControlFlow<T> {
        for element in &self.node.elements {
            match element {
                BlockElement::Expr(id) => {
                    store[id]
                        .borrow()
                        .iter()
                        .try_for_each(store, vcb, tcb, visited)?;
                }

                BlockElement::Local(id) => {
                    let local_variable = &store[id].borrow();

                    store[&local_variable.ty]
                        .iter()
                        .try_for_each(store, vcb, tcb, visited)?;

                    if let Some(init) = &local_variable.init {
                        store[init]
                            .borrow()
                            .iter()
                            .try_for_each(store, vcb, tcb, visited)?;
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
        visited: &mut HashSet<*const ()>,
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
                        .try_for_each(store, vcb, tcb, visited)?;
                }
            }

            Value::EnumVariant {
                enum_path: _,
                variant: _,
                value,
            } => {
                store[value]
                    .borrow()
                    .iter()
                    .try_for_each(store, vcb, tcb, visited)?;
            }

            Value::Binary { left, op: _, right } => {
                store[left]
                    .borrow()
                    .iter()
                    .try_for_each(store, vcb, tcb, visited)?;

                store[right]
                    .borrow()
                    .iter()
                    .try_for_each(store, vcb, tcb, visited)?;
            }

            Value::Unary { op: _, operand } => {
                store[operand]
                    .borrow()
                    .iter()
                    .try_for_each(store, vcb, tcb, visited)?;
            }

            Value::FieldAccess {
                expr,
                field_name: _,
            } => {
                store[expr]
                    .borrow()
                    .iter()
                    .try_for_each(store, vcb, tcb, visited)?;
            }

            Value::Assign { place, value } => {
                store[place]
                    .borrow()
                    .iter()
                    .try_for_each(store, vcb, tcb, visited)?;

                store[value]
                    .borrow()
                    .iter()
                    .try_for_each(store, vcb, tcb, visited)?;
            }

            Value::Deref { place } => {
                store[place]
                    .borrow()
                    .iter()
                    .try_for_each(store, vcb, tcb, visited)?;
            }

            Value::Cast {
                value: expr,
                target_type: to,
            } => {
                store[expr]
                    .borrow()
                    .iter()
                    .try_for_each(store, vcb, tcb, visited)?;

                store[to].iter().try_for_each(store, vcb, tcb, visited)?;
            }

            Value::Borrow {
                exclusive: _,
                mutable: _,
                place,
            } => {
                store[place]
                    .borrow()
                    .iter()
                    .try_for_each(store, vcb, tcb, visited)?;
            }

            Value::List { elements } => {
                for element in elements {
                    element.iter().try_for_each(store, vcb, tcb, visited)?;
                }
            }

            Value::Tuple { elements } => {
                for element in elements {
                    element.iter().try_for_each(store, vcb, tcb, visited)?;
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
                    .try_for_each(store, vcb, tcb, visited)?;

                store[true_branch]
                    .borrow()
                    .iter()
                    .try_for_each(store, vcb, tcb, visited)?;

                if let Some(false_branch) = false_branch {
                    store[false_branch]
                        .borrow()
                        .iter()
                        .try_for_each(store, vcb, tcb, visited)?;
                }
            }

            Value::While { condition, body } => {
                store[condition]
                    .borrow()
                    .iter()
                    .try_for_each(store, vcb, tcb, visited)?;

                store[body]
                    .borrow()
                    .iter()
                    .try_for_each(store, vcb, tcb, visited)?;
            }

            Value::Loop { body } => {
                store[body]
                    .borrow()
                    .iter()
                    .try_for_each(store, vcb, tcb, visited)?;
            }

            Value::Break { label: _ } => {}

            Value::Continue { label: _ } => {}

            Value::Return { value } => {
                store[value]
                    .borrow()
                    .iter()
                    .try_for_each(store, vcb, tcb, visited)?;
            }

            Value::Block { block } => {
                store[block]
                    .borrow()
                    .iter()
                    .try_for_each(store, vcb, tcb, visited)?;
            }

            Value::Closure {
                captures: _,
                callee,
            } => {
                store[callee]
                    .borrow()
                    .iter()
                    .try_for_each(store, vcb, tcb, visited)?;
            }

            Value::Call {
                callee,
                positional,
                named,
            } => {
                store[callee]
                    .borrow()
                    .iter()
                    .try_for_each(store, vcb, tcb, visited)?;

                for argument in positional {
                    store[argument]
                        .borrow()
                        .iter()
                        .try_for_each(store, vcb, tcb, visited)?;
                }

                for (_name, argument) in named {
                    store[argument]
                        .borrow()
                        .iter()
                        .try_for_each(store, vcb, tcb, visited)?;
                }
            }

            Value::MethodCall {
                object,
                method_name: _,
                positional,
                named,
            } => {
                store[object]
                    .borrow()
                    .iter()
                    .try_for_each(store, vcb, tcb, visited)?;

                for argument in positional {
                    store[argument]
                        .borrow()
                        .iter()
                        .try_for_each(store, vcb, tcb, visited)?;
                }

                for (_name, argument) in named {
                    store[argument]
                        .borrow()
                        .iter()
                        .try_for_each(store, vcb, tcb, visited)?;
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

impl GlobalVariableIter<'_> {
    pub(crate) fn try_for_each<T>(
        &self,
        store: &Store,
        vcb: &mut dyn FnMut(&Value) -> ControlFlow<T>,
        tcb: &mut dyn FnMut(&Type) -> ControlFlow<T>,
        visited: &mut HashSet<*const ()>,
    ) -> ControlFlow<T> {
        store[&self.node.ty]
            .iter()
            .try_for_each(store, vcb, tcb, visited)?;

        store[&self.node.init]
            .borrow()
            .iter()
            .try_for_each(store, vcb, tcb, visited)?;

        ControlFlow::Continue(())
    }
}

impl ModuleIter<'_> {
    pub(crate) fn try_for_each<T>(
        &self,
        store: &Store,
        vcb: &mut dyn FnMut(&Value) -> ControlFlow<T>,
        tcb: &mut dyn FnMut(&Type) -> ControlFlow<T>,
        visited: &mut HashSet<*const ()>,
    ) -> ControlFlow<T> {
        for item in &self.node.items {
            match item {
                Item::Module(id) => {
                    store[id]
                        .borrow()
                        .iter()
                        .try_for_each(store, vcb, tcb, visited)?;
                }

                Item::GlobalVariable(id) => {
                    store[id]
                        .borrow()
                        .iter()
                        .try_for_each(store, vcb, tcb, visited)?;
                }

                Item::Function(id) => {
                    store[id]
                        .borrow()
                        .iter()
                        .try_for_each(store, vcb, tcb, visited)?;
                }

                Item::TypeAliasDef(id) => {
                    store[id]
                        .borrow()
                        .iter()
                        .try_for_each(store, vcb, tcb, visited)?;
                }

                Item::StructDef(id) => {
                    store[id]
                        .borrow()
                        .iter()
                        .try_for_each(store, vcb, tcb, visited)?;
                }

                Item::EnumDef(id) => {
                    store[id]
                        .borrow()
                        .iter()
                        .try_for_each(store, vcb, tcb, visited)?;
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
        visited: &mut HashSet<*const ()>,
    ) -> ControlFlow<T> {
        store[&self.node.type_id]
            .iter()
            .try_for_each(store, vcb, tcb, visited)?;

        ControlFlow::Continue(())
    }
}

impl StructDefIter<'_> {
    pub(crate) fn try_for_each<T>(
        &self,
        store: &Store,
        vcb: &mut dyn FnMut(&Value) -> ControlFlow<T>,
        tcb: &mut dyn FnMut(&Type) -> ControlFlow<T>,
        visited: &mut HashSet<*const ()>,
    ) -> ControlFlow<T> {
        for (_vis, default) in &self.node.field_extras {
            if let Some(default_value) = default {
                store[default_value]
                    .borrow()
                    .iter()
                    .try_for_each(store, vcb, tcb, visited)?;
            }
        }

        store[&self.node.struct_id]
            .iter()
            .try_for_each(store, vcb, tcb, visited)?;

        ControlFlow::Continue(())
    }
}

impl EnumDefIter<'_> {
    pub(crate) fn try_for_each<T>(
        &self,
        store: &Store,
        vcb: &mut dyn FnMut(&Value) -> ControlFlow<T>,
        tcb: &mut dyn FnMut(&Type) -> ControlFlow<T>,
        visited: &mut HashSet<*const ()>,
    ) -> ControlFlow<T> {
        for default in &self.node.variant_extras {
            if let Some(default_value) = default {
                store[default_value]
                    .borrow()
                    .iter()
                    .try_for_each(store, vcb, tcb, visited)?;
            }
        }

        store[&self.node.enum_id]
            .iter()
            .try_for_each(store, vcb, tcb, visited)?;

        ControlFlow::Continue(())
    }
}

impl FunctionIter<'_> {
    pub(crate) fn try_for_each<T>(
        &self,
        store: &Store,
        vcb: &mut dyn FnMut(&Value) -> ControlFlow<T>,
        tcb: &mut dyn FnMut(&Type) -> ControlFlow<T>,
        visited: &mut HashSet<*const ()>,
    ) -> ControlFlow<T> {
        for param in &self.node.params {
            let parameter = store[param].borrow();

            store[&parameter.ty]
                .iter()
                .try_for_each(store, vcb, tcb, visited)?;

            if let Some(default_value) = &parameter.default_value {
                store[default_value]
                    .borrow()
                    .iter()
                    .try_for_each(store, vcb, tcb, visited)?;
            }
        }

        store[&self.node.return_type]
            .iter()
            .try_for_each(store, vcb, tcb, visited)?;

        if let Some(body) = &self.node.body {
            store[body]
                .borrow()
                .iter()
                .try_for_each(store, vcb, tcb, visited)?;
        }

        ControlFlow::Continue(())
    }
}
