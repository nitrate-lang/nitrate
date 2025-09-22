use crate::{
    Order, ParseTreeIter, RefNode,
    expr::{Object, PathTypeArgument, Switch, SwitchCase, UnitLit},
    kind::{
        Await, BStringLit, BinExpr, Block, BlockItem, BooleanLit, Break, Call, CallArgument, Cast,
        Closure, Continue, DoWhileLoop, Expr, ExprParentheses, ExprSyntaxError, FloatLit, ForEach,
        If, IndexAccess, IntegerLit, List, Path, Return, StringLit, TypeInfo, UnaryExpr, WhileLoop,
    },
};

impl ParseTreeIter for ExprSyntaxError {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ExprSyntaxError);
        f(Order::Post, RefNode::ExprSyntaxError);
    }
}

impl ParseTreeIter for ExprParentheses {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ExprParentheses(self));

        self.inner.depth_first_iter(f);

        f(Order::Post, RefNode::ExprParentheses(self));
    }
}

impl ParseTreeIter for BooleanLit {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ExprBooleanLit(self));
        f(Order::Post, RefNode::ExprBooleanLit(self));
    }
}

impl ParseTreeIter for IntegerLit {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ExprIntegerLit(self));
        f(Order::Post, RefNode::ExprIntegerLit(self));
    }
}

impl ParseTreeIter for FloatLit {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ExprFloatLit(self));
        f(Order::Post, RefNode::ExprFloatLit(self));
    }
}

impl ParseTreeIter for StringLit {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ExprStringLit(self));
        f(Order::Post, RefNode::ExprStringLit(self));
    }
}

impl ParseTreeIter for BStringLit {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ExprBStringLit(self));
        f(Order::Post, RefNode::ExprBStringLit(self));
    }
}

impl ParseTreeIter for UnitLit {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ExprUnitLit);
        f(Order::Post, RefNode::ExprUnitLit);
    }
}

impl ParseTreeIter for TypeInfo {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ExprTypeInfo(self));

        self.the.depth_first_iter(f);

        f(Order::Post, RefNode::ExprTypeInfo(self));
    }
}

impl ParseTreeIter for List {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ExprList(self));

        for item in &self.elements {
            item.depth_first_iter(f);
        }

        f(Order::Post, RefNode::ExprList(self));
    }
}

impl ParseTreeIter for Object {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ExprObject(self));

        for (key, value) in &self.fields {
            let _ = key;
            value.depth_first_iter(f);
        }

        f(Order::Post, RefNode::ExprObject(self));
    }
}

impl ParseTreeIter for UnaryExpr {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ExprUnaryExpr(self));

        let _ = self.operator;
        self.operand.depth_first_iter(f);

        f(Order::Post, RefNode::ExprUnaryExpr(self));
    }
}

impl ParseTreeIter for BinExpr {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ExprBinExpr(self));

        self.left.depth_first_iter(f);
        let _ = self.operator;
        self.right.depth_first_iter(f);

        f(Order::Post, RefNode::ExprBinExpr(self));
    }
}

impl ParseTreeIter for Cast {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ExprCast(self));

        self.value.depth_first_iter(f);
        self.to.depth_first_iter(f);

        f(Order::Post, RefNode::ExprCast(self));
    }
}

impl ParseTreeIter for BlockItem {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        match self {
            BlockItem::Variable(v) => v.depth_first_iter(f),
            BlockItem::Expr(e) => e.depth_first_iter(f),
        }
    }
}

impl ParseTreeIter for Block {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ExprBlock(self));

        let _ = self.safety;
        let _ = self.ends_with_semi;

        for item in &self.elements {
            item.depth_first_iter(f);
        }

        f(Order::Post, RefNode::ExprBlock(self));
    }
}

impl ParseTreeIter for Closure {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ExprClosure(self));

        if let Some(attrs) = &self.attributes {
            for attr in attrs {
                attr.depth_first_iter(f);
            }
        }

        for param in &self.parameters {
            param.depth_first_iter(f);
        }

        if let Some(ret_type) = &self.return_type {
            ret_type.depth_first_iter(f);
        }

        self.definition.depth_first_iter(f);

        f(Order::Post, RefNode::ExprClosure(self));
    }
}

impl ParseTreeIter for PathTypeArgument {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ExprPathTypeArgument(self));

        let _ = self.name;
        self.value.depth_first_iter(f);

        f(Order::Post, RefNode::ExprPathTypeArgument(self));
    }
}

impl ParseTreeIter for Path {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ExprPath(self));

        for arg in &self.type_arguments {
            arg.depth_first_iter(f);
        }

        f(Order::Post, RefNode::ExprPath(self));
    }
}

impl ParseTreeIter for IndexAccess {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ExprIndexAccess(self));

        self.collection.depth_first_iter(f);
        self.index.depth_first_iter(f);

        f(Order::Post, RefNode::ExprIndexAccess(self));
    }
}

impl ParseTreeIter for If {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ExprIf(self));

        self.condition.depth_first_iter(f);

        self.then_branch.depth_first_iter(f);

        if let Some(else_branch) = &self.else_branch {
            else_branch.depth_first_iter(f);
        }

        f(Order::Post, RefNode::ExprIf(self));
    }
}

impl ParseTreeIter for WhileLoop {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ExprWhile(self));

        self.condition.depth_first_iter(f);
        self.body.depth_first_iter(f);

        f(Order::Post, RefNode::ExprWhile(self));
    }
}

impl ParseTreeIter for DoWhileLoop {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ExprDoWhileLoop(self));

        self.body.depth_first_iter(f);
        self.condition.depth_first_iter(f);

        f(Order::Post, RefNode::ExprDoWhileLoop(self));
    }
}

impl ParseTreeIter for SwitchCase {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ExprSwitchCase(self));

        self.condition.depth_first_iter(f);
        self.body.depth_first_iter(f);

        f(Order::Post, RefNode::ExprSwitchCase(self));
    }
}

impl ParseTreeIter for Switch {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ExprSwitch(self));

        self.condition.depth_first_iter(f);

        for case in &self.cases {
            case.depth_first_iter(f);
        }

        if let Some(default) = &self.default {
            default.depth_first_iter(f);
        }

        f(Order::Post, RefNode::ExprSwitch(self));
    }
}

impl ParseTreeIter for Break {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ExprBreak(self));
        f(Order::Post, RefNode::ExprBreak(self));
    }
}

impl ParseTreeIter for Continue {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ExprContinue(self));
        f(Order::Post, RefNode::ExprContinue(self));
    }
}

impl ParseTreeIter for Return {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ExprReturn(self));

        if let Some(value) = &self.value {
            value.depth_first_iter(f);
        }

        f(Order::Post, RefNode::ExprReturn(self));
    }
}

impl ParseTreeIter for ForEach {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ExprFor(self));

        if let Some(attrs) = &self.attributes {
            for attr in attrs {
                attr.depth_first_iter(f);
            }
        }

        self.iterable.depth_first_iter(f);

        for (var, ty) in &self.bindings {
            let _ = var;

            if let Some(t) = ty {
                t.depth_first_iter(f);
            }
        }

        self.body.depth_first_iter(f);

        f(Order::Post, RefNode::ExprFor(self));
    }
}

impl ParseTreeIter for Await {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ExprAwait(self));

        self.future.depth_first_iter(f);

        f(Order::Post, RefNode::ExprAwait(self));
    }
}

impl ParseTreeIter for CallArgument {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ExprCallArgument(self));

        let _ = self.name;
        self.value.depth_first_iter(f);

        f(Order::Post, RefNode::ExprCallArgument(self));
    }
}

impl ParseTreeIter for Call {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Pre, RefNode::ExprCall(self));

        self.callee.depth_first_iter(f);

        for arg in &self.arguments {
            arg.depth_first_iter(f);
        }

        f(Order::Post, RefNode::ExprCall(self));
    }
}

impl ParseTreeIter for Expr {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        match self {
            Expr::SyntaxError(e) => e.depth_first_iter(f),
            Expr::Parentheses(e) => e.depth_first_iter(f),
            Expr::Boolean(e) => e.depth_first_iter(f),
            Expr::Float(e) => e.depth_first_iter(f),
            Expr::String(e) => e.depth_first_iter(f),
            Expr::BString(e) => e.depth_first_iter(f),
            Expr::Unit(e) => e.depth_first_iter(f),
            Expr::Integer(e) => e.depth_first_iter(f),
            Expr::TypeInfo(e) => e.depth_first_iter(f),
            Expr::List(e) => e.depth_first_iter(f),
            Expr::Object(e) => e.depth_first_iter(f),
            Expr::UnaryExpr(e) => e.depth_first_iter(f),
            Expr::BinExpr(e) => e.depth_first_iter(f),
            Expr::Cast(e) => e.depth_first_iter(f),
            Expr::Block(e) => e.depth_first_iter(f),
            Expr::Closure(e) => e.depth_first_iter(f),
            Expr::Variable(e) => e.depth_first_iter(f),
            Expr::Path(e) => e.depth_first_iter(f),
            Expr::IndexAccess(e) => e.depth_first_iter(f),
            Expr::If(e) => e.depth_first_iter(f),
            Expr::While(e) => e.depth_first_iter(f),
            Expr::DoWhileLoop(e) => e.depth_first_iter(f),
            Expr::Switch(e) => e.depth_first_iter(f),
            Expr::Break(e) => e.depth_first_iter(f),
            Expr::Continue(e) => e.depth_first_iter(f),
            Expr::Return(e) => e.depth_first_iter(f),
            Expr::For(e) => e.depth_first_iter(f),
            Expr::Await(e) => e.depth_first_iter(f),
            Expr::Call(e) => e.depth_first_iter(f),
        }
    }
}
