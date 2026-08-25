use nitrate_hir::prelude::*;
use nitrate_tree::SrcPos;
use std::collections::BTreeMap;
use std::ops::Deref;

/// Substitute generic parameters (by index) inside a type. Used to resolve
/// the return type of a generic call with explicit turbofish type arguments
/// (`foo::<i32>()`) before monomorphization has run.
fn substitute_generic_params(ty: &Type, mapping: &BTreeMap<u32, TypeId>) -> Type {
    match ty {
        Type::GenericParam { index, .. } => mapping
            .get(index)
            .map(|c| (**c).clone())
            .unwrap_or_else(|| ty.clone()),
        Type::Array { element_type, len, .. } => Type::Array {
            span: ty.span(),
            element_type: TypeId::from(substitute_generic_params(element_type, mapping)),
            len: *len,
        },
        Type::Tuple { element_types, .. } => Type::Tuple {
            span: ty.span(),
            element_types: element_types
                .iter()
                .map(|et| TypeId::from(substitute_generic_params(et, mapping)))
                .collect(),
        },
        Type::Reference {
            lifetime,
            exclusive,
            mutable,
            to,
            ..
        } => Type::Reference {
            span: ty.span(),
            lifetime: lifetime.clone(),
            exclusive: *exclusive,
            mutable: *mutable,
            to: TypeId::from(substitute_generic_params(to, mapping)),
        },
        Type::Pointer {
            lifetime,
            exclusive,
            mutable,
            to,
            ..
        } => Type::Pointer {
            span: ty.span(),
            lifetime: lifetime.clone(),
            exclusive: *exclusive,
            mutable: *mutable,
            to: TypeId::from(substitute_generic_params(to, mapping)),
        },
        Type::SliceRef {
            lifetime,
            exclusive,
            mutable,
            element_type,
            ..
        } => Type::SliceRef {
            span: ty.span(),
            lifetime: lifetime.clone(),
            exclusive: *exclusive,
            mutable: *mutable,
            element_type: TypeId::from(substitute_generic_params(element_type, mapping)),
        },
        Type::SlicePtr {
            lifetime,
            exclusive,
            mutable,
            element_type,
            ..
        } => Type::SlicePtr {
            span: ty.span(),
            lifetime: lifetime.clone(),
            exclusive: *exclusive,
            mutable: *mutable,
            element_type: TypeId::from(substitute_generic_params(element_type, mapping)),
        },
        Type::Parameterized { base, args, .. } => {
            let new_base = substitute_generic_params(base, mapping);
            let new_positional: Vec<TypeId> = args
                .positional
                .iter()
                .map(|a| TypeId::from(substitute_generic_params(a, mapping)))
                .collect();
            let new_named: Vec<(nitrate_nstring::NString, TypeId)> = args
                .named
                .iter()
                .map(|(k, v)| (k.clone(), TypeId::from(substitute_generic_params(v, mapping))))
                .collect();
            Type::Parameterized {
                span: ty.span(),
                base: TypeId::from(new_base),
                args: Arguments {
                    positional: new_positional.into(),
                    named: new_named.into(),
                },
            }
        }
        Type::Function { function_type, .. } => {
            let params: Vec<(nitrate_nstring::NString, TypeId)> = function_type
                .params
                .iter()
                .map(|(n, p)| (n.clone(), TypeId::from(substitute_generic_params(p, mapping))))
                .collect();
            let ret = substitute_generic_params(&function_type.return_type, mapping);
            Type::Function {
                span: ty.span(),
                function_type: Box::new(FunctionType {
                    attributes: function_type.attributes.clone(),
                    params: params.into(),
                    return_type: TypeId::from(ret),
                }),
            }
        }
        Type::Refine { base, min, max, .. } => Type::Refine {
            span: ty.span(),
            base: TypeId::from(substitute_generic_params(base, mapping)),
            min: *min,
            max: *max,
        },
        _ => ty.clone(),
    }
}

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
                span: SrcPos::default(),
            }),
            Lit::Bool(_) => Ok(Type::Bool {
                span: SrcPos::default(),
            }),
            Lit::I8(_) => Ok(Type::I8 {
                span: SrcPos::default(),
            }),
            Lit::I16(_) => Ok(Type::I16 {
                span: SrcPos::default(),
            }),
            Lit::I32(_) => Ok(Type::I32 {
                span: SrcPos::default(),
            }),
            Lit::I64(_) => Ok(Type::I64 {
                span: SrcPos::default(),
            }),
            Lit::I128(_) => Ok(Type::I128 {
                span: SrcPos::default(),
            }),
            Lit::U8(_) => Ok(Type::U8 {
                span: SrcPos::default(),
            }),
            Lit::U16(_) => Ok(Type::U16 {
                span: SrcPos::default(),
            }),
            Lit::U32(_) => Ok(Type::U32 {
                span: SrcPos::default(),
            }),
            Lit::U64(_) => Ok(Type::U64 {
                span: SrcPos::default(),
            }),
            Lit::U128(_) => Ok(Type::U128 {
                span: SrcPos::default(),
            }),
            Lit::F32(_) => Ok(Type::F32 {
                span: SrcPos::default(),
            }),
            Lit::F64(_) => Ok(Type::F64 {
                span: SrcPos::default(),
            }),
            Lit::USize(_, _) => Ok(Type::USize {
                span: SrcPos::default(),
            }),
        }
    }
}

impl HirGetType for Block {
    fn determine_type(&self, ctx: &SymbolTab) -> Result<Type, TypeInferenceError> {
        match self.elements.last() {
            Some(BlockElement::Expr(last)) => last.borrow().determine_type(ctx),
            Some(BlockElement::Local(_)) | None => Ok(Type::Unit {
                span: SrcPos::default(),
            }),
        }
    }
}

impl HirGetType for Value {
    fn determine_type(&self, ctx: &SymbolTab) -> Result<Type, TypeInferenceError> {
        match self {
            Value::Unit { .. } => Ok(Type::Unit {
                span: SrcPos::default(),
            }),
            Value::Bool { .. } => Ok(Type::Bool {
                span: SrcPos::default(),
            }),
            Value::I8 { .. } => Ok(Type::I8 {
                span: SrcPos::default(),
            }),
            Value::I16 { .. } => Ok(Type::I16 {
                span: SrcPos::default(),
            }),
            Value::I32 { .. } => Ok(Type::I32 {
                span: SrcPos::default(),
            }),
            Value::I64 { .. } => Ok(Type::I64 {
                span: SrcPos::default(),
            }),
            Value::I128 { .. } => Ok(Type::I128 {
                span: SrcPos::default(),
            }),
            Value::U8 { .. } => Ok(Type::U8 {
                span: SrcPos::default(),
            }),
            Value::U16 { .. } => Ok(Type::U16 {
                span: SrcPos::default(),
            }),
            Value::U32 { .. } => Ok(Type::U32 {
                span: SrcPos::default(),
            }),
            Value::U64 { .. } => Ok(Type::U64 {
                span: SrcPos::default(),
            }),
            Value::U128 { .. } => Ok(Type::U128 {
                span: SrcPos::default(),
            }),
            Value::F32 { .. } => Ok(Type::F32 {
                span: SrcPos::default(),
            }),
            Value::F64 { .. } => Ok(Type::F64 {
                span: SrcPos::default(),
            }),
            Value::USize { .. } => Ok(Type::USize {
                span: SrcPos::default(),
            }),
            Value::InferredInteger { .. } => Ok(Type::InferredInteger {
                span: SrcPos::default(),
            }),
            Value::InferredFloat { .. } => Ok(Type::InferredFloat {
                span: SrcPos::default(),
            }),

            Value::StringLit { .. } => Ok(Type::Str {
                span: SrcPos::default(),
            }),

            Value::BStringLit { .. } => Ok(Type::Str {
                span: SrcPos::default(),
            }),

            Value::StructObject { struct_def, .. } => Ok(Type::Struct {
                span: SrcPos::default(),
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
                    span: SrcPos::default(),
                }),
            },

            Value::Unary { operand: expr, .. } => match expr.borrow().determine_type(ctx) {
                Ok(Type::InferredFloat { .. }) => Ok(Type::InferredFloat {
                    span: SrcPos::default(),
                }),
                Ok(Type::InferredInteger { .. }) => Ok(Type::InferredInteger {
                    span: SrcPos::default(),
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
                span: SrcPos::default(),
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
                    span: SrcPos::default(),
                    lifetime: Lifetime::Inferred,
                    exclusive: *exclusive,
                    mutable: *mutable,
                    to: place_type.into(),
                })
            }

            Value::List { elements, .. } => {
                let element_type = if elements.is_empty() {
                    Type::Unit {
                        span: SrcPos::default(),
                    }
                    .into()
                } else {
                    elements[0].borrow().determine_type(ctx)?.into()
                };
                Ok(Type::Array {
                    span: SrcPos::default(),
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
                    span: SrcPos::default(),
                    element_types: element_types.into(),
                })
            }

            Value::If { false_branch, .. } => match false_branch {
                None => Ok(Type::Unit {
                    span: SrcPos::default(),
                }),
                Some(false_branch) => false_branch.borrow().determine_type(ctx),
            },

            Value::While { .. } => Ok(Type::Unit {
                span: SrcPos::default(),
            }),
            Value::Loop { .. } => Ok(Type::Unit {
                span: SrcPos::default(),
            }),
            Value::Break { .. } => Ok(Type::Never {
                span: SrcPos::default(),
            }),
            Value::Continue { .. } => Ok(Type::Never {
                span: SrcPos::default(),
            }),
            Value::Return { .. } => Ok(Type::Never {
                span: SrcPos::default(),
            }),

            Value::Block { block, .. } => block.borrow().determine_type(ctx),

            Value::Call { callee, type_args, .. } => {
                if let Type::Function { function_type, .. } = callee.borrow().determine_type(ctx)? {
                    let mut return_type = function_type.return_type.deref().clone();
                    // When calling a generic function with explicit type
                    // arguments (turbofish), substitute them into the return
                    // type so the caller observes the concrete instantiation.
                    if !type_args.positional.is_empty() || !type_args.named.is_empty() {
                        if let Value::FunctionSymbol { id, .. } = &*callee.borrow() {
                            let func = id.borrow();
                            if let Some(generics) = &func.generics
                                && !generics.is_empty()
                            {
                                let mut mapping: BTreeMap<u32, TypeId> = BTreeMap::new();
                                for (i, (_name, _)) in generics.iter().enumerate() {
                                    if let Some(arg) = type_args.positional.get(i) {
                                        mapping.insert(i as u32, *arg);
                                    }
                                }
                                for (name, arg) in &type_args.named {
                                    for (i, (gn, _)) in generics.iter().enumerate() {
                                        if gn == name {
                                            mapping.insert(i as u32, *arg);
                                        }
                                    }
                                }
                                return_type = substitute_generic_params(&return_type, &mapping);
                            }
                        }
                    }
                    return Ok(return_type);
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
                    span: SrcPos::default(),
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
                        span: SrcPos::default(),
                        def: struct_def.clone(),
                    }),
                    None => Err(TypeInferenceError::RangeStructNotRegistered),
                }
            }
        }
    }
}
