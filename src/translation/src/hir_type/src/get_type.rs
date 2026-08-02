use nitrate_hir::prelude::*;
use nitrate_tree::ByteSpan;
use std::ops::Deref;

#[derive(Debug)]
pub enum TypeInferenceError {
    EnumVariantNotPresent,
    FieldAccessOnNonStruct,
    StructMissingField,
    CalleeIsNotFunctionType,
    MethodNotFound,
    CannotDeref,
    ClosureHasNoType,
    RangeStructNotRegistered,
}

pub trait HirGetType {
    fn determine_type(&self, ctx: &SymbolTab) -> Result<Type, TypeInferenceError>;
}

/// Resolve a Type::Refine to its base type for type inference purposes.
pub fn resolve_refine(ty: &Type) -> Result<Type, TypeInferenceError> {
    match ty {
        Type::Refine { base, .. } => Ok(base.deref().clone()),
        _ => Ok(ty.clone()),
    }
}

pub fn lit_to_u128(lit: &Lit) -> Option<u128> {
    match lit {
        Lit::U8(w) => Some(*w as u128),
        Lit::U16(w) => Some(*w as u128),
        Lit::U32(w) => Some(*w as u128),
        Lit::U64(w) => Some(*w as u128),
        Lit::U128(w) => Some(*w),
        Lit::USize(_, w) => Some(*w as u128),
        Lit::I8(w) if *w >= 0 => Some(*w as u128),
        Lit::I16(w) if *w >= 0 => Some(*w as u128),
        Lit::I32(w) if *w >= 0 => Some(*w as u128),
        Lit::I64(w) if *w >= 0 => Some(*w as u128),
        Lit::I128(w) if *w >= 0 => Some(*w as u128),
        _ => None,
    }
}

impl HirGetType for Lit {
    fn determine_type(&self, _ctx: &SymbolTab) -> Result<Type, TypeInferenceError> {
        match self {
            Lit::Unit => Ok(Type::Unit {
                span: ByteSpan::default(),
            }),
            Lit::Bool(_) => Ok(Type::Bool {
                span: ByteSpan::default(),
            }),
            Lit::I8(_) => Ok(Type::I8 {
                span: ByteSpan::default(),
            }),
            Lit::I16(_) => Ok(Type::I16 {
                span: ByteSpan::default(),
            }),
            Lit::I32(_) => Ok(Type::I32 {
                span: ByteSpan::default(),
            }),
            Lit::I64(_) => Ok(Type::I64 {
                span: ByteSpan::default(),
            }),
            Lit::I128(_) => Ok(Type::I128 {
                span: ByteSpan::default(),
            }),
            Lit::U8(_) => Ok(Type::U8 {
                span: ByteSpan::default(),
            }),
            Lit::U16(_) => Ok(Type::U16 {
                span: ByteSpan::default(),
            }),
            Lit::U32(_) => Ok(Type::U32 {
                span: ByteSpan::default(),
            }),
            Lit::U64(_) => Ok(Type::U64 {
                span: ByteSpan::default(),
            }),
            Lit::U128(_) => Ok(Type::U128 {
                span: ByteSpan::default(),
            }),
            Lit::F32(_) => Ok(Type::F32 {
                span: ByteSpan::default(),
            }),
            Lit::F64(_) => Ok(Type::F64 {
                span: ByteSpan::default(),
            }),
            Lit::USize(_, _) => Ok(Type::USize {
                span: ByteSpan::default(),
            }),
        }
    }
}

impl HirGetType for Block {
    fn determine_type(&self, ctx: &SymbolTab) -> Result<Type, TypeInferenceError> {
        match self.elements.last() {
            Some(BlockElement::Expr(last)) => last.borrow().determine_type(ctx),
            Some(BlockElement::Local(_)) | None => Ok(Type::Unit {
                span: ByteSpan::default(),
            }),
        }
    }
}

impl HirGetType for Value {
    fn determine_type(&self, ctx: &SymbolTab) -> Result<Type, TypeInferenceError> {
        match self {
            Value::Unit { .. } => Ok(Type::Unit {
                span: ByteSpan::default(),
            }),
            Value::Bool { .. } => Ok(Type::Bool {
                span: ByteSpan::default(),
            }),
            Value::I8 { .. } => Ok(Type::I8 {
                span: ByteSpan::default(),
            }),
            Value::I16 { .. } => Ok(Type::I16 {
                span: ByteSpan::default(),
            }),
            Value::I32 { .. } => Ok(Type::I32 {
                span: ByteSpan::default(),
            }),
            Value::I64 { .. } => Ok(Type::I64 {
                span: ByteSpan::default(),
            }),
            Value::I128 { .. } => Ok(Type::I128 {
                span: ByteSpan::default(),
            }),
            Value::U8 { .. } => Ok(Type::U8 {
                span: ByteSpan::default(),
            }),
            Value::U16 { .. } => Ok(Type::U16 {
                span: ByteSpan::default(),
            }),
            Value::U32 { .. } => Ok(Type::U32 {
                span: ByteSpan::default(),
            }),
            Value::U64 { .. } => Ok(Type::U64 {
                span: ByteSpan::default(),
            }),
            Value::U128 { .. } => Ok(Type::U128 {
                span: ByteSpan::default(),
            }),
            Value::F32 { .. } => Ok(Type::F32 {
                span: ByteSpan::default(),
            }),
            Value::F64 { .. } => Ok(Type::F64 {
                span: ByteSpan::default(),
            }),
            Value::USize { .. } => Ok(Type::USize {
                span: ByteSpan::default(),
            }),
            Value::InferredInteger { .. } => Ok(Type::InferredInteger {
                span: ByteSpan::default(),
            }),
            Value::InferredFloat { .. } => Ok(Type::InferredFloat {
                span: ByteSpan::default(),
            }),

            Value::StringLit { .. } => Ok(Type::Str {
                span: ByteSpan::default(),
            }),

            Value::BStringLit { .. } => Ok(Type::Str {
                span: ByteSpan::default(),
            }),

            Value::StructObject { struct_def, .. } => Ok(Type::Struct {
                span: ByteSpan::default(),
                def: struct_def.clone(),
            }),

            Value::EnumVariant { enum_def, variant, .. } => {
                let enum_def = enum_def.borrow();
                match enum_def.variants.iter().find(|x| &x.name == variant) {
                    Some(variant) => Ok(variant.ty.deref().clone()),
                    None => Err(TypeInferenceError::EnumVariantNotPresent),
                }
            }

            Value::Binary { left, op, .. } => match op {
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
                | BinaryOp::Ne => Ok(Type::Bool {
                    span: ByteSpan::default(),
                }),
            },

            Value::Unary { operand: expr, .. } => match expr.borrow().determine_type(ctx) {
                Ok(Type::InferredFloat { .. }) => Ok(Type::InferredFloat {
                    span: ByteSpan::default(),
                }),
                Ok(Type::InferredInteger { .. }) => Ok(Type::InferredInteger {
                    span: ByteSpan::default(),
                }),
                Ok(other) => Ok(other),
                Err(e) => Err(e),
            },

            Value::IndexAccess { collection, .. } => {
                let collection_type = collection.borrow().determine_type(ctx)?;
                match collection_type {
                    Type::Array { element_type, .. } => Ok((*element_type).clone()),
                    Type::SliceRef { element_type, .. } => Ok((*element_type).clone()),
                    Type::SlicePtr { element_type, .. } => Ok((*element_type).clone()),
                    _ => {
                        let collection_type_id = collection_type.clone().into();
                        if let Some(method) = ctx.get_method(&collection_type_id, &"index".into()) {
                            Ok(method.borrow().return_type.deref().clone())
                        } else {
                            Ok(collection_type)
                        }
                    }
                }
            }

            Value::FieldAccess { expr, field_name, .. } => {
                if let Type::Struct { def, .. } = expr.borrow().determine_type(ctx)? {
                    let struct_def = &def.borrow();
                    if let Some(field) = struct_def.fields.get(field_name) {
                        return Ok(field.ty.deref().clone());
                    }
                    return Err(TypeInferenceError::StructMissingField);
                }
                Err(TypeInferenceError::FieldAccessOnNonStruct)
            }

            Value::Assign { .. } => Ok(Type::Unit {
                span: ByteSpan::default(),
            }),

            Value::Deref { place, .. } => {
                let place_type = place.borrow().determine_type(ctx)?;
                match place_type {
                    Type::Reference { to, .. }
                    | Type::Pointer { to, .. }
                    | Type::SliceRef { element_type: to, .. }
                    | Type::SlicePtr { element_type: to, .. } => Ok((*to).clone()),
                    _ => Err(TypeInferenceError::CannotDeref),
                }
            }

            Value::Cast { target_type, .. } => Ok(target_type.deref().clone()),

            Value::Borrow {
                mutable,
                exclusive,
                place,
                ..
            } => {
                let place_type = place.borrow().determine_type(ctx)?;
                Ok(Type::Reference {
                    span: ByteSpan::default(),
                    lifetime: Lifetime::Inferred,
                    exclusive: *exclusive,
                    mutable: *mutable,
                    to: place_type.into(),
                })
            }

            Value::List { elements, .. } => {
                let element_type = if elements.is_empty() {
                    Type::Unit {
                        span: ByteSpan::default(),
                    }
                    .into()
                } else {
                    elements[0].borrow().determine_type(ctx)?.into()
                };
                Ok(Type::Array {
                    span: ByteSpan::default(),
                    element_type,
                    len: elements.len() as u32,
                })
            }

            Value::Tuple { elements, .. } => {
                let mut element_types = Vec::with_capacity(elements.len());
                for elem in elements {
                    element_types.push(elem.borrow().determine_type(ctx)?.into());
                }
                Ok(Type::Tuple {
                    span: ByteSpan::default(),
                    element_types: element_types.into(),
                })
            }

            Value::If { false_branch, .. } => match false_branch {
                None => Ok(Type::Unit {
                    span: ByteSpan::default(),
                }),
                Some(false_branch) => false_branch.borrow().determine_type(ctx),
            },

            Value::While { .. } => Ok(Type::Unit {
                span: ByteSpan::default(),
            }),
            Value::Loop { .. } => Ok(Type::Unit {
                span: ByteSpan::default(),
            }),
            Value::Break { .. } => Ok(Type::Never {
                span: ByteSpan::default(),
            }),
            Value::Continue { .. } => Ok(Type::Never {
                span: ByteSpan::default(),
            }),
            Value::Return { .. } => Ok(Type::Never {
                span: ByteSpan::default(),
            }),

            Value::Block { block, .. } => block.borrow().determine_type(ctx),

            Value::Call { callee, .. } => {
                if let Type::Function { function_type, .. } = callee.borrow().determine_type(ctx)? {
                    return Ok(function_type.return_type.deref().clone());
                }
                Err(TypeInferenceError::CalleeIsNotFunctionType)
            }

            Value::MethodCall {
                object, method_name, ..
            } => {
                let object_type = object.borrow().determine_type(ctx)?.into();
                let method_type = ctx
                    .get_method(&object_type, method_name)
                    .ok_or(TypeInferenceError::MethodNotFound)?;
                Ok(method_type.borrow().return_type.deref().clone())
            }

            Value::FunctionSymbol { id, .. } => {
                let function = id.borrow();
                Ok(Type::Function {
                    span: ByteSpan::default(),
                    function_type: function.get_type().into(),
                })
            }

            Value::GlobalVariableSymbol { id, .. } => resolve_refine(id.borrow().ty.deref()),
            Value::LocalVariableSymbol { id, .. } => resolve_refine(id.borrow().ty.deref()),
            Value::ParameterSymbol { id, .. } => resolve_refine(id.borrow().ty.deref()),
            Value::Range {
                start, end, inclusive, ..
            } => {
                let has_start = start.is_some();
                let has_end = end.is_some();

                let struct_name = match (has_start, has_end) {
                    (true, true) if *inclusive => "RangeInclusive",
                    (true, true) => "Range",
                    (true, false) => "RangeFrom",
                    (false, true) if *inclusive => "RangeToInclusive",
                    (false, true) => "RangeTo",
                    (false, false) => "RangeFull",
                };

                match ctx.get_struct(&struct_name.into()) {
                    Some(struct_def) => Ok(Type::Struct {
                        span: ByteSpan::default(),
                        def: struct_def.clone(),
                    }),
                    None => Err(TypeInferenceError::RangeStructNotRegistered),
                }
            }
        }
    }
}
