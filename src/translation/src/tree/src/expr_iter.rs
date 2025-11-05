use crate::{
    Order, ParseTreeIter, RefNode,
    ast::{
        Await, BStringLit, BinExpr, Block, BlockItem, BooleanLit, Break, Cast, Closure, Continue,
        Expr, ExprParentheses, ExprSyntaxError, FieldAccess, FloatLit, ForEach, FunctionCall, If,
        IndexAccess, IntegerLit, List, LocalVariable, Return, StringLit, TypeInfo, UnaryExpr,
        WhileLoop,
    },
    expr::{
        AttributeList, ElseIf, ExprPath, Match, MatchCase, MethodCall, Safety, StructInit, Tuple,
        TypeArgument,
    },
};

impl ParseTreeIter for ExprSyntaxError {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ExprSyntaxError);
        f(Order::Leave, RefNode::ExprSyntaxError);
    }
}

impl ParseTreeIter for ExprParentheses {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ExprParentheses(self));

        self.inner.depth_first_iter(f);

        f(Order::Leave, RefNode::ExprParentheses(self));
    }
}

impl ParseTreeIter for BooleanLit {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ExprBooleanLit(self));
        f(Order::Leave, RefNode::ExprBooleanLit(self));
    }
}

impl ParseTreeIter for IntegerLit {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ExprIntegerLit(self));
        f(Order::Leave, RefNode::ExprIntegerLit(self));
    }
}

impl ParseTreeIter for FloatLit {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ExprFloatLit(self));
        f(Order::Leave, RefNode::ExprFloatLit(self));
    }
}

impl ParseTreeIter for StringLit {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ExprStringLit(self));
        f(Order::Leave, RefNode::ExprStringLit(self));
    }
}

impl ParseTreeIter for BStringLit {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ExprBStringLit(self));
        f(Order::Leave, RefNode::ExprBStringLit(self));
    }
}

impl ParseTreeIter for TypeInfo {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ExprTypeInfo(self));

        self.the.depth_first_iter(f);

        f(Order::Leave, RefNode::ExprTypeInfo(self));
    }
}

impl ParseTreeIter for List {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ExprList(self));

        for item in &self.elements {
            item.depth_first_iter(f);
        }

        f(Order::Leave, RefNode::ExprList(self));
    }
}

impl ParseTreeIter for Tuple {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ExprTuple(self));

        for item in &self.elements {
            item.depth_first_iter(f);
        }

        f(Order::Leave, RefNode::ExprTuple(self));
    }
}

impl ParseTreeIter for StructInit {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ExprStructInit(self));

        self.path.depth_first_iter(f);

        for (key, value) in &self.fields {
            let _ = key;
            value.depth_first_iter(f);
        }

        f(Order::Leave, RefNode::ExprStructInit(self));
    }
}

impl ParseTreeIter for UnaryExpr {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ExprUnaryExpr(self));

        let _ = self.operator;
        self.operand.depth_first_iter(f);

        f(Order::Leave, RefNode::ExprUnaryExpr(self));
    }
}

impl ParseTreeIter for BinExpr {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ExprBinExpr(self));

        self.left.depth_first_iter(f);
        let _ = self.operator;
        self.right.depth_first_iter(f);

        f(Order::Leave, RefNode::ExprBinExpr(self));
    }
}

impl ParseTreeIter for Cast {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ExprCast(self));

        self.value.depth_first_iter(f);
        self.to.depth_first_iter(f);

        f(Order::Leave, RefNode::ExprCast(self));
    }
}

impl ParseTreeIter for LocalVariable {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ExprLocalVariable(self));

        let _ = self.kind;
        let _ = self.mutability;
        let _ = self.name;

        if let Some(attributes) = &self.attributes {
            attributes.depth_first_iter(f);
        }

        if let Some(var_type) = &self.ty {
            var_type.depth_first_iter(f);
        }

        if let Some(initializer) = &self.initializer {
            initializer.depth_first_iter(f);
        }

        f(Order::Leave, RefNode::ExprLocalVariable(self));
    }
}

impl ParseTreeIter for BlockItem {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ExprBlockItem(self));

        match self {
            BlockItem::Variable(v) => v.depth_first_iter(f),
            BlockItem::Expr(e) => e.depth_first_iter(f),
            BlockItem::Stmt(s) => s.depth_first_iter(f),
        }

        f(Order::Leave, RefNode::ExprBlockItem(self));
    }
}

impl ParseTreeIter for Block {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ExprBlock(self));

        if let Some(safety) = &self.safety {
            match safety {
                Safety::Safe | Safety::Unsafe(None) => {}

                Safety::Unsafe(Some(e)) => {
                    e.depth_first_iter(f);
                }
            }
        }

        for item in &self.elements {
            item.depth_first_iter(f);
        }

        f(Order::Leave, RefNode::ExprBlock(self));
    }
}

impl ParseTreeIter for AttributeList {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ExprAttributeList(self));

        for element in self.iter() {
            element.depth_first_iter(f);
        }

        f(Order::Leave, RefNode::ExprAttributeList(self));
    }
}

impl ParseTreeIter for Closure {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ExprClosure(self));

        if let Some(attributes) = &self.attributes {
            attributes.depth_first_iter(f);
        }

        if let Some(parameters) = &self.parameters {
            for param in parameters {
                param.depth_first_iter(f);
            }
        }

        if let Some(ret_type) = &self.return_type {
            ret_type.depth_first_iter(f);
        }

        self.definition.depth_first_iter(f);

        f(Order::Leave, RefNode::ExprClosure(self));
    }
}

impl ParseTreeIter for TypeArgument {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ExprPathTypeArgument(self));

        let _ = self.name;
        self.value.depth_first_iter(f);

        f(Order::Leave, RefNode::ExprPathTypeArgument(self));
    }
}

impl ParseTreeIter for ExprPath {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ExprPath(self));

        for segment in &self.segments {
            let _ = &segment.name;

            if let Some(type_args) = &segment.type_arguments {
                for type_arg in type_args {
                    type_arg.depth_first_iter(f);
                }
            }
        }

        let _ = self.resolved_path;

        f(Order::Leave, RefNode::ExprPath(self));
    }
}

impl ParseTreeIter for IndexAccess {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ExprIndexAccess(self));

        self.collection.depth_first_iter(f);
        self.index.depth_first_iter(f);

        f(Order::Leave, RefNode::ExprIndexAccess(self));
    }
}

impl ParseTreeIter for FieldAccess {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ExprFieldAccess(self));

        let _ = &self.field;
        self.object.depth_first_iter(f);

        f(Order::Leave, RefNode::ExprFieldAccess(self));
    }
}

impl ParseTreeIter for If {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ExprIf(self));

        self.condition.depth_first_iter(f);

        self.true_branch.depth_first_iter(f);

        if let Some(else_branch) = &self.false_branch {
            match else_branch {
                ElseIf::If(else_if) => else_if.depth_first_iter(f),
                ElseIf::Block(else_block) => else_block.depth_first_iter(f),
            }
        }

        f(Order::Leave, RefNode::ExprIf(self));
    }
}

impl ParseTreeIter for WhileLoop {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ExprWhile(self));

        if let Some(condition) = &self.condition {
            condition.depth_first_iter(f);
        }

        self.body.depth_first_iter(f);

        f(Order::Leave, RefNode::ExprWhile(self));
    }
}

impl ParseTreeIter for MatchCase {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ExprMatchCase(self));

        self.condition.depth_first_iter(f);
        self.body.depth_first_iter(f);

        f(Order::Leave, RefNode::ExprMatchCase(self));
    }
}

impl ParseTreeIter for Match {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ExprMatch(self));

        self.condition.depth_first_iter(f);

        for case in &self.cases {
            case.depth_first_iter(f);
        }

        if let Some(default) = &self.default_case {
            default.depth_first_iter(f);
        }

        f(Order::Leave, RefNode::ExprMatch(self));
    }
}

impl ParseTreeIter for Break {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ExprBreak(self));
        f(Order::Leave, RefNode::ExprBreak(self));
    }
}

impl ParseTreeIter for Continue {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ExprContinue(self));
        f(Order::Leave, RefNode::ExprContinue(self));
    }
}

impl ParseTreeIter for Return {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ExprReturn(self));

        if let Some(value) = &self.value {
            value.depth_first_iter(f);
        }

        f(Order::Leave, RefNode::ExprReturn(self));
    }
}

impl ParseTreeIter for ForEach {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ExprFor(self));

        let _ = &self.bindings;

        if let Some(attributes) = &self.attributes {
            attributes.depth_first_iter(f);
        }

        self.iterable.depth_first_iter(f);
        self.body.depth_first_iter(f);

        f(Order::Leave, RefNode::ExprFor(self));
    }
}

impl ParseTreeIter for Await {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ExprAwait(self));

        self.future.depth_first_iter(f);

        f(Order::Leave, RefNode::ExprAwait(self));
    }
}

impl ParseTreeIter for FunctionCall {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ExprFunctionCall(self));

        self.callee.depth_first_iter(f);

        for arg in &self.positional {
            arg.depth_first_iter(f);
        }

        for (_, value) in &self.named {
            value.depth_first_iter(f);
        }

        f(Order::Leave, RefNode::ExprFunctionCall(self));
    }
}

impl ParseTreeIter for MethodCall {
    fn depth_first_iter(&self, f: &mut dyn FnMut(Order, RefNode)) {
        f(Order::Enter, RefNode::ExprMethodCall(self));

        self.object.depth_first_iter(f);

        let _ = &self.method_name;

        for arg in &self.positional {
            arg.depth_first_iter(f);
        }

        for (_, value) in &self.named {
            value.depth_first_iter(f);
        }

        f(Order::Leave, RefNode::ExprMethodCall(self));
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
            Expr::Integer(e) => e.depth_first_iter(f),
            Expr::TypeInfo(e) => e.depth_first_iter(f),
            Expr::List(e) => e.depth_first_iter(f),
            Expr::Tuple(e) => e.depth_first_iter(f),
            Expr::StructInit(e) => e.depth_first_iter(f),
            Expr::UnaryExpr(e) => e.depth_first_iter(f),
            Expr::BinExpr(e) => e.depth_first_iter(f),
            Expr::Cast(e) => e.depth_first_iter(f),
            Expr::Block(e) => e.depth_first_iter(f),
            Expr::Closure(e) => e.depth_first_iter(f),
            Expr::Path(e) => e.depth_first_iter(f),
            Expr::IndexAccess(e) => e.depth_first_iter(f),
            Expr::FieldAccess(e) => e.depth_first_iter(f),
            Expr::If(e) => e.depth_first_iter(f),
            Expr::While(e) => e.depth_first_iter(f),
            Expr::Match(e) => e.depth_first_iter(f),
            Expr::Break(e) => e.depth_first_iter(f),
            Expr::Continue(e) => e.depth_first_iter(f),
            Expr::Return(e) => e.depth_first_iter(f),
            Expr::For(e) => e.depth_first_iter(f),
            Expr::Await(e) => e.depth_first_iter(f),
            Expr::FunctionCall(e) => e.depth_first_iter(f),
            Expr::MethodCall(e) => e.depth_first_iter(f),
        }
    }
}
