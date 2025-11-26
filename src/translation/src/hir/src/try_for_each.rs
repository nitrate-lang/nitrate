use crate::prelude::*;
use std::{
    collections::HashSet,
    ops::{ControlFlow, Deref},
};

impl FunctionTypeIter<'_> {
    pub(crate) fn try_for_each<T>(
        &self,
        vcb: &mut dyn FnMut(&Value) -> ControlFlow<T>,
        tcb: &mut dyn FnMut(&Type) -> ControlFlow<T>,
        visited: &mut HashSet<*const ()>,
    ) -> ControlFlow<T> {
        for param in &self.node.params {
            param.1.iter().try_for_each(vcb, tcb, visited)?;
        }

        self.node.return_type.iter().try_for_each(vcb, tcb, visited)?;

        ControlFlow::Continue(())
    }
}

impl TypeIter<'_> {
    pub(crate) fn try_for_each<T>(
        &self,

        vcb: &mut dyn FnMut(&Value) -> ControlFlow<T>,
        tcb: &mut dyn FnMut(&Type) -> ControlFlow<T>,
        visited: &mut HashSet<*const ()>,
    ) -> ControlFlow<T> {
        if !visited.insert(std::ptr::from_ref::<Type>(self.node).cast::<()>()) {
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

            Type::Array { element_type, len: _ } => {
                element_type.iter().try_for_each(vcb, tcb, visited)?;
            }

            Type::Tuple { element_types } => {
                for element_type in element_types {
                    element_type.iter().try_for_each(vcb, tcb, visited)?;
                }
            }

            Type::Struct { def } => {
                for field in def.borrow().fields.values() {
                    field.ty.iter().try_for_each(vcb, tcb, visited)?;
                }
            }

            Type::Enum { def } => {
                for variant in &def.borrow().variants {
                    variant.ty.iter().try_for_each(vcb, tcb, visited)?;
                }
            }

            Type::TypeAlias { def } => {
                let type_alias = &def.borrow().type_id;
                type_alias.iter().try_for_each(vcb, tcb, visited)?;
            }

            Type::Refine { base, min: _, max: _ } => {
                base.iter().try_for_each(vcb, tcb, visited)?;
            }

            Type::Function { function_type } => {
                for param in &function_type.deref().params {
                    param.1.iter().try_for_each(vcb, tcb, visited)?;
                }

                function_type.return_type.iter().try_for_each(vcb, tcb, visited)?;
            }

            Type::Reference { to, .. } => {
                to.iter().try_for_each(vcb, tcb, visited)?;
            }

            Type::SliceRef { element_type, .. } => {
                element_type.iter().try_for_each(vcb, tcb, visited)?;
            }

            Type::Pointer { to, .. } => {
                to.iter().try_for_each(vcb, tcb, visited)?;
            }

            Type::InferredFloat => {}
            Type::InferredInteger => {}
            Type::Inferred { id: _ } => {}
        }

        visited.remove(&std::ptr::from_ref::<Type>(self.node).cast::<()>());

        ControlFlow::Continue(())
    }
}

impl BlockIter<'_> {
    pub(crate) fn try_for_each<T>(
        &self,

        vcb: &mut dyn FnMut(&Value) -> ControlFlow<T>,
        tcb: &mut dyn FnMut(&Type) -> ControlFlow<T>,
        visited: &mut HashSet<*const ()>,
    ) -> ControlFlow<T> {
        for element in &self.node.elements {
            match element {
                BlockElement::Expr(id) => {
                    id.borrow().iter().try_for_each(vcb, tcb, visited)?;
                }

                BlockElement::Local(id) => {
                    let local_variable = &id.borrow();

                    local_variable.ty.iter().try_for_each(vcb, tcb, visited)?;

                    local_variable
                        .initializer
                        .borrow()
                        .iter()
                        .try_for_each(vcb, tcb, visited)?;
                }
            }
        }

        ControlFlow::Continue(())
    }
}

impl ValueIter<'_> {
    pub(crate) fn try_for_each<T>(
        &self,

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

            Value::StructObject { struct_def: _, fields } => {
                for (_field_name, field_value) in fields {
                    field_value.borrow().iter().try_for_each(vcb, tcb, visited)?;
                }
            }

            Value::EnumVariant {
                enum_def: _,
                variant: _,
                value,
            } => {
                value.borrow().iter().try_for_each(vcb, tcb, visited)?;
            }

            Value::Binary { left, op: _, right } => {
                left.borrow().iter().try_for_each(vcb, tcb, visited)?;

                right.borrow().iter().try_for_each(vcb, tcb, visited)?;
            }

            Value::Unary { op: _, operand } => {
                operand.borrow().iter().try_for_each(vcb, tcb, visited)?;
            }

            Value::FieldAccess { expr, field_name: _ } => {
                expr.borrow().iter().try_for_each(vcb, tcb, visited)?;
            }

            Value::Assign { place, value } => {
                place.borrow().iter().try_for_each(vcb, tcb, visited)?;

                value.borrow().iter().try_for_each(vcb, tcb, visited)?;
            }

            Value::Deref { place } => {
                place.borrow().iter().try_for_each(vcb, tcb, visited)?;
            }

            Value::Cast {
                value: expr,
                target_type: to,
            } => {
                expr.borrow().iter().try_for_each(vcb, tcb, visited)?;

                to.iter().try_for_each(vcb, tcb, visited)?;
            }

            Value::Borrow {
                exclusive: _,
                mutable: _,
                place,
            } => {
                place.borrow().iter().try_for_each(vcb, tcb, visited)?;
            }

            Value::List { elements } => {
                for element in elements {
                    element.borrow_mut().iter().try_for_each(vcb, tcb, visited)?;
                }
            }

            Value::Tuple { elements } => {
                for element in elements {
                    element.borrow_mut().iter().try_for_each(vcb, tcb, visited)?;
                }
            }

            Value::If {
                condition,
                true_branch,
                false_branch,
            } => {
                condition.borrow().iter().try_for_each(vcb, tcb, visited)?;

                true_branch.borrow().iter().try_for_each(vcb, tcb, visited)?;

                if let Some(false_branch) = false_branch {
                    false_branch.borrow().iter().try_for_each(vcb, tcb, visited)?;
                }
            }

            Value::While { condition, body } => {
                condition.borrow().iter().try_for_each(vcb, tcb, visited)?;

                body.borrow().iter().try_for_each(vcb, tcb, visited)?;
            }

            Value::Loop { body } => {
                body.borrow().iter().try_for_each(vcb, tcb, visited)?;
            }

            Value::Break { label: _ } => {}

            Value::Continue { label: _ } => {}

            Value::Return { value } => {
                value.borrow().iter().try_for_each(vcb, tcb, visited)?;
            }

            Value::Block { block } => {
                block.borrow().iter().try_for_each(vcb, tcb, visited)?;
            }

            Value::Call {
                callee,
                positional,
                named,
            } => {
                callee.borrow().iter().try_for_each(vcb, tcb, visited)?;

                for argument in positional {
                    argument.borrow().iter().try_for_each(vcb, tcb, visited)?;
                }

                for (_name, argument) in named {
                    argument.borrow().iter().try_for_each(vcb, tcb, visited)?;
                }
            }

            Value::MethodCall {
                object,
                method_name: _,
                positional,
                named,
            } => {
                object.borrow().iter().try_for_each(vcb, tcb, visited)?;

                for argument in positional {
                    argument.borrow().iter().try_for_each(vcb, tcb, visited)?;
                }

                for (_name, argument) in named {
                    argument.borrow().iter().try_for_each(vcb, tcb, visited)?;
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

        vcb: &mut dyn FnMut(&Value) -> ControlFlow<T>,
        tcb: &mut dyn FnMut(&Type) -> ControlFlow<T>,
        visited: &mut HashSet<*const ()>,
    ) -> ControlFlow<T> {
        self.node.ty.iter().try_for_each(vcb, tcb, visited)?;

        self.node.initializer.borrow().iter().try_for_each(vcb, tcb, visited)?;

        ControlFlow::Continue(())
    }
}

impl ModuleIter<'_> {
    pub(crate) fn try_for_each<T>(
        &self,

        vcb: &mut dyn FnMut(&Value) -> ControlFlow<T>,
        tcb: &mut dyn FnMut(&Type) -> ControlFlow<T>,
        visited: &mut HashSet<*const ()>,
    ) -> ControlFlow<T> {
        for item in &self.node.items {
            match item {
                Item::Module(id) => {
                    id.borrow().iter().try_for_each(vcb, tcb, visited)?;
                }

                Item::GlobalVariable(id) => {
                    id.borrow().iter().try_for_each(vcb, tcb, visited)?;
                }

                Item::Function(id) => {
                    id.borrow().iter().try_for_each(vcb, tcb, visited)?;
                }

                Item::TypeAliasDef(id) => {
                    id.borrow().iter().try_for_each(vcb, tcb, visited)?;
                }

                Item::StructDef(id) => {
                    id.borrow().iter().try_for_each(vcb, tcb, visited)?;
                }

                Item::EnumDef(id) => {
                    id.borrow().iter().try_for_each(vcb, tcb, visited)?;
                }
            }
        }

        ControlFlow::Continue(())
    }
}

impl TypeAliasDefIter<'_> {
    pub(crate) fn try_for_each<T>(
        &self,

        vcb: &mut dyn FnMut(&Value) -> ControlFlow<T>,
        tcb: &mut dyn FnMut(&Type) -> ControlFlow<T>,
        visited: &mut HashSet<*const ()>,
    ) -> ControlFlow<T> {
        self.node.type_id.iter().try_for_each(vcb, tcb, visited)?;

        ControlFlow::Continue(())
    }
}

impl StructDefIter<'_> {
    pub(crate) fn try_for_each<T>(
        &self,

        vcb: &mut dyn FnMut(&Value) -> ControlFlow<T>,
        tcb: &mut dyn FnMut(&Type) -> ControlFlow<T>,
        visited: &mut HashSet<*const ()>,
    ) -> ControlFlow<T> {
        for field in self.node.fields.values() {
            if let Some(default_value) = &field.default_value {
                default_value.borrow().iter().try_for_each(vcb, tcb, visited)?;
            }
        }

        for field in self.node.fields.values() {
            field.ty.iter().try_for_each(vcb, tcb, visited)?;
        }

        ControlFlow::Continue(())
    }
}

impl EnumDefIter<'_> {
    pub(crate) fn try_for_each<T>(
        &self,
        vcb: &mut dyn FnMut(&Value) -> ControlFlow<T>,
        tcb: &mut dyn FnMut(&Type) -> ControlFlow<T>,
        visited: &mut HashSet<*const ()>,
    ) -> ControlFlow<T> {
        for variant in self.node.variants.iter() {
            if let Some(default_value) = &variant.default_value {
                default_value.borrow().iter().try_for_each(vcb, tcb, visited)?;
            }
        }

        for variant in &self.node.variants {
            variant.ty.iter().try_for_each(vcb, tcb, visited)?;
        }

        ControlFlow::Continue(())
    }
}

impl FunctionIter<'_> {
    pub(crate) fn try_for_each<T>(
        &self,

        vcb: &mut dyn FnMut(&Value) -> ControlFlow<T>,
        tcb: &mut dyn FnMut(&Type) -> ControlFlow<T>,
        visited: &mut HashSet<*const ()>,
    ) -> ControlFlow<T> {
        for param in &self.node.params {
            let parameter = param.borrow();

            parameter.ty.iter().try_for_each(vcb, tcb, visited)?;

            if let Some(default_value) = &parameter.default_value {
                default_value.borrow().iter().try_for_each(vcb, tcb, visited)?;
            }
        }

        self.node.return_type.iter().try_for_each(vcb, tcb, visited)?;

        if let Some(body) = &self.node.body {
            for element in body {
                match element {
                    BlockElement::Expr(id) => id.borrow().iter().try_for_each(vcb, tcb, visited)?,
                    BlockElement::Local(id) => {
                        let local_variable = &id.borrow();
                        local_variable
                            .initializer
                            .borrow()
                            .iter()
                            .try_for_each(vcb, tcb, visited)?;
                    }
                }
            }
        }

        ControlFlow::Continue(())
    }
}
