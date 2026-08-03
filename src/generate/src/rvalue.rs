use crate::Gen;
use nitrate_translation::parsetree::ast::{self};
use std::{todo, unreachable};

impl Gen {
    fn select_rvalue_kind(&mut self, ty: &ast::Type) -> ast::RValueKind {
        // TODO: select a random rvalue kind compatible with the given type
        todo!()
    }

    pub(crate) fn gen_rvalue(&mut self, ty: &ast::Type) -> ast::Expr {
        match self.select_rvalue_kind(ty) {
            ast::RValueKind::SyntaxError => unreachable!("select_rvalue_kind should not return SyntaxError"),
            ast::RValueKind::Parentheses => self.gen_rvalue_parentheses(ty),
            ast::RValueKind::Boolean => self.gen_rvalue_boolean(ty),
            ast::RValueKind::Integer => self.gen_rvalue_integer(ty),
            ast::RValueKind::Float => self.gen_rvalue_float(ty),
            ast::RValueKind::String => self.gen_rvalue_string(ty),
            ast::RValueKind::BString => self.gen_rvalue_bstring(ty),
            ast::RValueKind::TypeInfo => self.gen_rvalue_type_info(ty),
            ast::RValueKind::List => self.gen_rvalue_list(ty),
            ast::RValueKind::Tuple => self.gen_rvalue_tuple(ty),
            ast::RValueKind::StructInit => self.gen_rvalue_struct_init(ty),
            ast::RValueKind::UnaryExpr => self.gen_rvalue_unary_expr(ty),
            ast::RValueKind::BinExpr => self.gen_rvalue_bin_expr(ty),
            ast::RValueKind::Range => self.gen_rvalue_range(ty),
            ast::RValueKind::Cast => self.gen_rvalue_cast(ty),
            ast::RValueKind::Block => self.gen_rvalue_block(ty),
            ast::RValueKind::Closure => self.gen_rvalue_closure(ty),
            ast::RValueKind::Path => self.gen_rvalue_path(ty),
            ast::RValueKind::IndexAccess => self.gen_rvalue_index_access(ty),
            ast::RValueKind::FieldAccess => self.gen_rvalue_field_access(ty),
            ast::RValueKind::If => self.gen_rvalue_if(ty),
            ast::RValueKind::While => self.gen_rvalue_while(ty),
            ast::RValueKind::Match => self.gen_rvalue_match(ty),
            ast::RValueKind::Break => self.gen_rvalue_break(ty),
            ast::RValueKind::Continue => self.gen_rvalue_continue(ty),
            ast::RValueKind::Return => self.gen_rvalue_return(ty),
            ast::RValueKind::ForEach => self.gen_rvalue_foreach(ty),
            ast::RValueKind::Await => self.gen_rvalue_await(ty),
            ast::RValueKind::FunctionCall => self.gen_rvalue_function_call(ty),
            ast::RValueKind::MethodCall => self.gen_rvalue_method_call(ty),
        }
    }

    fn gen_rvalue_parentheses(&mut self, ty: &ast::Type) -> ast::Expr {
        // TODO: generate a parenthesized expression
        todo!()
    }

    fn gen_rvalue_boolean(&mut self, ty: &ast::Type) -> ast::Expr {
        // TODO: generate a boolean literal expression
        todo!()
    }

    fn gen_rvalue_integer(&mut self, ty: &ast::Type) -> ast::Expr {
        // TODO: generate an integer literal expression matching the given type
        todo!()
    }

    fn gen_rvalue_float(&mut self, ty: &ast::Type) -> ast::Expr {
        // TODO: generate a floating-point literal expression
        todo!()
    }

    fn gen_rvalue_string(&mut self, ty: &ast::Type) -> ast::Expr {
        // TODO: generate a string literal expression
        todo!()
    }

    fn gen_rvalue_bstring(&mut self, ty: &ast::Type) -> ast::Expr {
        // TODO: generate a byte string literal expression
        todo!()
    }

    fn gen_rvalue_type_info(&mut self, ty: &ast::Type) -> ast::Expr {
        // TODO: generate a type info expression (compile-time type reflection)
        todo!()
    }

    fn gen_rvalue_list(&mut self, ty: &ast::Type) -> ast::Expr {
        // TODO: generate a list literal expression with random elements
        todo!()
    }

    fn gen_rvalue_tuple(&mut self, ty: &ast::Type) -> ast::Expr {
        // TODO: generate a tuple literal expression with random fields
        todo!()
    }

    fn gen_rvalue_struct_init(&mut self, ty: &ast::Type) -> ast::Expr {
        // TODO: generate a struct initialization expression
        todo!()
    }

    fn gen_rvalue_unary_expr(&mut self, ty: &ast::Type) -> ast::Expr {
        // TODO: generate a unary operator expression (negation, dereference, etc.)
        todo!()
    }

    fn gen_rvalue_bin_expr(&mut self, ty: &ast::Type) -> ast::Expr {
        // TODO: generate a binary operator expression (arithmetic, comparison, logical)
        todo!()
    }

    fn gen_rvalue_range(&mut self, ty: &ast::Type) -> ast::Expr {
        // TODO: generate a range expression (e.g., start..end)
        todo!()
    }

    fn gen_rvalue_cast(&mut self, ty: &ast::Type) -> ast::Expr {
        // TODO: generate a type cast expression
        todo!()
    }

    fn gen_rvalue_block(&mut self, ty: &ast::Type) -> ast::Expr {
        // TODO: generate a block expression with multiple statements
        todo!()
    }

    fn gen_rvalue_closure(&mut self, ty: &ast::Type) -> ast::Expr {
        // TODO: generate a closure/lambda expression
        todo!()
    }

    fn gen_rvalue_path(&mut self, ty: &ast::Type) -> ast::Expr {
        // TODO: generate a path expression (variable reference, function call target)
        todo!()
    }

    fn gen_rvalue_index_access(&mut self, ty: &ast::Type) -> ast::Expr {
        // TODO: generate an index access expression (e.g., array[index])
        todo!()
    }

    fn gen_rvalue_field_access(&mut self, ty: &ast::Type) -> ast::Expr {
        // TODO: generate a field access expression (e.g., struct.field)
        todo!()
    }

    fn gen_rvalue_if(&mut self, ty: &ast::Type) -> ast::Expr {
        // TODO: generate an if/else expression
        todo!()
    }

    fn gen_rvalue_while(&mut self, ty: &ast::Type) -> ast::Expr {
        // TODO: generate a while loop expression
        todo!()
    }

    fn gen_rvalue_match(&mut self, ty: &ast::Type) -> ast::Expr {
        // TODO: generate a match expression with random arms
        todo!()
    }

    fn gen_rvalue_break(&mut self, ty: &ast::Type) -> ast::Expr {
        // TODO: generate a break expression with optional value
        todo!()
    }

    fn gen_rvalue_continue(&mut self, ty: &ast::Type) -> ast::Expr {
        // TODO: generate a continue expression
        todo!()
    }

    fn gen_rvalue_return(&mut self, ty: &ast::Type) -> ast::Expr {
        // TODO: generate a return expression with optional value
        todo!()
    }

    fn gen_rvalue_foreach(&mut self, ty: &ast::Type) -> ast::Expr {
        // TODO: generate a for-each loop expression
        todo!()
    }

    fn gen_rvalue_await(&mut self, ty: &ast::Type) -> ast::Expr {
        // TODO: generate an await expression
        todo!()
    }

    fn gen_rvalue_function_call(&mut self, ty: &ast::Type) -> ast::Expr {
        // TODO: generate a function call expression with random arguments
        todo!()
    }

    fn gen_rvalue_method_call(&mut self, ty: &ast::Type) -> ast::Expr {
        // TODO: generate a method call expression on a random receiver
        todo!()
    }
}
