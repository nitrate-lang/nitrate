use crate::bugs::SyntaxBug;

use super::parse::Parser;
use log::error;
use nitrate_parsetree::kind::{
    Block, EnumDefinition, EnumVariant, Expr, GenericParameter, GlobalVariable, Item, Module,
    NamedFunction, StructDefinition, StructField, Type, TypeAlias,
};
use nitrate_tokenize::{Keyword, Op, Punct, Token};

impl Parser<'_, '_> {
    fn parse_generic_parameters(&mut self) -> Vec<GenericParameter> {
        fn parse_generic_parameter(this: &mut Parser) -> Option<GenericParameter> {
            let Some(name) = this.lexer.next_if_name() else {
                let bug = SyntaxBug::GenericParameterMissingName(this.lexer.peek_pos());
                this.bugs.push(&bug);
                return None;
            };

            let default = if this.lexer.skip_if(&Token::Op(Op::Set)) {
                this.parse_type()
            } else {
                None
            };

            Some(GenericParameter { name, default })
        }

        let mut parameters = Vec::new();

        if !self.lexer.skip_if(&Token::Op(Op::LogicLt)) {
            return parameters;
        }

        while !self.lexer.skip_if(&Token::Op(Op::LogicGt)) {
            const MAX_GENERIC_PARAMETERS: usize = 65_536;

            if parameters.len() >= MAX_GENERIC_PARAMETERS {
                let bug = SyntaxBug::TooManyGenericParameters(self.lexer.peek_pos());
                self.bugs.push(&bug);
                break;
            }

            let Some(param) = parse_generic_parameter(self) else {
                self.lexer.skip_until_inclusive(&Token::Op(Op::LogicGt));
                break;
            };

            parameters.push(param);

            if !self.lexer.skip_if(&Token::Punct(Punct::Comma))
                && !self.lexer.next_is(&Token::Op(Op::LogicGt))
            {
                let bug = SyntaxBug::ExpectedCommaOrClosingAngleBracket(self.lexer.peek_pos());
                self.bugs.push(&bug);

                self.lexer.skip_until_inclusive(&Token::Op(Op::LogicGt));
                break;
            }
        }

        parameters
    }

    fn parse_module(&mut self) -> Module {
        let module_start_pos = self.lexer.peek_pos();

        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Mod));
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();

        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxBug::ModuleMissingName(self.lexer.peek_pos());
            self.bugs.push(&bug);
            "".into()
        });

        if !self.lexer.skip_if(&Token::Punct(Punct::LeftBrace)) {
            let bug = SyntaxBug::ExpectedOpeningBrace(self.lexer.peek_pos());
            self.bugs.push(&bug);
        }

        let mut items = Vec::new();
        let mut already_reported_too_many_items = false;

        while !self.lexer.skip_if_or_end(&Token::Punct(Punct::RightBrace)) {
            const MAX_ITEMS_PER_MODULE: usize = 65_536;

            if !already_reported_too_many_items && items.len() >= MAX_ITEMS_PER_MODULE {
                already_reported_too_many_items = true;

                let bug = SyntaxBug::TooManyModuleItems(module_start_pos.clone());
                self.bugs.push(&bug);
            }

            let item = self.parse_item();
            items.push(item);
        }

        Module {
            attributes,
            name,
            items,
        }
    }

    fn parse_type_alias(&mut self) -> TypeAlias {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Type));
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();

        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxBug::TypeAliasMissingName(self.lexer.peek_pos());
            self.bugs.push(&bug);
            "".into()
        });

        let generic_parameters = self.parse_generic_parameters();

        if !self.lexer.skip_if(&Token::Op(Op::Set)) {
            let bug = SyntaxBug::ExpectedEquals(self.lexer.peek_pos());
            self.bugs.push(&bug);
        }

        let aliased_type = self.parse_type().unwrap_or_else(|| {
            self.lexer.skip_until(&Token::Punct(Punct::Semicolon));
            Type::InferType
        });

        if !self.lexer.skip_if(&Token::Punct(Punct::Semicolon)) {
            let bug = SyntaxBug::ExpectedSemicolon(self.lexer.peek_pos());
            self.bugs.push(&bug);
        }

        TypeAlias {
            attributes,
            name,
            type_params: generic_parameters,
            aliased_type,
        }
    }

    fn parse_enum_variant(&mut self) -> Option<EnumVariant> {
        // TODO: Cleanup

        let attributes = self.parse_attributes();

        let Some(name) = self.lexer.next_if_name() else {
            error!(
                "[P????]: enum variant: expected variant name\n--> {}",
                self.lexer.position()
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

    fn parse_enum(&mut self) -> EnumDefinition {
        // TODO: Cleanup

        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Enum));
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();
        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            error!(
                "[P????]: enum: expected enum name\n--> {}",
                self.lexer.position()
            );

            "".into()
        });

        let generic_parameters = self.parse_generic_parameters();

        if !self.lexer.skip_if(&Token::Punct(Punct::LeftBrace)) {
            self.set_failed_bit();
            error!(
                "[P????]: enum: expected opening brace\n--> {}",
                self.lexer.position()
            );
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
                    self.lexer.position()
                );
                break;
            }
        }

        EnumDefinition {
            attributes,
            name,
            type_params: generic_parameters,
            variants,
        }
    }

    fn parse_struct_field(&mut self) -> Option<StructField> {
        // TODO: Cleanup

        let attributes = self.parse_attributes();

        let Some(name) = self.lexer.next_if_name() else {
            error!(
                "[P????]: struct field: expected field name\n--> {}",
                self.lexer.position()
            );
            return None;
        };

        if !self.lexer.skip_if(&Token::Punct(Punct::Colon)) {
            self.set_failed_bit();
            error!(
                "[P????]: struct field: expected colon\n--> {}",
                self.lexer.position()
            );
            return None;
        }

        let field_type = self.parse_type()?;

        let default = if self.lexer.skip_if(&Token::Op(Op::Set)) {
            Some(self.parse_expression()?)
        } else {
            None
        };

        Some(StructField {
            attributes,
            name,
            field_type,
            default,
        })
    }

    fn parse_struct(&mut self) -> StructDefinition {
        // TODO: Cleanup

        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Struct));
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();
        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            error!(
                "[P????]: struct: expected struct name\n--> {}",
                self.lexer.position()
            );
            "".into()
        });

        let generic_parameters = self.parse_generic_parameters();

        if !self.lexer.skip_if(&Token::Punct(Punct::LeftBrace)) {
            self.set_failed_bit();
            error!(
                "[P????]: struct: expected opening brace\n--> {}",
                self.lexer.position()
            );
        }

        let mut fields = Vec::new();

        while !self.lexer.skip_if(&Token::Punct(Punct::RightBrace)) {
            let Some(field) = self.parse_struct_field() else {
                self.set_failed_bit();
                break;
            };

            fields.push(field);

            if !self.lexer.skip_if(&Token::Punct(Punct::Comma))
                && !self.lexer.next_is(&Token::Punct(Punct::RightBrace))
            {
                self.set_failed_bit();
                error!(
                    "[P????]: struct: expected comma or closing brace\n--> {}",
                    self.lexer.position()
                );
                break;
            }
        }

        StructDefinition {
            attributes,
            name,
            type_params: generic_parameters,
            fields,
            methods: Vec::new(),
        }
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

    fn parse_named_function(&mut self) -> NamedFunction {
        // TODO: Cleanup

        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Fn));
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();
        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            error!(
                "[P????]: function: expected function name\n--> {}",
                self.lexer.position()
            );

            "".into()
        });

        let type_params = self.parse_generic_parameters();
        let parameters = self.parse_function_parameters().unwrap_or(Vec::new());

        let return_type = if self.lexer.skip_if(&Token::Op(Op::Arrow)) {
            self.parse_type().unwrap_or(Type::InferType)
        } else {
            Type::InferType
        };

        let definition = if self.lexer.next_is(&Token::Punct(Punct::LeftBrace)) {
            Some(self.parse_block().unwrap_or(Block {
                elements: Vec::new(),
                ends_with_semi: false,
            }))
        } else if self.lexer.skip_if(&Token::Op(Op::BlockArrow)) {
            let expr = self.parse_expression().unwrap_or_else(|| {
                self.set_failed_bit();
                error!(
                    "[P????]: function: expected expression after '->'\n--> {}",
                    self.lexer.position()
                );

                Expr::Unit
            });
            Some(Block {
                elements: vec![expr],
                ends_with_semi: false,
            })
        } else {
            None
        };

        NamedFunction {
            attributes,
            name,
            type_params,
            parameters,
            return_type,
            definition,
        }
    }

    fn parse_static_variable(&mut self) -> GlobalVariable {
        // TODO: Cleanup

        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Static));
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();

        let is_mutable = self.lexer.skip_if(&Token::Keyword(Keyword::Mut));
        if !is_mutable {
            self.lexer.skip_if(&Token::Keyword(Keyword::Const));
        }

        let variable_name = self.lexer.next_if_name().unwrap_or_else(|| {
            error!(
                "[P????]: static: expected variable name\n--> {}",
                self.lexer.position()
            );
            "".into()
        });

        let type_annotation = if self.lexer.skip_if(&Token::Punct(Punct::Colon)) {
            self.parse_type().unwrap_or(Type::InferType)
        } else {
            Type::InferType
        };

        let initializer = if self.lexer.skip_if(&Token::Op(Op::Set)) {
            Some(self.parse_expression().unwrap_or(Expr::Unit))
        } else {
            None
        };

        if !self.lexer.skip_if(&Token::Punct(Punct::Semicolon)) {
            self.set_failed_bit();
            error!(
                "[P????]: static: expected semicolon after variable declaration\n--> {}",
                self.lexer.position()
            );
        }

        GlobalVariable {
            attributes: attributes,
            is_mutable,
            name: variable_name.clone(),
            var_type: type_annotation,
            initializer: initializer,
        }
    }

    pub(crate) fn parse_item(&mut self) -> Item {
        // TODO: Cleanup

        match self.lexer.peek_t() {
            Token::Keyword(Keyword::Mod) => {
                let module = self.parse_module();
                Item::Module(Box::new(module))
            }

            Token::Keyword(Keyword::Type) => {
                let type_alias = self.parse_type_alias();
                Item::TypeAlias(Box::new(type_alias))
            }

            Token::Keyword(Keyword::Enum) => {
                let enum_def = self.parse_enum();
                Item::EnumDefinition(Box::new(enum_def))
            }

            Token::Keyword(Keyword::Struct) => {
                let struct_def = self.parse_struct();
                Item::StructDefinition(Box::new(struct_def))
            }

            Token::Keyword(Keyword::Trait) => {
                let trait_def = self.parse_trait();
                todo!("Trait parsing not implemented yet")
            }

            Token::Keyword(Keyword::Impl) => {
                let impl_def = self.parse_implementation();
                todo!("Implementation parsing not implemented yet")
            }

            Token::Keyword(Keyword::Contract) => {
                let contract_def = self.parse_contract();
                todo!("Contract parsing not implemented yet")
            }

            Token::Keyword(Keyword::Fn) => {
                let func = self.parse_named_function();
                Item::NamedFunction(Box::new(func))
            }

            Token::Keyword(Keyword::Static) => {
                let static_var = self.parse_static_variable();
                Item::GlobalVariable(Box::new(static_var))
            }

            Token::Name(_)
            | Token::Integer(_)
            | Token::Float(_)
            | Token::String(_)
            | Token::BString(_)
            | Token::Keyword(_)
            | Token::Op(_)
            | Token::Punct(_)
            | Token::Comment(_)
            | Token::Eof
            | Token::Illegal => {
                self.lexer.skip_tok();

                self.set_failed_bit();
                error!(
                    "[P????]: item: unexpected token\n--> {}",
                    self.lexer.position()
                );

                Item::Module(Box::new(Module {
                    attributes: Vec::new(),
                    name: "".into(),
                    items: Vec::new(),
                }))
            }
        }
    }
}
