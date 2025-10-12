use crate::{HirCtx, TryIntoHir, diagnosis::HirErr};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_parsetree::kind as ast;

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

    fn try_into_hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: Lower ast::String to HIR
        log.report(&HirErr::UnimplementedFeature("ast::Expr::String".into()));
        Err(())
    }
}

impl TryIntoHir for ast::BStringLit {
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: Lower ast::BString to HIR
        log.report(&HirErr::UnimplementedFeature("ast::Expr::BString".into()));
        Err(())
    }
}

impl TryIntoHir for ast::TypeInfo {
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: Lower ast::TypeInfo to HIR
        log.report(&HirErr::UnimplementedFeature("ast::Expr::TypeInfo".into()));
        Err(())
    }
}

impl TryIntoHir for ast::List {
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: Lower ast::List to HIR
        log.report(&HirErr::UnimplementedFeature("ast::Expr::List".into()));
        Err(())
    }
}

impl TryIntoHir for ast::Tuple {
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: Lower ast::Tuple to HIR
        log.report(&HirErr::UnimplementedFeature("ast::Expr::Tuple".into()));
        Err(())
    }
}

impl TryIntoHir for ast::StructInit {
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: Lower ast::StructInit to HIR
        log.report(&HirErr::UnimplementedFeature(
            "ast::Expr::StructInit".into(),
        ));
        Err(())
    }
}

impl TryIntoHir for ast::UnaryExpr {
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: Lower ast::Unary to HIR
        log.report(&HirErr::UnimplementedFeature("ast::Expr::UnaryExpr".into()));
        Err(())
    }
}

impl TryIntoHir for ast::BinExpr {
    type Hir = Value;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let left = self.left.try_into_hir(ctx, log)?.into_id(ctx.store());
        let right = self.right.try_into_hir(ctx, log)?.into_id(ctx.store());

        match self.operator {
            ast::BinExprOp::Add => Ok(Value::Binary {
                op: BinaryOp::Add,
                left,
                right,
            }),

            ast::BinExprOp::Sub => Ok(Value::Binary {
                op: BinaryOp::Sub,
                left,
                right,
            }),

            ast::BinExprOp::Mul => Ok(Value::Binary {
                op: BinaryOp::Mul,
                left,
                right,
            }),

            ast::BinExprOp::Div => Ok(Value::Binary {
                op: BinaryOp::Div,
                left,
                right,
            }),

            ast::BinExprOp::Mod => Ok(Value::Binary {
                op: BinaryOp::Mod,
                left,
                right,
            }),

            ast::BinExprOp::BitAnd => Ok(Value::Binary {
                op: BinaryOp::And,
                left,
                right,
            }),

            ast::BinExprOp::BitOr => Ok(Value::Binary {
                op: BinaryOp::Or,
                left,
                right,
            }),

            ast::BinExprOp::BitXor => Ok(Value::Binary {
                op: BinaryOp::Xor,
                left,
                right,
            }),

            ast::BinExprOp::BitShl => Ok(Value::Binary {
                op: BinaryOp::Shl,
                left,
                right,
            }),

            ast::BinExprOp::BitShr => Ok(Value::Binary {
                op: BinaryOp::Shr,
                left,
                right,
            }),

            ast::BinExprOp::BitRol => Ok(Value::Binary {
                op: BinaryOp::Rol,
                left,
                right,
            }),

            ast::BinExprOp::BitRor => Ok(Value::Binary {
                op: BinaryOp::Ror,
                left,
                right,
            }),

            ast::BinExprOp::LogicAnd => {
                // TODO: Short-circuiting logic ops
                log.report(&HirErr::UnimplementedFeature(
                    "short-circuiting logic ops".into(),
                ));
                Err(())
            }

            ast::BinExprOp::LogicOr => {
                // TODO: Short-circuiting logic ops
                log.report(&HirErr::UnimplementedFeature(
                    "short-circuiting logic ops".into(),
                ));
                Err(())
            }

            ast::BinExprOp::LogicXor => {
                // TODO: Short-circuiting logic ops
                log.report(&HirErr::UnimplementedFeature(
                    "short-circuiting logic ops".into(),
                ));
                Err(())
            }

            ast::BinExprOp::LogicLt => Ok(Value::Binary {
                op: BinaryOp::Lt,
                left,
                right,
            }),

            ast::BinExprOp::LogicGt => Ok(Value::Binary {
                op: BinaryOp::Gt,
                left,
                right,
            }),

            ast::BinExprOp::LogicLe => Ok(Value::Binary {
                op: BinaryOp::Lte,
                left,
                right,
            }),

            ast::BinExprOp::LogicGe => Ok(Value::Binary {
                op: BinaryOp::Gte,
                left,
                right,
            }),

            ast::BinExprOp::LogicEq => Ok(Value::Binary {
                op: BinaryOp::Eq,
                left,
                right,
            }),

            ast::BinExprOp::LogicNe => Ok(Value::Binary {
                op: BinaryOp::Ne,
                left,
                right,
            }),

            ast::BinExprOp::Set
            | ast::BinExprOp::SetPlus
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
            | ast::BinExprOp::SetLogicOr
            | ast::BinExprOp::SetLogicXor => {
                // TODO: Lower ast::Binary to HIR
                log.report(&HirErr::UnimplementedFeature(
                    "ast::Expr::BinExpr with assignment op".into(),
                ));
                Err(())
            }

            ast::BinExprOp::Dot => {
                // TODO: Lower field access to HIR
                log.report(&HirErr::UnimplementedFeature(
                    "field access with . operator".into(),
                ));
                Err(())
            }

            ast::BinExprOp::Arrow => {
                // TODO: Lower field access to HIR
                log.report(&HirErr::UnimplementedFeature(
                    "method call with -> operator".into(),
                ));
                Err(())
            }

            ast::BinExprOp::Range => {
                // TODO: Lower range to HIR
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
        let expr = self.value.try_into_hir(ctx, log)?.into_id(ctx.store());
        let to = self.to.try_into_hir(ctx, log)?.into_id(ctx.store());
        Ok(Value::Cast { expr, to })
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
        // TODO: Lower ast::Closure to HIR
        log.report(&HirErr::UnimplementedFeature("ast::Expr::Closure".into()));
        Err(())
    }
}

impl TryIntoHir for ast::ExprPath {
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: Lower ast::ExprPath to HIR
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
        // TODO: Lower ast::Match to HIR
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
        // TODO: Lower ast::ForEach to HIR
        log.report(&HirErr::UnimplementedFeature("ast::Expr::erFor".into()));
        Err(())
    }
}

impl TryIntoHir for ast::Await {
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: Lower ast::Await to HIR
        log.report(&HirErr::UnimplementedFeature("ast::Expr::Await".into()));
        Err(())
    }
}

impl TryIntoHir for ast::Call {
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: Lower ast::Call to HIR
        log.report(&HirErr::UnimplementedFeature("ast::Expr::Call".into()));
        Err(())
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
