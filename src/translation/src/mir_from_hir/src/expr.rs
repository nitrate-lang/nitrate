use super::LoweringCtx;
use crate::block;
use crate::ty;
use nitrate_hir::prelude as hir;
use nitrate_hir_type::HirGetType;
use nitrate_mir::prelude as mir;
use nitrate_nstring::NString;

// ─────────────────────────────────────────────────────────────
// Expression lowering
// ─────────────────────────────────────────────────────────────

/// Lower an HIR `Value` into MIR. Returns the `Operand` representing the
/// computed value. For tail-position expressions (`is_tail = true`), the
/// result may be passed via block arguments rather than returned directly.
pub fn lower_value(
    ctx: &mut LoweringCtx,
    func: &mut mir::MirFunctionBuilder,
    value_id: &hir::ValueId,
    is_tail: bool,
) -> mir::Operand {
    let value = value_id.borrow();
    match &*value {
        // ── Literals ─────────────────────────────────────
        hir::Value::Unit { .. } => mir::Operand::Constant(mir::MirLiteral::Unit),
        hir::Value::Bool { value: v, .. } => mir::Operand::Constant(mir::MirLiteral::Bool(*v)),
        hir::Value::I8 { value: v, .. } => mir::Operand::Constant(mir::MirLiteral::I8(*v)),
        hir::Value::I16 { value: v, .. } => mir::Operand::Constant(mir::MirLiteral::I16(*v)),
        hir::Value::I32 { value: v, .. } => mir::Operand::Constant(mir::MirLiteral::I32(*v)),
        hir::Value::I64 { value: v, .. } => mir::Operand::Constant(mir::MirLiteral::I64(*v)),
        hir::Value::I128 { value: v, .. } => mir::Operand::Constant(mir::MirLiteral::I128(**v)),
        hir::Value::U8 { value: v, .. } => mir::Operand::Constant(mir::MirLiteral::U8(*v)),
        hir::Value::U16 { value: v, .. } => mir::Operand::Constant(mir::MirLiteral::U16(*v)),
        hir::Value::U32 { value: v, .. } => mir::Operand::Constant(mir::MirLiteral::U32(*v)),
        hir::Value::U64 { value: v, .. } => mir::Operand::Constant(mir::MirLiteral::U64(*v)),
        hir::Value::U128 { value: v, .. } => mir::Operand::Constant(mir::MirLiteral::U128(**v)),
        hir::Value::F32 { value: v, .. } => mir::Operand::Constant(mir::MirLiteral::F32(*v)),
        hir::Value::F64 { value: v, .. } => mir::Operand::Constant(mir::MirLiteral::F64(*v)),
        hir::Value::USize { bits, value: v, .. } => {
            mir::Operand::Constant(mir::MirLiteral::USize { bits: *bits, value: *v })
        }
        hir::Value::StringLit { value: s, .. } => mir::Operand::Constant(mir::MirLiteral::Str(s.clone())),
        hir::Value::BStringLit { value: b, .. } => mir::Operand::Constant(mir::MirLiteral::BStr(b.clone())),

        // ── Symbols ─────────────────────────────────────
        hir::Value::ParameterSymbol { id, .. } => {
            let param = id.borrow();
            if let Some(local_id) = ctx.local_map.get(&param.name).cloned() {
                mir::Operand::Copy(mir::Place::Local(local_id))
            } else {
                mir::Operand::Constant(mir::MirLiteral::Unit)
            }
        }

        hir::Value::LocalVariableSymbol { id, .. } => {
            let local_var = id.borrow();
            if let Some(local_id) = ctx.local_map.get(&local_var.name).cloned() {
                mir::Operand::Copy(mir::Place::Local(local_id))
            } else {
                mir::Operand::Constant(mir::MirLiteral::Unit)
            }
        }

        hir::Value::GlobalVariableSymbol { id, .. } => {
            let gv = id.borrow();
            mir::Operand::Copy(mir::Place::Static(gv.name.clone()))
        }

        hir::Value::FunctionSymbol { id, .. } => {
            let func_data = id.borrow();
            let name = func_data.mangled_name.clone().unwrap_or_else(|| func_data.name.clone());
            mir::Operand::Copy(mir::Place::Static(name))
        }

        // ── Binary operations ───────────────────────────
        hir::Value::Binary { left, op, right, .. } => {
            let lhs = lower_value(ctx, func, left, false);
            let rhs = lower_value(ctx, func, right, false);
            let mir_op = lower_binary_op(op);
            let rv = mir::Rvalue::BinaryOp { op: mir_op, lhs, rhs };
            assign_rvalue_to_temp(ctx, func, &value, rv)
        }

        // ── Unary operations ────────────────────────────
        hir::Value::Unary { op, operand, .. } => {
            let mir_operand = lower_value(ctx, func, operand, false);
            let mir_op = lower_unary_op(op);
            let rv = mir::Rvalue::UnaryOp {
                op: mir_op,
                operand: mir_operand,
            };
            assign_rvalue_to_temp(ctx, func, &value, rv)
        }

        // ── Assign ──────────────────────────────────────
        hir::Value::Assign { place, value: rhs, .. } => {
            let lhs_place = lower_value_as_place(ctx, func, place);
            let rhs_operand = lower_value(ctx, func, rhs, false);
            func.push_assign(lhs_place, mir::Rvalue::Use(rhs_operand));
            mir::Operand::Constant(mir::MirLiteral::Unit)
        }

        // ── Field access ────────────────────────────────
        hir::Value::FieldAccess { expr, field_name, .. } => {
            let base_place = lower_value_as_place(ctx, func, expr);
            let place = mir::Place::Field {
                base: Box::new(base_place),
                field_name: field_name.clone(),
            };
            mir::Operand::Copy(place)
        }

        // ── Index access ────────────────────────────────
        hir::Value::IndexAccess { collection, index, .. } => {
            let collection_place = lower_value_as_place(ctx, func, collection);
            let index_place = lower_value_as_place(ctx, func, index);
            let place = mir::Place::Index {
                base: Box::new(collection_place),
                index: Box::new(index_place),
            };
            mir::Operand::Copy(place)
        }

        // ── Deref ───────────────────────────────────────
        hir::Value::Deref { place: inner, .. } => {
            let base_place = lower_value_as_place(ctx, func, inner);
            let place = mir::Place::Deref(Box::new(base_place));
            mir::Operand::Copy(place)
        }

        // ── Borrow ──────────────────────────────────────
        hir::Value::Borrow {
            exclusive: _,
            mutable,
            place: inner,
            ..
        } => {
            let base_place = lower_value_as_place(ctx, func, inner);
            let borrow_kind = if *mutable {
                mir::BorrowKind::Mutable
            } else {
                mir::BorrowKind::Shared
            };
            let rv = mir::Rvalue::Ref {
                region: borrow_kind,
                place: base_place,
            };
            assign_rvalue_to_temp(ctx, func, &value, rv)
        }

        // ── Cast ────────────────────────────────────────
        hir::Value::Cast {
            value: inner,
            target_type,
            ..
        } => {
            let operand = lower_value(ctx, func, inner, false);
            let mir_target_ty = ty::lower_type(target_type);
            let rv = mir::Rvalue::Cast {
                value: operand,
                target_ty: mir_target_ty,
            };
            assign_rvalue_to_temp(ctx, func, &value, rv)
        }

        // ── Struct construction ─────────────────────────
        hir::Value::StructObject { struct_def, fields, .. } => {
            let sd = struct_def.borrow();
            let operands: thin_vec::ThinVec<mir::Operand> = fields
                .iter()
                .map(|(_, field_value)| lower_value(ctx, func, field_value, false))
                .collect();
            let field_names: thin_vec::ThinVec<NString> = fields.iter().map(|(name, _)| name.clone()).collect();
            let rv = mir::Rvalue::Aggregate(mir::AggregateKind::Struct(sd.name.clone(), field_names), operands);
            assign_rvalue_to_temp(ctx, func, &value, rv)
        }

        // ── Enum variant construction ───────────────────
        hir::Value::EnumVariant {
            enum_def,
            variant,
            value: inner,
            ..
        } => {
            let ed = enum_def.borrow();
            let inner_operand = lower_value(ctx, func, inner, false);
            let operands: thin_vec::ThinVec<mir::Operand> = [inner_operand].into_iter().collect();
            let rv = mir::Rvalue::Aggregate(mir::AggregateKind::Enum(ed.name.clone(), variant.clone()), operands);
            assign_rvalue_to_temp(ctx, func, &value, rv)
        }

        // ── Tuple ───────────────────────────────────────
        hir::Value::Tuple { elements, .. } => {
            let operands: thin_vec::ThinVec<mir::Operand> =
                elements.iter().map(|e| lower_value(ctx, func, e, false)).collect();
            let rv = mir::Rvalue::Aggregate(mir::AggregateKind::Tuple, operands);
            assign_rvalue_to_temp(ctx, func, &value, rv)
        }

        // ── Array literal (List) ────────────────────────
        hir::Value::List { elements, .. } => {
            let operands: thin_vec::ThinVec<mir::Operand> =
                elements.iter().map(|e| lower_value(ctx, func, e, false)).collect();
            let elem_ty = if let Some(first) = elements.first() {
                let val = first.borrow();
                if let Ok(hir_ty) = val.determine_type(ctx.symbol_tab) {
                    ty::lower_type(&hir_ty)
                } else {
                    func.store_type(mir::MirType::U8)
                }
            } else {
                func.store_type(mir::MirType::U8)
            };
            let rv = mir::Rvalue::Aggregate(mir::AggregateKind::Array(elem_ty), operands);
            assign_rvalue_to_temp(ctx, func, &value, rv)
        }

        // ── Call ────────────────────────────────────────
        hir::Value::Call { callee, args, .. } => {
            let callee_op = lower_value(ctx, func, callee, false);
            let mir_args: thin_vec::ThinVec<mir::Operand> = args
                .clone()
                .into_iter()
                .map(|arg| lower_value(ctx, func, &arg, false))
                .collect();

            if is_tail {
                func.call(callee_op, mir_args);
                mir::Operand::Constant(mir::MirLiteral::Unit)
            } else {
                let return_ty = value_result_type(ctx, func, &value);
                let ret_temp = func.new_temp(return_ty.clone(), false);
                let merge_block = func.reserve_block();
                func.call_return(callee_op, mir_args, mir::Place::Local(ret_temp.clone()), merge_block);
                func.switch_to_block(merge_block);
                mir::Operand::Copy(mir::Place::Local(ret_temp))
            }
        }

        // ── Method call ─────────────────────────────────
        hir::Value::MethodCall {
            object, method_name: _, ..
        } => {
            let callee_op = lower_value(ctx, func, object, false);
            let return_ty = value_result_type(ctx, func, &value);

            if is_tail {
                func.call(callee_op, thin_vec::ThinVec::new());
                mir::Operand::Constant(mir::MirLiteral::Unit)
            } else {
                let ret_temp = func.new_temp(return_ty.clone(), false);
                let merge_block = func.reserve_block();
                func.call_return(
                    callee_op,
                    thin_vec::ThinVec::new(),
                    mir::Place::Local(ret_temp.clone()),
                    merge_block,
                );
                func.switch_to_block(merge_block);
                mir::Operand::Copy(mir::Place::Local(ret_temp))
            }
        }

        // ── Return ──────────────────────────────────────
        hir::Value::Return { value: ret_val, .. } => {
            let borrowed = ret_val.borrow();
            let operand = if matches!(&*borrowed, hir::Value::Unit { .. }) {
                None
            } else {
                Some(lower_value(ctx, func, ret_val, false))
            };
            func.ret(operand);
            mir::Operand::Constant(mir::MirLiteral::Unit)
        }

        // ── If / else ───────────────────────────────────
        hir::Value::If {
            condition,
            true_branch,
            false_branch,
            ..
        } => lower_if(ctx, func, condition, true_branch, false_branch.as_ref(), is_tail),

        // ── While loop ──────────────────────────────────
        hir::Value::While { condition, body, .. } => lower_while(ctx, func, condition, body),

        // ── Loop ────────────────────────────────────────
        hir::Value::Loop { body, .. } => lower_loop(ctx, func, body),

        // ── Break ───────────────────────────────────────
        hir::Value::Break { .. } => {
            if let Some((_, break_target)) = ctx.loop_targets().cloned() {
                func.goto(break_target);
            } else {
                func.unreachable();
            }
            mir::Operand::Constant(mir::MirLiteral::Unit)
        }

        // ── Continue ────────────────────────────────────
        hir::Value::Continue { .. } => {
            if let Some((continue_target, _)) = ctx.loop_targets().cloned() {
                func.goto(continue_target);
            } else {
                func.unreachable();
            }
            mir::Operand::Constant(mir::MirLiteral::Unit)
        }

        // ── Block expression ────────────────────────────
        hir::Value::Block { block, .. } => {
            let block_data = block.borrow();
            block::lower_block(ctx, func, &block_data)
        }

        // ── Range ───────────────────────────────────────
        hir::Value::Range { start, end, .. } => {
            let (struct_name, field_names, operands) = match (start.as_ref(), end.as_ref()) {
                (Some(s), Some(e)) => {
                    let s_op = lower_value(ctx, func, s, false);
                    let e_op = lower_value(ctx, func, e, false);
                    (
                        "Range".into(),
                        thin_vec::ThinVec::from(["start".into(), "end".into()].as_slice()),
                        thin_vec::ThinVec::from([s_op, e_op].as_slice()),
                    )
                }
                (Some(s), None) => {
                    let s_op = lower_value(ctx, func, s, false);
                    (
                        "RangeFrom".into(),
                        thin_vec::ThinVec::from(["start".into()].as_slice()),
                        thin_vec::ThinVec::from([s_op].as_slice()),
                    )
                }
                (None, Some(e)) => {
                    let e_op = lower_value(ctx, func, e, false);
                    (
                        "RangeTo".into(),
                        thin_vec::ThinVec::from(["end".into()].as_slice()),
                        thin_vec::ThinVec::from([e_op].as_slice()),
                    )
                }
                (None, None) => ("RangeFull".into(), thin_vec::ThinVec::new(), thin_vec::ThinVec::new()),
            };
            let rv = mir::Rvalue::Aggregate(mir::AggregateKind::Struct(struct_name, field_names), operands);
            assign_rvalue_to_temp(ctx, func, &value, rv)
        }

        // ── Inferred types (should not appear in validated HIR) ──
        hir::Value::InferredInteger { .. } | hir::Value::InferredFloat { .. } => {
            mir::Operand::Constant(mir::MirLiteral::Unit)
        }
    }
}

// ─────────────────────────────────────────────────────────────
// Place lowering
// ─────────────────────────────────────────────────────────────

/// Lower an HIR value to a MIR `Place`.
pub fn lower_value_as_place(
    ctx: &mut LoweringCtx,
    func: &mut mir::MirFunctionBuilder,
    value_id: &hir::ValueId,
) -> mir::Place {
    let value = value_id.borrow();
    match &*value {
        hir::Value::ParameterSymbol { id, .. } => {
            let param = id.borrow();
            if let Some(local_id) = ctx.local_map.get(&param.name).cloned() {
                mir::Place::Local(local_id)
            } else {
                let unit_ty = func.store_type(mir::MirType::Unit);
                let temp = func.new_temp(unit_ty, false);
                mir::Place::Local(temp)
            }
        }
        hir::Value::LocalVariableSymbol { id, .. } => {
            let local_var = id.borrow();
            if let Some(local_id) = ctx.local_map.get(&local_var.name).cloned() {
                mir::Place::Local(local_id)
            } else {
                let unit_ty = func.store_type(mir::MirType::Unit);
                let temp = func.new_temp(unit_ty, false);
                mir::Place::Local(temp)
            }
        }
        hir::Value::GlobalVariableSymbol { id, .. } => {
            let gv = id.borrow();
            mir::Place::Static(gv.name.clone())
        }
        hir::Value::FieldAccess { expr, field_name, .. } => {
            let base = lower_value_as_place(ctx, func, expr);
            mir::Place::Field {
                base: Box::new(base),
                field_name: field_name.clone(),
            }
        }
        hir::Value::IndexAccess { collection, index, .. } => {
            let base = lower_value_as_place(ctx, func, collection);
            let ix = lower_value_as_place(ctx, func, index);
            mir::Place::Index {
                base: Box::new(base),
                index: Box::new(ix),
            }
        }
        hir::Value::StringLit { value: s, .. } => {
            let data: thin_vec::ThinVec<u8> = s.as_bytes().iter().cloned().collect();
            let name = func.register_string_global(data);
            mir::Place::Static(name)
        }
        hir::Value::BStringLit { value: b, .. } => {
            let name = func.register_string_global(b.clone());
            mir::Place::Static(name)
        }
        hir::Value::Deref { place: inner, .. } => {
            let base = lower_value_as_place(ctx, func, inner);
            mir::Place::Deref(Box::new(base))
        }
        _ => {
            let operand = lower_value(ctx, func, value_id, false);
            operand_to_place(func, operand)
        }
    }
}

fn operand_to_place(func: &mut mir::MirFunctionBuilder, operand: mir::Operand) -> mir::Place {
    match operand {
        mir::Operand::Copy(p) | mir::Operand::Move(p) => p,
        mir::Operand::Constant(_) => {
            let unit_ty = func.store_type(mir::MirType::Unit);
            let temp = func.new_temp(unit_ty, false);
            func.push_assign(mir::Place::Local(temp.clone()), mir::Rvalue::Use(operand));
            mir::Place::Local(temp)
        }
    }
}

// ─────────────────────────────────────────────────────────────
// Control flow lowering helpers
// ─────────────────────────────────────────────────────────────

/// Lower an if/else expression using block arguments for the merged value.
///
/// When `is_tail` is false and both branches produce values, a merge block
/// with a block argument is created. Each branch passes its result value
/// as a block argument via goto_with_args.
fn lower_if(
    ctx: &mut LoweringCtx,
    func: &mut mir::MirFunctionBuilder,
    condition: &hir::ValueId,
    true_branch: &hir::BlockId,
    false_branch: Option<&hir::BlockId>,
    is_tail: bool,
) -> mir::Operand {
    let cond_op = lower_value(ctx, func, condition, false);

    // Determine the result type
    let result_ty = if is_tail {
        None
    } else {
        Some(value_result_type(ctx, func, &condition.borrow()))
    };

    // Reserve all blocks before setting the terminator on the current block
    let merge_types: thin_vec::ThinVec<mir::MirTypeId> = result_ty
        .as_ref()
        .map(|t| thin_vec::ThinVec::from([t.clone()].as_slice()))
        .unwrap_or_default();
    let merge = func.reserve_block_with_args(&merge_types);
    let merge_arg_local = merge.arg_locals.first().cloned();

    let then_block = func.reserve_block();
    let else_block = false_branch.as_ref().map(|_| func.reserve_block());

    // Branch from condition (which is in the current block) to then/else
    func.if_br(cond_op, then_block, else_block.unwrap_or(merge.block.clone()));

    // Lower then branch
    func.switch_to_block(then_block);
    let true_block_data = true_branch.borrow();
    let then_result = block::lower_block(ctx, func, &true_block_data);
    if func.current_block.is_some() {
        if is_tail {
            func.goto(merge.block.clone());
        } else {
            func.goto_with_args(merge.block.clone(), thin_vec::ThinVec::from([then_result].as_slice()));
        }
    }

    // Lower else branch if present
    if let Some(false_id) = false_branch {
        let else_block = else_block.unwrap();
        func.switch_to_block(else_block);
        let false_block_data = false_id.borrow();
        let else_result = block::lower_block(ctx, func, &false_block_data);
        if func.current_block.is_some() {
            if is_tail {
                func.goto(merge.block.clone());
            } else {
                func.goto_with_args(merge.block.clone(), thin_vec::ThinVec::from([else_result].as_slice()));
            }
        }
    }

    // Switch to the merge block so the caller can continue adding statements
    func.switch_to_block(merge.block);

    // Return the merge block argument as the result operand
    if let Some(arg_local) = merge_arg_local {
        mir::Operand::Copy(mir::Place::Local(arg_local))
    } else {
        mir::Operand::Constant(mir::MirLiteral::Unit)
    }
}

fn lower_while(
    ctx: &mut LoweringCtx,
    func: &mut mir::MirFunctionBuilder,
    condition: &hir::ValueId,
    body: &hir::BlockId,
) -> mir::Operand {
    // Reserve all blocks first
    let header_block = func.reserve_block();
    let body_block = func.reserve_block();
    let exit_block = func.reserve_block();

    // Set goto on the current block to header
    func.goto(header_block);

    // Header: evaluate condition
    func.switch_to_block(header_block);
    let cond_op = lower_value(ctx, func, condition, false);
    func.if_br(cond_op, body_block, exit_block);

    // Body block
    func.switch_to_block(body_block);
    ctx.push_loop(header_block, exit_block);
    let body_data = body.borrow();
    block::lower_block_elements(ctx, func, &body_data.elements);
    ctx.pop_loop();
    func.goto(header_block);

    // Switch to exit block so caller can continue
    func.switch_to_block(exit_block);
    mir::Operand::Constant(mir::MirLiteral::Unit)
}

fn lower_loop(ctx: &mut LoweringCtx, func: &mut mir::MirFunctionBuilder, body: &hir::BlockId) -> mir::Operand {
    // Reserve blocks first
    let loop_body = func.reserve_block();
    let loop_exit = func.reserve_block();

    // Terminate current block with goto to loop body
    func.goto(loop_body);

    func.switch_to_block(loop_body);
    ctx.push_loop(loop_body, loop_exit);
    let body_data = body.borrow();
    block::lower_block_elements(ctx, func, &body_data.elements);
    ctx.pop_loop();
    func.goto(loop_body);

    // Switch to exit block so caller can continue
    func.switch_to_block(loop_exit);
    mir::Operand::Constant(mir::MirLiteral::Unit)
}

// ─────────────────────────────────────────────────────────────
// Low-level helpers
// ─────────────────────────────────────────────────────────────

fn assign_rvalue_to_temp(
    ctx: &mut LoweringCtx,
    func: &mut mir::MirFunctionBuilder,
    hir_value: &hir::Value,
    rvalue: mir::Rvalue,
) -> mir::Operand {
    let mir_ty = value_result_type(ctx, func, hir_value);
    let temp = func.new_temp(mir_ty, false);
    func.push_assign(mir::Place::Local(temp.clone()), rvalue);
    mir::Operand::Copy(mir::Place::Local(temp))
}

/// Get the MIR type for an HIR value's result.
fn value_result_type(
    ctx: &mut LoweringCtx,
    func: &mut mir::MirFunctionBuilder,
    hir_value: &hir::Value,
) -> mir::MirTypeId {
    if let Ok(hir_ty) = hir_value.determine_type(ctx.symbol_tab) {
        ty::lower_type(&hir_ty)
    } else {
        func.store_type(mir::MirType::Unit)
    }
}

// ─────────────────────────────────────────────────────────────
// Operator lowering
// ─────────────────────────────────────────────────────────────

fn lower_binary_op(op: &hir::BinaryOp) -> mir::MirBinaryOp {
    match op {
        hir::BinaryOp::Add => mir::MirBinaryOp::Add,
        hir::BinaryOp::Sub => mir::MirBinaryOp::Sub,
        hir::BinaryOp::Mul => mir::MirBinaryOp::Mul,
        hir::BinaryOp::Div => mir::MirBinaryOp::Div,
        hir::BinaryOp::Mod => mir::MirBinaryOp::Mod,
        hir::BinaryOp::And => mir::MirBinaryOp::And,
        hir::BinaryOp::Or => mir::MirBinaryOp::Or,
        hir::BinaryOp::Xor => mir::MirBinaryOp::Xor,
        hir::BinaryOp::Shl => mir::MirBinaryOp::Shl,
        hir::BinaryOp::Shr => mir::MirBinaryOp::Shr,
        hir::BinaryOp::Rol => mir::MirBinaryOp::Rol,
        hir::BinaryOp::Ror => mir::MirBinaryOp::Ror,
        hir::BinaryOp::LogicAnd => mir::MirBinaryOp::LogicAnd,
        hir::BinaryOp::LogicOr => mir::MirBinaryOp::LogicOr,
        hir::BinaryOp::Lt => mir::MirBinaryOp::Lt,
        hir::BinaryOp::Gt => mir::MirBinaryOp::Gt,
        hir::BinaryOp::Lte => mir::MirBinaryOp::Lte,
        hir::BinaryOp::Gte => mir::MirBinaryOp::Gte,
        hir::BinaryOp::Eq => mir::MirBinaryOp::Eq,
        hir::BinaryOp::Ne => mir::MirBinaryOp::Ne,
    }
}

fn lower_unary_op(op: &hir::UnaryOp) -> mir::MirUnaryOp {
    match op {
        hir::UnaryOp::Add => mir::MirUnaryOp::Neg,
        hir::UnaryOp::Sub => mir::MirUnaryOp::Neg,
        hir::UnaryOp::Not => mir::MirUnaryOp::Not,
    }
}
