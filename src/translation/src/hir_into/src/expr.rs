use std::{collections::HashMap, ops::Deref};

use crate::{TryIntoHir, diagnosis::HirErr};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_hir_get_type::get_type;
use nitrate_parsetree::kind::{self as ast, CallArgument, UnaryExprOp};

impl TryIntoHir for ast::ExprSyntaxError {
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Err(())
    }
}

impl TryIntoHir for ast::ExprParentheses {
    type Hir = Value;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        self.inner.try_into_hir(ctx, log)
    }
}

impl TryIntoHir for ast::BooleanLit {
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        match self.value {
            true => Ok(Value::Bool(true)),
            false => Ok(Value::Bool(false)),
        }
    }
}

impl TryIntoHir for ast::IntegerLit {
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Value::InferredInteger(Box::new(self.value)))
    }
}

impl TryIntoHir for ast::FloatLit {
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Value::InferredFloat(self.value.into()))
    }
}

impl TryIntoHir for ast::StringLit {
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Value::StringLit(self.value.to_string().into()))
    }
}

impl TryIntoHir for ast::BStringLit {
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Value::BStringLit(self.value.into()))
    }
}

impl TryIntoHir for ast::TypeInfo {
    type Hir = Value;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let hir_type = self.the.try_into_hir(ctx, log)?;
        Ok(ctx.create_std_meta_type_instance(hir_type))
    }
}

impl TryIntoHir for ast::List {
    type Hir = Value;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let mut elements = Vec::with_capacity(self.elements.len());
        for element in self.elements {
            let hir_element = element.try_into_hir(ctx, log)?;
            elements.push(hir_element);
        }

        Ok(Value::List {
            elements: elements.into(),
        })
    }
}

impl TryIntoHir for ast::Tuple {
    type Hir = Value;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let mut elements = Vec::with_capacity(self.elements.len());
        for element in self.elements {
            let hir_element = element.try_into_hir(ctx, log)?;
            elements.push(hir_element);
        }

        Ok(Value::Tuple {
            elements: elements.into(),
        })
    }
}

impl TryIntoHir for ast::StructInit {
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: lower ast::StructInit to HIR
        log.report(&HirErr::UnimplementedFeature(
            "ast::Expr::StructInit".into(),
        ));
        Err(())
    }
}

impl TryIntoHir for ast::UnaryExpr {
    type Hir = Value;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let expr = self.operand.try_into_hir(ctx, log)?;

        match self.operator {
            UnaryExprOp::Add => Ok(Value::Unary {
                op: UnaryOp::Add,
                expr: expr.into_id(ctx.store()),
            }),

            UnaryExprOp::Sub => Ok(Value::Unary {
                op: UnaryOp::Sub,
                expr: expr.into_id(ctx.store()),
            }),

            UnaryExprOp::LogicNot => Ok(Value::Unary {
                op: UnaryOp::LogicNot,
                expr: expr.into_id(ctx.store()),
            }),

            UnaryExprOp::BitNot => Ok(Value::Unary {
                op: UnaryOp::BitNot,
                expr: expr.into_id(ctx.store()),
            }),

            UnaryExprOp::Deref => Ok(Value::Deref {
                place: expr.into_id(ctx.store()),
            }),

            UnaryExprOp::Borrow => Ok(Value::Borrow {
                mutable: false,
                place: expr.into_id(ctx.store()),
            }),

            UnaryExprOp::Typeof => match get_type(&expr, ctx.store()) {
                Ok(t) => Ok(ctx.create_std_meta_type_instance(t)),
                Err(_) => {
                    log.report(&HirErr::TypeInferenceError);
                    Err(())
                }
            },
        }
    }
}

impl TryIntoHir for ast::BinExpr {
    type Hir = Value;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let left = self.left.try_into_hir(ctx, log)?.into_id(ctx.store());
        let right = self.right.try_into_hir(ctx, log)?.into_id(ctx.store());

        match self.operator {
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
                .into_id(ctx.store()),
            }),

            ast::BinExprOp::SetMinus => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::Sub,
                    right,
                }
                .into_id(ctx.store()),
            }),

            ast::BinExprOp::SetTimes => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::Mul,
                    right,
                }
                .into_id(ctx.store()),
            }),

            ast::BinExprOp::SetSlash => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::Div,
                    right,
                }
                .into_id(ctx.store()),
            }),

            ast::BinExprOp::SetPercent => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::Mod,
                    right,
                }
                .into_id(ctx.store()),
            }),

            ast::BinExprOp::SetBitAnd => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::And,
                    right,
                }
                .into_id(ctx.store()),
            }),

            ast::BinExprOp::SetBitOr => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::Or,
                    right,
                }
                .into_id(ctx.store()),
            }),

            ast::BinExprOp::SetBitXor => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::Xor,
                    right,
                }
                .into_id(ctx.store()),
            }),

            ast::BinExprOp::SetBitShl => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::Shl,
                    right,
                }
                .into_id(ctx.store()),
            }),

            ast::BinExprOp::SetBitShr => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::Shr,
                    right,
                }
                .into_id(ctx.store()),
            }),

            ast::BinExprOp::SetBitRotl => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::Rol,
                    right,
                }
                .into_id(ctx.store()),
            }),

            ast::BinExprOp::SetBitRotr => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::Ror,
                    right,
                }
                .into_id(ctx.store()),
            }),

            ast::BinExprOp::SetLogicAnd => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::And,
                    right,
                }
                .into_id(ctx.store()),
            }),

            ast::BinExprOp::SetLogicOr => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::Or,
                    right,
                }
                .into_id(ctx.store()),
            }),

            ast::BinExprOp::Dot => {
                // TODO: lower field access to HIR
                log.report(&HirErr::UnimplementedFeature(
                    "field access with . operator".into(),
                ));
                Err(())
            }

            ast::BinExprOp::Arrow => {
                // TODO: lower field access to HIR
                log.report(&HirErr::UnimplementedFeature(
                    "method call with -> operator".into(),
                ));
                Err(())
            }

            ast::BinExprOp::Range => {
                // TODO: lower range to HIR
                log.report(&HirErr::UnimplementedFeature(
                    "range with .. operator".into(),
                ));
                Err(())
            }
        }
    }
}

impl TryIntoHir for ast::Cast {
    type Hir = Value;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        fn failed_to_cast(log: &CompilerLog) -> Result<Value, ()> {
            log.report(&HirErr::IntegerCastOutOfRange);
            Err(())
        }

        let expr = self.value.try_into_hir(ctx, log)?;
        let to = self.to.try_into_hir(ctx, log)?;

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

            (Value::InferredFloat(value), Type::F8) => Ok(Value::F8(value as f32)),
            (Value::InferredFloat(value), Type::F16) => Ok(Value::F16(value as f32)),
            (Value::InferredFloat(value), Type::F32) => Ok(Value::F32(value as f32)),
            (Value::InferredFloat(value), Type::F64) => Ok(Value::F64(value as f64)),
            (Value::InferredFloat(value), Type::F128) => Ok(Value::F128(value)),

            (expr, to) => Ok(Value::Cast {
                expr: expr.into_id(ctx.store()),
                to: to.into_id(ctx.store()),
            }),
        }
    }
}

impl TryIntoHir for ast::Block {
    type Hir = Block;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let mut elements = Vec::with_capacity(self.elements.len());
        let mut ends_with_unit = false;

        for element in self.elements {
            match element {
                ast::BlockItem::Expr(e) => {
                    let hir_element = e.try_into_hir(ctx, log)?;
                    elements.push(hir_element);
                    ends_with_unit = false;
                }

                ast::BlockItem::Stmt(s) => {
                    let hir_element = s.try_into_hir(ctx, log)?;
                    elements.push(hir_element);
                    ends_with_unit = true;
                }

                ast::BlockItem::Variable(_v) => {
                    // TODO: Handle variable declarations properly
                    log.report(&HirErr::UnimplementedFeature(
                        "variable declaration in block".into(),
                    ));

                    ends_with_unit = true;
                }
            }
        }

        if ends_with_unit {
            elements.push(Value::Unit);
        }

        let safety = match self.safety {
            Some(ast::Safety::Unsafe(None)) => BlockSafety::Unsafe,
            Some(ast::Safety::Safe) | None => BlockSafety::Safe,

            Some(ast::Safety::Unsafe(Some(_))) => {
                log.report(&HirErr::UnimplementedFeature(
                    "block safety unsafe expression".into(),
                ));
                return Err(());
            }
        };

        Ok(Block { safety, elements })
    }
}

impl TryIntoHir for ast::Closure {
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: lower ast::Closure to HIR
        log.report(&HirErr::UnimplementedFeature("ast::Expr::Closure".into()));
        Err(())
    }
}

impl TryIntoHir for ast::ExprPath {
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: lower ast::ExprPath to HIR
        log.report(&HirErr::UnimplementedFeature("ast::Expr::Path".into()));
        Err(())
    }
}

impl TryIntoHir for ast::IndexAccess {
    type Hir = Value;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let collection = self.collection.try_into_hir(ctx, log)?.into_id(ctx.store());
        let index = self.index.try_into_hir(ctx, log)?.into_id(ctx.store());
        Ok(Value::IndexAccess { collection, index })
    }
}

impl TryIntoHir for ast::If {
    type Hir = Value;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let condition = self.condition.try_into_hir(ctx, log)?.into_id(ctx.store());

        let true_branch = self
            .true_branch
            .try_into_hir(ctx, log)?
            .into_id(ctx.store());

        let false_branch = match self.false_branch {
            Some(ast::ElseIf::If(else_if)) => {
                let else_if_value = else_if.try_into_hir(ctx, log)?;
                let block = Block {
                    safety: BlockSafety::Safe,
                    elements: vec![else_if_value],
                }
                .into_id(ctx.store());
                Some(block)
            }
            Some(ast::ElseIf::Block(block)) => {
                let block = block.try_into_hir(ctx, log)?.into_id(ctx.store());
                Some(block)
            }
            None => None,
        };

        Ok(Value::If {
            condition,
            true_branch,
            false_branch,
        })
    }
}

impl TryIntoHir for ast::WhileLoop {
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let condition = match self.condition {
            Some(cond) => cond.try_into_hir(_ctx, log)?.into_id(_ctx.store()),
            None => Value::Bool(true).into_id(_ctx.store()),
        };

        let body = self.body.try_into_hir(_ctx, log)?.into_id(_ctx.store());

        Ok(Value::While { condition, body })
    }
}

impl TryIntoHir for ast::Match {
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: lower ast::Match to HIR
        log.report(&HirErr::UnimplementedFeature("ast::Expr::Match".into()));
        Err(())
    }
}

impl TryIntoHir for ast::Break {
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Value::Break {
            label: self.label.map(|l| l.to_string().into()),
        })
    }
}

impl TryIntoHir for ast::Continue {
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Value::Continue {
            label: self.label.map(|l| l.to_string().into()),
        })
    }
}

impl TryIntoHir for ast::Return {
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let value = match self.value {
            Some(v) => v.try_into_hir(_ctx, log)?.into_id(_ctx.store()),
            None => Value::Unit.into_id(_ctx.store()),
        };

        Ok(Value::Return { value })
    }
}

impl TryIntoHir for ast::ForEach {
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: lower ast::ForEach to HIR
        log.report(&HirErr::UnimplementedFeature("ast::Expr::erFor".into()));
        Err(())
    }
}

impl TryIntoHir for ast::Await {
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: lower ast::Await to HIR
        log.report(&HirErr::UnimplementedFeature("ast::Expr::Await".into()));
        Err(())
    }
}

impl TryIntoHir for ast::Call {
    type Hir = Value;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        fn find_first_hole(arguments: &Vec<Option<ValueId>>) -> usize {
            if let Some((index, _)) = arguments.iter().enumerate().find(|x| x.1.is_none()) {
                return index;
            }

            // Must be a variadic argument
            arguments.len()
        }

        fn place_argument(
            position: usize,
            value: ValueId,
            log: &CompilerLog,
            call_arguments: &mut Vec<Option<ValueId>>,
        ) -> Result<(), ()> {
            // Push variadic argument
            if position == call_arguments.len() {
                call_arguments.push(Some(value));
                return Ok(());
            }

            if let Some(_) = call_arguments[position] {
                log.report(&HirErr::DuplicateFunctionArguments);
                return Err(());
            }

            call_arguments[position] = Some(value);
            Ok(())
        }

        let callee = self.callee.try_into_hir(ctx, log)?;
        let callee_type = get_type(&callee, ctx.store()).map_err(|_| {
            log.report(&HirErr::TypeInferenceError);
        })?;

        if !callee_type.is_function() {
            log.report(&HirErr::CalleeIsNotFunctionType);
            return Err(());
        }

        let function_type = match callee_type {
            Type::Function { function_type } => ctx[&function_type].to_owned(),
            _ => unreachable!(),
        };

        let mut name_to_pos = HashMap::new();
        for (index, (name, _)) in function_type.parameters.iter().enumerate() {
            name_to_pos.insert(name.to_string(), index);
        }

        let mut next_pos = 0;
        let mut call_arguments: Vec<Option<ValueId>> = Vec::new();
        call_arguments.resize(function_type.parameters.len(), None);

        for CallArgument { name, value } in self.arguments {
            match name {
                Some(name) => {
                    if let Some(position) = name_to_pos.get(name.deref()) {
                        let value = value.try_into_hir(ctx, log)?.into_id(ctx.store());
                        place_argument(position.to_owned(), value, log, &mut call_arguments)?;
                        next_pos = find_first_hole(&call_arguments);
                    } else {
                        log.report(&HirErr::NoSuchParameter(name.to_string()));
                        return Err(());
                    }
                }

                None => {
                    let value = value.try_into_hir(ctx, log)?.into_id(ctx.store());
                    place_argument(next_pos, value, log, &mut call_arguments)?;
                    next_pos = find_first_hole(&call_arguments);
                }
            };
        }

        for arg in call_arguments.iter_mut() {
            if arg.is_none() {
                log.report(&HirErr::MissingFunctionArguments);
                return Err(());
            }
        }

        if !function_type
            .attributes
            .contains(&FunctionAttribute::Variadic)
        {
            if call_arguments.len() > function_type.parameters.len() {
                log.report(&HirErr::TooManyFunctionArguments);
                return Err(());
            }
        }

        let call_arguments: Vec<ValueId> = call_arguments.into_iter().map(|v| v.unwrap()).collect();

        Ok(Value::Call {
            callee: callee.into_id(ctx.store()),
            arguments: call_arguments.into(),
        })
    }
}

impl TryIntoHir for ast::Expr {
    type Hir = Value;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        match self {
            ast::Expr::SyntaxError(e) => e.try_into_hir(ctx, log),
            ast::Expr::Parentheses(e) => e.try_into_hir(ctx, log),
            ast::Expr::Boolean(e) => e.try_into_hir(ctx, log),
            ast::Expr::Integer(e) => e.try_into_hir(ctx, log),
            ast::Expr::Float(e) => e.try_into_hir(ctx, log),
            ast::Expr::String(e) => e.try_into_hir(ctx, log),
            ast::Expr::BString(e) => e.try_into_hir(ctx, log),
            ast::Expr::TypeInfo(e) => e.try_into_hir(ctx, log),
            ast::Expr::List(e) => e.try_into_hir(ctx, log),
            ast::Expr::Tuple(e) => e.try_into_hir(ctx, log),
            ast::Expr::StructInit(e) => e.try_into_hir(ctx, log),
            ast::Expr::UnaryExpr(e) => e.try_into_hir(ctx, log),
            ast::Expr::BinExpr(e) => e.try_into_hir(ctx, log),
            ast::Expr::Cast(e) => e.try_into_hir(ctx, log),
            ast::Expr::Block(e) => Ok(Value::Block {
                block: e.try_into_hir(ctx, log)?.into_id(ctx.store()),
            }),
            ast::Expr::Closure(e) => e.try_into_hir(ctx, log),
            ast::Expr::Path(e) => e.try_into_hir(ctx, log),
            ast::Expr::IndexAccess(e) => e.try_into_hir(ctx, log),
            ast::Expr::If(e) => e.try_into_hir(ctx, log),
            ast::Expr::While(e) => e.try_into_hir(ctx, log),
            ast::Expr::Match(e) => e.try_into_hir(ctx, log),
            ast::Expr::Break(e) => e.try_into_hir(ctx, log),
            ast::Expr::Continue(e) => e.try_into_hir(ctx, log),
            ast::Expr::Return(e) => e.try_into_hir(ctx, log),
            ast::Expr::For(e) => e.try_into_hir(ctx, log),
            ast::Expr::Await(e) => e.try_into_hir(ctx, log),
            ast::Expr::Call(e) => e.try_into_hir(ctx, log),
        }
    }
}
