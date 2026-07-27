use crate::diagnosis::HirErr;
use crate::{context::Ast2HirCtx, ty::lower_type};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_nstring::NString;
use nitrate_tree::ast::{self as ast, SymbolKind, UnaryExprOp};
use ordered_float::OrderedFloat;
use std::collections::BTreeSet;

pub(crate) fn lower_boolean_literal(boolean_lit: ast::BooleanLit) -> Result<Value, ()> {
    match boolean_lit.value {
        true => Ok(Value::Bool(true)),
        false => Ok(Value::Bool(false)),
    }
}

pub(crate) fn lower_integer_literal(integer_lit: ast::IntegerLit) -> Result<Value, ()> {
    Ok(Value::InferredInteger(integer_lit.value.into()))
}

pub(crate) fn lower_float_literal(float_lit: ast::FloatLit) -> Result<Value, ()> {
    Ok(Value::InferredFloat((*float_lit.value).into()))
}

pub(crate) fn lower_string_literal(string_lit: ast::StringLit) -> Result<Value, ()> {
    Ok(Value::StringLit(string_lit.value.into()))
}

pub(crate) fn lower_bstring_literal(bstring_lit: ast::BStringLit) -> Result<Value, ()> {
    Ok(Value::BStringLit(bstring_lit.value.into()))
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
    let mut elements = Vec::with_capacity(list.elements.len());

    for element in list.elements {
        let hir_element = lower_expr(element, ctx, log)?;
        elements.push(hir_element.into());
    }

    Ok(Value::List {
        elements: elements.into(),
    })
}

pub(crate) fn lower_tuple(tuple: ast::Tuple, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    let mut elements = Vec::with_capacity(tuple.elements.len());

    for element in tuple.elements {
        let hir_element = lower_expr(element, ctx, log)?;
        elements.push(hir_element.into());
    }

    Ok(Value::Tuple {
        elements: elements.into(),
    })
}

pub(crate) fn lower_struct_init(
    struct_init: ast::StructInit,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Value, ()> {
    // Process any generic type arguments in the struct path but don't error
    // The type arguments will be inferred from field types during monomorphization
    if struct_init.path.segments.iter().any(|seg| seg.type_arguments.is_some()) {
        // Just acknowledge them and continue - inference will handle it
    }

    let mut fields = Vec::with_capacity(struct_init.fields.len());

    for (field_name, value) in struct_init.fields {
        let field_value = lower_expr(value, ctx, log)?;
        fields.push((field_name, field_value.into()));
    }

    match struct_init.path.resolved_path {
        Some(resolved_path) => {
            return Ok(Value::StructObject {
                struct_def: ctx.tab.get_struct_or_insert_placeholder(&resolved_path),
                fields: fields.into(),
            });
        }

        None => {
            log.report(&HirErr::UnresolvedTypePath);
            Err(())
        }
    }
}

pub(crate) fn lower_unary(unary: ast::UnaryExpr, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    let operand = lower_expr(unary.operand, ctx, log)?;

    match unary.operator {
        UnaryExprOp::Add => Ok(Value::Unary {
            op: UnaryOp::Add,
            operand: operand.into(),
        }),

        UnaryExprOp::Sub => Ok(Value::Unary {
            op: UnaryOp::Sub,
            operand: operand.into(),
        }),

        UnaryExprOp::Not => Ok(Value::Unary {
            op: UnaryOp::Not,
            operand: operand.into(),
        }),

        UnaryExprOp::Deref => Ok(Value::Deref { place: operand.into() }),

        UnaryExprOp::Borrow => Ok(Value::Borrow {
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
    let left = lower_expr(binary.left, ctx, log)?.into();
    let right = lower_expr(binary.right, ctx, log)?.into();

    match binary.operator {
        ast::BinExprOp::Add => Ok(Value::Binary {
            left,
            op: BinaryOp::Add,
            right,
        }),

        ast::BinExprOp::Sub => Ok(Value::Binary {
            left,
            op: BinaryOp::Sub,
            right,
        }),

        ast::BinExprOp::Mul => Ok(Value::Binary {
            left,
            op: BinaryOp::Mul,
            right,
        }),

        ast::BinExprOp::Div => Ok(Value::Binary {
            left,
            op: BinaryOp::Div,
            right,
        }),

        ast::BinExprOp::Mod => Ok(Value::Binary {
            left,
            op: BinaryOp::Mod,
            right,
        }),

        ast::BinExprOp::BitAnd => Ok(Value::Binary {
            left,
            op: BinaryOp::And,
            right,
        }),

        ast::BinExprOp::BitOr => Ok(Value::Binary {
            left,
            op: BinaryOp::Or,
            right,
        }),

        ast::BinExprOp::BitXor => Ok(Value::Binary {
            left,
            op: BinaryOp::Xor,
            right,
        }),

        ast::BinExprOp::BitShl => Ok(Value::Binary {
            left,
            op: BinaryOp::Shl,
            right,
        }),

        ast::BinExprOp::BitShr => Ok(Value::Binary {
            left,
            op: BinaryOp::Shr,
            right,
        }),

        ast::BinExprOp::BitRol => Ok(Value::Binary {
            left,
            op: BinaryOp::Rol,
            right,
        }),

        ast::BinExprOp::BitRor => Ok(Value::Binary {
            left,
            op: BinaryOp::Ror,
            right,
        }),

        ast::BinExprOp::LogicAnd => Ok(Value::Binary {
            left,
            op: BinaryOp::LogicAnd,
            right,
        }),

        ast::BinExprOp::LogicOr => Ok(Value::Binary {
            left,
            op: BinaryOp::LogicOr,
            right,
        }),

        ast::BinExprOp::LogicLt => Ok(Value::Binary {
            left,
            op: BinaryOp::Lt,
            right,
        }),

        ast::BinExprOp::LogicGt => Ok(Value::Binary {
            left,
            op: BinaryOp::Gt,
            right,
        }),

        ast::BinExprOp::LogicLe => Ok(Value::Binary {
            left,
            op: BinaryOp::Lte,
            right,
        }),

        ast::BinExprOp::LogicGe => Ok(Value::Binary {
            left,
            op: BinaryOp::Gte,
            right,
        }),

        ast::BinExprOp::LogicEq => Ok(Value::Binary {
            left,
            op: BinaryOp::Eq,
            right,
        }),

        ast::BinExprOp::LogicNe => Ok(Value::Binary {
            left,
            op: BinaryOp::Ne,
            right,
        }),

        ast::BinExprOp::Set => Ok(Value::Assign {
            place: left,
            value: right,
        }),

        ast::BinExprOp::SetPlus => Ok(Value::Assign {
            place: left.clone(),
            value: Value::Binary {
                left,
                op: BinaryOp::Add,
                right,
            }
            .into(),
        }),

        ast::BinExprOp::SetMinus => Ok(Value::Assign {
            place: left.clone(),
            value: Value::Binary {
                left,
                op: BinaryOp::Sub,
                right,
            }
            .into(),
        }),

        ast::BinExprOp::SetTimes => Ok(Value::Assign {
            place: left.clone(),
            value: Value::Binary {
                left,
                op: BinaryOp::Mul,
                right,
            }
            .into(),
        }),

        ast::BinExprOp::SetSlash => Ok(Value::Assign {
            place: left.clone(),
            value: Value::Binary {
                left,
                op: BinaryOp::Div,
                right,
            }
            .into(),
        }),

        ast::BinExprOp::SetPercent => Ok(Value::Assign {
            place: left.clone(),
            value: Value::Binary {
                left,
                op: BinaryOp::Mod,
                right,
            }
            .into(),
        }),

        ast::BinExprOp::SetBitAnd => Ok(Value::Assign {
            place: left.clone(),
            value: Value::Binary {
                left,
                op: BinaryOp::And,
                right,
            }
            .into(),
        }),

        ast::BinExprOp::SetBitOr => Ok(Value::Assign {
            place: left.clone(),
            value: Value::Binary {
                left,
                op: BinaryOp::Or,
                right,
            }
            .into(),
        }),

        ast::BinExprOp::SetBitXor => Ok(Value::Assign {
            place: left.clone(),
            value: Value::Binary {
                left,
                op: BinaryOp::Xor,
                right,
            }
            .into(),
        }),

        ast::BinExprOp::SetBitShl => Ok(Value::Assign {
            place: left.clone(),
            value: Value::Binary {
                left,
                op: BinaryOp::Shl,
                right,
            }
            .into(),
        }),

        ast::BinExprOp::SetBitShr => Ok(Value::Assign {
            place: left.clone(),
            value: Value::Binary {
                left,
                op: BinaryOp::Shr,
                right,
            }
            .into(),
        }),

        ast::BinExprOp::SetBitRotl => Ok(Value::Assign {
            place: left.clone(),
            value: Value::Binary {
                left,
                op: BinaryOp::Rol,
                right,
            }
            .into(),
        }),

        ast::BinExprOp::SetBitRotr => Ok(Value::Assign {
            place: left.clone(),
            value: Value::Binary {
                left,
                op: BinaryOp::Ror,
                right,
            }
            .into(),
        }),

        ast::BinExprOp::SetLogicAnd => Ok(Value::Assign {
            place: left.clone(),
            value: Value::Binary {
                left,
                op: BinaryOp::And,
                right,
            }
            .into(),
        }),

        ast::BinExprOp::SetLogicOr => Ok(Value::Assign {
            place: left.clone(),
            value: Value::Binary {
                left,
                op: BinaryOp::Or,
                right,
            }
            .into(),
        }),

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

    let expr = lower_expr(cast.value, ctx, log)?;
    let to = lower_type(cast.to, ctx, log)?;

    match (expr, to) {
        (Value::InferredInteger(value), Type::U8) => match u8::try_from(*value) {
            Ok(v) => Ok(Value::U8(v)),
            Err(_) => failed_to_cast(log),
        },

        (Value::InferredInteger(value), Type::U16) => match u16::try_from(*value) {
            Ok(v) => Ok(Value::U16(v)),
            Err(_) => failed_to_cast(log),
        },

        (Value::InferredInteger(value), Type::U32) => match u32::try_from(*value) {
            Ok(v) => Ok(Value::U32(v)),
            Err(_) => failed_to_cast(log),
        },

        (Value::InferredInteger(value), Type::U64) => match u64::try_from(*value) {
            Ok(v) => Ok(Value::U64(v)),
            Err(_) => failed_to_cast(log),
        },

        (Value::InferredInteger(value), Type::U128) => match u128::try_from(*value) {
            Ok(v) => Ok(Value::U128(Box::new(v))),
            Err(_) => failed_to_cast(log),
        },

        (Value::InferredInteger(value), Type::USize) => match ctx.ptr_size {
            PtrSize::U32 => match u32::try_from(*value) {
                Ok(v) => Ok(Value::USize32(v)),
                Err(_) => failed_to_cast(log),
            },

            PtrSize::U64 => match u64::try_from(*value) {
                Ok(v) => Ok(Value::USize64(v)),
                Err(_) => failed_to_cast(log),
            },
        },

        (Value::InferredInteger(value), Type::I8) => match i8::try_from(*value) {
            Ok(v) => Ok(Value::I8(v)),
            Err(_) => failed_to_cast(log),
        },

        (Value::InferredInteger(value), Type::I16) => match i16::try_from(*value) {
            Ok(v) => Ok(Value::I16(v)),
            Err(_) => failed_to_cast(log),
        },

        (Value::InferredInteger(value), Type::I32) => match i32::try_from(*value) {
            Ok(v) => Ok(Value::I32(v)),
            Err(_) => failed_to_cast(log),
        },

        (Value::InferredInteger(value), Type::I64) => match i64::try_from(*value) {
            Ok(v) => Ok(Value::I64(v)),
            Err(_) => failed_to_cast(log),
        },

        (Value::InferredInteger(value), Type::I128) => match i128::try_from(*value) {
            Ok(v) => Ok(Value::I128(Box::new(v))),
            Err(_) => failed_to_cast(log),
        },

        (Value::InferredFloat(v), Type::F32) => Ok(Value::F32(OrderedFloat::from(*v as f32))),
        (Value::InferredFloat(v), Type::F64) => Ok(Value::F64(OrderedFloat::from(v))),

        (expr, to) => Ok(Value::Cast {
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
                    elements.push(BlockElement::Expr(Value::Unit.into()));
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

    Ok(Block { safety, elements })
}

pub(crate) fn lower_block_value(block: ast::Block, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    let block = lower_block(block, ctx, log)?;
    Ok(Value::Block { block: block.into() })
}

pub(crate) fn lower_closure(_closure: ast::Closure, _ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    log.report(&HirErr::UnimplementedFeature("ast::Expr::Closure".into()));
    Err(())
}

pub(crate) fn lower_expr_path(expr_path: ast::ExprPath, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    // Check for generic type arguments in expression paths - we store them on the
    // function symbol so the monomorphization pass can use them for disambiguation
    let explicit_type_args: Option<Vec<TypeId>> = if expr_path.segments.iter().any(|seg| seg.type_arguments.is_some()) {
        // Collect explicit type args from the last segment that has them
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
                enum_def: ctx.tab.get_enum_variant_or_insert_placeholder(&resolved_path),
                variant: resolved_path.split("::").last().unwrap().to_string().into(),
                value: Value::Unit.into(),
            }),

            Some(SymbolKind::Function) => {
                let func_id = ctx.tab.get_function_or_insert_placeholder(&resolved_path);
                // If there are explicit type args, store them for monomorphization
                if let Some(_type_args) = explicit_type_args {
                    // We currently only support inference-based monomorphization,
                    // but we record the explicit args for potential future use
                    // TODO: Use explicit type args for monomorphization
                }
                Ok(Value::FunctionSymbol { id: func_id })
            }

            Some(SymbolKind::GlobalVariable) => Ok(Value::GlobalVariableSymbol {
                id: ctx.tab.get_global_variable_or_insert_placeholder(&resolved_path),
            }),

            Some(SymbolKind::LocalVariable) => Ok(Value::LocalVariableSymbol {
                id: ctx.tab.get_local_variable_or_insert_placeholder(&resolved_path),
            }),

            Some(SymbolKind::Parameter) => Ok(Value::ParameterSymbol {
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
    let collection: ValueId = lower_expr(index_access.collection, ctx, log)?.into();
    let index: ValueId = lower_expr(index_access.index, ctx, log)?.into();

    Ok(Value::IndexAccess { collection, index })
}

pub(crate) fn lower_field_access(
    field_access: ast::FieldAccess,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Value, ()> {
    let object = lower_expr(field_access.object, ctx, log)?.into();
    let field = field_access.field.to_string().into();

    Ok(Value::FieldAccess {
        expr: object,
        field_name: field,
    })
}

pub(crate) fn lower_if(if_: ast::If, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    let condition = lower_expr(if_.condition, ctx, log)?.into();

    let true_branch = lower_block(if_.true_branch, ctx, log)?.into();

    let false_branch = match if_.false_branch {
        Some(ast::ElseIf::If(else_if)) => {
            let else_if_value = lower_if(*else_if, ctx, log)?;
            let block = Block {
                safety: BlockSafety::Safe,
                elements: vec![BlockElement::Expr(else_if_value.into())],
            };

            Some(block.into())
        }
        Some(ast::ElseIf::Block(block)) => Some(lower_block(block, ctx, log)?.into()),
        None => None,
    };

    Ok(Value::If {
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
    let condition = match while_loop.condition {
        Some(cond) => lower_expr(cond, ctx, log)?.into(),
        None => Value::Bool(true).into(),
    };

    let body = lower_block(while_loop.body, ctx, log)?.into();

    Ok(Value::While { condition, body })
}

pub(crate) fn lower_match(_match_: ast::Match, _ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    log.report(&HirErr::UnimplementedFeature("Match expressions".into()));
    Err(())
}

pub(crate) fn lower_break(break_: ast::Break, _ctx: &mut Ast2HirCtx, _log: &CompilerLog) -> Result<Value, ()> {
    Ok(Value::Break {
        label: break_.label.map(|l| l.to_string().into()),
    })
}

pub(crate) fn lower_continue(continue_: ast::Continue, _ctx: &mut Ast2HirCtx, _log: &CompilerLog) -> Result<Value, ()> {
    Ok(Value::Continue {
        label: continue_.label.map(|l| l.to_string().into()),
    })
}

pub(crate) fn lower_return(return_: ast::Return, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    let value = match return_.value {
        Some(v) => lower_expr(v, ctx, log)?.into(),
        None => Value::Unit.into(),
    };

    Ok(Value::Return { value })
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
        callee: callee.into(),
        args,
    })
}

pub(crate) fn lower_method_call(
    method_call: ast::MethodCall,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Value, ()> {
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
