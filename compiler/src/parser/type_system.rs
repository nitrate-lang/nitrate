use super::parse::Parser;
use crate::lexer::{Keyword, Name, Op, Punct, Token};
use crate::parsetree::{Builder, ExprKey, TypeKey, node::FunctionParameter};

use slog::{error, info};

#[allow(unused_imports)]
use crate::lexer::Lexer;
#[allow(unused_imports)]
use crate::parsetree::Storage;

#[derive(Default)]
struct RefinementOptions<'a> {
    minimum: Option<ExprKey<'a>>,
    maximum: Option<ExprKey<'a>>,
    width: Option<ExprKey<'a>>,
}

impl RefinementOptions<'_> {
    fn has_any(&self) -> bool {
        self.minimum.is_some() || self.maximum.is_some() || self.width.is_some()
    }
}

impl<'a> Parser<'_, '_, 'a> {
    fn parse_refinement_range(&mut self) -> Option<(Option<ExprKey<'a>>, Option<ExprKey<'a>>)> {
        assert!(self.lexer.peek_t() == Token::Punct(Punct::LeftBracket));
        self.lexer.skip_tok();

        let mut minimum_bound = None;
        let mut maximum_bound = None;

        if !self.lexer.skip_if(&Token::Punct(Punct::Colon)) {
            let minimum = self.parse_expression()?;

            if !self.lexer.skip_if(&Token::Punct(Punct::Colon)) {
                error!(
                    self.log,
                    "[P0???]: refinement type: expected ':' after minimum range constraint\n--> {}",
                    self.lexer.sync_position()
                );

                return None;
            }

            minimum_bound = Some(minimum);
        }

        if !self.lexer.skip_if(&Token::Punct(Punct::RightBracket)) {
            let maximum = self.parse_expression()?;

            if !self.lexer.skip_if(&Token::Punct(Punct::RightBracket)) {
                error!(
                    self.log,
                    "[P0???]: refinement type: expected ']' to close range constraints\n--> {}",
                    self.lexer.sync_position()
                );

                return None;
            }

            maximum_bound = Some(maximum);
        }

        Some((minimum_bound, maximum_bound))
    }

    fn parse_refinement_options(&mut self) -> Option<RefinementOptions<'a>> {
        if self.generic_type_suffix_terminator_ambiguity
            || !self.lexer.skip_if(&Token::Punct(Punct::Colon))
        {
            return Some(RefinementOptions::default());
        }

        if self.lexer.next_is(&Token::Punct(Punct::LeftBracket)) {
            let (minimum, maximum) = self.parse_refinement_range()?;

            return Some(RefinementOptions {
                minimum,
                maximum,
                width: None,
            });
        }

        let width = self.parse_expression()?;

        if !self.lexer.skip_if(&Token::Punct(Punct::Colon)) {
            return Some(RefinementOptions {
                width: Some(width),
                minimum: None,
                maximum: None,
            });
        }

        if !self.lexer.next_is(&Token::Punct(Punct::LeftBracket)) {
            error!(
                self.log,
                "[P0???]: refinement type: expected '[' for range constraints\n--> {}",
                self.lexer.sync_position()
            );

            return None;
        }

        let (minimum, maximum) = self.parse_refinement_range()?;

        Some(RefinementOptions {
            width: Some(width),
            minimum,
            maximum,
        })
    }

    fn parse_generic_argument(&mut self) -> Option<(&'a str, ExprKey<'a>)> {
        let mut argument_name: &'a str = "";

        if let Token::Name(name) = self.lexer.peek_t() {
            /* Named generic argument syntax is ambiguous,
             * an identifier can be followed by a colon
             * to indicate a named argument (followed by the expression value).
             * However, if it is not followed by a colon, the identifier is
             * to be parsed as an expression.
             */
            let rewind_pos = self.lexer.sync_position();
            self.lexer.skip_tok();

            if self.lexer.skip_if(&Token::Punct(Punct::Colon)) {
                argument_name = name.name();
            } else {
                self.lexer.rewind(rewind_pos);
            }
        }

        let current_pos = self.lexer.sync_position();
        let type_or_expression = match self.lexer.peek_t() {
            Token::Integer(_)
            | Token::Float(_)
            | Token::String(_)
            | Token::BString(_)
            | Token::Char(_) => self.parse_expression(),

            Token::Op(Op::Add) => {
                self.lexer.skip_tok();
                self.parse_expression()
            }

            _ => self.parse_type().map(std::convert::Into::into),
        };

        let Some(argument_value) = type_or_expression else {
            error!(
                self.log,
                "[P0???]: generic type: expected type or expression after generic argument name\n--> {}",
                current_pos
            );
            info!(
                self.log,
                "[P0???]: generic type: syntax hint: if you want to use a Refinement Type as the generic argument, wrap the type in parentheses, e.g. `Vec<(i32: [1: 10])>`"
            );
            return None;
        };

        Some((argument_name, argument_value))
    }

    fn parse_generic_arguments(&mut self) -> Option<Vec<(&'a str, ExprKey<'a>)>> {
        assert!(self.lexer.peek_t() == Token::Op(Op::LogicLt));
        self.lexer.skip_tok();

        self.generic_type_depth += 1;
        self.generic_type_suffix_terminator_ambiguity = false;

        let mut arguments = Vec::new();
        self.lexer.skip_if(&Token::Punct(Punct::Comma));

        while self.generic_type_depth > 0 {
            if self.lexer.skip_if(&Token::Op(Op::LogicGt)) {
                self.generic_type_depth -= 1;
                break;
            }

            if self.lexer.skip_if(&Token::Op(Op::BitShr)) {
                self.generic_type_depth -= 2;
                self.generic_type_suffix_terminator_ambiguity = true;
                break;
            }

            if self.lexer.skip_if(&Token::Op(Op::BitRotr)) {
                self.generic_type_depth -= 3;
                self.generic_type_suffix_terminator_ambiguity = true;
                break;
            }

            let generic_argument = self.parse_generic_argument()?;

            arguments.push(generic_argument);

            if self.generic_type_depth == 0 {
                break;
            }

            if !self.lexer.skip_if(&Token::Punct(Punct::Comma)) {
                let any_terminator = self.lexer.next_is(&Token::Op(Op::LogicGt))
                    || self.lexer.next_is(&Token::Op(Op::BitShr))
                    || self.lexer.next_is(&Token::Op(Op::BitRotr));

                if !any_terminator {
                    error!(
                        self.log,
                        "[P0???]: generic type: expected ',' or '>' after generic argument\n--> {}",
                        self.lexer.sync_position()
                    );

                    return None;
                }
            }
        }

        Some(arguments)
    }

    fn parse_named_type(&mut self, type_name: &'a str) -> Option<TypeKey<'a>> {
        assert!(self.lexer.peek_t() == Token::Name(Name::new(type_name)));
        self.lexer.skip_tok();

        let basis_type = match type_name {
            "_" => Builder::new(self.storage).get_infer_type(),
            type_name => Builder::new(self.storage).create_type_name(type_name),
        };

        if !self.lexer.next_is(&Token::Op(Op::LogicLt)) {
            return Some(basis_type);
        }

        let is_already_parsing_generic_type = self.generic_type_depth != 0;

        let generic_args = self.parse_generic_arguments()?;

        if !is_already_parsing_generic_type {
            match self.generic_type_depth {
                0 => {}
                -1 => {
                    error!(
                        self.log,
                        "[P0???]: generic type: unexpected '>' delimiter\n--> {}",
                        self.lexer.sync_position()
                    );

                    return None;
                }
                _ => {
                    error!(
                        self.log,
                        "[P0???]: generic type: unexpected '>>' delimiter\n--> {}",
                        self.lexer.sync_position()
                    );

                    return None;
                }
            }

            self.generic_type_depth = 0;
            self.generic_type_suffix_terminator_ambiguity = false;
        }

        Some(
            Builder::new(self.storage)
                .create_generic_type()
                .with_base(basis_type)
                .add_arguments(generic_args)
                .build(),
        )
    }

    fn parse_tuple_type(&mut self) -> Option<TypeKey<'a>> {
        assert!(self.lexer.peek_t() == Token::Punct(Punct::LeftBrace));
        self.lexer.skip_tok();

        let mut tuple_elements = Vec::new();
        self.lexer.skip_if(&Token::Punct(Punct::Comma));

        loop {
            if self.lexer.skip_if(&Token::Punct(Punct::RightBrace)) {
                break;
            }

            let element = self.parse_type()?;

            tuple_elements.push(element);

            if !self.lexer.skip_if(&Token::Punct(Punct::Comma)) {
                if self.lexer.skip_if(&Token::Punct(Punct::RightBrace)) {
                    break;
                }
                error!(
                    self.log,
                    "[P0???]: tuple type: expected ',' or '}}' after element type\n--> {}",
                    self.lexer.sync_position()
                );
                info!(
                    self.log,
                    "[P0???]: tuple type: syntax hint: {{<type1>, <type2>, ...}}"
                );

                return None;
            }
        }

        Some(
            Builder::new(self.storage)
                .create_tuple_type()
                .add_elements(tuple_elements)
                .build(),
        )
    }

    fn parse_rest_of_array(&mut self, element_type: TypeKey<'a>) -> Option<TypeKey<'a>> {
        assert!(self.lexer.peek_t() == Token::Punct(Punct::Semicolon));
        self.lexer.skip_tok();

        let array_count = self.parse_expression()?;

        if !self.lexer.skip_if(&Token::Punct(Punct::RightBracket)) {
            error!(
                self.log,
                "[P0???]: array type: expected ']' to close\n--> {}",
                self.lexer.sync_position()
            );

            return None;
        }

        Some(
            Builder::new(self.storage)
                .create_array_type()
                .with_element(element_type)
                .with_count(array_count)
                .build(),
        )
    }

    fn parse_rest_of_map_type(&mut self, key_type: TypeKey<'a>) -> Option<TypeKey<'a>> {
        assert!(self.lexer.peek_t() == Token::Op(Op::Arrow));
        self.lexer.skip_tok();

        let value_type = self.parse_type()?;

        if !self.lexer.skip_if(&Token::Punct(Punct::RightBracket)) {
            error!(
                self.log,
                "[P0???]: map type: expected ']' to close\n--> {}",
                self.lexer.sync_position()
            );

            return None;
        }

        Some(
            Builder::new(self.storage)
                .create_map_type()
                .with_key(key_type)
                .with_value(value_type)
                .build(),
        )
    }

    fn parse_rest_of_slice_type(&mut self, element_type: TypeKey<'a>) -> Option<TypeKey<'a>> {
        /*
         * The syntax for defining a slice type is as follows:
         * [<type>]
         *
         * The '[' and ']' symbols indicate that the type is a slice.
         */

        assert!(self.lexer.peek_t() == Token::Punct(Punct::RightBracket));
        self.lexer.skip_tok();

        Some(
            Builder::new(self.storage)
                .create_slice_type()
                .with_element(element_type)
                .build(),
        )
    }

    fn parse_array_or_slice_or_map(&mut self) -> Option<TypeKey<'a>> {
        assert!(self.lexer.peek_t() == Token::Punct(Punct::LeftBracket));
        self.lexer.skip_tok();

        let something_type = self.parse_type()?;

        if self.lexer.next_is(&Token::Punct(Punct::Semicolon)) {
            return self.parse_rest_of_array(something_type);
        }

        if self.lexer.next_is(&Token::Op(Op::Arrow)) {
            return self.parse_rest_of_map_type(something_type);
        }

        if self.lexer.next_is(&Token::Punct(Punct::RightBracket)) {
            return self.parse_rest_of_slice_type(something_type);
        }

        error!(
            self.log,
            "[P0???]: type: expected ';', ']', or '->' for array, slice, and map type respectively\n--> {}",
            self.lexer.sync_position()
        );
        info!(
            self.log,
            "[P0???]: array type: syntax hint: [<type>; <length>]"
        );
        info!(self.log, "[P0???]: slice type: syntax hint: [<type>]");
        info!(
            self.log,
            "[P0???]: map   type: syntax hint: [<key_type> -> <value_type>]"
        );

        None
    }

    fn parse_managed_type(&mut self) -> Option<TypeKey<'a>> {
        /*
         * The syntax for defining a managed reference type is as follows:
         * &mut <type>
         * &const <type>
         * &<type>
         *
         * The '&' symbol indicates that the type is managed, and the 'mut' or 'const' keywords
         * indicate whether the type is mutable or immutable.
         * If neither 'mut' nor 'const' is specified, the type is considered immutable
         * by default.
         */

        assert!(self.lexer.peek_t() == Token::Op(Op::BitAnd));
        self.lexer.skip_tok();

        let is_mutable = self.lexer.skip_if(&Token::Keyword(Keyword::Mut));
        if !is_mutable {
            self.lexer.skip_if(&Token::Keyword(Keyword::Const));
        }

        let target = self.parse_type()?;

        Some(
            Builder::new(self.storage)
                .create_managed_type()
                .with_target(target)
                .with_mutability(is_mutable)
                .build(),
        )
    }

    fn parse_unmanaged_type(&mut self) -> Option<TypeKey<'a>> {
        /*
         * The syntax for defining an unmanaged reference type is as follows:
         * *mut <type>
         * *const <type>
         * *<type>
         *
         * The '*' symbol indicates that the type is unmanaged, and the 'mut' or 'const' keywords
         * indicate whether the type is mutable or immutable.
         * If neither 'mut' nor 'const' is specified, the type is considered immutable
         * by default.
         */

        assert!(self.lexer.peek_t() == Token::Op(Op::Mul));
        self.lexer.skip_tok();

        let is_mutable = self.lexer.skip_if(&Token::Keyword(Keyword::Mut));
        if !is_mutable {
            self.lexer.skip_if(&Token::Keyword(Keyword::Const));
        }

        let target = self.parse_type()?;

        Some(
            Builder::new(self.storage)
                .create_unmanaged_type()
                .with_target(target)
                .with_mutability(is_mutable)
                .build(),
        )
    }

    fn parse_function_attributes(&mut self) -> Option<Vec<ExprKey<'a>>> {
        let mut attributes = Vec::new();

        if !self.lexer.skip_if(&Token::Punct(Punct::LeftBracket)) {
            return Some(attributes);
        }

        self.lexer.skip_if(&Token::Punct(Punct::Comma));

        loop {
            if self.lexer.skip_if(&Token::Punct(Punct::RightBracket)) {
                break;
            }

            attributes.push(self.parse_expression()?);

            if !self.lexer.skip_if(&Token::Punct(Punct::Comma)) {
                if self.lexer.skip_if(&Token::Punct(Punct::RightBracket)) {
                    break;
                }
                error!(
                    self.log,
                    "[P0???]: function type: expected ',' or ']' after attribute expression\n--> {}",
                    self.lexer.sync_position()
                );

                return None;
            }
        }

        Some(attributes)
    }

    fn parse_function_parameters(&mut self) -> Option<Vec<FunctionParameter<'a>>> {
        /*
         * Syntax for defining function parameters is as follows:
         *  <parameter> ::= <name>? (':' <type>)? ('=' <expression>)?
         */

        let mut parameters = Vec::new();

        if !self.lexer.skip_if(&Token::Punct(Punct::LeftParen)) {
            return Some(parameters);
        }

        self.lexer.skip_if(&Token::Punct(Punct::Comma));

        loop {
            if self.lexer.skip_if(&Token::Punct(Punct::RightParen)) {
                break;
            }

            let parameter_name = self
                .lexer
                .next_if_name()
                .unwrap_or(Name::new(""))
                .into_name();

            let parameter_type = if self.lexer.skip_if(&Token::Punct(Punct::Colon)) {
                self.parse_type()?
            } else {
                Builder::new(self.storage).get_infer_type()
            };

            let parameter_default = if self.lexer.skip_if(&Token::Op(Op::Set)) {
                Some(self.parse_expression()?)
            } else {
                None
            };

            parameters.push(FunctionParameter::new(
                parameter_name,
                parameter_type,
                parameter_default,
            ));

            if !self.lexer.skip_if(&Token::Punct(Punct::Comma)) {
                if self.lexer.skip_if(&Token::Punct(Punct::RightParen)) {
                    break;
                }
                error!(
                    self.log,
                    "[P0???]: function type: expected ',' or ')' after parameter\n--> {}",
                    self.lexer.sync_position()
                );

                return None;
            }
        }

        Some(parameters)
    }

    fn parse_function_type(&mut self) -> Option<TypeKey<'a>> {
        /*
         * The syntax for defining a function type is as follows:
         * <return_type> ::= "->" <type>
         * <function_type> ::= fn <attributes>? <name>? <parameters>? <return_type>?
         */

        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Fn));
        self.lexer.skip_tok();

        let attributes = self.parse_function_attributes()?;
        let _ignored_name = self.lexer.next_if_name();
        let parameters = self.parse_function_parameters()?;

        let return_type = if self.lexer.skip_if(&Token::Op(Op::Arrow)) {
            self.parse_type()?
        } else {
            Builder::new(self.storage).get_infer_type()
        };

        Some(
            Builder::new(self.storage)
                .create_function_type()
                .add_attributes(attributes)
                .add_parameters(parameters)
                .with_return_type(return_type)
                .build(),
        )
    }

    fn parse_opaque_type(&mut self) -> Option<TypeKey<'a>> {
        /*
         * The syntax for defining an opaque type is as follows:
         * opaque(<string>)
         */

        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Opaque));
        self.lexer.skip_tok();

        if !self.lexer.skip_if(&Token::Punct(Punct::LeftParen)) {
            error!(
                self.log,
                "[P0???]: opaque type: expected '(' after 'opaque'\n--> {}",
                self.lexer.sync_position()
            );
            info!(
                self.log,
                "[P0???]: opaque type: syntax hint: opaque(<string>)"
            );

            return None;
        }

        let Some(opaque_identity) = self.lexer.next_if_string() else {
            error!(
                self.log,
                "[P0???]: opaque type: expected string literal after 'opaque('\n--> {}",
                self.lexer.sync_position()
            );
            info!(
                self.log,
                "[P0???]: opaque type: syntax hint: opaque(<string>)"
            );

            return None;
        };

        if !self.lexer.skip_if(&Token::Punct(Punct::RightParen)) {
            error!(
                self.log,
                "[P0???]: opaque type: expected ')' to close\n--> {}",
                self.lexer.sync_position()
            );
            info!(
                self.log,
                "[P0???]: opaque type: syntax hint: opaque(<string>)"
            );

            return None;
        }

        Builder::new(self.storage).create_opaque_type(opaque_identity)
    }

    fn parse_type_primary(&mut self) -> Option<TypeKey<'a>> {
        let first_token = self.lexer.peek_tok();
        let current_pos = first_token.start();

        let old_generic_type_depth = self.generic_type_depth;
        let must_preserve_generic_depth = !matches!(first_token.token(), Token::Name(_));
        if must_preserve_generic_depth {
            self.generic_type_depth = 0;
        }

        let result = match first_token.into_token() {
            Token::Keyword(Keyword::Bool) => {
                self.lexer.skip_tok();
                Some(Builder::new(self.storage).get_bool())
            }

            Token::Keyword(Keyword::U8) => {
                self.lexer.skip_tok();
                Some(Builder::new(self.storage).get_u8())
            }

            Token::Keyword(Keyword::U16) => {
                self.lexer.skip_tok();
                Some(Builder::new(self.storage).get_u16())
            }

            Token::Keyword(Keyword::U32) => {
                self.lexer.skip_tok();
                Some(Builder::new(self.storage).get_u32())
            }

            Token::Keyword(Keyword::U64) => {
                self.lexer.skip_tok();
                Some(Builder::new(self.storage).get_u64())
            }

            Token::Keyword(Keyword::U128) => {
                self.lexer.skip_tok();
                Some(Builder::new(self.storage).get_u128())
            }

            Token::Keyword(Keyword::I8) => {
                self.lexer.skip_tok();
                Some(Builder::new(self.storage).get_i8())
            }

            Token::Keyword(Keyword::I16) => {
                self.lexer.skip_tok();
                Some(Builder::new(self.storage).get_i16())
            }

            Token::Keyword(Keyword::I32) => {
                self.lexer.skip_tok();
                Some(Builder::new(self.storage).get_i32())
            }

            Token::Keyword(Keyword::I64) => {
                self.lexer.skip_tok();
                Some(Builder::new(self.storage).get_i64())
            }

            Token::Keyword(Keyword::I128) => {
                self.lexer.skip_tok();
                Some(Builder::new(self.storage).get_i128())
            }

            Token::Keyword(Keyword::F8) => {
                self.lexer.skip_tok();
                Some(Builder::new(self.storage).get_f8())
            }

            Token::Keyword(Keyword::F16) => {
                self.lexer.skip_tok();
                Some(Builder::new(self.storage).get_f16())
            }

            Token::Keyword(Keyword::F32) => {
                self.lexer.skip_tok();
                Some(Builder::new(self.storage).get_f32())
            }

            Token::Keyword(Keyword::F64) => {
                self.lexer.skip_tok();
                Some(Builder::new(self.storage).get_f64())
            }

            Token::Keyword(Keyword::F128) => {
                self.lexer.skip_tok();
                Some(Builder::new(self.storage).get_f128())
            }

            Token::Name(name) => self.parse_named_type(name.name()),
            Token::Punct(Punct::LeftBrace) => self.parse_tuple_type(),
            Token::Punct(Punct::LeftBracket) => self.parse_array_or_slice_or_map(),
            Token::Op(Op::BitAnd) => self.parse_managed_type(),
            Token::Op(Op::Mul) => self.parse_unmanaged_type(),
            Token::Keyword(Keyword::Fn) => self.parse_function_type(),
            Token::Keyword(Keyword::Opaque) => self.parse_opaque_type(),

            Token::Integer(int) => {
                error!(
                    self.log,
                    "[P0???]: type: unexpected integer '{}'\n--> {}", int, current_pos
                );

                None
            }

            Token::Float(float) => {
                error!(
                    self.log,
                    "[P0???]: type: unexpected float '{}'\n--> {}", float, current_pos
                );

                None
            }

            Token::Keyword(func) => {
                error!(
                    self.log,
                    "[P0???]: type: unexpected keyword '{}'\n--> {}", func, current_pos
                );

                None
            }

            Token::String(string) => {
                error!(
                    self.log,
                    "[P0???]: type: unexpected string '{}'\n--> {}", string, current_pos
                );

                None
            }

            Token::BString(bstring) => {
                error!(
                    self.log,
                    "[P0???]: type: unexpected bstring '{}'\n--> {}", bstring, current_pos
                );

                None
            }

            Token::Char(char) => {
                error!(
                    self.log,
                    "[P0???]: type: unexpected character '{}'\n--> {}", char, current_pos
                );

                None
            }

            Token::Punct(punc) => {
                error!(
                    self.log,
                    "[P0???]: type: unexpected punctuation '{}'\n--> {}", punc, current_pos
                );

                None
            }

            Token::Op(op) => {
                error!(
                    self.log,
                    "[P0???]: type: unexpected operator '{}'\n--> {}", op, current_pos
                );

                None
            }

            Token::Comment(_) => {
                error!(
                    self.log,
                    "[P0???]: type: unexpected comment\n--> {}", current_pos
                );

                None
            }

            Token::Eof => {
                error!(
                    self.log,
                    "[P0???]: type: unexpected end of file\n--> {}", current_pos
                );

                None
            }

            Token::Illegal => {
                error!(
                    self.log,
                    "[P0???]: type: unexpected invalid token\n--> {}", current_pos
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
        /*
         * The syntax for defining a type is as follows:
         * <type>
         * (<type>)
         *
         * The parentheses may be used for type precedence or grouping,
         * but they are not required for simple types.
         */

        if self.lexer.skip_if(&Token::Punct(Punct::LeftParen)) {
            let inner = self.parse_type();

            if !self.lexer.skip_if(&Token::Punct(Punct::RightParen)) {
                self.set_failed_bit();
                error!(
                    self.log,
                    "[P0???]: type: expected ')' to close\n--> {}",
                    self.lexer.sync_position()
                );

                return None;
            }

            inner.inspect(|v| v.add_parentheses(self.storage))
        } else {
            let Some(the_type) = self.parse_type_primary() else {
                self.set_failed_bit();
                return None;
            };

            let Some(refinement) = self.parse_refinement_options() else {
                self.set_failed_bit();
                return None;
            };

            if refinement.has_any() {
                return Some(
                    Builder::new(self.storage)
                        .create_refinement_type()
                        .with_base(the_type)
                        .with_width(refinement.width)
                        .with_minimum(refinement.minimum)
                        .with_maximum(refinement.maximum)
                        .build(),
                );
            }

            Some(the_type)
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
                                                    kind: Dec,
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
                                                    kind: Dec,
                                                },
                                            ),
                                            min: Some(
                                                IntegerLit {
                                                    value: 1,
                                                    kind: Dec,
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
            kind: Dec,
        },
    ),
    min: None,
    max: None,
}"#;

    let lexer = Lexer::new(source.as_bytes(), "test").expect("Failed to create lexer");
    let mut storage = Storage::new();
    let mut logger = slog::Logger::root(slog::Discard, slog::o!());

    let mut parser = Parser::new(lexer, &mut storage, &mut logger);
    let model = parser.parse_type().expect("Failed to parse source");
    assert!(!parser.has_failed(), "Parsing failed with errors");

    let tree = model.as_printable(&storage);
    let serialized = format!("{:#?}", tree);

    assert_eq!(
        serialized, expected,
        "Parsed type does not match expected structure"
    );
}
