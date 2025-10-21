use crate::{
    Store,
    hir::{Block, BlockElement, Function, Value},
    ty::Type,
};
use std::ops::ControlFlow;

pub struct TypeIter<'a> {
    node: &'a Type,
}

impl TypeIter<'_> {
    pub fn try_for_each<T, Expr, Ty, Item>(
        &self,
        store: &Store,
        mut vcb: Expr,
        mut tcb: Ty,
        mut icb: Item,
    ) -> ControlFlow<T>
    where
        Expr: FnMut(&Value) -> ControlFlow<T>,
        Ty: FnMut(&Type) -> ControlFlow<T>,
        Item: FnMut(&Value) -> ControlFlow<T>,
    {
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
                store[element_type]
                    .iter()
                    .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;
            }

            Type::Tuple { element_types } => {
                for element_type in element_types {
                    store[element_type]
                        .iter()
                        .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;
                }
            }

            Type::Slice { element_type } => {
                store[element_type]
                    .iter()
                    .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;
            }

            Type::Struct { struct_type } => {
                for field in &store[struct_type].fields {
                    store[&field.ty]
                        .iter()
                        .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;
                }
            }

            Type::Enum { enum_type } => {
                for variant in &store[enum_type].variants {
                    store[&variant.ty]
                        .iter()
                        .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;
                }
            }

            Type::Refine {
                base,
                min: _,
                max: _,
            } => {
                store[base]
                    .iter()
                    .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;
            }

            Type::Bitfield { base, bits: _ } => {
                store[base]
                    .iter()
                    .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;
            }

            Type::Function { function_type } => {
                let function = &store[function_type];
                for param in &function.params {
                    let parameter = store[param].borrow();

                    store[&parameter.ty]
                        .iter()
                        .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;

                    if let Some(default_value) = &parameter.default_value {
                        store[default_value]
                            .borrow()
                            .iter()
                            .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;
                    }
                }

                store[&function.return_type]
                    .iter()
                    .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;
            }

            Type::Reference {
                lifetime: _,
                exclusive: _,
                mutable: _,
                to,
            } => {
                store[to]
                    .iter()
                    .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;
            }

            Type::Pointer {
                exclusive: _,
                mutable: _,
                to,
            } => {
                store[to]
                    .iter()
                    .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;
            }

            Type::Symbol { path: _ } => {}
            Type::InferredFloat => {}
            Type::InferredInteger => {}
            Type::Inferred { id: _ } => {}
        }

        ControlFlow::Continue(())
    }
}

pub struct BlockIter<'a> {
    node: &'a Block,
}

impl BlockIter<'_> {
    pub fn try_for_each<T, Expr, Ty, Item>(
        &self,
        store: &Store,
        mut vcb: Expr,
        mut tcb: Ty,
        mut icb: Item,
    ) -> ControlFlow<T>
    where
        Expr: FnMut(&Value) -> ControlFlow<T>,
        Ty: FnMut(&Type) -> ControlFlow<T>,
        Item: FnMut(&Value) -> ControlFlow<T>,
    {
        for element in &self.node.elements {
            match element {
                BlockElement::Expr(id) => {
                    store[id]
                        .borrow()
                        .iter()
                        .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;
                }

                BlockElement::Stmt(id) => {
                    store[id]
                        .borrow()
                        .iter()
                        .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;
                }

                BlockElement::Local(id) => {
                    let local_variable = &store[id].borrow();

                    store[&local_variable.ty]
                        .iter()
                        .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;

                    if let Some(init) = &local_variable.init {
                        store[init]
                            .borrow()
                            .iter()
                            .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;
                    }
                }
            }
        }

        ControlFlow::Continue(())
    }
}

pub struct ValueIter<'a> {
    node: &'a Value,
}

impl ValueIter<'_> {
    pub fn try_for_each<T, Expr, Ty, Item>(
        &self,
        store: &Store,
        mut vcb: Expr,
        mut tcb: Ty,
        mut icb: Item,
    ) -> ControlFlow<T>
    where
        Expr: FnMut(&Value) -> ControlFlow<T>,
        Ty: FnMut(&Type) -> ControlFlow<T>,
        Item: FnMut(&Value) -> ControlFlow<T>,
    {
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
                        .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;
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
                    .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;
            }

            Value::Binary { left, op: _, right } => {
                store[left]
                    .borrow()
                    .iter()
                    .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;

                store[right]
                    .borrow()
                    .iter()
                    .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;
            }

            Value::Unary { op: _, operand } => {
                store[operand]
                    .borrow()
                    .iter()
                    .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;
            }

            Value::FieldAccess { expr, field: _ } => {
                store[expr]
                    .borrow()
                    .iter()
                    .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;
            }

            Value::IndexAccess { collection, index } => {
                store[collection]
                    .borrow()
                    .iter()
                    .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;

                store[index]
                    .borrow()
                    .iter()
                    .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;
            }

            Value::Assign { place, value } => {
                store[place]
                    .borrow()
                    .iter()
                    .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;

                store[value]
                    .borrow()
                    .iter()
                    .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;
            }

            Value::Deref { place } => {
                store[place]
                    .borrow()
                    .iter()
                    .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;
            }

            Value::Cast { expr, to } => {
                store[expr]
                    .borrow()
                    .iter()
                    .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;

                store[to]
                    .iter()
                    .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;
            }

            Value::Borrow { mutable: _, place } => {
                store[place]
                    .borrow()
                    .iter()
                    .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;
            }

            Value::List { elements } => {
                for element in elements {
                    element
                        .iter()
                        .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;
                }
            }

            Value::Tuple { elements } => {
                for element in elements {
                    element
                        .iter()
                        .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;
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
                    .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;

                store[true_branch]
                    .borrow()
                    .iter()
                    .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;

                if let Some(false_branch) = false_branch {
                    store[false_branch]
                        .borrow()
                        .iter()
                        .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;
                }
            }

            Value::While { condition, body } => {
                store[condition]
                    .borrow()
                    .iter()
                    .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;

                store[body]
                    .borrow()
                    .iter()
                    .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;
            }

            Value::Loop { body } => {
                store[body]
                    .borrow()
                    .iter()
                    .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;
            }

            Value::Break { label: _ } => {}

            Value::Continue { label: _ } => {}

            Value::Return { value } => {
                store[value]
                    .borrow()
                    .iter()
                    .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;
            }

            Value::Block { block } => {
                store[block]
                    .borrow()
                    .iter()
                    .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;
            }

            Value::Closure {
                captures: _,
                callee,
            } => {
                store[callee]
                    .borrow()
                    .iter()
                    .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;
            }

            Value::Call { callee, arguments } => {
                store[callee]
                    .borrow()
                    .iter()
                    .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;

                for (_name, argument) in arguments {
                    store[argument]
                        .borrow()
                        .iter()
                        .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;
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
                    .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;

                for (_name, argument) in arguments {
                    store[argument]
                        .borrow()
                        .iter()
                        .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;
                }
            }

            Value::Symbol { path: _ } => {}
        }

        ControlFlow::Continue(())
    }
}

pub struct FunctionIter<'a> {
    node: &'a Function,
}

impl FunctionIter<'_> {
    pub fn try_for_each<T, Expr, Ty, Item>(
        &self,
        store: &Store,
        mut vcb: Expr,
        mut tcb: Ty,
        mut icb: Item,
    ) -> ControlFlow<T>
    where
        Expr: FnMut(&Value) -> ControlFlow<T>,
        Ty: FnMut(&Type) -> ControlFlow<T>,
        Item: FnMut(&Value) -> ControlFlow<T>,
    {
        for param in &self.node.params {
            let parameter = store[param].borrow();

            store[&parameter.ty]
                .iter()
                .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;

            if let Some(default_value) = &parameter.default_value {
                store[default_value]
                    .borrow()
                    .iter()
                    .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;
            }
        }

        store[&self.node.return_type]
            .iter()
            .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;

        if let Some(body) = &self.node.body {
            store[body]
                .borrow()
                .iter()
                .try_for_each(store, &mut vcb, &mut tcb, &mut icb)?;
        }

        ControlFlow::Continue(())
    }
}

impl Type {
    pub fn iter(&self) -> TypeIter<'_> {
        TypeIter { node: self }
    }
}

impl Block {
    pub fn iter(&self) -> BlockIter<'_> {
        BlockIter { node: self }
    }
}

impl Value {
    pub fn iter(&self) -> ValueIter<'_> {
        ValueIter { node: self }
    }
}

impl Function {
    pub fn iter(&self) -> FunctionIter<'_> {
        FunctionIter { node: self }
    }
}
