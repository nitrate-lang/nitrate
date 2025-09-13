use super::parse::Parser;
use interned_string::IString;
use log::error;
use nitrate_parsetree::kind::{
    AnonymousFunction, Await, BinExpr, BinExprOp, Block, Break, Call, CallArguments, Continue,
    DoWhileLoop, Expr, ForEach, GenericParameter, If, IndexAccess, Integer, Item, List, Path,
    Return, Type, TypeAlias, UnaryExpr, UnaryExprOp, Variable, VariableKind, WhileLoop,
};
use nitrate_tokenize::{Keyword, Op, Punct, Token};
use smallvec::smallvec;

impl Parser<'_, '_> {
    fn parse_generic_parameters(&mut self) -> Option<Vec<GenericParameter>> {
        // TODO:
        None
    }

    fn parse_type_alias(&mut self) -> Option<Item> {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Type));
        self.lexer.skip_tok();

        let attributes = self.parse_attributes()?;
        let Some(name) = self.lexer.next_if_name() else {
            error!(
                "[P????]: type alias: expected alias name\n--> {}",
                self.lexer.sync_position()
            );
            return None;
        };

        let generic_parameters = self.parse_generic_parameters()?;

        Some(Item::TypeAlias(Box::new(TypeAlias {
            attributes,
            name,
            type_params: generic_parameters,
            aliased_type: self.parse_type()?,
        })))
    }

    fn parse_enum(&mut self) -> Option<Expr> {
        // TODO: enum parsing logic
        self.set_failed_bit();
        error!("Enum parsing not implemented yet");
        None
    }

    fn parse_struct(&mut self) -> Option<Expr> {
        // TODO: struct parsing logic
        self.set_failed_bit();
        error!("Struct parsing not implemented yet");
        None
    }

    fn parse_class(&mut self) -> Option<Expr> {
        // TODO: class parsing logic
        self.set_failed_bit();
        error!("Class parsing not implemented yet");
        None
    }

    fn parse_trait(&mut self) -> Option<Expr> {
        // TODO: trait parsing logic
        self.set_failed_bit();
        error!("Trait parsing not implemented yet");
        None
    }

    fn parse_implementation(&mut self) -> Option<Expr> {
        // TODO: implementation parsing logic
        self.set_failed_bit();
        error!("Implementation parsing not implemented yet");
        None
    }

    fn parse_contract(&mut self) -> Option<Expr> {
        // TODO: contract parsing logic
        self.set_failed_bit();
        error!("Contract parsing not implemented yet");
        None
    }

    fn parse_named_function(&mut self) -> Option<Expr> {
        if self.lexer.peek_t() == Token::Punct(Punct::LeftBrace) {
            let definition = self.parse_block()?;

            let infer_type = Type::InferType;

            return Some(Expr::Function(Box::new(AnonymousFunction {
                attributes: Vec::new(),
                parameters: Vec::new(),
                return_type: infer_type,
                definition,
            })));
        }

        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Fn));
        self.lexer.skip_tok();

        let attributes = self.parse_attributes()?;
        let parameters = self.parse_function_parameters()?;

        let return_type = if self.lexer.skip_if(&Token::Op(Op::Arrow)) {
            self.parse_type()?
        } else {
            Type::InferType
        };

        let definition = if self.lexer.next_is(&Token::Punct(Punct::LeftBrace)) {
            self.parse_block()?
        } else if self.lexer.skip_if(&Token::Op(Op::BlockArrow)) {
            let expr = self.parse_expression()?;
            Block {
                elements: vec![expr],
                ends_with_semi: false,
            }
        } else {
            self.set_failed_bit();
            error!(
                "[P????]: function: expected function body\n--> {}",
                self.lexer.sync_position()
            );
            return None;
        };

        let function = Expr::Function(Box::new(AnonymousFunction {
            attributes,
            parameters,
            return_type,
            definition,
        }));

        Some(function)
    }

    fn parse_let_variable(&mut self) -> Option<Expr> {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Let));
        self.lexer.skip_tok();

        let attributes = self.parse_attributes()?;

        let is_mutable = self.lexer.skip_if(&Token::Keyword(Keyword::Mut));
        if !is_mutable {
            self.lexer.skip_if(&Token::Keyword(Keyword::Const));
        }

        let variable_name = if let Some(name_token) = self.lexer.next_if_name() {
            name_token
        } else {
            error!(
                "[P????]: let: expected variable name\n--> {}",
                self.lexer.sync_position()
            );
            return None;
        };

        let type_annotation = if self.lexer.skip_if(&Token::Punct(Punct::Colon)) {
            self.parse_type()?
        } else {
            Type::InferType
        };

        let initializer = if self.lexer.skip_if(&Token::Op(Op::Set)) {
            Some(self.parse_expression()?)
        } else {
            None
        };

        let variable = Expr::Variable(Box::new(Variable {
            kind: VariableKind::Let,
            name: variable_name.clone(),
            var_type: type_annotation,
            is_mutable,
            attributes: attributes,
            initializer: initializer,
        }));

        Some(variable)
    }

    fn parse_var_variable(&mut self) -> Option<Expr> {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Var));
        self.lexer.skip_tok();

        let attributes = self.parse_attributes()?;

        let is_mutable = self.lexer.skip_if(&Token::Keyword(Keyword::Mut));
        if !is_mutable {
            self.lexer.skip_if(&Token::Keyword(Keyword::Const));
        }

        let variable_name = if let Some(name_token) = self.lexer.next_if_name() {
            name_token
        } else {
            error!(
                "[P????]: var: expected variable name\n--> {}",
                self.lexer.sync_position()
            );
            return None;
        };

        let type_annotation = if self.lexer.skip_if(&Token::Punct(Punct::Colon)) {
            self.parse_type()?
        } else {
            Type::InferType
        };

        let initializer = if self.lexer.skip_if(&Token::Op(Op::Set)) {
            Some(self.parse_expression()?)
        } else {
            None
        };

        let variable = Expr::Variable(Box::new(Variable {
            kind: VariableKind::Var,
            name: variable_name.clone(),
            var_type: type_annotation,
            is_mutable,
            attributes: attributes,
            initializer: initializer,
        }));

        Some(variable)
    }
}
