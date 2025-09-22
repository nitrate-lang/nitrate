use crate::{
    Order, ParseTreeIterMut, RefNodeMut,
    expr::{GenericArgument, Object, Switch, UnitLit},
    kind::{
        Await, BStringLit, BinExpr, Block, BlockItem, BooleanLit, Break, Call, CallArgument, Cast,
        Closure, Continue, DoWhileLoop, Expr, ExprParentheses, ExprSyntaxError, FloatLit, ForEach,
        If, IndexAccess, IntegerLit, List, Path, Return, StringLit, TypeInfo, UnaryExpr, WhileLoop,
    },
};

impl ParseTreeIterMut for ExprSyntaxError {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::ExprSyntaxError);
        f(Order::Post, RefNodeMut::ExprSyntaxError);
    }
}

impl ParseTreeIterMut for ExprParentheses {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::ExprParentheses(self));

        self.inner.depth_first_iter_mut(f);

        f(Order::Post, RefNodeMut::ExprParentheses(self));
    }
}

impl ParseTreeIterMut for BooleanLit {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::ExprBooleanLit(self));
        f(Order::Post, RefNodeMut::ExprBooleanLit(self));
    }
}

impl ParseTreeIterMut for IntegerLit {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::ExprIntegerLit(self));
        f(Order::Post, RefNodeMut::ExprIntegerLit(self));
    }
}

impl ParseTreeIterMut for FloatLit {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::ExprFloatLit(self));
        f(Order::Post, RefNodeMut::ExprFloatLit(self));
    }
}

impl ParseTreeIterMut for StringLit {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::ExprStringLit(self));
        f(Order::Post, RefNodeMut::ExprStringLit(self));
    }
}

impl ParseTreeIterMut for BStringLit {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::ExprBStringLit(self));
        f(Order::Post, RefNodeMut::ExprBStringLit(self));
    }
}

impl ParseTreeIterMut for UnitLit {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::ExprUnitLit);
        f(Order::Post, RefNodeMut::ExprUnitLit);
    }
}

impl ParseTreeIterMut for TypeInfo {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for List {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for Object {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for UnaryExpr {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for BinExpr {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for Cast {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for BlockItem {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for Block {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for Closure {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for GenericArgument {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for Path {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for IndexAccess {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for If {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for WhileLoop {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for DoWhileLoop {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for Switch {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for Break {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for Continue {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for Return {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for ForEach {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for Await {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for CallArgument {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
    }
}

impl ParseTreeIterMut for Call {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        // TODO: Traverse
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
