use crate::diagnosis::HirErr;
use crate::{context::Ast2HirCtx, ty::lower_type};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_nstring::NString;
use nitrate_tree::ByteSpan;
use nitrate_tree::ast::{self as ast, SymbolKind, UnaryExprOp};
use ordered_float::OrderedFloat;
use std::collections::BTreeSet;
use std::ops::Deref;

pub(crate) fn lower_boolean_literal(boolean_lit: ast::BooleanLit) -> Result<Value, ()> {
    let span = boolean_lit.span;
    match boolean_lit.value {
        true => Ok(Value::Bool { span, value: true }),
        false => Ok(Value::Bool { span, value: false }),
    }
}

pub(crate) fn lower_integer_literal(integer_lit: ast::IntegerLit) -> Result<Value, ()> {
    Ok(Value::InferredInteger {
        span: integer_lit.span,
        value: integer_lit.value,
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

pub(crate) fn lower_type_reflection(
    _type_info: ast::TypeInfo,
    _ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Value, ()> {
    log.report(&HirErr::UnimplementedFeature("Type reflection".into()));
    Err(())
}

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

pub(crate) fn lower_struct_init(
    struct_init: ast::StructInit,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Value, ()> {
    let span = struct_init.span;

    // Collect explicit type arguments from turbofish syntax (e.g., Pair::<i32>)
    let explicit_type_args: Vec<TypeId> = struct_init
        .path
        .segments
        .iter()
        .filter_map(|seg| {
            seg.type_arguments.as_ref().map(|type_args| {
                type_args
                    .iter()
                    .filter_map(|type_arg| {
                        lower_type(type_arg.value.clone(), ctx, log).ok().map(|t| {
                            let hir_type_arg: TypeId = t.into();
                            hir_type_arg
                        })
                    })
                    .collect::<Vec<_>>()
            })
        })
        .flatten()
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
            // by creating a uniquely-named concrete copy
            if !explicit_type_args.is_empty() {
                let struct_def = struct_def_id.borrow();
                if struct_def.generics.is_some() {
                    let generic_params: Vec<&NString> = struct_def
                        .generics
                        .as_ref()
                        .map(|g| g.keys().collect())
                        .unwrap_or_default();

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
                        return Ok(Value::StructObject {
                            span,
                            struct_def: existing_mono.clone(),
                            fields: fields.into(),
                        });
                    }

                    use std::collections::BTreeMap;
                    let mut new_fields = BTreeMap::new();
                    let mut new_layout = Vec::new();

                    for (field_name, field) in &struct_def.fields {
                        let new_field_ty =
                            substitute_generic_params_in_type(&field.ty, &generic_params, &explicit_type_args);
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

                    let mono_struct = StructDef {
                        span: ByteSpan::default(),
                        visibility: struct_def.visibility,
                        name: mono_name_ns,
                        attributes: struct_def.attributes.clone(),
                        fields: new_fields,
                        generics: None,
                        layout: new_layout.into(),
                    };
                    let mono_id: StructDefId = mono_struct.into();
                    ctx.tab.add_struct(mono_id.clone());
                    return Ok(Value::StructObject {
                        span,
                        struct_def: mono_id,
                        fields: fields.into(),
                    });
                }
            }

            Ok(Value::StructObject {
                span,
                struct_def: struct_def_id,
                fields: fields.into(),
            })
        }

        None => {
            log.report(&HirErr::UnresolvedTypePath);
            Err(())
        }
    }
}

/// Helper function: substitute generic params in a type given explicit type args.
/// Used in lower_struct_init to monomorphize struct types at HIR lowering time.
fn substitute_generic_params_in_type(ty: &Type, param_names: &[&NString], type_args: &[TypeId]) -> Type {
    match ty {
        Type::GenericParam { index, name, .. } => {
            // Find this name's position among generic params
            for (i, param_name) in param_names.iter().enumerate() {
                if *param_name == name
                    && let Some(concrete_ty) = type_args.get(i)
                {
                    return concrete_ty.deref().clone();
                }
            }
            // If index matches, use that directly
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
            // Substitute in the base type and all type arguments
            let new_base = substitute_generic_params_in_type(base, param_names, type_args);
            let new_args: thin_vec::ThinVec<TypeId> = args
                .positional
                .iter()
                .map(|arg| TypeId::from(substitute_generic_params_in_type(arg, param_names, type_args)))
                .collect();
            // If the base was a GenericParam that got substituted, just return the substituted type
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
        Type::Struct { def, .. } => {
            // For struct types, also substitute generics within
            let struct_def = def.borrow();
            if struct_def.generics.is_some() {
                // Need to create a monomorphized copy of this struct
                let _generic_params: Vec<&NString> = struct_def
                    .generics
                    .as_ref()
                    .map(|g| g.keys().collect())
                    .unwrap_or_default();
                // Check if all type params are provided by the outer call
                // This case should generally be handled via the Parameterized path above
            }
            ty.clone()
        }
        _ => ty.clone(),
    }
}

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
            log.report(&HirErr::UnimplementedFeature("Type reflection".into()));
            Err(())
        }
    }
}

pub(crate) fn lower_binary(binary: ast::BinExpr, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    let span = binary.span;
    let left = lower_expr(binary.left, ctx, log)?.into();
    let right = lower_expr(binary.right, ctx, log)?.into();

    let make_binary = |op: BinaryOp| Value::Binary { span, left, op, right };
    let make_assign = |val: Value| Value::Assign {
        span,
        place: left.clone(),
        value: val.into(),
    };

    match binary.operator {
        ast::BinExprOp::Add => Ok(make_binary(BinaryOp::Add)),
        ast::BinExprOp::Sub => Ok(make_binary(BinaryOp::Sub)),
        ast::BinExprOp::Mul => Ok(make_binary(BinaryOp::Mul)),
        ast::BinExprOp::Div => Ok(make_binary(BinaryOp::Div)),
        ast::BinExprOp::Mod => Ok(make_binary(BinaryOp::Mod)),
        ast::BinExprOp::BitAnd => Ok(make_binary(BinaryOp::And)),
        ast::BinExprOp::BitOr => Ok(make_binary(BinaryOp::Or)),
        ast::BinExprOp::BitXor => Ok(make_binary(BinaryOp::Xor)),
        ast::BinExprOp::BitShl => Ok(make_binary(BinaryOp::Shl)),
        ast::BinExprOp::BitShr => Ok(make_binary(BinaryOp::Shr)),
        ast::BinExprOp::BitRol => Ok(make_binary(BinaryOp::Rol)),
        ast::BinExprOp::BitRor => Ok(make_binary(BinaryOp::Ror)),
        ast::BinExprOp::LogicAnd => Ok(make_binary(BinaryOp::LogicAnd)),
        ast::BinExprOp::LogicOr => Ok(make_binary(BinaryOp::LogicOr)),
        ast::BinExprOp::LogicLt => Ok(make_binary(BinaryOp::Lt)),
        ast::BinExprOp::LogicGt => Ok(make_binary(BinaryOp::Gt)),
        ast::BinExprOp::LogicLe => Ok(make_binary(BinaryOp::Lte)),
        ast::BinExprOp::LogicGe => Ok(make_binary(BinaryOp::Gte)),
        ast::BinExprOp::LogicEq => Ok(make_binary(BinaryOp::Eq)),
        ast::BinExprOp::LogicNe => Ok(make_binary(BinaryOp::Ne)),
        ast::BinExprOp::Set => Ok(Value::Assign {
            span,
            place: left,
            value: right,
        }),
        ast::BinExprOp::SetPlus => Ok(make_assign(Value::Binary {
            span,
            left,
            op: BinaryOp::Add,
            right,
        })),
        ast::BinExprOp::SetMinus => Ok(make_assign(Value::Binary {
            span,
            left,
            op: BinaryOp::Sub,
            right,
        })),
        ast::BinExprOp::SetTimes => Ok(make_assign(Value::Binary {
            span,
            left,
            op: BinaryOp::Mul,
            right,
        })),
        ast::BinExprOp::SetSlash => Ok(make_assign(Value::Binary {
            span,
            left,
            op: BinaryOp::Div,
            right,
        })),
        ast::BinExprOp::SetPercent => Ok(make_assign(Value::Binary {
            span,
            left,
            op: BinaryOp::Mod,
            right,
        })),
        ast::BinExprOp::SetBitAnd => Ok(make_assign(Value::Binary {
            span,
            left,
            op: BinaryOp::And,
            right,
        })),
        ast::BinExprOp::SetBitOr => Ok(make_assign(Value::Binary {
            span,
            left,
            op: BinaryOp::Or,
            right,
        })),
        ast::BinExprOp::SetBitXor => Ok(make_assign(Value::Binary {
            span,
            left,
            op: BinaryOp::Xor,
            right,
        })),
        ast::BinExprOp::SetBitShl => Ok(make_assign(Value::Binary {
            span,
            left,
            op: BinaryOp::Shl,
            right,
        })),
        ast::BinExprOp::SetBitShr => Ok(make_assign(Value::Binary {
            span,
            left,
            op: BinaryOp::Shr,
            right,
        })),
        ast::BinExprOp::SetBitRotl => Ok(make_assign(Value::Binary {
            span,
            left,
            op: BinaryOp::Rol,
            right,
        })),
        ast::BinExprOp::SetBitRotr => Ok(make_assign(Value::Binary {
            span,
            left,
            op: BinaryOp::Ror,
            right,
        })),
        ast::BinExprOp::SetLogicAnd => Ok(make_assign(Value::Binary {
            span,
            left,
            op: BinaryOp::And,
            right,
        })),
        ast::BinExprOp::SetLogicOr => Ok(make_assign(Value::Binary {
            span,
            left,
            op: BinaryOp::Or,
            right,
        })),
        ast::BinExprOp::Range => {
            log.report(&HirErr::UnimplementedFeature("range .. operator".into()));
            Err(())
        }
    }
}

pub(crate) fn lower_cast(cast: ast::Cast, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    fn failed_to_cast(log: &CompilerLog) -> Result<Value, ()> {
        log.report(&HirErr::IntegerCastOutOfRange);
        Err(())
    }

    let span = cast.span;
    let expr = lower_expr(cast.value, ctx, log)?;
    let to = lower_type(cast.to, ctx, log)?;

    match (expr, to) {
        (Value::InferredInteger { value, .. }, Type::U8 { .. }) => match u8::try_from(*value) {
            Ok(v) => Ok(Value::U8 { span, value: v }),
            Err(_) => failed_to_cast(log),
        },
        (Value::InferredInteger { value, .. }, Type::U16 { .. }) => match u16::try_from(*value) {
            Ok(v) => Ok(Value::U16 { span, value: v }),
            Err(_) => failed_to_cast(log),
        },
        (Value::InferredInteger { value, .. }, Type::U32 { .. }) => match u32::try_from(*value) {
            Ok(v) => Ok(Value::U32 { span, value: v }),
            Err(_) => failed_to_cast(log),
        },
        (Value::InferredInteger { value, .. }, Type::U64 { .. }) => match u64::try_from(*value) {
            Ok(v) => Ok(Value::U64 { span, value: v }),
            Err(_) => failed_to_cast(log),
        },
        (Value::InferredInteger { value, .. }, Type::U128 { .. }) => match u128::try_from(*value) {
            Ok(v) => Ok(Value::U128 {
                span,
                value: Box::new(v),
            }),
            Err(_) => failed_to_cast(log),
        },
        (Value::InferredInteger { value, .. }, Type::USize { .. }) => match ctx.ptr_size {
            PtrSize::U32 => match u32::try_from(*value) {
                Ok(v) => Ok(Value::USize {
                    span,
                    bits: 32,
                    value: v as u64,
                }),
                Err(_) => failed_to_cast(log),
            },
            PtrSize::U64 => match u64::try_from(*value) {
                Ok(v) => Ok(Value::USize {
                    span,
                    bits: 64,
                    value: v,
                }),
                Err(_) => failed_to_cast(log),
            },
        },
        (Value::InferredInteger { value, .. }, Type::I8 { .. }) => match i8::try_from(*value) {
            Ok(v) => Ok(Value::I8 { span, value: v }),
            Err(_) => failed_to_cast(log),
        },
        (Value::InferredInteger { value, .. }, Type::I16 { .. }) => match i16::try_from(*value) {
            Ok(v) => Ok(Value::I16 { span, value: v }),
            Err(_) => failed_to_cast(log),
        },
        (Value::InferredInteger { value, .. }, Type::I32 { .. }) => match i32::try_from(*value) {
            Ok(v) => Ok(Value::I32 { span, value: v }),
            Err(_) => failed_to_cast(log),
        },
        (Value::InferredInteger { value, .. }, Type::I64 { .. }) => match i64::try_from(*value) {
            Ok(v) => Ok(Value::I64 { span, value: v }),
            Err(_) => failed_to_cast(log),
        },
        (Value::InferredInteger { value, .. }, Type::I128 { .. }) => match i128::try_from(*value) {
            Ok(v) => Ok(Value::I128 {
                span,
                value: Box::new(v),
            }),
            Err(_) => failed_to_cast(log),
        },
        (Value::InferredFloat { value: v, .. }, Type::F32 { .. }) => Ok(Value::F32 {
            span,
            value: OrderedFloat::from(*v as f32),
        }),
        (Value::InferredFloat { value: v, .. }, Type::F64 { .. }) => Ok(Value::F64 { span, value: *v }),
        (expr, to) => Ok(Value::Cast {
            span,
            value: expr.into(),
            target_type: to.into(),
        }),
    }
}

fn ast_local_variable(
    local_var: &ast::LocalVariable,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<LocalVariableId, ()> {
    let kind = match local_var.kind {
        ast::LocalVariableKind::Let => LocalKind::Let,
        ast::LocalVariableKind::Var => LocalKind::Var,
    };

    let attributes = BTreeSet::new();
    if let Some(ast_attributes) = &local_var.attributes {
        for _attr in ast_attributes {
            log.report(&HirErr::UnrecognizedLocalVariableAttribute);
        }
    }

    let is_mutable = match local_var.mutability {
        Some(ast::Mutability::Mut) => true,
        Some(ast::Mutability::Const) | None => false,
    };

    let name = ctx.qualify_name(&local_var.name).into();

    let ty = match local_var.ty.to_owned() {
        None => ctx.create_inference_placeholder().into(),
        Some(t) => lower_type(t, ctx, log)?.into(),
    };

    let initializer = match local_var.initializer.to_owned() {
        Some(expr) => lower_expr(expr, ctx, log)?.into(),
        None => {
            log.report(&HirErr::LocalVariableMissingInitializer);
            return Err(());
        }
    };

    let localvar_id: LocalVariableId = LocalVariable {
        span: ByteSpan::default(),
        kind,
        attributes,
        is_mutable,
        name,
        ty,
        initializer,
    }
    .into();

    ctx.tab.add_local_variable(localvar_id.clone());

    Ok(localvar_id)
}

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
                let var_hir = ast_local_variable(&var, ctx, log)?;
                elements.push(BlockElement::Local(var_hir));
            }
        }
    }

    let safety = match block.safety {
        Some(ast::Safety::Unsafe(None)) => BlockSafety::Unsafe,
        Some(ast::Safety::Safe) | None => BlockSafety::Safe,
        Some(ast::Safety::Unsafe(Some(_))) => {
            log.report(&HirErr::UnimplementedFeature("block safety unsafe expression".into()));
            return Err(());
        }
    };

    Ok(Block { span, safety, elements })
}

pub(crate) fn lower_block_value(block: ast::Block, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    let span = block.span;
    let block = lower_block(block, ctx, log)?;
    Ok(Value::Block {
        span,
        block: block.into(),
    })
}

pub(crate) fn lower_closure(_closure: ast::Closure, _ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    log.report(&HirErr::UnimplementedFeature("ast::Expr::Closure".into()));
    Err(())
}

pub(crate) fn lower_expr_path(expr_path: ast::ExprPath, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    let span = expr_path.span;

    // Check for generic type arguments in expression paths - we store them on the
    // function symbol so the monomorphization pass can use them for disambiguation
    let explicit_type_args: Option<Vec<TypeId>> = if expr_path.segments.iter().any(|seg| seg.type_arguments.is_some()) {
        let args: Vec<TypeId> = expr_path
            .segments
            .iter()
            .filter_map(|seg| {
                seg.type_arguments.as_ref().map(|type_args| {
                    type_args
                        .iter()
                        .filter_map(|type_arg| {
                            lower_type(type_arg.value.clone(), ctx, log).ok().map(|t| {
                                let hir_type_arg: TypeId = t.into();
                                hir_type_arg
                            })
                        })
                        .collect::<Vec<_>>()
                })
            })
            .flatten()
            .collect();
        if args.is_empty() { None } else { Some(args) }
    } else {
        None
    };

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
                if let Some(_type_args) = explicit_type_args {
                    // We currently only support inference-based monomorphization,
                }
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
            _ => {
                println!("Unresolved symbol: {}", resolved_path);
                log.report(&HirErr::UnresolvedSymbol);
                Err(())
            }
        },
        None => {
            println!("Unresolved path in expr: {:?}", expr_path.segments);
            log.report(&HirErr::UnresolvedSymbol);
            Err(())
        }
    }
}

pub(crate) fn lower_index_access(
    index_access: ast::IndexAccess,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Value, ()> {
    let span = index_access.span;
    let collection: ValueId = lower_expr(index_access.collection, ctx, log)?.into();
    let index: ValueId = lower_expr(index_access.index, ctx, log)?.into();

    Ok(Value::IndexAccess {
        span,
        collection,
        index,
    })
}

pub(crate) fn lower_field_access(
    field_access: ast::FieldAccess,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Value, ()> {
    let span = field_access.span;
    let object = lower_expr(field_access.object, ctx, log)?.into();
    let field = field_access.field.to_string().into();

    Ok(Value::FieldAccess {
        span,
        expr: object,
        field_name: field,
    })
}

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

pub(crate) fn lower_match(_match_: ast::Match, _ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    log.report(&HirErr::UnimplementedFeature("Match expressions".into()));
    Err(())
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

pub(crate) fn lower_for_each(_for_each: ast::ForEach, _ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    log.report(&HirErr::UnimplementedFeature("ForEach expressions".into()));
    Err(())
}

pub(crate) fn lower_await(_await_: ast::Await, _ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    log.report(&HirErr::UnimplementedFeature("Await expressions".into()));
    Err(())
}

pub(crate) fn lower_function_call(
    function_call: ast::FunctionCall,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Value, ()> {
    let span = function_call.span;
    let callee = lower_expr(function_call.callee, ctx, log)?;

    let mut positional = Vec::with_capacity(function_call.positional.len());
    let mut named = Vec::with_capacity(function_call.named.len());

    for arg in function_call.positional {
        let value = lower_expr(arg, ctx, log)?.into();
        positional.push(value);
    }

    for (name, arg) in function_call.named {
        let name = NString::from(name.to_string());
        let value = lower_expr(arg, ctx, log)?.into();
        named.push((name, value));
    }

    let args = Arguments {
        positional: positional.into(),
        named: named.into(),
    };

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

    let mut positional = Vec::with_capacity(method_call.positional.len());
    let mut named = Vec::with_capacity(method_call.named.len());

    for arg in method_call.positional {
        let value = lower_expr(arg, ctx, log)?.into();
        positional.push(value);
    }

    for (name, arg) in method_call.named {
        let name = NString::from(name.to_string());
        let value = lower_expr(arg, ctx, log)?.into();
        named.push((name, value));
    }

    let args = Arguments {
        positional: positional.into(),
        named: named.into(),
    };

    Ok(Value::MethodCall {
        span,
        object,
        method_name: method,
        args,
    })
}

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
