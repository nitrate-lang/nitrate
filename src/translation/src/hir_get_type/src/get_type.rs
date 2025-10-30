use nitrate_hir::prelude::*;

#[derive(Debug)]
pub enum TypeInferenceError {
    EnumVariantNotPresent,
    FieldAccessOnNonStruct,
    IndexAccessOnNonCollection,
    CalleeIsNotFunctionType,
    TraitHasNoType,
    UnresolvedSymbol,
    StructObjectDoesNotHaveStructType,
    EnumVariantDoesNotHaveEnumType,
    MethodNotFound,
    CannotDeref,
}

pub trait HirGetType {
    fn get_type(&self, store: &Store, tab: &SymbolTab) -> Result<Type, TypeInferenceError>;
}

impl HirGetType for Lit {
    fn get_type(&self, _store: &Store, _tab: &SymbolTab) -> Result<Type, TypeInferenceError> {
        match self {
            Lit::Unit => Ok(Type::Unit),
            Lit::Bool(_) => Ok(Type::Bool),
            Lit::I8(_) => Ok(Type::I8),
            Lit::I16(_) => Ok(Type::I16),
            Lit::I32(_) => Ok(Type::I32),
            Lit::I64(_) => Ok(Type::I64),
            Lit::I128(_) => Ok(Type::I128),
            Lit::U8(_) => Ok(Type::U8),
            Lit::U16(_) => Ok(Type::U16),
            Lit::U32(_) => Ok(Type::U32),
            Lit::U64(_) => Ok(Type::U64),
            Lit::U128(_) => Ok(Type::U128),
            Lit::F32(_) => Ok(Type::F32),
            Lit::F64(_) => Ok(Type::F64),
            Lit::USize32(_) => Ok(Type::USize),
            Lit::USize64(_) => Ok(Type::USize),
        }
    }
}

impl HirGetType for Block {
    fn get_type(&self, store: &Store, tab: &SymbolTab) -> Result<Type, TypeInferenceError> {
        match self.elements.last() {
            Some(BlockElement::Expr(last)) => store[last].borrow().get_type(store, tab),
            Some(BlockElement::Stmt(_)) | Some(BlockElement::Local(_)) | None => Ok(Type::Unit),
        }
    }
}

impl HirGetType for Value {
    fn get_type(&self, store: &Store, tab: &SymbolTab) -> Result<Type, TypeInferenceError> {
        match self {
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
                    len: str.len() as u32,
                };

                Ok(array)
            }

            Value::BStringLit(vec) => {
                let element_type = Type::U8.into_id(store);
                let array = Type::Array {
                    element_type,
                    len: vec.len() as u32,
                };

                Ok(array)
            }

            Value::StructObject {
                struct_path,
                fields: _,
            } => match tab.get_type(struct_path) {
                None => Err(TypeInferenceError::UnresolvedSymbol),

                Some(TypeDefinition::StructDef(struct_def)) => Ok(Type::Struct {
                    struct_type: store[struct_def].borrow().struct_id.to_owned(),
                }),

                Some(TypeDefinition::EnumDef(_)) | Some(TypeDefinition::TypeAliasDef(_)) => {
                    Err(TypeInferenceError::StructObjectDoesNotHaveStructType)
                }
            },

            Value::EnumVariant {
                enum_path,
                variant,
                value: _,
            } => match tab.get_type(enum_path) {
                None => return Err(TypeInferenceError::UnresolvedSymbol),

                Some(TypeDefinition::EnumDef(enum_def)) => {
                    let enum_type = store[enum_def].borrow().enum_id;
                    let found = store[&enum_type]
                        .variants
                        .iter()
                        .find(|x| &x.name == variant);
                    match found {
                        Some(variant) => Ok(store[&variant.ty].clone()),
                        None => Err(TypeInferenceError::EnumVariantNotPresent),
                    }
                }

                Some(TypeDefinition::StructDef(_)) | Some(TypeDefinition::TypeAliasDef(_)) => {
                    return Err(TypeInferenceError::EnumVariantDoesNotHaveEnumType);
                }
            },

            Value::Binary { left, op, right: _ } => match op {
                BinaryOp::Add
                | BinaryOp::Sub
                | BinaryOp::Mul
                | BinaryOp::Div
                | BinaryOp::Mod
                | BinaryOp::And
                | BinaryOp::Or
                | BinaryOp::Xor => Ok(store[left].borrow().get_type(store, tab)?),

                BinaryOp::Shl | BinaryOp::Shr | BinaryOp::Rol | BinaryOp::Ror => {
                    Ok(store[left].borrow().get_type(store, tab)?)
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

            Value::Unary { op, operand: expr } => match op {
                UnaryOp::Add | UnaryOp::Sub | UnaryOp::BitNot => {
                    store[expr].borrow().get_type(store, tab)
                }
                UnaryOp::LogicNot => Ok(Type::Bool),
            },

            Value::FieldAccess { expr, field } => {
                let expr = &store[expr].borrow();

                if let Type::Struct { struct_type } = expr.get_type(store, tab)? {
                    let struct_def = &store[&struct_type];
                    let found_field = struct_def.fields.iter().find(|x| &x.name == field);
                    if let Some(field) = found_field {
                        return Ok(store[&field.ty].clone());
                    }
                }

                Err(TypeInferenceError::FieldAccessOnNonStruct)
            }

            Value::IndexAccess {
                collection,
                index: _,
            } => {
                let collection = &store[collection].borrow();
                if let Type::Array { element_type, .. } = collection.get_type(store, tab)? {
                    return Ok((&store[&element_type]).clone());
                }

                Err(TypeInferenceError::IndexAccessOnNonCollection)
            }

            Value::Assign { place: _, value: _ } => Ok(Type::Unit),

            Value::Deref { place } => {
                let place = &store[place].borrow();
                let place_type = place.get_type(store, tab)?;

                match place_type {
                    Type::Reference { to, .. } | Type::Pointer { to, .. } => {
                        return Ok((&store[&to]).clone());
                    }

                    _ => Err(TypeInferenceError::CannotDeref),
                }
            }

            Value::Cast { expr: _, to } => Ok((&store[to]).clone()),

            Value::Borrow {
                mutable,
                exclusive,
                place,
            } => {
                let place_type = store[place].borrow().get_type(store, tab)?;
                Ok(Type::Reference {
                    lifetime: Lifetime::Inferred,
                    exclusive: *exclusive,
                    mutable: *mutable,
                    to: place_type.into_id(store),
                })
            }

            Value::List { elements } => {
                let element_type = if elements.is_empty() {
                    Type::Unit.into_id(store)
                } else {
                    elements[0].get_type(store, tab)?.into_id(store)
                };

                let array = Type::Array {
                    element_type,
                    len: elements.len() as u32,
                };

                Ok(array)
            }

            Value::Tuple { elements } => {
                let mut element_types = Vec::with_capacity(elements.len());
                for elem in elements {
                    let elem_type = elem.get_type(store, tab)?.into_id(store);
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
                    let true_block = &store[true_branch].borrow();
                    let true_branch_type = match true_block.elements.last() {
                        Some(BlockElement::Expr(last)) => {
                            store[last].borrow().get_type(store, tab)?
                        }
                        Some(BlockElement::Stmt(_)) | Some(BlockElement::Local(_)) | None => {
                            Type::Unit
                        }
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

            Value::Block { block } => store[block].borrow().get_type(store, tab),

            Value::Closure {
                captures: _,
                callee: _,
            } => {
                // TODO: Determine the type of a closure
                unimplemented!()
            }

            Value::Call {
                callee,
                positional: _,
                named: _,
            } => {
                let callee = &store[callee].borrow();
                if let Type::Function { function_type } = callee.get_type(store, tab)? {
                    let func = &store[&function_type];
                    return Ok(store[&func.return_type].clone());
                }

                Err(TypeInferenceError::CalleeIsNotFunctionType)
            }

            Value::MethodCall {
                object,
                method_name,
                positional: _,
                named: _,
            } => {
                let object = &store[object].borrow();
                let object_type = object.get_type(store, tab)?.into_id(store);

                let method = tab
                    .get_method(&object_type, method_name)
                    .ok_or(TypeInferenceError::MethodNotFound)?;

                let method = &store[method].borrow();
                Ok(store[&method.return_type].clone())
            }

            Value::Symbol { path } => match tab.get_symbol(path) {
                Some(SymbolId::GlobalVariable(glb)) => Ok(store[&store[glb].borrow().ty].clone()),

                Some(SymbolId::LocalVariable(loc)) => Ok(store[&store[loc].borrow().ty].clone()),

                Some(SymbolId::Parameter(param)) => Ok(store[&store[param].borrow().ty].clone()),

                Some(SymbolId::Function(function)) => {
                    let function = &store[function].borrow();
                    let parameters = function
                        .params
                        .iter()
                        .map(|param_id| {
                            let param = store[param_id].borrow();
                            (param.name.clone(), param.ty.clone())
                        })
                        .collect::<Vec<_>>();

                    Ok(Type::Function {
                        function_type: FunctionType {
                            attributes: function.attributes.to_owned(),
                            params: parameters,
                            return_type: function.return_type.to_owned(),
                        }
                        .into_id(store),
                    })
                }

                None => Err(TypeInferenceError::UnresolvedSymbol),
            },
        }
    }
}
