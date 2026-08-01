use crate::{context::Ast2HirCtx, diagnosis::HirErr, helpers, ty::lower_type};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_nstring::NString;
use nitrate_tree::ByteSpan;
use nitrate_tree::ast::{self as ast, SymbolKind, UnaryExprOp};
use ordered_float::OrderedFloat;
use std::collections::{BTreeMap, BTreeSet};
use std::ops::Deref;

// ═══════════════════════════════════════════════════════════════════════════
// Literal Lowering
// ═══════════════════════════════════════════════════════════════════════════

pub(crate) fn lower_boolean_literal(boolean_lit: ast::BooleanLit) -> Result<Value, ()> {
    Ok(Value::Bool {
        span: boolean_lit.span,
        value: boolean_lit.value,
    })
}

pub(crate) fn lower_integer_literal(integer_lit: ast::IntegerLit) -> Result<Value, ()> {
    Ok(Value::InferredInteger {
        span: integer_lit.span,
        value: Box::new(integer_lit.value),
    })
}

pub(crate) fn lower_float_literal(float_lit: ast::FloatLit) -> Result<Value, ()> {
    Ok(Value::InferredFloat {
        span: float_lit.span,
        value: (*float_lit.value).into(),
    })
}

pub(crate) fn lower_string_literal(string_lit: ast::StringLit) -> Result<Value, ()> {
    Ok(Value::StringLit {
        span: string_lit.span,
        value: string_lit.value.into(),
    })
}

pub(crate) fn lower_bstring_literal(bstring_lit: ast::BStringLit) -> Result<Value, ()> {
    Ok(Value::BStringLit {
        span: bstring_lit.span,
        value: bstring_lit.value.into(),
    })
}

// ═══════════════════════════════════════════════════════════════════════════
// Collection Literals
// ═══════════════════════════════════════════════════════════════════════════

pub(crate) fn lower_list(list: ast::List, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    let span = list.span;
    let mut elements = Vec::with_capacity(list.elements.len());

    for element in list.elements {
        let hir_element = lower_expr(element, ctx, log)?;
        elements.push(hir_element.into());
    }

    Ok(Value::List {
        span,
        elements: elements.into(),
    })
}

pub(crate) fn lower_tuple(tuple: ast::Tuple, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    let span = tuple.span;
    let mut elements = Vec::with_capacity(tuple.elements.len());

    for element in tuple.elements {
        let hir_element = lower_expr(element, ctx, log)?;
        elements.push(hir_element.into());
    }

    Ok(Value::Tuple {
        span,
        elements: elements.into(),
    })
}

// ═══════════════════════════════════════════════════════════════════════════
// Struct Initialization
// ═══════════════════════════════════════════════════════════════════════════

pub(crate) fn lower_struct_init(
    struct_init: ast::StructInit,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Value, ()> {
    let span = struct_init.span;

    // Collect explicit type arguments from turbofish syntax
    let explicit_type_args: Vec<TypeId> = struct_init
        .path
        .segments
        .iter()
        .flat_map(|seg| helpers::extract_type_args(&seg.type_arguments, ctx, log))
        .collect();

    let mut fields = Vec::with_capacity(struct_init.fields.len());
    for (field_name, value) in struct_init.fields {
        let field_value = lower_expr(value, ctx, log)?;
        fields.push((field_name, field_value.into()));
    }

    match struct_init.path.resolved_path {
        Some(resolved_path) => {
            let struct_def_id = ctx.tab.get_struct_or_insert_placeholder(&resolved_path);

            // If explicit type args were provided (turbofish), monomorphize the struct
            if !explicit_type_args.is_empty() {
                return handle_monomorphized_struct(
                    struct_def_id,
                    &explicit_type_args,
                    &resolved_path,
                    span,
                    fields,
                    ctx,
                );
            }

            Ok(Value::StructObject {
                span,
                struct_def: struct_def_id,
                fields: fields.into(),
            })
        }
        None => {
            log.report(&HirErr::UnresolvedTypePath {
                span,
                name: "struct path".into(),
            });
            Err(())
        }
    }
}

/// Handles monomorphization of a struct with explicit type arguments (turbofish).
fn handle_monomorphized_struct(
    struct_def_id: StructDefId,
    explicit_type_args: &[TypeId],
    resolved_path: &NString,
    span: ByteSpan,
    fields: Vec<(NString, ValueId)>,
    ctx: &mut Ast2HirCtx,
) -> Result<Value, ()> {
    let borrowed = struct_def_id.borrow();
    let Some(generics) = &borrowed.generics else {
        // No generics to substitute, return as-is
        drop(borrowed);
        return Ok(Value::StructObject {
            span,
            struct_def: struct_def_id,
            fields: fields.into(),
        });
    };

    let generic_params: Vec<&NString> = generics.keys().collect();

    // Build a unique suffix from the concrete type args
    let type_suffix: String = explicit_type_args
        .iter()
        .map(|t| format!("{:?}", t.deref()))
        .collect::<Vec<_>>()
        .join("_");
    let mono_name = format!("{}::<{}>", resolved_path, type_suffix);

    // Check if we already created this monomorphized version
    let mono_name_ns: NString = mono_name.clone().into();
    if let Some(existing_mono) = ctx.tab.get_struct(&mono_name_ns) {
        drop(borrowed);
        return Ok(Value::StructObject {
            span,
            struct_def: existing_mono.clone(),
            fields: fields.into(),
        });
    }

    let struct_def = &*borrowed;

    // Create a monomorphized copy of the struct
    let mut new_fields = BTreeMap::new();
    let mut new_layout = Vec::new();

    for (field_name, field) in &struct_def.fields {
        let new_field_ty = substitute_generic_params_in_type(&field.ty, &generic_params, explicit_type_args);
        let new_field = StructField {
            span: ByteSpan::default(),
            visibility: field.visibility,
            attributes: field.attributes.clone(),
            name: field.name.clone(),
            ty: TypeId::from(new_field_ty),
            default_value: field.default_value.clone(),
        };
        new_fields.insert(field_name.clone(), new_field);
        new_layout.push(StructMemoryLayoutCell::Field {
            field_name: field_name.clone(),
        });
    }

    let visibility = borrowed.visibility;
    let attributes = borrowed.attributes.clone();

    drop(borrowed);

    let mono_struct = StructDef {
        span: ByteSpan::default(),
        visibility,
        name: mono_name_ns,
        attributes,
        fields: new_fields,
        generics: None,
        layout: new_layout.into(),
    };
    let mono_id: StructDefId = mono_struct.into();
    ctx.tab.add_struct(mono_id.clone());

    Ok(Value::StructObject {
        span,
        struct_def: mono_id,
        fields: fields.into(),
    })
}

/// Substitute generic params in a type given explicit type args.
fn substitute_generic_params_in_type(ty: &Type, param_names: &[&NString], type_args: &[TypeId]) -> Type {
    match ty {
        Type::GenericParam { index, name, .. } => {
            for (i, param_name) in param_names.iter().enumerate() {
                if *param_name == name
                    && let Some(concrete_ty) = type_args.get(i)
                {
                    return concrete_ty.deref().clone();
                }
            }
            if let Some(concrete_ty) = type_args.get(*index as usize) {
                concrete_ty.deref().clone()
            } else {
                ty.clone()
            }
        }
        Type::Array { element_type, len, .. } => {
            let new_elem = substitute_generic_params_in_type(element_type, param_names, type_args);
            Type::Array {
                span: ByteSpan::default(),
                element_type: TypeId::from(new_elem),
                len: *len,
            }
        }
        Type::Tuple { element_types, .. } => {
            let new_elements: Vec<TypeId> = element_types
                .iter()
                .map(|et| TypeId::from(substitute_generic_params_in_type(et, param_names, type_args)))
                .collect();
            Type::Tuple {
                span: ByteSpan::default(),
                element_types: new_elements.into(),
            }
        }
        Type::Function { function_type, .. } => {
            let new_params: Vec<(NString, TypeId)> = function_type
                .params
                .iter()
                .map(|(n, p)| {
                    (
                        n.clone(),
                        TypeId::from(substitute_generic_params_in_type(p, param_names, type_args)),
                    )
                })
                .collect();
            let new_ret = substitute_generic_params_in_type(&function_type.return_type, param_names, type_args);
            Type::Function {
                span: ByteSpan::default(),
                function_type: Box::new(FunctionType {
                    attributes: function_type.attributes.clone(),
                    params: new_params.into(),
                    return_type: TypeId::from(new_ret),
                }),
            }
        }
        Type::Reference {
            lifetime,
            exclusive,
            mutable,
            to,
            ..
        } => {
            let new_to = substitute_generic_params_in_type(to, param_names, type_args);
            Type::Reference {
                span: ByteSpan::default(),
                lifetime: lifetime.clone(),
                exclusive: *exclusive,
                mutable: *mutable,
                to: TypeId::from(new_to),
            }
        }
        Type::Pointer {
            lifetime,
            exclusive,
            mutable,
            to,
            ..
        } => {
            let new_to = substitute_generic_params_in_type(to, param_names, type_args);
            Type::Pointer {
                span: ByteSpan::default(),
                lifetime: lifetime.clone(),
                exclusive: *exclusive,
                mutable: *mutable,
                to: TypeId::from(new_to),
            }
        }
        Type::SliceRef {
            lifetime,
            exclusive,
            mutable,
            element_type,
            ..
        } => {
            let new_elem = substitute_generic_params_in_type(element_type, param_names, type_args);
            Type::SliceRef {
                span: ByteSpan::default(),
                lifetime: lifetime.clone(),
                exclusive: *exclusive,
                mutable: *mutable,
                element_type: TypeId::from(new_elem),
            }
        }
        Type::SlicePtr {
            lifetime,
            exclusive,
            mutable,
            element_type,
            ..
        } => {
            let new_elem = substitute_generic_params_in_type(element_type, param_names, type_args);
            Type::SlicePtr {
                span: ByteSpan::default(),
                lifetime: lifetime.clone(),
                exclusive: *exclusive,
                mutable: *mutable,
                element_type: TypeId::from(new_elem),
            }
        }
        Type::TraitObject { bounds, .. } => Type::TraitObject {
            span: ByteSpan::default(),
            bounds: bounds.clone(),
        },
        Type::Refine { base, min, max, .. } => {
            let new_base = substitute_generic_params_in_type(base, param_names, type_args);
            Type::Refine {
                span: ByteSpan::default(),
                base: TypeId::from(new_base),
                min: *min,
                max: *max,
            }
        }
        Type::Parameterized { base, args, .. } => {
            let new_base = substitute_generic_params_in_type(base, param_names, type_args);
            let new_args: thin_vec::ThinVec<TypeId> = args
                .positional
                .iter()
                .map(|arg| TypeId::from(substitute_generic_params_in_type(arg, param_names, type_args)))
                .collect();
            match &new_base {
                Type::Struct { .. } | Type::TypeAlias { .. } => new_base,
                _ => Type::Parameterized {
                    span: ByteSpan::default(),
                    base: TypeId::from(new_base),
                    args: Arguments {
                        positional: new_args,
                        named: thin_vec::ThinVec::new(),
                    },
                },
            }
        }
        Type::TypeAlias { def, .. } => {
            let type_alias = def.borrow();
            substitute_generic_params_in_type(&type_alias.type_id, param_names, type_args)
        }
        _ => ty.clone(),
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Unary and Binary Expressions
// ═══════════════════════════════════════════════════════════════════════════

pub(crate) fn lower_unary(unary: ast::UnaryExpr, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    let span = unary.span;
    let operand = lower_expr(unary.operand, ctx, log)?;

    match unary.operator {
        UnaryExprOp::Add => Ok(Value::Unary {
            span,
            op: UnaryOp::Add,
            operand: operand.into(),
        }),
        UnaryExprOp::Sub => Ok(Value::Unary {
            span,
            op: UnaryOp::Sub,
            operand: operand.into(),
        }),
        UnaryExprOp::Not => Ok(Value::Unary {
            span,
            op: UnaryOp::Not,
            operand: operand.into(),
        }),
        UnaryExprOp::Deref => Ok(Value::Deref {
            span,
            place: operand.into(),
        }),
        UnaryExprOp::Borrow => Ok(Value::Borrow {
            span,
            exclusive: false,
            mutable: false,
            place: operand.into(),
        }),
        UnaryExprOp::Typeof => {
            log.report(&HirErr::TypeofNotImplemented { span });
            Err(())
        }
    }
}

pub(crate) fn lower_binary(binary: ast::BinExpr, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    let span = binary.span;
    let left = lower_expr(binary.left, ctx, log)?.into();
    let right = lower_expr(binary.right, ctx, log)?.into();

    match binary.operator {
        // Arithmetic
        ast::BinExprOp::Add => Ok(Value::Binary {
            span,
            left,
            op: BinaryOp::Add,
            right,
        }),
        ast::BinExprOp::Sub => Ok(Value::Binary {
            span,
            left,
            op: BinaryOp::Sub,
            right,
        }),
        ast::BinExprOp::Mul => Ok(Value::Binary {
            span,
            left,
            op: BinaryOp::Mul,
            right,
        }),
        ast::BinExprOp::Div => Ok(Value::Binary {
            span,
            left,
            op: BinaryOp::Div,
            right,
        }),
        ast::BinExprOp::Mod => Ok(Value::Binary {
            span,
            left,
            op: BinaryOp::Mod,
            right,
        }),

        // Bitwise
        ast::BinExprOp::BitAnd => Ok(Value::Binary {
            span,
            left,
            op: BinaryOp::And,
            right,
        }),
        ast::BinExprOp::BitOr => Ok(Value::Binary {
            span,
            left,
            op: BinaryOp::Or,
            right,
        }),
        ast::BinExprOp::BitXor => Ok(Value::Binary {
            span,
            left,
            op: BinaryOp::Xor,
            right,
        }),
        ast::BinExprOp::BitShl => Ok(Value::Binary {
            span,
            left,
            op: BinaryOp::Shl,
            right,
        }),
        ast::BinExprOp::BitShr => Ok(Value::Binary {
            span,
            left,
            op: BinaryOp::Shr,
            right,
        }),
        ast::BinExprOp::BitRol => Ok(Value::Binary {
            span,
            left,
            op: BinaryOp::Rol,
            right,
        }),
        ast::BinExprOp::BitRor => Ok(Value::Binary {
            span,
            left,
            op: BinaryOp::Ror,
            right,
        }),

        // Logical
        ast::BinExprOp::LogicAnd => Ok(Value::Binary {
            span,
            left,
            op: BinaryOp::LogicAnd,
            right,
        }),
        ast::BinExprOp::LogicOr => Ok(Value::Binary {
            span,
            left,
            op: BinaryOp::LogicOr,
            right,
        }),

        // Comparison
        ast::BinExprOp::LogicLt => Ok(Value::Binary {
            span,
            left,
            op: BinaryOp::Lt,
            right,
        }),
        ast::BinExprOp::LogicGt => Ok(Value::Binary {
            span,
            left,
            op: BinaryOp::Gt,
            right,
        }),
        ast::BinExprOp::LogicLe => Ok(Value::Binary {
            span,
            left,
            op: BinaryOp::Lte,
            right,
        }),
        ast::BinExprOp::LogicGe => Ok(Value::Binary {
            span,
            left,
            op: BinaryOp::Gte,
            right,
        }),
        ast::BinExprOp::LogicEq => Ok(Value::Binary {
            span,
            left,
            op: BinaryOp::Eq,
            right,
        }),
        ast::BinExprOp::LogicNe => Ok(Value::Binary {
            span,
            left,
            op: BinaryOp::Ne,
            right,
        }),

        // Assignment
        ast::BinExprOp::Set => Ok(Value::Assign {
            span,
            place: left,
            value: right,
        }),

        // Compound assignment operators
        op @ (ast::BinExprOp::SetPlus
        | ast::BinExprOp::SetMinus
        | ast::BinExprOp::SetTimes
        | ast::BinExprOp::SetSlash
        | ast::BinExprOp::SetPercent
        | ast::BinExprOp::SetBitAnd
        | ast::BinExprOp::SetBitOr
        | ast::BinExprOp::SetBitXor
        | ast::BinExprOp::SetBitShl
        | ast::BinExprOp::SetBitShr
        | ast::BinExprOp::SetBitRotl
        | ast::BinExprOp::SetBitRotr
        | ast::BinExprOp::SetLogicAnd
        | ast::BinExprOp::SetLogicOr) => Ok(lower_compound_assignment(span, left, right, op)),
    }
}

pub(crate) fn lower_range(range: ast::Range, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    let span = range.span;
    let start = range.start.map(|s| lower_expr(*s, ctx, log).unwrap().into());
    let end = range.end.map(|e| lower_expr(*e, ctx, log).unwrap().into());

    Ok(Value::Range {
        span,
        start,
        end,
        inclusive: range.kind == ast::RangeKind::RangeInclusive || range.kind == ast::RangeKind::RangeToInclusive,
    })
}

/// Lower compound assignment operators (e.g., `+=`, `-=`) by expanding
/// them into `place = place OP value`.
fn lower_compound_assignment(span: ByteSpan, place: ValueId, value: ValueId, op: ast::BinExprOp) -> Value {
    let binary_op = match op {
        ast::BinExprOp::SetPlus => BinaryOp::Add,
        ast::BinExprOp::SetMinus => BinaryOp::Sub,
        ast::BinExprOp::SetTimes => BinaryOp::Mul,
        ast::BinExprOp::SetSlash => BinaryOp::Div,
        ast::BinExprOp::SetPercent => BinaryOp::Mod,
        ast::BinExprOp::SetBitAnd => BinaryOp::And,
        ast::BinExprOp::SetBitOr => BinaryOp::Or,
        ast::BinExprOp::SetBitXor => BinaryOp::Xor,
        ast::BinExprOp::SetBitShl => BinaryOp::Shl,
        ast::BinExprOp::SetBitShr => BinaryOp::Shr,
        ast::BinExprOp::SetBitRotl => BinaryOp::Rol,
        ast::BinExprOp::SetBitRotr => BinaryOp::Ror,
        ast::BinExprOp::SetLogicAnd => BinaryOp::And,
        ast::BinExprOp::SetLogicOr => BinaryOp::Or,
        _ => unreachable!(),
    };

    let binary = Value::Binary {
        span,
        left: place.clone(),
        op: binary_op,
        right: value,
    };

    Value::Assign {
        span,
        place,
        value: binary.into(),
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Cast Expressions
// ═══════════════════════════════════════════════════════════════════════════

pub(crate) fn lower_cast(cast: ast::Cast, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    let span = cast.span;
    let expr = lower_expr(cast.value, ctx, log)?;
    let to = lower_type(cast.to, ctx, log)?;

    if let Value::InferredInteger { value, .. } = &expr {
        let cast_result = try_cast_inferred_integer(**value, &to, span, ctx, log);
        if cast_result.is_some() {
            return cast_result.unwrap();
        }
    }

    if let Value::InferredFloat { value: v, .. } = &expr {
        match &to {
            Type::F32 { .. } => {
                return Ok(Value::F32 {
                    span,
                    value: OrderedFloat::from(f64::from(*v) as f32),
                });
            }
            Type::F64 { .. } => {
                return Ok(Value::F64 { span, value: *v });
            }
            _ => {}
        }
    }

    Ok(Value::Cast {
        span,
        value: expr.into(),
        target_type: to.into(),
    })
}

/// Try to cast an inferred integer literal directly to a concrete type.
fn try_cast_inferred_integer(
    value: u128,
    target_type: &Type,
    span: ByteSpan,
    ctx: &Ast2HirCtx,
    log: &CompilerLog,
) -> Option<Result<Value, ()>> {
    Some(match target_type {
        Type::U8 { .. } => match u8::try_from(value) {
            Ok(v) => Ok(Value::U8 { span, value: v }),
            Err(_) => {
                log.report(&HirErr::IntegerCastOutOfRange {
                    span,
                    value: format!("{}", value),
                    target_type: "u8".into(),
                });
                return None;
            }
        },
        Type::U16 { .. } => match u16::try_from(value) {
            Ok(v) => Ok(Value::U16 { span, value: v }),
            Err(_) => {
                log.report(&HirErr::IntegerCastOutOfRange {
                    span,
                    value: format!("{}", value),
                    target_type: "u16".into(),
                });
                return None;
            }
        },
        Type::U32 { .. } => match u32::try_from(value) {
            Ok(v) => Ok(Value::U32 { span, value: v }),
            Err(_) => {
                log.report(&HirErr::IntegerCastOutOfRange {
                    span,
                    value: format!("{}", value),
                    target_type: "u32".into(),
                });
                return None;
            }
        },
        Type::U64 { .. } => match u64::try_from(value) {
            Ok(v) => Ok(Value::U64 { span, value: v }),
            Err(_) => {
                log.report(&HirErr::IntegerCastOutOfRange {
                    span,
                    value: format!("{}", value),
                    target_type: "u64".into(),
                });
                return None;
            }
        },
        Type::U128 { .. } => Ok(Value::U128 {
            span,
            value: Box::new(value),
        }),
        Type::USize { .. } => match ctx.ptr_size {
            PtrSize::U32 => match u32::try_from(value) {
                Ok(v) => Ok(Value::USize {
                    span,
                    bits: 32,
                    value: v as u64,
                }),
                Err(_) => {
                    log.report(&HirErr::IntegerCastOutOfRange {
                        span,
                        value: format!("{}", value),
                        target_type: "usize".into(),
                    });
                    return None;
                }
            },
            PtrSize::U64 => match u64::try_from(value) {
                Ok(v) => Ok(Value::USize {
                    span,
                    bits: 64,
                    value: v,
                }),
                Err(_) => {
                    log.report(&HirErr::IntegerCastOutOfRange {
                        span,
                        value: format!("{}", value),
                        target_type: "usize".into(),
                    });
                    return None;
                }
            },
        },
        Type::I8 { .. } => match i8::try_from(value) {
            Ok(v) => Ok(Value::I8 { span, value: v }),
            Err(_) => {
                log.report(&HirErr::IntegerCastOutOfRange {
                    span,
                    value: format!("{}", value),
                    target_type: "i8".into(),
                });
                return None;
            }
        },
        Type::I16 { .. } => match i16::try_from(value) {
            Ok(v) => Ok(Value::I16 { span, value: v }),
            Err(_) => {
                log.report(&HirErr::IntegerCastOutOfRange {
                    span,
                    value: format!("{}", value),
                    target_type: "i16".into(),
                });
                return None;
            }
        },
        Type::I32 { .. } => match i32::try_from(value) {
            Ok(v) => Ok(Value::I32 { span, value: v }),
            Err(_) => {
                log.report(&HirErr::IntegerCastOutOfRange {
                    span,
                    value: format!("{}", value),
                    target_type: "i32".into(),
                });
                return None;
            }
        },
        Type::I64 { .. } => match i64::try_from(value) {
            Ok(v) => Ok(Value::I64 { span, value: v }),
            Err(_) => {
                log.report(&HirErr::IntegerCastOutOfRange {
                    span,
                    value: format!("{}", value),
                    target_type: "i64".into(),
                });
                return None;
            }
        },
        Type::I128 { .. } => match i128::try_from(value) {
            Ok(v) => Ok(Value::I128 {
                span,
                value: Box::new(v),
            }),
            Err(_) => {
                log.report(&HirErr::IntegerCastOutOfRange {
                    span,
                    value: format!("{}", value),
                    target_type: "i128".into(),
                });
                return None;
            }
        },
        _ => return None,
    })
}

// ═══════════════════════════════════════════════════════════════════════════
// Local Variable Lowering
// ═══════════════════════════════════════════════════════════════════════════

fn lower_local_variable(
    local_var: &ast::LocalVariable,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<LocalVariableId, ()> {
    let span = local_var.span;
    let kind = match local_var.kind {
        ast::LocalVariableKind::Let => LocalKind::Let,
        ast::LocalVariableKind::Var => LocalKind::Var,
    };

    helpers::reject_all_attributes(
        &local_var.attributes,
        |name, span| HirErr::UnrecognizedLocalVarAttribute { span, name },
        log,
    );

    let is_mutable = helpers::is_mutable(local_var.mutability);

    let name = helpers::qualify(&local_var.name, ctx);

    let ty = match local_var.ty.to_owned() {
        None => ctx.create_inference_placeholder().into(),
        Some(t) => lower_type(t, ctx, log)?.into(),
    };

    let initializer = match local_var.initializer.to_owned() {
        Some(expr) => lower_expr(expr, ctx, log)?.into(),
        None => {
            log.report(&HirErr::LocalVariableMissingInitializer {
                span,
                name: name.to_string(),
            });
            return Err(());
        }
    };

    let localvar_id: LocalVariableId = LocalVariable {
        span,
        kind,
        attributes: BTreeSet::new(),
        is_mutable,
        name,
        ty,
        initializer,
    }
    .into();

    ctx.tab.add_local_variable(localvar_id.clone());
    Ok(localvar_id)
}

// ═══════════════════════════════════════════════════════════════════════════
// Block Expressions
// ═══════════════════════════════════════════════════════════════════════════

pub(crate) fn lower_block(block: ast::Block, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Block, ()> {
    let span = block.span;
    let elements_len = block.elements.len();
    let mut elements = Vec::with_capacity(elements_len);

    for (i, element) in block.elements.into_iter().enumerate() {
        match element {
            ast::BlockItem::Expr(e) => {
                let hir_element = lower_expr(e, ctx, log)?.into();
                elements.push(BlockElement::Expr(hir_element));
            }
            ast::BlockItem::Stmt(s) => {
                let hir_element = lower_expr(s, ctx, log)?.into();
                elements.push(BlockElement::Expr(hir_element));
                if i == elements_len - 1 {
                    elements.push(BlockElement::Expr(
                        Value::Unit {
                            span: ByteSpan::default(),
                        }
                        .into(),
                    ));
                }
            }
            ast::BlockItem::Variable(var) => {
                let var_hir = lower_local_variable(&var, ctx, log)?;
                elements.push(BlockElement::Local(var_hir));
            }
        }
    }

    let safety = match block.safety {
        Some(ast::Safety::Unsafe(None)) => BlockSafety::Unsafe,
        Some(ast::Safety::Safe) | None => BlockSafety::Safe,
        Some(ast::Safety::Unsafe(Some(_))) => {
            log.report(&HirErr::UnsafeExprBodyNotImplemented { span });
            return Err(());
        }
    };

    Ok(Block { span, safety, elements })
}

pub(crate) fn lower_block_value(block: ast::Block, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    let block = lower_block(block, ctx, log)?;
    Ok(Value::Block {
        span: block.span,
        block: block.into(),
    })
}

// ═══════════════════════════════════════════════════════════════════════════
// Expression Path / Symbol Resolution
// ═══════════════════════════════════════════════════════════════════════════

pub(crate) fn lower_expr_path(expr_path: ast::ExprPath, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    let span = expr_path.span;

    let _explicit_type_args = helpers::extract_type_args_from_expr_path(&expr_path.segments, ctx, log);

    match expr_path.resolved_path {
        Some(resolved_path) => match ctx.ast_symbol_map.get(&resolved_path) {
            Some(SymbolKind::EnumVariant) => Ok(Value::EnumVariant {
                span,
                enum_def: ctx.tab.get_enum_variant_or_insert_placeholder(&resolved_path),
                variant: resolved_path.split("::").last().unwrap().to_string().into(),
                value: Value::Unit {
                    span: ByteSpan::default(),
                }
                .into(),
            }),
            Some(SymbolKind::Function) => {
                let func_id = ctx.tab.get_function_or_insert_placeholder(&resolved_path);
                Ok(Value::FunctionSymbol { span, id: func_id })
            }
            Some(SymbolKind::GlobalVariable) => Ok(Value::GlobalVariableSymbol {
                span,
                id: ctx.tab.get_global_variable_or_insert_placeholder(&resolved_path),
            }),
            Some(SymbolKind::LocalVariable) => Ok(Value::LocalVariableSymbol {
                span,
                id: ctx.tab.get_local_variable_or_insert_placeholder(&resolved_path),
            }),
            Some(SymbolKind::Parameter) => Ok(Value::ParameterSymbol {
                span,
                id: ctx.tab.get_parameter_or_insert_placeholder(&resolved_path),
            }),
            Some(_) => {
                log.report(&HirErr::UnresolvedSymbol {
                    span,
                    name: resolved_path.to_string(),
                });
                Err(())
            }
            None => {
                log.report(&HirErr::UnresolvedSymbol {
                    span,
                    name: resolved_path.to_string(),
                });
                Err(())
            }
        },
        None => {
            let path_str = expr_path
                .segments
                .iter()
                .map(|s| s.name.to_string())
                .collect::<Vec<_>>()
                .join("::");
            log.report(&HirErr::UnresolvedSymbol { span, name: path_str });
            Err(())
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Access Expressions
// ═══════════════════════════════════════════════════════════════════════════

pub(crate) fn lower_index_access(
    index_access: ast::IndexAccess,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Value, ()> {
    let collection = lower_expr(index_access.collection, ctx, log)?.into();
    let index = lower_expr(index_access.index, ctx, log)?.into();

    Ok(Value::IndexAccess {
        span: index_access.span,
        collection,
        index,
    })
}

pub(crate) fn lower_field_access(
    field_access: ast::FieldAccess,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Value, ()> {
    let object = lower_expr(field_access.object, ctx, log)?.into();
    let field = NString::from(field_access.field.to_string());

    Ok(Value::FieldAccess {
        span: field_access.span,
        expr: object,
        field_name: field,
    })
}

// ═══════════════════════════════════════════════════════════════════════════
// Control Flow
// ═══════════════════════════════════════════════════════════════════════════

pub(crate) fn lower_if(if_: ast::If, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    let span = if_.span;
    let condition = lower_expr(if_.condition, ctx, log)?.into();
    let true_branch = lower_block(if_.true_branch, ctx, log)?.into();

    let false_branch = match if_.false_branch {
        Some(ast::ElseIf::If(else_if)) => {
            let else_if_value = lower_if(*else_if, ctx, log)?;
            let block = Block {
                span: ByteSpan::default(),
                safety: BlockSafety::Safe,
                elements: vec![BlockElement::Expr(else_if_value.into())],
            };
            Some(block.into())
        }
        Some(ast::ElseIf::Block(block)) => Some(lower_block(block, ctx, log)?.into()),
        None => None,
    };

    Ok(Value::If {
        span,
        condition,
        true_branch,
        false_branch,
    })
}

pub(crate) fn lower_while_loop(
    while_loop: ast::WhileLoop,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Value, ()> {
    let span = while_loop.span;
    let condition = match while_loop.condition {
        Some(cond) => lower_expr(cond, ctx, log)?.into(),
        None => Value::Bool {
            span: ByteSpan::default(),
            value: true,
        }
        .into(),
    };

    let body = lower_block(while_loop.body, ctx, log)?.into();

    Ok(Value::While { span, condition, body })
}

pub(crate) fn lower_break(break_: ast::Break, _ctx: &mut Ast2HirCtx, _log: &CompilerLog) -> Result<Value, ()> {
    Ok(Value::Break {
        span: break_.span,
        label: break_.label.map(|l| l.to_string().into()),
    })
}

pub(crate) fn lower_continue(continue_: ast::Continue, _ctx: &mut Ast2HirCtx, _log: &CompilerLog) -> Result<Value, ()> {
    Ok(Value::Continue {
        span: continue_.span,
        label: continue_.label.map(|l| l.to_string().into()),
    })
}

pub(crate) fn lower_return(return_: ast::Return, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    let span = return_.span;
    let value = match return_.value {
        Some(v) => lower_expr(v, ctx, log)?.into(),
        None => Value::Unit {
            span: ByteSpan::default(),
        }
        .into(),
    };

    Ok(Value::Return { span, value })
}

// ═══════════════════════════════════════════════════════════════════════════
// Call Expressions
// ═══════════════════════════════════════════════════════════════════════════

pub(crate) fn lower_function_call(
    function_call: ast::FunctionCall,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Value, ()> {
    let span = function_call.span;
    let callee = lower_expr(function_call.callee, ctx, log)?;

    let args = lower_call_arguments(function_call.positional, function_call.named, ctx, log);

    Ok(Value::Call {
        span,
        callee: callee.into(),
        args,
    })
}

pub(crate) fn lower_method_call(
    method_call: ast::MethodCall,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Value, ()> {
    let span = method_call.span;
    let object = lower_expr(method_call.object, ctx, log)?.into();
    let method = NString::from(method_call.method_name);

    let args = lower_call_arguments(method_call.positional, method_call.named, ctx, log);

    Ok(Value::MethodCall {
        span,
        object,
        method_name: method,
        args,
    })
}

/// Lower function/method call arguments (positional + named) to a single `Arguments` struct.
fn lower_call_arguments(
    positional: Vec<ast::Expr>,
    named: Vec<(NString, ast::Expr)>,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Arguments<ValueId> {
    let mut pos = Vec::with_capacity(positional.len());
    for arg in positional {
        if let Ok(value) = lower_expr(arg, ctx, log) {
            pos.push(value.into());
        }
    }

    let mut nam = Vec::with_capacity(named.len());
    for (name, arg) in named {
        if let Ok(value) = lower_expr(arg, ctx, log) {
            nam.push((name, value.into()));
        }
    }

    Arguments {
        positional: pos.into(),
        named: nam.into(),
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Unimplemented Feature Stubs
// ═══════════════════════════════════════════════════════════════════════════

pub(crate) fn lower_closure(closure: ast::Closure, _ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    log.report(&HirErr::ClosureNotImplemented { span: closure.span });
    Err(())
}

pub(crate) fn lower_match(match_: ast::Match, _ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    log.report(&HirErr::MatchNotImplemented { span: match_.span });
    Err(())
}

pub(crate) fn lower_for_each(for_each: ast::ForEach, _ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    log.report(&HirErr::ForLoopNotImplemented { span: for_each.span });
    Err(())
}

pub(crate) fn lower_await(await_: ast::Await, _ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    log.report(&HirErr::AwaitNotImplemented { span: await_.span });
    Err(())
}

pub(crate) fn lower_type_reflection(
    type_info: ast::TypeInfo,
    _ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Value, ()> {
    log.report(&HirErr::TypeReflectionNotImplemented { span: type_info.span });
    Err(())
}

// ═══════════════════════════════════════════════════════════════════════════
// Main Expression Dispatch
// ═══════════════════════════════════════════════════════════════════════════

pub(crate) fn lower_expr(x: ast::Expr, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    match x {
        ast::Expr::SyntaxError(_) => Err(()),
        ast::Expr::Parentheses(e) => lower_expr(e.inner, ctx, log),
        ast::Expr::Boolean(e) => lower_boolean_literal(e),
        ast::Expr::Integer(e) => lower_integer_literal(*e),
        ast::Expr::Float(e) => lower_float_literal(e),
        ast::Expr::String(e) => lower_string_literal(e),
        ast::Expr::BString(e) => lower_bstring_literal(*e),
        ast::Expr::TypeInfo(e) => lower_type_reflection(*e, ctx, log),
        ast::Expr::List(e) => lower_list(*e, ctx, log),
        ast::Expr::Tuple(e) => lower_tuple(*e, ctx, log),
        ast::Expr::StructInit(e) => lower_struct_init(*e, ctx, log),
        ast::Expr::UnaryExpr(e) => lower_unary(*e, ctx, log),
        ast::Expr::BinExpr(e) => lower_binary(*e, ctx, log),
        ast::Expr::Range(e) => lower_range(*e, ctx, log),
        ast::Expr::Cast(e) => lower_cast(*e, ctx, log),
        ast::Expr::Block(e) => lower_block_value(*e, ctx, log),
        ast::Expr::Closure(e) => lower_closure(*e, ctx, log),
        ast::Expr::Path(e) => lower_expr_path(*e, ctx, log),
        ast::Expr::IndexAccess(e) => lower_index_access(*e, ctx, log),
        ast::Expr::FieldAccess(e) => lower_field_access(*e, ctx, log),
        ast::Expr::If(e) => lower_if(*e, ctx, log),
        ast::Expr::While(e) => lower_while_loop(*e, ctx, log),
        ast::Expr::Match(e) => lower_match(*e, ctx, log),
        ast::Expr::Break(e) => lower_break(*e, ctx, log),
        ast::Expr::Continue(e) => lower_continue(*e, ctx, log),
        ast::Expr::Return(e) => lower_return(*e, ctx, log),
        ast::Expr::For(e) => lower_for_each(*e, ctx, log),
        ast::Expr::Await(e) => lower_await(*e, ctx, log),
        ast::Expr::FunctionCall(e) => lower_function_call(*e, ctx, log),
        ast::Expr::MethodCall(e) => lower_method_call(*e, ctx, log),
    }
}
