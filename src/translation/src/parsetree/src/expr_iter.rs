use crate::{
    Order, ParseTreeIterMut, RefNodeMut,
    expr::{Object, PathTypeArgument, Switch, UnitLit},
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
        f(Order::Pre, RefNodeMut::ExprTypeInfo(self));

        self.the.depth_first_iter_mut(f);

        f(Order::Post, RefNodeMut::ExprTypeInfo(self));
    }
}

impl ParseTreeIterMut for List {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::ExprList(self));

        for item in &mut self.elements {
            item.depth_first_iter_mut(f);
        }

        f(Order::Post, RefNodeMut::ExprList(self));
    }
}

impl ParseTreeIterMut for Object {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::ExprObject(self));

        for (key, value) in &mut self.fields {
            let _ = key;
            value.depth_first_iter_mut(f);
        }

        f(Order::Post, RefNodeMut::ExprObject(self));
    }
}

impl ParseTreeIterMut for UnaryExpr {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::ExprUnaryExpr(self));

        let _ = self.operator;
        self.operand.depth_first_iter_mut(f);

        f(Order::Post, RefNodeMut::ExprUnaryExpr(self));
    }
}

impl ParseTreeIterMut for BinExpr {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::ExprBinExpr(self));

        self.left.depth_first_iter_mut(f);
        let _ = self.operator;
        self.right.depth_first_iter_mut(f);

        f(Order::Post, RefNodeMut::ExprBinExpr(self));
    }
}

impl ParseTreeIterMut for Cast {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::ExprCast(self));

        self.value.depth_first_iter_mut(f);
        self.to.depth_first_iter_mut(f);

        f(Order::Post, RefNodeMut::ExprCast(self));
    }
}

impl ParseTreeIterMut for BlockItem {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        match self {
            BlockItem::Variable(v) => v.depth_first_iter_mut(f),
            BlockItem::Expr(e) => e.depth_first_iter_mut(f),
        }
    }
}

impl ParseTreeIterMut for Block {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::ExprBlock(self));

        let _ = self.safety;
        let _ = self.ends_with_semi;

        for item in &mut self.elements {
            item.depth_first_iter_mut(f);
        }

        f(Order::Post, RefNodeMut::ExprBlock(self));
    }
}

impl ParseTreeIterMut for Closure {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::ExprClosure(self));

        if let Some(attrs) = &mut self.attributes {
            for attr in attrs {
                attr.depth_first_iter_mut(f);
            }
        }

        for param in &mut self.parameters {
            param.depth_first_iter_mut(f);
        }

        self.return_type.as_mut().map(|t| t.depth_first_iter_mut(f));
        self.definition.depth_first_iter_mut(f);

        f(Order::Post, RefNodeMut::ExprClosure(self));
    }
}

impl ParseTreeIterMut for PathTypeArgument {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::ExprPathTypeArgument(self));

        let _ = self.name;
        self.value.depth_first_iter_mut(f);

        f(Order::Post, RefNodeMut::ExprPathTypeArgument(self));
    }
}

impl ParseTreeIterMut for Path {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::ExprPath(self));

        for arg in &mut self.type_arguments {
            arg.depth_first_iter_mut(f);
        }

        f(Order::Post, RefNodeMut::ExprPath(self));
    }
}

impl ParseTreeIterMut for IndexAccess {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Pre, RefNodeMut::ExprIndexAccess(self));

        self.collection.depth_first_iter_mut(f);
        self.index.depth_first_iter_mut(f);

        f(Order::Post, RefNodeMut::ExprIndexAccess(self));
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
