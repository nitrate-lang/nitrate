use crate::{
    Order, ParseTreeIterMut, RefNodeMut,
    expr::{ExprPath, Object, Safety, Switch, SwitchCase, TypeArgument, UnitLit},
    kind::{
        Await, BStringLit, BinExpr, Block, BlockItem, BooleanLit, Break, Call, CallArgument, Cast,
        Closure, Continue, DoWhileLoop, Expr, ExprParentheses, ExprSyntaxError, FloatLit, ForEach,
        If, IndexAccess, IntegerLit, List, Return, StringLit, TypeInfo, UnaryExpr, WhileLoop,
    },
};

impl ParseTreeIterMut for ExprSyntaxError {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprSyntaxError);
        f(Order::Leave, RefNodeMut::ExprSyntaxError);
    }
}

impl ParseTreeIterMut for ExprParentheses {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprParentheses(self));

        self.inner.depth_first_iter_mut(f);

        f(Order::Leave, RefNodeMut::ExprParentheses(self));
    }
}

impl ParseTreeIterMut for BooleanLit {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprBooleanLit(self));
        f(Order::Leave, RefNodeMut::ExprBooleanLit(self));
    }
}

impl ParseTreeIterMut for IntegerLit {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprIntegerLit(self));
        f(Order::Leave, RefNodeMut::ExprIntegerLit(self));
    }
}

impl ParseTreeIterMut for FloatLit {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprFloatLit(self));
        f(Order::Leave, RefNodeMut::ExprFloatLit(self));
    }
}

impl ParseTreeIterMut for StringLit {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprStringLit(self));
        f(Order::Leave, RefNodeMut::ExprStringLit(self));
    }
}

impl ParseTreeIterMut for BStringLit {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprBStringLit(self));
        f(Order::Leave, RefNodeMut::ExprBStringLit(self));
    }
}

impl ParseTreeIterMut for UnitLit {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprUnitLit);
        f(Order::Leave, RefNodeMut::ExprUnitLit);
    }
}

impl ParseTreeIterMut for TypeInfo {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprTypeInfo(self));

        self.the.depth_first_iter_mut(f);

        f(Order::Leave, RefNodeMut::ExprTypeInfo(self));
    }
}

impl ParseTreeIterMut for List {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprList(self));

        for item in &mut self.elements {
            item.depth_first_iter_mut(f);
        }

        f(Order::Leave, RefNodeMut::ExprList(self));
    }
}

impl ParseTreeIterMut for Object {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprObject(self));

        for (key, value) in &mut self.fields {
            let _ = key;
            value.depth_first_iter_mut(f);
        }

        f(Order::Leave, RefNodeMut::ExprObject(self));
    }
}

impl ParseTreeIterMut for UnaryExpr {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprUnaryExpr(self));

        let _ = self.operator;
        self.operand.depth_first_iter_mut(f);

        f(Order::Leave, RefNodeMut::ExprUnaryExpr(self));
    }
}

impl ParseTreeIterMut for BinExpr {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprBinExpr(self));

        self.left.depth_first_iter_mut(f);
        let _ = self.operator;
        self.right.depth_first_iter_mut(f);

        f(Order::Leave, RefNodeMut::ExprBinExpr(self));
    }
}

impl ParseTreeIterMut for Cast {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprCast(self));

        self.value.depth_first_iter_mut(f);
        self.to.depth_first_iter_mut(f);

        f(Order::Leave, RefNodeMut::ExprCast(self));
    }
}

impl ParseTreeIterMut for BlockItem {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprBlockItem(self));

        match self {
            BlockItem::Variable(v) => v.depth_first_iter_mut(f),
            BlockItem::Expr(e) => e.depth_first_iter_mut(f),
            BlockItem::Stmt(s) => s.depth_first_iter_mut(f),
        }

        f(Order::Leave, RefNodeMut::ExprBlockItem(self));
    }
}

impl ParseTreeIterMut for Block {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprBlock(self));

        if let Some(safety) = &mut self.safety {
            match safety {
                Safety::Safe => {}

                Safety::Unsafe(e) => {
                    e.depth_first_iter_mut(f);
                }
            }
        }

        for item in &mut self.elements {
            item.depth_first_iter_mut(f);
        }

        f(Order::Leave, RefNodeMut::ExprBlock(self));
    }
}

impl ParseTreeIterMut for Closure {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprClosure(self));

        if let Some(attrs) = &mut self.attributes {
            for attr in attrs {
                attr.depth_first_iter_mut(f);
            }
        }

        for param in &mut self.parameters {
            param.depth_first_iter_mut(f);
        }

        if let Some(ret_type) = &mut self.return_type {
            ret_type.depth_first_iter_mut(f);
        }

        self.definition.depth_first_iter_mut(f);

        f(Order::Leave, RefNodeMut::ExprClosure(self));
    }
}

impl ParseTreeIterMut for TypeArgument {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprPathTypeArgument(self));

        let _ = self.name;
        self.value.depth_first_iter_mut(f);

        f(Order::Leave, RefNodeMut::ExprPathTypeArgument(self));
    }
}

impl ParseTreeIterMut for ExprPath {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprPath(self));

        for segment in &mut self.segments {
            let _ = &segment.identifier;

            if let Some(type_args) = &mut segment.type_arguments {
                for type_arg in type_args {
                    type_arg.depth_first_iter_mut(f);
                }
            }
        }

        let _ = self.to;

        f(Order::Leave, RefNodeMut::ExprPath(self));
    }
}

impl ParseTreeIterMut for IndexAccess {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprIndexAccess(self));

        self.collection.depth_first_iter_mut(f);
        self.index.depth_first_iter_mut(f);

        f(Order::Leave, RefNodeMut::ExprIndexAccess(self));
    }
}

impl ParseTreeIterMut for If {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprIf(self));

        self.condition.depth_first_iter_mut(f);

        self.then_branch.depth_first_iter_mut(f);

        if let Some(else_branch) = &mut self.else_branch {
            else_branch.depth_first_iter_mut(f);
        }

        f(Order::Leave, RefNodeMut::ExprIf(self));
    }
}

impl ParseTreeIterMut for WhileLoop {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprWhile(self));

        self.condition.depth_first_iter_mut(f);
        self.body.depth_first_iter_mut(f);

        f(Order::Leave, RefNodeMut::ExprWhile(self));
    }
}

impl ParseTreeIterMut for DoWhileLoop {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprDoWhileLoop(self));

        self.body.depth_first_iter_mut(f);
        self.condition.depth_first_iter_mut(f);

        f(Order::Leave, RefNodeMut::ExprDoWhileLoop(self));
    }
}

impl ParseTreeIterMut for SwitchCase {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprSwitchCase(self));

        self.condition.depth_first_iter_mut(f);
        self.body.depth_first_iter_mut(f);

        f(Order::Leave, RefNodeMut::ExprSwitchCase(self));
    }
}

impl ParseTreeIterMut for Switch {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprSwitch(self));

        self.condition.depth_first_iter_mut(f);

        for case in &mut self.cases {
            case.depth_first_iter_mut(f);
        }

        if let Some(default) = &mut self.default {
            default.depth_first_iter_mut(f);
        }

        f(Order::Leave, RefNodeMut::ExprSwitch(self));
    }
}

impl ParseTreeIterMut for Break {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprBreak(self));
        f(Order::Leave, RefNodeMut::ExprBreak(self));
    }
}

impl ParseTreeIterMut for Continue {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprContinue(self));
        f(Order::Leave, RefNodeMut::ExprContinue(self));
    }
}

impl ParseTreeIterMut for Return {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprReturn(self));

        if let Some(value) = &mut self.value {
            value.depth_first_iter_mut(f);
        }

        f(Order::Leave, RefNodeMut::ExprReturn(self));
    }
}

impl ParseTreeIterMut for ForEach {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprFor(self));

        if let Some(attrs) = &mut self.attributes {
            for attr in attrs {
                attr.depth_first_iter_mut(f);
            }
        }

        self.iterable.depth_first_iter_mut(f);

        for (var, ty) in &mut self.bindings {
            let _ = var;

            if let Some(t) = ty {
                t.depth_first_iter_mut(f);
            }
        }

        self.body.depth_first_iter_mut(f);

        f(Order::Leave, RefNodeMut::ExprFor(self));
    }
}

impl ParseTreeIterMut for Await {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprAwait(self));

        self.future.depth_first_iter_mut(f);

        f(Order::Leave, RefNodeMut::ExprAwait(self));
    }
}

impl ParseTreeIterMut for CallArgument {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprCallArgument(self));

        let _ = self.name;
        self.value.depth_first_iter_mut(f);

        f(Order::Leave, RefNodeMut::ExprCallArgument(self));
    }
}

impl ParseTreeIterMut for Call {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprCall(self));

        self.callee.depth_first_iter_mut(f);

        for arg in &mut self.arguments {
            arg.depth_first_iter_mut(f);
        }

        f(Order::Leave, RefNodeMut::ExprCall(self));
    }
}

impl ParseTreeIterMut for Expr {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        match self {
            Expr::SyntaxError(e) => e.depth_first_iter_mut(f),
            Expr::Parentheses(e) => e.depth_first_iter_mut(f),
            Expr::Boolean(e) => e.depth_first_iter_mut(f),
            Expr::Float(e) => e.depth_first_iter_mut(f),
            Expr::String(e) => e.depth_first_iter_mut(f),
            Expr::BString(e) => e.depth_first_iter_mut(f),
            Expr::Unit(e) => e.depth_first_iter_mut(f),
            Expr::Integer(e) => e.depth_first_iter_mut(f),
            Expr::TypeInfo(e) => e.depth_first_iter_mut(f),
            Expr::List(e) => e.depth_first_iter_mut(f),
            Expr::Object(e) => e.depth_first_iter_mut(f),
            Expr::UnaryExpr(e) => e.depth_first_iter_mut(f),
            Expr::BinExpr(e) => e.depth_first_iter_mut(f),
            Expr::Cast(e) => e.depth_first_iter_mut(f),
            Expr::Block(e) => e.depth_first_iter_mut(f),
            Expr::Closure(e) => e.depth_first_iter_mut(f),
            Expr::Variable(e) => e.depth_first_iter_mut(f),
            Expr::Path(e) => e.depth_first_iter_mut(f),
            Expr::IndexAccess(e) => e.depth_first_iter_mut(f),
            Expr::If(e) => e.depth_first_iter_mut(f),
            Expr::While(e) => e.depth_first_iter_mut(f),
            Expr::DoWhileLoop(e) => e.depth_first_iter_mut(f),
            Expr::Switch(e) => e.depth_first_iter_mut(f),
            Expr::Break(e) => e.depth_first_iter_mut(f),
            Expr::Continue(e) => e.depth_first_iter_mut(f),
            Expr::Return(e) => e.depth_first_iter_mut(f),
            Expr::For(e) => e.depth_first_iter_mut(f),
            Expr::Await(e) => e.depth_first_iter_mut(f),
            Expr::Call(e) => e.depth_first_iter_mut(f),
        }
    }
}
