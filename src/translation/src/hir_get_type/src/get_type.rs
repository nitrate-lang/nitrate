use nitrate_hir::{
    Store,
    hir::{BinaryOp, FunctionType, IntoStoreId, Symbol, Type, UnaryOp, Value},
};

pub enum TypeInferenceError {
    EnumVariantNotPresent,
    FieldAccessOnNonStruct,
    IndexAccessOnNonCollection,
    CalleeIsNotFunctionType,
    UnresolvedSymbol,
    TraitHasNoType,
}

pub fn get_type(value: &Value, store: &Store) -> Result<Type, TypeInferenceError> {
    match value {
        Value::Unit => Ok(Type::Unit),
        Value::Bool(_) => Ok(Type::Bool),
        Value::I8(_) => Ok(Type::I8),
        Value::I16(_) => Ok(Type::I16),
        Value::I32(_) => Ok(Type::I32),
        Value::I64(_) => Ok(Type::I64),
        Value::I128(_) => Ok(Type::I128),
        Value::U8(_) => Ok(Type::U8),
        Value::U16(_) => Ok(Type::U16),
        Value::U32(_) => Ok(Type::U32),
        Value::U64(_) => Ok(Type::U64),
        Value::U128(_) => Ok(Type::U128),
        Value::F32(_) => Ok(Type::F32),
        Value::F64(_) => Ok(Type::F64),
        Value::USize32(_) => Ok(Type::USize),
        Value::USize64(_) => Ok(Type::USize),
        Value::InferredInteger(_) => Ok(Type::InferredInteger),
        Value::InferredFloat(_) => Ok(Type::InferredFloat),

        Value::StringLit(str) => {
            let element_type = Type::U8.into_id(store);
            let array = Type::Array {
                element_type,
                len: str.len() as u64,
            };

            Ok(array)
        }

        Value::BStringLit(vec) => {
            let element_type = Type::U8.into_id(store);
            let array = Type::Array {
                element_type,
                len: vec.len() as u64,
            };

            Ok(array)
        }

        Value::StructObject {
            struct_type,
            fields: _,
        } => Ok(Type::Struct {
            struct_type: struct_type.to_owned(),
        }),

        Value::EnumVariant {
            enum_type,
            variant,
            value: _,
        } => match store[enum_type].variants.get(variant) {
            Some(variant_type) => Ok(store[variant_type].clone()),
            None => Err(TypeInferenceError::EnumVariantNotPresent),
        },

        Value::Binary { left, op, right: _ } => match op {
            BinaryOp::Add
            | BinaryOp::Sub
            | BinaryOp::Mul
            | BinaryOp::Div
            | BinaryOp::Mod
            | BinaryOp::And
            | BinaryOp::Or
            | BinaryOp::Xor => Ok(get_type(&store[left].borrow(), store)?),

            BinaryOp::Shl | BinaryOp::Shr | BinaryOp::Rol | BinaryOp::Ror => {
                Ok(get_type(&store[left].borrow(), store)?)
            }

            BinaryOp::LogicAnd
            | BinaryOp::LogicOr
            | BinaryOp::Lt
            | BinaryOp::Gt
            | BinaryOp::Lte
            | BinaryOp::Gte
            | BinaryOp::Eq
            | BinaryOp::Ne => Ok(Type::Bool),
        },

        Value::Unary { op, expr } => match op {
            UnaryOp::Add | UnaryOp::Sub | UnaryOp::BitNot => get_type(&store[expr].borrow(), store),
            UnaryOp::LogicNot => Ok(Type::Bool),
        },

        Value::FieldAccess { expr, field } => {
            let expr = &store[expr].borrow();

            if let Type::Struct { struct_type } = get_type(expr, store)? {
                let struct_def = &store[&struct_type];
                if let Some(field_type) = struct_def.fields.get(field) {
                    return Ok(store[field_type].clone());
                }
            }

            Err(TypeInferenceError::FieldAccessOnNonStruct)
        }

        Value::IndexAccess {
            collection,
            index: _,
        } => {
            let collection = &store[collection].borrow();
            if let Type::Array { element_type, .. } = get_type(collection, store)? {
                return Ok((&store[&element_type]).clone());
            }

            Err(TypeInferenceError::IndexAccessOnNonCollection)
        }

        Value::Assign { place: _, value: _ } => Ok(Type::Unit),

        Value::Deref { place: _ } => {
            // TODO: inference for dereference
            todo!()
        }

        Value::Cast { expr: _, to } => Ok((&store[to]).clone()),

        Value::Borrow {
            mutable: _,
            place: _,
        } => {
            // TODO: inference for address-of
            todo!()
        }

        Value::List { elements } => {
            let element_type = if elements.is_empty() {
                Type::Unit.into_id(store)
            } else {
                get_type(&elements[0], store)?.into_id(store)
            };

            let array = Type::Array {
                element_type,
                len: elements.len() as u64,
            };

            Ok(array)
        }

        Value::Tuple { elements } => {
            let mut element_types = Vec::with_capacity(elements.len());
            for elem in elements {
                let elem_type = get_type(elem, store)?;
                element_types.push(elem_type);
            }

            let tuple_type = Type::Tuple {
                element_types: element_types.into(),
            };

            Ok(tuple_type)
        }

        Value::If {
            true_branch,
            false_branch,
            condition: _,
        } => match false_branch {
            None => Ok(Type::Unit),

            Some(_) => {
                let block = &store[true_branch].borrow();
                let true_branch_type = match block.elements.last() {
                    Some(last) => get_type(last, store)?,
                    None => Type::Unit,
                };

                Ok(true_branch_type)
            }
        },

        Value::While {
            condition: _,
            body: _,
        } => Ok(Type::Unit),

        Value::Loop { body: _ } => Ok(Type::Unit),
        Value::Break { label: _ } => Ok(Type::Never),
        Value::Continue { label: _ } => Ok(Type::Never),
        Value::Return { value: _ } => Ok(Type::Never),

        Value::Block { block } => match store[block].borrow().elements.last() {
            Some(last) => get_type(last, store),
            None => Ok(Type::Unit),
        },

        Value::Closure {
            captures: _,
            callee: _,
        } => {
            // TODO: Determine the type of the closure
            todo!()
        }

        Value::Call {
            callee,
            arguments: _,
        } => {
            let callee = &store[callee].borrow();
            if let Type::Function { function_type } = get_type(callee, store)? {
                let func = &store[&function_type];
                return Ok(store[&func.return_type].clone());
            }

            Err(TypeInferenceError::CalleeIsNotFunctionType)
        }

        Value::Symbol { symbol } => match &*store[symbol].borrow() {
            Symbol::Unresolved { name: _ } => Err(TypeInferenceError::UnresolvedSymbol),
            Symbol::GlobalVariable(glb) => Ok(store[&store[glb].borrow().ty].clone()),
            Symbol::LocalVariable(loc) => Ok(store[&store[loc].borrow().ty].clone()),
            Symbol::Trait(_) => Err(TypeInferenceError::TraitHasNoType),
            Symbol::Parameter(param) => Ok(store[&store[param].borrow().ty].clone()),
            Symbol::Function(function) => {
                let function = &store[function].borrow();
                Ok(Type::Function {
                    function_type: FunctionType {
                        attributes: function.attributes.to_owned(),
                        params: function.parameters.to_owned(),
                        return_type: function.return_type.to_owned(),
                    }
                    .into_id(store),
                })
            }
        },
    }
}
