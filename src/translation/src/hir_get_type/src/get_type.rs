use std::ops::Deref;

use nitrate_hir::prelude::*;

#[derive(Debug)]
pub enum TypeInferenceError {
    EnumVariantNotPresent,
    FieldAccessOnNonStruct,
    StructMissingField,
    CalleeIsNotFunctionType,
    MethodNotFound,
    CannotDeref,
    ClosureHasNoType,
}

pub trait HirGetType {
    fn determine_type(&self, ctx: &SymbolTab) -> Result<Type, TypeInferenceError>;
}

impl HirGetType for Lit {
    fn determine_type(&self, _ctx: &SymbolTab) -> Result<Type, TypeInferenceError> {
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
    fn determine_type(&self, ctx: &SymbolTab) -> Result<Type, TypeInferenceError> {
        match self.elements.last() {
            Some(BlockElement::Expr(last)) => last.borrow().determine_type(ctx),
            Some(BlockElement::Local(_)) | None => Ok(Type::Unit),
        }
    }
}

impl HirGetType for Value {
    fn determine_type(&self, ctx: &SymbolTab) -> Result<Type, TypeInferenceError> {
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
                let element_type = Type::U8.into();
                let array = Type::Array {
                    element_type,
                    len: str.len() as u32,
                };

                Ok(array)
            }

            Value::BStringLit(vec) => {
                let element_type = Type::U8.into();
                let array = Type::Array {
                    element_type,
                    len: vec.len() as u32,
                };

                Ok(array)
            }

            Value::StructObject { struct_def, fields: _ } => Ok(Type::Struct {
                def: struct_def.clone(),
            }),

            Value::EnumVariant {
                enum_def,
                variant,
                value: _,
            } => {
                let enum_def = enum_def.borrow();
                match enum_def.variants.iter().find(|x| &x.name == variant) {
                    Some(variant) => Ok(variant.ty.deref().clone()),
                    None => Err(TypeInferenceError::EnumVariantNotPresent),
                }
            }

            Value::Binary { left, op, right: _ } => match op {
                BinaryOp::Add
                | BinaryOp::Sub
                | BinaryOp::Mul
                | BinaryOp::Div
                | BinaryOp::Mod
                | BinaryOp::And
                | BinaryOp::Or
                | BinaryOp::Xor => Ok(left.borrow().determine_type(ctx)?),

                BinaryOp::Shl | BinaryOp::Shr | BinaryOp::Rol | BinaryOp::Ror => {
                    Ok(left.borrow().determine_type(ctx)?)
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
                UnaryOp::Add | UnaryOp::Sub | UnaryOp::Not => expr.borrow().determine_type(ctx),
            },

            Value::FieldAccess { expr, field_name } => {
                let expr = expr.borrow();

                if let Type::Struct { def } = expr.determine_type(ctx)? {
                    let struct_def = &def.borrow();
                    let found_field = struct_def.fields.get(field_name);
                    if let Some(field) = found_field {
                        return Ok(field.ty.deref().clone());
                    } else {
                        return Err(TypeInferenceError::StructMissingField);
                    }
                }

                Err(TypeInferenceError::FieldAccessOnNonStruct)
            }

            Value::Assign { place: _, value: _ } => Ok(Type::Unit),

            Value::Deref { place } => {
                let place = place.borrow();
                let place_type = place.determine_type(ctx)?;

                match place_type {
                    Type::Reference { to, .. } | Type::Pointer { to, .. } => {
                        return Ok((*to).clone());
                    }

                    _ => Err(TypeInferenceError::CannotDeref),
                }
            }

            Value::Cast { value: _, target_type } => Ok(target_type.deref().clone()),

            Value::Borrow {
                mutable,
                exclusive,
                place,
            } => {
                let place_type = place.borrow().determine_type(ctx)?;
                Ok(Type::Reference {
                    lifetime: Lifetime::Inferred,
                    exclusive: *exclusive,
                    mutable: *mutable,
                    to: place_type.into(),
                })
            }

            Value::List { elements } => {
                let element_type = if elements.is_empty() {
                    Type::Unit.into()
                } else {
                    elements[0].borrow().determine_type(ctx)?.into()
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
                    let elem_type = elem.borrow().determine_type(ctx)?.into();
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

                Some(false_branch) => {
                    let true_block = true_branch.borrow().determine_type(ctx)?;
                    if !true_block.is_diverging() {
                        return Ok(true_block);
                    }

                    false_branch.borrow().determine_type(ctx)
                }
            },

            Value::While { condition: _, body: _ } => Ok(Type::Unit),

            Value::Loop { body: _ } => Ok(Type::Unit),
            Value::Break { label: _ } => Ok(Type::Never),
            Value::Continue { label: _ } => Ok(Type::Never),
            Value::Return { value: _ } => Ok(Type::Never),

            Value::Block { block } => block.borrow().determine_type(ctx),

            Value::Call {
                callee,
                positional: _,
                named: _,
            } => {
                let callee = callee.borrow();
                if let Type::Function { function_type } = callee.determine_type(ctx)? {
                    return Ok(function_type.return_type.deref().clone());
                }

                Err(TypeInferenceError::CalleeIsNotFunctionType)
            }

            Value::MethodCall {
                object,
                method_name,
                positional: _,
                named: _,
            } => {
                let object_type = object.borrow().determine_type(ctx)?.into();
                let method_type = ctx
                    .get_method(&object_type, method_name)
                    .ok_or(TypeInferenceError::MethodNotFound)?;

                Ok(method_type.borrow().return_type.deref().clone())
            }

            Value::FunctionSymbol { id } => {
                let function = id.borrow();
                Ok(Type::Function {
                    function_type: function.get_type().into(),
                })
            }

            Value::GlobalVariableSymbol { id } => {
                let glb = &id.borrow();
                Ok(glb.ty.deref().clone())
            }

            Value::LocalVariableSymbol { id } => {
                let loc = id.borrow();
                Ok(loc.ty.deref().clone())
            }

            Value::ParameterSymbol { id } => {
                let param = id.borrow();
                Ok(param.ty.deref().clone())
            }
        }
    }
}
