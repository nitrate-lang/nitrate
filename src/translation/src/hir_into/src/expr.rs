use crate::{HirCtx, TryIntoHir, diagnosis::HirErr};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_parsetree::kind as ast;

impl TryIntoHir for ast::ExprSyntaxError {
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: Lower ast::SyntaxError to HIR
        log.report(&HirErr::UnimplementedFeature(
            "ast::Expr::SyntaxError".into(),
        ));
        Err(())
    }
}

impl TryIntoHir for ast::ExprParentheses {
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: Lower ast::Parentheses to HIR
        log.report(&HirErr::UnimplementedFeature(
            "ast::Expr::Parentheses".into(),
        ));
        Err(())
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

    fn try_into_hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Value::InferredInteger(Box::new(self.value)))
    }
}

impl TryIntoHir for ast::FloatLit {
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: Lower ast::Float to HIR
        log.report(&HirErr::UnimplementedFeature("ast::Expr::Float".into()));
        Err(())
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

    fn try_into_hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: Lower ast::Binary to HIR
        log.report(&HirErr::UnimplementedFeature("ast::Expr::BinExpr".into()));
        Err(())
    }
}

impl TryIntoHir for ast::Cast {
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: Lower ast::Cast to HIR
        log.report(&HirErr::UnimplementedFeature("ast::Expr::Cast".into()));
        Err(())
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

    fn try_into_hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: Lower ast::IndexAccess to HIR
        log.report(&HirErr::UnimplementedFeature(
            "ast::Expr::IndexAccess".into(),
        ));
        Err(())
    }
}

impl TryIntoHir for ast::If {
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: Lower ast::If to HIR
        log.report(&HirErr::UnimplementedFeature("ast::Expr::owIfer".into()));
        Err(())
    }
}

impl TryIntoHir for ast::WhileLoop {
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: Lower ast::While to HIR
        log.report(&HirErr::UnimplementedFeature("ast::Expr::While".into()));
        Err(())
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

    fn try_into_hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: Lower ast::Break to HIR
        log.report(&HirErr::UnimplementedFeature("ast::Expr::Break".into()));
        Err(())
    }
}

impl TryIntoHir for ast::Continue {
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: Lower ast::Continue to HIR
        log.report(&HirErr::UnimplementedFeature("ast::Expr::Continue".into()));
        Err(())
    }
}

impl TryIntoHir for ast::Return {
    type Hir = Value;

    fn try_into_hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: Lower ast::Return to HIR
        log.report(&HirErr::UnimplementedFeature("ast::Expr::Return".into()));
        Err(())
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
