use nitrate_hir::{
    Store, TypeId,
    hir::{BinaryOp, IntoStoreId, Type, UnaryOp, Value},
};

pub enum TypeInferenceError {
    NonHomogeneousList,
    IfElseBranchTypeMismatch,
    BinaryOpTypeMismatch,
    ShiftOrRotateByNonU32,
}

pub fn get_type(value: &Value, store: &Store) -> Result<TypeId, TypeInferenceError> {
    match value {
        Value::Unit => Ok(Type::Unit.into_id(store)),
        Value::Bool(_) => Ok(Type::Bool.into_id(store)),
        Value::I8(_) => Ok(Type::I8.into_id(store)),
        Value::I16(_) => Ok(Type::I16.into_id(store)),
        Value::I32(_) => Ok(Type::I32.into_id(store)),
        Value::I64(_) => Ok(Type::I64.into_id(store)),
        Value::I128(_) => Ok(Type::I128.into_id(store)),
        Value::U8(_) => Ok(Type::U8.into_id(store)),
        Value::U16(_) => Ok(Type::U16.into_id(store)),
        Value::U32(_) => Ok(Type::U32.into_id(store)),
        Value::U64(_) => Ok(Type::U64.into_id(store)),
        Value::U128(_) => Ok(Type::U128.into_id(store)),
        Value::F8(_) => Ok(Type::F8.into_id(store)),
        Value::F16(_) => Ok(Type::F16.into_id(store)),
        Value::F32(_) => Ok(Type::F32.into_id(store)),
        Value::F64(_) => Ok(Type::F64.into_id(store)),
        Value::F128(_) => Ok(Type::F128.into_id(store)),
        Value::USize32(_) => Ok(Type::USize.into_id(store)),
        Value::USize64(_) => Ok(Type::USize.into_id(store)),
        Value::InferredInteger(_) => Ok(Type::InferredInteger.into_id(store)),
        Value::InferredFloat(_) => Ok(Type::InferredFloat.into_id(store)),

        Value::StringLit(str) => {
            let element_type = Type::U8.into_id(store);
            let array = Type::Array {
                element_type,
                len: str.len() as u64,
            };

            Ok(array.into_id(store))
        }

        Value::BStringLit(vec) => {
            let element_type = Type::U8.into_id(store);
            let array = Type::Array {
                element_type,
                len: vec.len() as u64,
            };

            Ok(array.into_id(store))
        }

        Value::Struct {
            struct_type,
            fields,
        } => {
            // TODO: inference for struct
            todo!()
        }

        Value::Enum {
            enum_type,
            variant,
            value,
        } => {
            // TODO: inference for enum
            todo!()
        }

        Value::Binary { left, op, right } => match op {
            BinaryOp::Add
            | BinaryOp::Sub
            | BinaryOp::Mul
            | BinaryOp::Div
            | BinaryOp::Mod
            | BinaryOp::And
            | BinaryOp::Or
            | BinaryOp::Xor => {
                let left = &store[left].borrow();
                let right = &store[right].borrow();

                let left_type = get_type(left, store)?;
                let right_type = get_type(right, store)?;

                if left_type != right_type {
                    return Err(TypeInferenceError::BinaryOpTypeMismatch);
                }

                Ok(left_type)
            }

            BinaryOp::Shl | BinaryOp::Shr | BinaryOp::Rol | BinaryOp::Ror => {
                let left = &store[left].borrow();
                let right = &store[right].borrow();

                let left_type = get_type(left, store)?;
                let right_type = get_type(right, store)?;

                if right_type != Type::U32.into_id(store) {
                    return Err(TypeInferenceError::ShiftOrRotateByNonU32);
                }

                Ok(left_type)
            }

            BinaryOp::LogicAnd
            | BinaryOp::LogicOr
            | BinaryOp::Lt
            | BinaryOp::Gt
            | BinaryOp::Lte
            | BinaryOp::Gte
            | BinaryOp::Eq
            | BinaryOp::Ne => Ok(Type::Bool.into_id(store)),
        },

        Value::Unary { op, expr } => match op {
            UnaryOp::Add | UnaryOp::Sub | UnaryOp::BitNot => {
                let expr = &store[expr].borrow();
                get_type(expr, store)
            }
            UnaryOp::LogicNot => Ok(Type::Bool.into_id(store)),
        },

        Value::FieldAccess { expr, field } => {
            // TODO: inference for field access
            todo!()
        }

        Value::IndexAccess { collection, index } => {
            // TODO: inference for index access
            todo!()
        }

        Value::Assign { place: _, value: _ } => Ok(Type::Unit.into_id(store)),

        Value::Deref { place } => {
            // TODO: inference for dereference
            todo!()
        }

        Value::Cast { expr, to } => Ok(to.to_owned()),

        Value::GetAddressOf { place } => {
            // TODO: inference for address-of
            todo!()
        }

        Value::GetTypeOf { expr } => {
            // TODO: inference for type-of
            todo!()
        }

        Value::List { elements } => {
            let element_type = if elements.is_empty() {
                Type::Unit.into_id(store)
            } else {
                let first_type = get_type(&elements[0], store)?;
                for elem in &elements[1..] {
                    let elem_type = get_type(elem, store)?;
                    if elem_type != first_type {
                        return Err(TypeInferenceError::NonHomogeneousList);
                    }
                }

                first_type
            };

            let array = Type::Array {
                element_type,
                len: elements.len() as u64,
            };

            Ok(array.into_id(store))
        }

        Value::Tuple { elements } => {
            let mut element_types = Vec::with_capacity(elements.len());
            for elem in elements {
                let elem_type = get_type(elem, store)?;
                element_types.push(elem_type);
            }

            let tuple_type = Type::Tuple {
                element_types: element_types.into_id(store),
            };

            Ok(tuple_type.into_id(store))
        }

        Value::If {
            true_branch,
            false_branch,
            condition: _,
        } => match false_branch {
            None => Ok(Type::Unit.into_id(store)),

            Some(false_branch) => {
                let block = &store[true_branch].borrow();
                let true_branch_type = match block.elements.last() {
                    Some(last) => get_type(last, store)?,
                    None => Type::Unit.into_id(store),
                };

                let block = &store[false_branch].borrow();
                let false_branch_type = match block.elements.last() {
                    Some(last) => get_type(last, store)?,
                    None => Type::Unit.into_id(store),
                };

                if true_branch_type != false_branch_type {
                    return Err(TypeInferenceError::IfElseBranchTypeMismatch);
                }

                Ok(true_branch_type)
            }
        },

        Value::While {
            condition: _,
            body: _,
        } => Ok(Type::Unit.into_id(store)),

        Value::Loop { body: _ } => Ok(Type::Unit.into_id(store)),

        Value::Break { label: _ } => Ok(Type::Never.into_id(store)),
        Value::Continue { label: _ } => Ok(Type::Never.into_id(store)),
        Value::Return { value: _ } => Ok(Type::Never.into_id(store)),

        Value::Block { block } => match store[block].borrow().elements.last() {
            Some(last) => get_type(last, store),
            None => Ok(Type::Unit.into_id(store)),
        },

        Value::Call {
            callee,
            arguments: _,
        } => {
            // TODO: inference for function calls
            todo!()
        }

        Value::Symbol { symbol } => {
            // TODO: inference for symbols
            todo!()
        }
    }
}
