use crate::{
    Order, ParseTreeIterMut, RefNodeMut,
    ast::{
        Await, BStringLit, BinExpr, Block, BlockItem, BooleanLit, Break, CallArgument, Cast,
        Closure, Continue, Expr, ExprParentheses, ExprSyntaxError, FloatLit, ForEach, FunctionCall,
        If, IndexAccess, IntegerLit, List, Return, StringLit, TypeInfo, UnaryExpr, WhileLoop,
    },
    expr::{
        AttributeList, ElseIf, ExprPath, Match, MatchCase, MethodCall, Safety, StructInit, Tuple,
        TypeArgument,
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

impl ParseTreeIterMut for Tuple {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprTuple(self));

        for item in &mut self.elements {
            item.depth_first_iter_mut(f);
        }

        f(Order::Leave, RefNodeMut::ExprTuple(self));
    }
}

impl ParseTreeIterMut for StructInit {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprStructInit(self));

        self.type_name.depth_first_iter_mut(f);

        for (key, value) in &mut self.fields {
            let _ = key;
            value.depth_first_iter_mut(f);
        }

        f(Order::Leave, RefNodeMut::ExprStructInit(self));
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
                Safety::Safe | Safety::Unsafe(None) => {}

                Safety::Unsafe(Some(e)) => {
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

impl ParseTreeIterMut for AttributeList {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprAttributeList(self));

        for element in self.iter_mut() {
            element.depth_first_iter_mut(f);
        }

        f(Order::Leave, RefNodeMut::ExprAttributeList(self));
    }
}

impl ParseTreeIterMut for Closure {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprClosure(self));

        if let Some(attributes) = &mut self.attributes {
            attributes.depth_first_iter_mut(f);
        }

        if let Some(parameters) = &mut self.parameters {
            for param in parameters {
                param.depth_first_iter_mut(f);
            }
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
            let _ = &segment.name;

            if let Some(type_args) = &mut segment.type_arguments {
                for type_arg in type_args {
                    type_arg.depth_first_iter_mut(f);
                }
            }
        }

        let _ = self.resolved_path;

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

        self.true_branch.depth_first_iter_mut(f);

        if let Some(else_branch) = &mut self.false_branch {
            match else_branch {
                ElseIf::If(else_if) => else_if.depth_first_iter_mut(f),
                ElseIf::Block(else_block) => else_block.depth_first_iter_mut(f),
            }
        }

        f(Order::Leave, RefNodeMut::ExprIf(self));
    }
}

impl ParseTreeIterMut for WhileLoop {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprWhile(self));

        if let Some(condition) = &mut self.condition {
            condition.depth_first_iter_mut(f);
        }

        self.body.depth_first_iter_mut(f);

        f(Order::Leave, RefNodeMut::ExprWhile(self));
    }
}

impl ParseTreeIterMut for MatchCase {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprMatchCase(self));

        self.condition.depth_first_iter_mut(f);
        self.body.depth_first_iter_mut(f);

        f(Order::Leave, RefNodeMut::ExprMatchCase(self));
    }
}

impl ParseTreeIterMut for Match {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprMatch(self));

        self.condition.depth_first_iter_mut(f);

        for case in &mut self.cases {
            case.depth_first_iter_mut(f);
        }

        if let Some(default) = &mut self.default_case {
            default.depth_first_iter_mut(f);
        }

        f(Order::Leave, RefNodeMut::ExprMatch(self));
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

        let _ = &self.bindings;

        if let Some(attributes) = &mut self.attributes {
            attributes.depth_first_iter_mut(f);
        }

        self.iterable.depth_first_iter_mut(f);

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

impl ParseTreeIterMut for FunctionCall {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprFunctionCall(self));

        self.callee.depth_first_iter_mut(f);

        for arg in &mut self.arguments {
            arg.depth_first_iter_mut(f);
        }

        f(Order::Leave, RefNodeMut::ExprFunctionCall(self));
    }
}

impl ParseTreeIterMut for MethodCall {
    fn depth_first_iter_mut(&mut self, f: &mut dyn FnMut(Order, RefNodeMut)) {
        f(Order::Enter, RefNodeMut::ExprMethodCall(self));

        self.object.depth_first_iter_mut(f);

        let _ = &self.method_name;

        for arg in &mut self.arguments {
            arg.depth_first_iter_mut(f);
        }

        f(Order::Leave, RefNodeMut::ExprMethodCall(self));
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
            Expr::Integer(e) => e.depth_first_iter_mut(f),
            Expr::TypeInfo(e) => e.depth_first_iter_mut(f),
            Expr::List(e) => e.depth_first_iter_mut(f),
            Expr::Tuple(e) => e.depth_first_iter_mut(f),
            Expr::StructInit(e) => e.depth_first_iter_mut(f),
            Expr::UnaryExpr(e) => e.depth_first_iter_mut(f),
            Expr::BinExpr(e) => e.depth_first_iter_mut(f),
            Expr::Cast(e) => e.depth_first_iter_mut(f),
            Expr::Block(e) => e.depth_first_iter_mut(f),
            Expr::Closure(e) => e.depth_first_iter_mut(f),
            Expr::Path(e) => e.depth_first_iter_mut(f),
            Expr::IndexAccess(e) => e.depth_first_iter_mut(f),
            Expr::If(e) => e.depth_first_iter_mut(f),
            Expr::While(e) => e.depth_first_iter_mut(f),
            Expr::Match(e) => e.depth_first_iter_mut(f),
            Expr::Break(e) => e.depth_first_iter_mut(f),
            Expr::Continue(e) => e.depth_first_iter_mut(f),
            Expr::Return(e) => e.depth_first_iter_mut(f),
            Expr::For(e) => e.depth_first_iter_mut(f),
            Expr::Await(e) => e.depth_first_iter_mut(f),
            Expr::FunctionCall(e) => e.depth_first_iter_mut(f),
            Expr::MethodCall(e) => e.depth_first_iter_mut(f),
        }
    }
}
