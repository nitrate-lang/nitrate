use super::parse::*;
use crate::lexer::*;
use crate::parsetree::*;
use slog::error;

#[derive(Default)]
struct RefinementOptions<'a> {
    minimum: Option<ExprKey<'a>>,
    maximum: Option<ExprKey<'a>>,
    width: Option<ExprKey<'a>>,
}

impl<'a> RefinementOptions<'a> {
    fn has_any(&self) -> bool {
        self.minimum.is_some() || self.maximum.is_some() || self.width.is_some()
    }
}

impl<'storage, 'a> Parser<'storage, 'a> {
    fn parse_refinement_bounds(&mut self, options: &mut RefinementOptions<'a>) {
        assert!(self.lexer.peek_t() == Token::Punct(Punct::LeftBracket));
        self.lexer.skip();

        if !self.lexer.skip_if(&Token::Punct(Punct::Colon)) {
            if let Some(minimum) = self.parse_expression() {
                options.minimum = Some(minimum);
            } else {
                self.set_failed_bit();
                error!(
                    self.log,
                    "error[P????]: Unable to parse type's minimum refinement bound\n--> {}",
                    self.lexer.sync_position()
                );
            }

            if !self.lexer.skip_if(&Token::Punct(Punct::Colon)) {
                self.set_failed_bit();
                error!(
                    self.log,
                    "error[P????]: Expected a colon after type's minimum refinement bound\n--> {}",
                    self.lexer.sync_position()
                );
            }
        }

        if !self.lexer.skip_if(&Token::Punct(Punct::RightBracket)) {
            if let Some(maximum) = self.parse_expression() {
                options.maximum = Some(maximum);
            } else {
                self.set_failed_bit();
                error!(
                    self.log,
                    "error[P????]: Unable to parse type's maximum refinement bound\n--> {}",
                    self.lexer.sync_position()
                );
            }

            if !self.lexer.skip_if(&Token::Punct(Punct::RightBracket)) {
                self.set_failed_bit();
                error!(
                    self.log,
                    "error[P????]: Expected a right bracket to close minimum/maximum constraints\n--> {}",
                    self.lexer.sync_position()
                );
            }
        }
    }

    fn parse_refinement_options(&mut self) -> RefinementOptions<'a> {
        let mut options = RefinementOptions::default();
        if self.generic_type_suffix_terminator_ambiguity {
            return options;
        }

        if self.lexer.skip_if(&Token::Punct(Punct::Colon)) {
            if self.lexer.next_is(&Token::Punct(Punct::LeftBracket)) {
                self.parse_refinement_bounds(&mut options);
            } else {
                if let Some(width) = self.parse_expression() {
                    options.width = Some(width);
                } else {
                    self.set_failed_bit();
                    error!(
                        self.log,
                        "error[P????]: Unable to parse type's width refinement bound\n--> {}",
                        self.lexer.sync_position()
                    );
                }

                if self.lexer.skip_if(&Token::Punct(Punct::Colon)) {
                    if self.lexer.next_is(&Token::Punct(Punct::LeftBracket)) {
                        self.parse_refinement_bounds(&mut options);
                    } else {
                        self.set_failed_bit();
                        error!(
                            self.log,
                            "error[P????]: Expected a left bracket for type's range refinement bounds\n--> {}",
                            self.lexer.sync_position()
                        );
                    }
                }
            }
        }

        options
    }

    fn parse_generic_argument(&mut self) -> Option<(&'a str, ExprKey<'a>)> {
        let mut argname: &'a str = "";

        match self.lexer.peek().into_token() {
            Token::Name(name) => {
                /* Named generic argument syntax is ambiguous,
                 * an identifier can be followed by a colon
                 * to indicate a named argument (followed by the expression value).
                 * However, if it is not followed by a colon, the identifier is
                 * to be parsed as an expression.
                 */
                let rewind_pos = self.lexer.sync_position();
                self.lexer.skip();

                if self.lexer.skip_if(&Token::Punct(Punct::Colon)) {
                    argname = name.name();
                } else {
                    self.lexer.rewind(rewind_pos);
                }
            }

            _ => {}
        }

        let argval = if self.lexer.skip_if(&Token::Op(Operator::Add)) {
            self.parse_expression()
        } else {
            self.parse_type().map(|t| t.into())
        };

        if let Some(argval) = argval {
            Some((argname, argval))
        } else {
            self.set_failed_bit();
            error!(
                self.log,
                "error[P????]: Unable to parse generic type argument value\n--> {}",
                self.lexer.sync_position()
            );
            None
        }
    }

    fn parse_generic_arguments(&mut self) -> Vec<(&'a str, ExprKey<'a>)> {
        assert!(self.lexer.peek_t() == Token::Op(Operator::LogicLt));
        self.lexer.skip();

        self.generic_type_depth += 1;
        self.generic_type_suffix_terminator_ambiguity = false;

        let mut generic_arguments = Vec::new();

        self.lexer.skip_if(&Token::Punct(Punct::Comma));
        while self.generic_type_depth > 0 && !self.lexer.is_eof() {
            // '>'
            if self.lexer.skip_if(&Token::Op(Operator::LogicGt)) {
                self.generic_type_depth -= 1;
                break;
            }

            // '>>'
            if self.lexer.skip_if(&Token::Op(Operator::BitShr)) {
                self.generic_type_depth -= 2;
                self.generic_type_suffix_terminator_ambiguity = true;
                break;
            }

            // '>>>'
            if self.lexer.skip_if(&Token::Op(Operator::BitRotr)) {
                self.generic_type_depth -= 3;
                self.generic_type_suffix_terminator_ambiguity = true;
                break;
            }

            if let Some(generic_argument) = self.parse_generic_argument() {
                generic_arguments.push(generic_argument);
            } else {
                self.set_failed_bit();
                error!(
                    self.log,
                    "error[P????]: Unable to parse generic type\n--> {}",
                    self.lexer.sync_position()
                );
                break;
            }

            if self.lexer.skip_if(&Token::Punct(Punct::Comma)) {
                continue;
            }
        }

        generic_arguments
    }

    fn parse_named_type_name(&mut self, type_name: &'a str) -> Option<TypeKey<'a>> {
        assert!(self.lexer.peek_t() == Token::Name(Name::new(type_name)));
        self.lexer.skip();

        let mut bb = Builder::new(self.storage);
        match type_name {
            "u1" | "bool" => Some(bb.get_bool()),
            "u8" => Some(bb.get_u8()),
            "u16" => Some(bb.get_u16()),
            "u32" => Some(bb.get_u32()),
            "u64" => Some(bb.get_u64()),
            "u128" => Some(bb.get_u128()),
            "i8" => Some(bb.get_i8()),
            "i16" => Some(bb.get_i16()),
            "i32" => Some(bb.get_i32()),
            "i64" => Some(bb.get_i64()),
            "i128" => Some(bb.get_i128()),
            "f8" => Some(bb.get_f8()),
            "f16" => Some(bb.get_f16()),
            "f32" => Some(bb.get_f32()),
            "f64" => Some(bb.get_f64()),
            "f128" => Some(bb.get_f128()),
            "_" => Some(bb.get_infer_type()),
            type_name => bb.create_type_name(type_name),
        }
    }

    fn parse_named_type(&mut self, type_name: &'a str) -> Option<TypeKey<'a>> {
        assert!(self.lexer.peek_t() == Token::Name(Name::new(type_name)));

        let named_type_base = self.parse_named_type_name(type_name);
        if !self.lexer.next_is(&Token::Op(Operator::LogicLt)) {
            return named_type_base;
        }

        let is_already_parsing_generic_type = self.generic_type_depth != 0;
        let generic_args = self.parse_generic_arguments();

        if !is_already_parsing_generic_type {
            match self.generic_type_depth {
                0 => {}
                -1 => {
                    self.set_failed_bit();
                    error!(
                        self.log,
                        "error[P????]: Unexpected generic type '>' delimiter or invalid generic type\n--> {}",
                        self.lexer.sync_position()
                    );
                }
                _ => {
                    self.set_failed_bit();
                    error!(
                        self.log,
                        "error[P????]: Unexpected generic type '>>' delimiter or invalid generic type\n--> {}",
                        self.lexer.sync_position()
                    );
                }
            }

            self.generic_type_depth = 0;
            self.generic_type_suffix_terminator_ambiguity = false;
        }

        if let Some(generic_base) = named_type_base {
            Builder::new(self.storage)
                .create_generic_type()
                .with_base(generic_base)
                .add_arguments(generic_args)
                .build()
        } else {
            self.set_failed_bit();
            error!(
                self.log,
                "error[P????]: Unable to construct generic type due to previous errors\n--> {}",
                self.lexer.sync_position()
            );
            None
        }
    }

    fn parse_tuple_type(&mut self) -> Option<TypeKey<'a>> {
        assert!(self.lexer.peek_t() == Token::Punct(Punct::LeftBrace));
        self.lexer.skip();

        let mut tuple_elements = Vec::new();

        self.lexer.skip_if(&Token::Punct(Punct::Comma));
        while !self.lexer.is_eof() {
            if self.lexer.skip_if(&Token::Punct(Punct::RightBrace)) {
                break;
            }

            if let Some(element) = self.parse_type() {
                tuple_elements.push(element);
            } else {
                self.set_failed_bit();
                error!(
                    self.log,
                    "error[P????]: Unable to parse tuple element type\n--> {}",
                    self.lexer.sync_position()
                );
            }

            if !self.lexer.skip_if(&Token::Punct(Punct::Comma)) {
                if self.lexer.skip_if(&Token::Punct(Punct::RightBrace)) {
                    break;
                } else {
                    self.set_failed_bit();
                    error!(
                        self.log,
                        "error[P????]: Expected comma or right brace in tuple type\n--> {}",
                        self.lexer.sync_position()
                    );

                    break;
                }
            }
        }

        Builder::new(self.storage)
            .create_tuple_type()
            .add_elements(tuple_elements)
            .build()
    }

    fn parse_rest_of_array(&mut self, element_type: Option<TypeKey<'a>>) -> Option<TypeKey<'a>> {
        assert!(self.lexer.peek_t() == Token::Punct(Punct::Semicolon));
        self.lexer.skip();

        let array_type = if let Some(array_count) = self.parse_expression() {
            if let Some(element_type) = element_type {
                Builder::new(self.storage)
                    .create_array_type()
                    .with_element(element_type)
                    .with_count(array_count)
                    .build()
            } else {
                self.set_failed_bit();
                error!(
                    self.log,
                    "error[P????]: Unable to parse element type for array\n--> {}",
                    self.lexer.sync_position()
                );
                None
            }
        } else {
            self.set_failed_bit();
            error!(
                self.log,
                "error[P????]: Unable to parse length expression for array\n--> {}",
                self.lexer.sync_position()
            );
            None
        };

        if !self.lexer.skip_if(&Token::Punct(Punct::RightBracket)) {
            self.set_failed_bit();
            error!(
                self.log,
                "error[P????]: Expected right bracket to close array type\n--> {}",
                self.lexer.sync_position()
            );
        }

        array_type
    }

    fn parse_rest_of_map_type(&mut self, key_type: Option<TypeKey<'a>>) -> Option<TypeKey<'a>> {
        assert!(self.lexer.peek_t() == Token::Op(Operator::Arrow));
        self.lexer.skip();

        let map_type = if let Some(value_type) = self.parse_type() {
            if let Some(key_type) = key_type {
                Builder::new(self.storage)
                    .create_map_type()
                    .with_key(key_type)
                    .with_value(value_type)
                    .build()
            } else {
                self.set_failed_bit();
                error!(
                    self.log,
                    "error[P????]: Unable to parse map's key type\n--> {}",
                    self.lexer.sync_position()
                );
                None
            }
        } else {
            self.set_failed_bit();
            error!(
                self.log,
                "error[P????]: Unable to parse map's value type\n--> {}",
                self.lexer.sync_position()
            );
            None
        };

        if !self.lexer.skip_if(&Token::Punct(Punct::RightBracket)) {
            self.set_failed_bit();
            error!(
                self.log,
                "error[P????]: Expected right bracket to close map type\n--> {}",
                self.lexer.sync_position()
            );
        }

        map_type
    }

    fn parse_rest_of_slice_type(
        &mut self,
        element_type: Option<TypeKey<'a>>,
    ) -> Option<TypeKey<'a>> {
        assert!(self.lexer.peek_t() == Token::Punct(Punct::RightBracket));
        self.lexer.skip();

        let slice_type = if let Some(element_type) = element_type {
            Builder::new(self.storage)
                .create_slice_type()
                .with_element(element_type)
                .build()
        } else {
            self.set_failed_bit();
            error!(
                self.log,
                "error[P????]: Unable to parse element type for slice\n--> {}",
                self.lexer.sync_position()
            );
            None
        };

        slice_type
    }

    fn parse_array_or_slice_or_map(&mut self) -> Option<TypeKey<'a>> {
        assert!(self.lexer.peek_t() == Token::Punct(Punct::LeftBracket));
        self.lexer.skip();

        let something_type = self.parse_type();

        if self.lexer.next_is(&Token::Punct(Punct::Semicolon)) {
            self.parse_rest_of_array(something_type)
        } else if self.lexer.next_is(&Token::Op(Operator::Arrow)) {
            self.parse_rest_of_map_type(something_type)
        } else if self.lexer.next_is(&Token::Punct(Punct::RightBracket)) {
            self.parse_rest_of_slice_type(something_type)
        } else {
            self.set_failed_bit();
            error!(
                self.log,
                "error[P????]: Expected semicolon, right bracket, or arrow in array/slice/map type respectively\n--> {}",
                self.lexer.sync_position()
            );
            None
        }
    }

    fn parse_managed_type(&mut self) -> Option<TypeKey<'a>> {
        assert!(self.lexer.peek_t() == Token::Op(Operator::BitAnd));
        self.lexer.skip();

        let is_mutable = self.lexer.skip_if(&Token::Keyword(Keyword::Mut))
            || (self.lexer.skip_if(&Token::Keyword(Keyword::Const)) && false);

        if let Some(target) = self.parse_type() {
            Builder::new(self.storage)
                .create_managed_type()
                .with_target(target)
                .with_mutability(is_mutable)
                .build()
        } else {
            self.set_failed_bit();
            error!(
                self.log,
                "error[P????]: Unable to parse reference's target type\n--> {}",
                self.lexer.sync_position()
            );
            None
        }
    }

    fn parse_unmanaged_type(&mut self) -> Option<TypeKey<'a>> {
        assert!(self.lexer.peek_t() == Token::Op(Operator::Mul));
        self.lexer.skip();

        let is_mutable = self.lexer.skip_if(&Token::Keyword(Keyword::Mut))
            || (self.lexer.skip_if(&Token::Keyword(Keyword::Const)) && false);

        if let Some(target) = self.parse_type() {
            Builder::new(self.storage)
                .create_unmanaged_type()
                .with_target(target)
                .with_mutability(is_mutable)
                .build()
        } else {
            self.set_failed_bit();
            error!(
                self.log,
                "error[P????]: Unable to parse pointer's target type\n--> {}",
                self.lexer.sync_position()
            );
            None
        }
    }

    fn parse_function_attributes(&mut self) -> Vec<ExprKey<'a>> {
        let mut attributes = Vec::new();

        if self.lexer.skip_if(&Token::Punct(Punct::LeftBracket)) {
            self.lexer.skip_if(&Token::Punct(Punct::Comma));
            while !self.lexer.is_eof() {
                if self.lexer.skip_if(&Token::Punct(Punct::RightBracket)) {
                    break;
                }

                if let Some(attribute) = self.parse_expression() {
                    attributes.push(attribute);
                } else {
                    self.set_failed_bit();
                    error!(
                        self.log,
                        "error[P????]: Unable to parse function type attribute\n--> {}",
                        self.lexer.sync_position()
                    );
                }

                if !self.lexer.skip_if(&Token::Punct(Punct::Comma)) {
                    if self.lexer.skip_if(&Token::Punct(Punct::RightBracket)) {
                        break;
                    } else {
                        self.set_failed_bit();
                        error!(
                            self.log,
                            "error[P????]: Expected comma or right bracket in function type attributes\n--> {}",
                            self.lexer.sync_position()
                        );
                        break;
                    }
                }
            }
        }

        attributes
    }

    fn parse_function_parameters(
        &mut self,
    ) -> Vec<(&'a str, Option<TypeKey<'a>>, Option<ExprKey<'a>>)> {
        let mut parameters = Vec::new();

        if self.lexer.skip_if(&Token::Punct(Punct::LeftParen)) {
            self.lexer.skip_if(&Token::Punct(Punct::Comma));
            while !self.lexer.is_eof() {
                if self.lexer.skip_if(&Token::Punct(Punct::RightParen)) {
                    break;
                }

                if let Token::Name(param_name) = self.lexer.next().into_token() {
                    let ty = if self.lexer.skip_if(&Token::Punct(Punct::Colon)) {
                        self.parse_type()
                    } else {
                        None
                    };

                    let default = if self.lexer.skip_if(&Token::Op(Operator::Set)) {
                        self.parse_expression()
                    } else {
                        None
                    };

                    parameters.push((param_name.name(), ty, default));
                } else {
                    self.set_failed_bit();
                    error!(
                        self.log,
                        "error[P????]: Expected a parameter name in function parameters\n--> {}",
                        self.lexer.sync_position()
                    );
                }

                if !self.lexer.skip_if(&Token::Punct(Punct::Comma)) {
                    if self.lexer.skip_if(&Token::Punct(Punct::RightParen)) {
                        break;
                    } else {
                        self.set_failed_bit();
                        error!(
                            self.log,
                            "error[P????]: Expected comma or right parenthesis in function parameters\n--> {}",
                            self.lexer.sync_position()
                        );
                        break;
                    }
                }
            }
        }

        parameters
    }

    fn parse_function_type(&mut self) -> Option<TypeKey<'a>> {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Fn));
        self.lexer.skip();

        let attributes = self.parse_function_attributes();
        let parameters = self.parse_function_parameters();

        let return_type = if self.lexer.skip_if(&Token::Op(Operator::Arrow)) {
            if let Some(return_type) = self.parse_type() {
                return_type
            } else {
                self.set_failed_bit();
                error!(
                    self.log,
                    "error[P????]: Unable to parse function return type\n--> {}",
                    self.lexer.sync_position()
                );
                Builder::new(self.storage).get_infer_type()
            }
        } else {
            Builder::new(self.storage).get_infer_type()
        };

        Builder::new(self.storage)
            .create_function_type()
            .add_attributes(attributes)
            .add_parameters(parameters)
            .with_return_type(return_type)
            .build()
    }

    fn parse_opaque_type(&mut self) -> Option<TypeKey<'a>> {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Opaque));
        self.lexer.skip();

        if !self.lexer.skip_if(&Token::Punct(Punct::LeftParen)) {
            self.set_failed_bit();
            error!(
                self.log,
                "error[P????]: Expected left parenthesis after 'opaque' keyword\n--> {}",
                self.lexer.sync_position()
            );
        }

        let opaque_identity = if let Token::String(string) = self.lexer.peek().into_token() {
            self.lexer.skip();
            string
        } else {
            self.set_failed_bit();
            error!(
                self.log,
                "error[P????]: Expected a string literal for opaque type identity\n--> {}",
                self.lexer.sync_position()
            );
            StringData::from_ref("")
        };

        if !self.lexer.skip_if(&Token::Punct(Punct::RightParen)) {
            self.set_failed_bit();
            error!(
                self.log,
                "error[P????]: Expected right parenthesis to close opaque type declaration\n--> {}",
                self.lexer.sync_position()
            );
        }

        Builder::new(self.storage).create_opaque_type(opaque_identity)
    }

    fn parse_type_primary(&mut self) -> Option<TypeKey<'a>> {
        let first_token = self.lexer.peek();
        let start_pos = first_token.start();

        let old_generic_type_depth = self.generic_type_depth;
        let must_preserve_generic_depth = !matches!(first_token.token(), Token::Name(_));
        if must_preserve_generic_depth {
            self.generic_type_depth = 0;
        }

        let result = match first_token.into_token() {
            Token::Name(name) => self.parse_named_type(name.name()),
            Token::Punct(Punct::LeftBrace) => self.parse_tuple_type(),
            Token::Punct(Punct::LeftBracket) => self.parse_array_or_slice_or_map(),
            Token::Op(Operator::BitAnd) => self.parse_managed_type(),
            Token::Op(Operator::Mul) => self.parse_unmanaged_type(),
            Token::Keyword(Keyword::Fn) => self.parse_function_type(),
            Token::Keyword(Keyword::Opaque) => self.parse_opaque_type(),

            Token::Integer(int) => {
                self.set_failed_bit();
                error!(
                    self.log,
                    "error[P????]: Unexpected integer token '{}' while parsing type\n--> {}",
                    int,
                    start_pos
                );
                None
            }

            Token::Float(float) => {
                self.set_failed_bit();
                error!(
                    self.log,
                    "error[P????]: Unexpected float token '{}' while parsing type\n--> {}",
                    float,
                    start_pos
                );
                None
            }

            Token::Keyword(func) => {
                self.set_failed_bit();
                error!(
                    self.log,
                    "error[P????]: Unexpected keyword token '{}' while parsing type\n--> {}",
                    func,
                    start_pos
                );
                None
            }

            Token::String(string) => {
                self.set_failed_bit();
                error!(
                    self.log,
                    "error[P????]: Unexpected string token '{}' while parsing type\n--> {}",
                    string,
                    start_pos
                );
                None
            }

            Token::Binary(binary) => {
                self.set_failed_bit();
                error!(
                    self.log,
                    "error[P????]: Unexpected binary string token '{}' while parsing type\n--> {}",
                    binary,
                    start_pos
                );
                None
            }

            Token::Char(char) => {
                self.set_failed_bit();
                error!(
                    self.log,
                    "error[P????]: Unexpected character token '{}' while parsing type\n--> {}",
                    char,
                    start_pos
                );
                None
            }

            Token::Punct(punc) => {
                self.set_failed_bit();
                error!(
                    self.log,
                    "error[P????]: Unexpected punctuation token '{}' while parsing type\n--> {}",
                    punc,
                    start_pos
                );
                None
            }

            Token::Op(op) => {
                self.set_failed_bit();
                error!(
                    self.log,
                    "error[P????]: Unexpected operator token '{}' while parsing type\n--> {}",
                    op,
                    start_pos
                );
                None
            }

            Token::Comment(_) => {
                self.set_failed_bit();
                error!(
                    self.log,
                    "error[P????]: Unexpected comment token while parsing type\n--> {}", start_pos
                );
                None
            }

            Token::Eof => {
                self.set_failed_bit();
                error!(
                    self.log,
                    "error[P????]: Unexpected end of file while parsing type\n--> {}", start_pos
                );
                None
            }

            Token::Illegal => {
                self.set_failed_bit();
                error!(
                    self.log,
                    "error[P????]: Illegal token encountered during type parsing\n--> {}",
                    start_pos
                );
                None
            }
        };

        if must_preserve_generic_depth {
            self.generic_type_depth = old_generic_type_depth;
        }

        result
    }

    pub fn parse_type(&mut self) -> Option<TypeKey<'a>> {
        if self.lexer.skip_if(&Token::Punct(Punct::LeftParen)) {
            let inner = self.parse_type();

            if !self.lexer.skip_if(&Token::Punct(Punct::RightParen)) {
                self.set_failed_bit();
                error!(
                    self.log,
                    "error[P????]: Expected right parenthesis after type expression\n--> {}",
                    self.lexer.sync_position()
                );
            }

            inner.inspect(|v| v.add_parentheses(self.storage))
        } else {
            let primary = self.parse_type_primary();
            let options = self.parse_refinement_options();

            if options.has_any() {
                if let Some(base) = primary {
                    return Builder::new(self.storage)
                        .create_refinement_type()
                        .with_base(base)
                        .with_width(options.width)
                        .with_minimum(options.minimum)
                        .with_maximum(options.maximum)
                        .build();
                } else {
                    self.set_failed_bit();
                    error!(
                        self.log,
                        "error[P????]: Unable to construct refinement type due to previous errors\n--> {}",
                        self.lexer.sync_position()
                    );
                }
            }

            primary
        }
    }
}

#[test]
fn test_parse_type() {
    let source = "Option<[str -> Vec<{u8, str: 48, Set<Address<str>>: 2: [1:]}>]>: 1";

    let expected = r#"RefinementType {
    base: GenericType {
        base: TypeName {
            name: "Option",
        },
        args: [
            (
                "",
                MapType {
                    key: TypeName {
                        name: "str",
                    },
                    value: GenericType {
                        base: TypeName {
                            name: "Vec",
                        },
                        args: [
                            (
                                "",
                                TupleType {
                                    elements: [
                                        u8,
                                        RefinementType {
                                            base: TypeName {
                                                name: "str",
                                            },
                                            width: Some(
                                                IntegerLit {
                                                    value: 48,
                                                    kind: Decimal,
                                                },
                                            ),
                                            min: None,
                                            max: None,
                                        },
                                        RefinementType {
                                            base: GenericType {
                                                base: TypeName {
                                                    name: "Set",
                                                },
                                                args: [
                                                    (
                                                        "",
                                                        GenericType {
                                                            base: TypeName {
                                                                name: "Address",
                                                            },
                                                            args: [
                                                                (
                                                                    "",
                                                                    TypeName {
                                                                        name: "str",
                                                                    },
                                                                ),
                                                            ],
                                                        },
                                                    ),
                                                ],
                                            },
                                            width: Some(
                                                IntegerLit {
                                                    value: 2,
                                                    kind: Decimal,
                                                },
                                            ),
                                            min: Some(
                                                IntegerLit {
                                                    value: 1,
                                                    kind: Decimal,
                                                },
                                            ),
                                            max: None,
                                        },
                                    ],
                                },
                            ),
                        ],
                    },
                },
            ),
        ],
    },
    width: Some(
        IntegerLit {
            value: 1,
            kind: Decimal,
        },
    ),
    min: None,
    max: None,
}"#;

    let lexer = Lexer::new(source.as_bytes(), "test").expect("Failed to create lexer");
    let mut storage = Storage::new();

    let model = Parser::new(lexer, &mut storage, None)
        .parse()
        .expect("Failed to parse source");
    assert!(!model.any_errors(), "Parsing failed with errors");

    let tree = model.tree().as_printable(&storage);
    let serialized = format!("{:#?}", tree);

    assert_eq!(
        serialized, expected,
        "Parsed type does not match expected structure"
    );
}
