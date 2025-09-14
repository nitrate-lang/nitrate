use super::parse::Parser;
use log::error;
use nitrate_parsetree::kind::{
    Block, EnumDefinition, EnumVariant, GenericParameter, GlobalVariable, Item, Module,
    NamedFunction, Type, TypeAlias,
};
use nitrate_tokenize::{Keyword, Op, Punct, Token};

impl Parser<'_> {
    fn parse_generic_parameter(&mut self) -> Option<GenericParameter> {
        let Some(name) = self.lexer.next_if_name() else {
            error!(
                "[P????]: generic parameter: expected parameter name\n--> {}",
                self.lexer.sync_position()
            );
            return None;
        };

        let default = if self.lexer.skip_if(&Token::Op(Op::Set)) {
            Some(self.parse_type()?)
        } else {
            None
        };

        Some(GenericParameter { name, default })
    }

    fn parse_generic_parameters(&mut self) -> Option<Vec<GenericParameter>> {
        let mut params = Vec::new();

        if !self.lexer.skip_if(&Token::Op(Op::LogicLt)) {
            return Some(params);
        }

        loop {
            if self.lexer.skip_if(&Token::Op(Op::LogicGt)) {
                break;
            }

            let Some(param) = self.parse_generic_parameter() else {
                self.set_failed_bit();
                error!(
                    "[P????]: generic parameters: expected generic parameter\n--> {}",
                    self.lexer.sync_position()
                );
                return None;
            };

            params.push(param);

            if !self.lexer.skip_if(&Token::Punct(Punct::Comma))
                && !self.lexer.next_is(&Token::Op(Op::LogicGt))
            {
                self.set_failed_bit();
                error!(
                    "[P????]: generic parameters: expected comma or closing angle bracket\n--> {}",
                    self.lexer.sync_position()
                );
                return None;
            }
        }

        Some(params)
    }

    fn parse_module(&mut self) -> Option<Item> {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Mod));
        self.lexer.skip_tok();

        let attributes = self.parse_attributes()?;

        let Some(name) = self.lexer.next_if_name() else {
            error!(
                "[P????]: module: expected module name\n--> {}",
                self.lexer.sync_position()
            );
            return None;
        };

        if !self.lexer.skip_if(&Token::Punct(Punct::LeftBrace)) {
            self.set_failed_bit();
            error!(
                "[P????]: module: expected opening brace\n--> {}",
                self.lexer.sync_position()
            );

            return None;
        }

        let mut items = Vec::new();

        loop {
            if self.lexer.skip_if(&Token::Punct(Punct::RightBrace)) {
                break;
            }

            if self.lexer.next_if_comment().is_some() {
                continue;
            }

            let Some(item) = self.parse_item() else {
                self.set_failed_bit();
                break;
            };

            items.push(item);
        }

        Some(Item::Module(Box::new(Module {
            attributes,
            name,
            items,
        })))
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

    fn parse_enum_variant(&mut self) -> Option<EnumVariant> {
        let attributes = self.parse_attributes()?;

        let Some(name) = self.lexer.next_if_name() else {
            error!(
                "[P????]: enum variant: expected variant name\n--> {}",
                self.lexer.sync_position()
            );
            return None;
        };

        let variant_type = if self.lexer.next_is(&Token::Punct(Punct::LeftParen)) {
            Some(self.parse_type()?)
        } else {
            None
        };

        let value = if self.lexer.skip_if(&Token::Op(Op::Set)) {
            Some(self.parse_expression()?)
        } else {
            None
        };

        Some(EnumVariant {
            attributes,
            name,
            variant_type,
            value,
        })
    }

    fn parse_enum(&mut self) -> Option<Item> {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Enum));
        self.lexer.skip_tok();

        let attributes = self.parse_attributes()?;
        let Some(name) = self.lexer.next_if_name() else {
            error!(
                "[P????]: enum: expected enum name\n--> {}",
                self.lexer.sync_position()
            );
            return None;
        };

        let generic_parameters = self.parse_generic_parameters()?;

        if !self.lexer.skip_if(&Token::Punct(Punct::LeftBrace)) {
            self.set_failed_bit();
            error!(
                "[P????]: enum: expected opening brace\n--> {}",
                self.lexer.sync_position()
            );

            return None;
        }

        let mut variants = Vec::new();

        while !self.lexer.skip_if(&Token::Punct(Punct::RightBrace)) {
            let Some(variant) = self.parse_enum_variant() else {
                self.set_failed_bit();
                break;
            };

            variants.push(variant);

            if !self.lexer.skip_if(&Token::Punct(Punct::Comma))
                && !self.lexer.next_is(&Token::Punct(Punct::RightBrace))
            {
                self.set_failed_bit();
                error!(
                    "[P????]: enum: expected comma or closing brace\n--> {}",
                    self.lexer.sync_position()
                );
                break;
            }
        }

        Some(Item::EnumDefinition(Box::new(EnumDefinition {
            attributes,
            name,
            type_params: generic_parameters,
            variants,
        })))
    }

    fn parse_struct(&mut self) -> Option<Item> {
        // TODO: struct parsing logic
        self.set_failed_bit();
        error!("Struct parsing not implemented yet");
        None
    }

    fn parse_class(&mut self) -> Option<Item> {
        // TODO: class parsing logic
        self.set_failed_bit();
        error!("Class parsing not implemented yet");
        None
    }

    fn parse_trait(&mut self) -> Option<Item> {
        // TODO: trait parsing logic
        self.set_failed_bit();
        error!("Trait parsing not implemented yet");
        None
    }

    fn parse_implementation(&mut self) -> Option<Item> {
        // TODO: implementation parsing logic
        self.set_failed_bit();
        error!("Implementation parsing not implemented yet");
        None
    }

    fn parse_contract(&mut self) -> Option<Item> {
        // TODO: contract parsing logic
        self.set_failed_bit();
        error!("Contract parsing not implemented yet");
        None
    }

    fn parse_named_function(&mut self) -> Option<Item> {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Fn));
        self.lexer.skip_tok();

        let attributes = self.parse_attributes()?;
        let Some(name) = self.lexer.next_if_name() else {
            error!(
                "[P????]: function: expected function name\n--> {}",
                self.lexer.sync_position()
            );
            return None;
        };
        let parameters = self.parse_function_parameters()?;

        let return_type = if self.lexer.skip_if(&Token::Op(Op::Arrow)) {
            self.parse_type()?
        } else {
            Type::InferType
        };

        let definition = if self.lexer.next_is(&Token::Punct(Punct::LeftBrace)) {
            Some(self.parse_block()?)
        } else if self.lexer.skip_if(&Token::Op(Op::BlockArrow)) {
            let expr = self.parse_expression()?;
            Some(Block {
                elements: vec![expr],
                ends_with_semi: false,
            })
        } else {
            None
        };

        Some(Item::NamedFunction(Box::new(NamedFunction {
            attributes,
            name,
            type_params: Vec::new(),
            parameters,
            return_type,
            definition,
        })))
    }

    fn parse_static_variable(&mut self) -> Option<Item> {
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
                "[P????]: static: expected variable name\n--> {}",
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

        Some(Item::GlobalVariable(Box::new(GlobalVariable {
            attributes: attributes,
            is_mutable,
            name: variable_name.clone(),
            var_type: type_annotation,
            initializer: initializer,
        })))
    }

    pub(crate) fn parse_item(&mut self) -> Option<Item> {
        match self.lexer.peek_t() {
            Token::Keyword(Keyword::Mod) => self.parse_module(),
            Token::Keyword(Keyword::Type) => self.parse_type_alias(),
            Token::Keyword(Keyword::Enum) => self.parse_enum(),
            Token::Keyword(Keyword::Struct) => self.parse_struct(),
            Token::Keyword(Keyword::Class) => self.parse_class(),
            Token::Keyword(Keyword::Trait) => self.parse_trait(),
            Token::Keyword(Keyword::Impl) => self.parse_implementation(),
            Token::Keyword(Keyword::Contract) => self.parse_contract(),
            Token::Keyword(Keyword::Fn) => self.parse_named_function(),
            Token::Keyword(Keyword::Static) => self.parse_static_variable(),

            Token::Name(name) => {
                self.set_failed_bit();
                error!(
                    "[P????]: item: unexpected name '{}'\n--> {}",
                    name,
                    self.lexer.sync_position()
                );

                None
            }

            Token::Integer(int) => {
                self.set_failed_bit();
                error!(
                    "[P????]: item: unexpected integer '{}'\n--> {}",
                    int,
                    self.lexer.sync_position()
                );

                None
            }

            Token::Float(float) => {
                self.set_failed_bit();
                error!(
                    "[P????]: item: unexpected float '{}'\n--> {}",
                    float,
                    self.lexer.sync_position()
                );

                None
            }

            Token::String(string) => {
                self.set_failed_bit();
                error!(
                    "[P????]: item: unexpected string '{}'\n--> {}",
                    string,
                    self.lexer.sync_position()
                );

                None
            }

            Token::BString(_) => {
                self.set_failed_bit();
                error!(
                    "[P????]: item: unexpected bstring\n--> {}",
                    self.lexer.sync_position()
                );

                None
            }

            Token::Keyword(keyword) => {
                self.set_failed_bit();
                error!(
                    "[P????]: item: unexpected keyword '{}'\n--> {}",
                    keyword,
                    self.lexer.sync_position()
                );

                None
            }

            Token::Op(op) => {
                self.set_failed_bit();
                error!(
                    "[P????]: item: unexpected operator '{}'\n--> {}",
                    op,
                    self.lexer.sync_position()
                );

                None
            }

            Token::Punct(punct) => {
                self.set_failed_bit();
                error!(
                    "[P????]: item: unexpected punctuation '{}'\n--> {}",
                    punct,
                    self.lexer.sync_position()
                );

                None
            }

            Token::Comment(_) => {
                self.set_failed_bit();
                error!(
                    "[P????]: item: unexpected comment\n--> {}",
                    self.lexer.sync_position()
                );

                None
            }

            Token::Eof => {
                self.set_failed_bit();
                error!(
                    "[P????]: item: unexpected end of file\n--> {}",
                    self.lexer.sync_position()
                );

                None
            }

            Token::Illegal => {
                self.set_failed_bit();
                error!(
                    "[P????]: item: illegal token\n--> {}",
                    self.lexer.sync_position()
                );

                None
            }
        }
    }
}
