use std::ops::Deref;

use crate::bugs::SyntaxBug;

use super::parse::Parser;
use interned_string::IString;
use log::{error, info};
use nitrate_parsetree::kind::{
    ArrayType, Expr, FunctionParameter, FunctionType, FunctionTypeParameter, GenericArgument,
    GenericType, Lifetime, MapType, Path, ReferenceType, RefinementType, SliceType, TupleType,
    Type,
};
use nitrate_tokenize::{Keyword, Op, Punct, Token};

#[allow(unused_imports)]
use nitrate_tokenize::Lexer;

#[derive(Default)]
struct RefinementOptions {
    minimum: Option<Expr>,
    maximum: Option<Expr>,
    width: Option<Expr>,
}

impl RefinementOptions {
    fn has_any(&self) -> bool {
        self.minimum.is_some() || self.maximum.is_some() || self.width.is_some()
    }
}

impl Parser<'_, '_> {
    fn parse_refinement_range(&mut self) -> Option<(Option<Expr>, Option<Expr>)> {
        // TODO: Cleanup

        assert!(self.lexer.peek_t() == Token::Punct(Punct::LeftBracket));
        self.lexer.skip_tok();

        let mut minimum_bound = None;
        let mut maximum_bound = None;

        if !self.lexer.skip_if(&Token::Punct(Punct::Colon)) {
            let minimum = self.parse_expression();

            if !self.lexer.skip_if(&Token::Punct(Punct::Colon)) {
                error!(
                    "[P0???]: refinement type: expected ':' after minimum range constraint\n--> {}",
                    self.lexer.position()
                );

                return None;
            }

            minimum_bound = Some(minimum);
        }

        if !self.lexer.skip_if(&Token::Punct(Punct::RightBracket)) {
            let maximum = self.parse_expression();

            if !self.lexer.skip_if(&Token::Punct(Punct::RightBracket)) {
                error!(
                    "[P0???]: refinement type: expected ']' to close range constraints\n--> {}",
                    self.lexer.position()
                );

                return None;
            }

            maximum_bound = Some(maximum);
        }

        Some((minimum_bound, maximum_bound))
    }

    fn parse_refinement_options(&mut self) -> Option<RefinementOptions> {
        // TODO: Cleanup

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

        let width = self.parse_expression();

        if !self.lexer.skip_if(&Token::Punct(Punct::Colon)) {
            return Some(RefinementOptions {
                width: Some(width),
                minimum: None,
                maximum: None,
            });
        }

        if !self.lexer.next_is(&Token::Punct(Punct::LeftBracket)) {
            error!(
                "[P0???]: refinement type: expected '[' for range constraints\n--> {}",
                self.lexer.position()
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

    fn parse_generic_argument(&mut self) -> Option<GenericArgument> {
        // TODO: Cleanup

        let mut argument_name: Option<IString> = None;

        if let Token::Name(name) = self.lexer.peek_t() {
            /* Named generic argument syntax is ambiguous,
             * an identifier can be followed by a colon
             * to indicate a named argument (followed by the expression value).
             * However, if it is not followed by a colon, the identifier is
             * to be parsed as an expression.
             */
            let rewind_pos = self.lexer.position();
            self.lexer.skip_tok();

            if self.lexer.skip_if(&Token::Punct(Punct::Colon)) {
                argument_name = Some(name);
            } else {
                self.lexer.rewind(rewind_pos);
            }
        }

        let argument_value = self.parse_type();

        Some(GenericArgument {
            name: argument_name,
            value: Some(argument_value),
        })
    }

    fn parse_generic_arguments(&mut self) -> Option<Vec<GenericArgument>> {
        // TODO: Cleanup

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

            if self.lexer.skip_if(&Token::Op(Op::BitRor)) {
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
                    || self.lexer.next_is(&Token::Op(Op::BitRor));

                if !any_terminator {
                    error!(
                        "[P0???]: generic type: expected ',' or '>' after generic argument\n--> {}",
                        self.lexer.position()
                    );

                    return None;
                }
            }
        }

        Some(arguments)
    }

    fn parse_type_name(&mut self) -> Option<Type> {
        // TODO: Cleanup

        match self.lexer.peek_t() {
            Token::Name(name) if name.deref() == "_" => {
                self.lexer.skip_tok();
                return Some(Type::InferType);
            }
            _ => {}
        }

        let mut path = Vec::new();
        let mut last_was_scope = false;

        let pos = self.lexer.position();

        loop {
            match self.lexer.peek_t() {
                Token::Name(name) => {
                    if last_was_scope || path.is_empty() {
                        self.lexer.skip_tok();
                        path.push(name);

                        last_was_scope = false;
                    } else {
                        break;
                    }
                }

                Token::Op(Op::Scope) => {
                    if last_was_scope {
                        break;
                    }

                    self.lexer.skip_tok();

                    // For absolute scoping
                    if path.is_empty() {
                        path.push(IString::default());
                    }

                    last_was_scope = true;
                }

                _ => {
                    break;
                }
            }
        }

        if path.is_empty() {
            error!("[P????]: type name: expected type name\n--> {pos}");
            return None;
        }

        if last_was_scope {
            error!(
                "[P0???]: type name: unexpected '::' at end of type name\n--> {}",
                self.lexer.position()
            );

            return None;
        }

        if !self.lexer.next_is(&Token::Op(Op::LogicLt)) {
            return Some(Type::TypeName(Box::new(Path { path: path.into() })));
        }

        let is_already_parsing_generic_type = self.generic_type_depth != 0;
        let generic_arguments = self.parse_generic_arguments()?;

        if !is_already_parsing_generic_type {
            match self.generic_type_depth {
                0 => {}
                -1 => {
                    error!(
                        "[P0???]: generic type: unexpected '>' delimiter\n--> {}",
                        self.lexer.position()
                    );

                    return None;
                }
                _ => {
                    error!(
                        "[P0???]: generic type: unexpected '>>' delimiter\n--> {}",
                        self.lexer.position()
                    );

                    return None;
                }
            }

            self.generic_type_depth = 0;
            self.generic_type_suffix_terminator_ambiguity = false;
        }

        Some(Type::GenericType(Box::new(GenericType {
            basis_type: Type::TypeName(Box::new(Path { path: path.into() })),
            arguments: generic_arguments,
        })))
    }

    fn parse_rest_of_array(&mut self, element_type: Type) -> Option<Type> {
        // TODO: Cleanup

        assert!(self.lexer.peek_t() == Token::Punct(Punct::Semicolon));
        self.lexer.skip_tok();

        let array_count = self.parse_expression();

        if !self.lexer.skip_if(&Token::Punct(Punct::RightBracket)) {
            error!(
                "[P0???]: array type: expected ']' to close\n--> {}",
                self.lexer.position()
            );

            return None;
        }

        Some(Type::ArrayType(Box::new(ArrayType {
            element_type: element_type,
            len: array_count,
        })))
    }

    fn parse_rest_of_map_type(&mut self, key_type: Type) -> Option<Type> {
        // TODO: Cleanup

        assert!(self.lexer.peek_t() == Token::Op(Op::Arrow));
        self.lexer.skip_tok();

        let value_type = self.parse_type();

        if !self.lexer.skip_if(&Token::Punct(Punct::RightBracket)) {
            error!(
                "[P0???]: map type: expected ']' to close\n--> {}",
                self.lexer.position()
            );

            return None;
        }

        Some(Type::MapType(Box::new(MapType {
            key_type,
            value_type,
        })))
    }

    fn parse_rest_of_slice_type(&mut self, element_type: Type) -> Option<Type> {
        // TODO: Cleanup

        assert!(self.lexer.peek_t() == Token::Punct(Punct::RightBracket));
        self.lexer.skip_tok();

        Some(Type::SliceType(Box::new(SliceType { element_type })))
    }

    fn parse_array_or_slice_or_map(&mut self) -> Option<Type> {
        // TODO: Cleanup

        assert!(self.lexer.peek_t() == Token::Punct(Punct::LeftBracket));
        self.lexer.skip_tok();

        let something_type = self.parse_type();

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
            "[P0???]: type: expected ';', ']', or '->' for array, slice, and map type respectively\n--> {}",
            self.lexer.position()
        );
        info!("[P0???]: array type: syntax hint: [<type>; <length>]");
        info!("[P0???]: slice type: syntax hint: [<type>]");
        info!("[P0???]: map   type: syntax hint: [<key_type> -> <value_type>]");

        None
    }

    fn parse_reference_type(&mut self) -> Option<Type> {
        // TODO: Cleanup

        assert!(self.lexer.peek_t() == Token::Op(Op::BitAnd));
        self.lexer.skip_tok();

        let mut exclusive = None;
        if self.lexer.skip_if(&Token::Keyword(Keyword::Poly)) {
            exclusive = Some(false);
        } else if self.lexer.skip_if(&Token::Keyword(Keyword::Iso)) {
            exclusive = Some(true);
        }

        let mut mutability = None;
        if self.lexer.skip_if(&Token::Keyword(Keyword::Mut)) {
            mutability = Some(true);
        } else if self.lexer.skip_if(&Token::Keyword(Keyword::Const)) {
            mutability = Some(false);
        }

        let to = self.parse_type();

        Some(Type::ReferenceType(Box::new(ReferenceType {
            lifetime: Some(Lifetime::CollectorManaged),
            mutability,
            exclusive,
            to,
        })))
    }

    fn parse_pointer_type(&mut self) -> Option<Type> {
        // TODO: Cleanup

        assert!(self.lexer.peek_t() == Token::Op(Op::Mul));
        self.lexer.skip_tok();

        let mut exclusive = None;
        if self.lexer.skip_if(&Token::Keyword(Keyword::Poly)) {
            exclusive = Some(false);
        } else if self.lexer.skip_if(&Token::Keyword(Keyword::Iso)) {
            exclusive = Some(true);
        }

        let mut mutability = None;
        if self.lexer.skip_if(&Token::Keyword(Keyword::Mut)) {
            mutability = Some(true);
        } else if self.lexer.skip_if(&Token::Keyword(Keyword::Const)) {
            mutability = Some(false);
        }

        let to = self.parse_type();

        Some(Type::ReferenceType(Box::new(ReferenceType {
            lifetime: None,
            exclusive,
            mutability,
            to,
        })))
    }

    fn parse_function_type_parameter(&mut self) -> FunctionTypeParameter {
        // TODO: Cleanup

        let attributes = self.parse_attributes();

        let name = self.lexer.next_if_name();

        let param_type = if self.lexer.skip_if(&Token::Punct(Punct::Colon)) {
            Some(self.parse_type())
        } else {
            None
        };

        let default = if self.lexer.skip_if(&Token::Op(Op::Set)) {
            Some(self.parse_expression())
        } else {
            None
        };

        FunctionTypeParameter {
            name,
            param_type,
            default,
            attributes,
        }
    }

    fn parse_function_type_parameters(&mut self) -> Vec<FunctionTypeParameter> {
        // TODO: Cleanup

        let mut params = Vec::new();
        if !self.lexer.skip_if(&Token::Punct(Punct::LeftParen)) {
            return params;
        }

        self.lexer.skip_if(&Token::Punct(Punct::Comma));

        let mut already_reported_too_many_parameters = false;

        while !self.lexer.skip_if(&Token::Punct(Punct::RightParen)) {
            if self.lexer.is_eof() {
                let bug = SyntaxBug::FunctionParametersExpectedEnd(self.lexer.peek_pos());
                self.bugs.push(&bug);
                break;
            }

            const MAX_FUNCTION_PARAMETERS: usize = 65_536;

            if !already_reported_too_many_parameters && params.len() >= MAX_FUNCTION_PARAMETERS {
                already_reported_too_many_parameters = true;

                let bug = SyntaxBug::FunctionParameterLimit(self.lexer.peek_pos());
                self.bugs.push(&bug);
            }

            let param = self.parse_function_type_parameter();
            params.push(param);

            if !self.lexer.skip_if(&Token::Punct(Punct::Comma))
                && !self.lexer.next_is(&Token::Punct(Punct::RightParen))
            {
                let bug = SyntaxBug::FunctionParametersExpectedEnd(self.lexer.peek_pos());
                self.bugs.push(&bug);
                self.lexer.skip_while(&Token::Punct(Punct::RightParen));
                break;
            }
        }

        params
    }

    fn parse_function_type(&mut self) -> Option<Type> {
        // TODO: Cleanup

        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Fn));
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();
        let _ignored_name = self.lexer.next_if_name();
        let parameters = self.parse_function_type_parameters();

        let return_type = if self.lexer.skip_if(&Token::Op(Op::Arrow)) {
            Some(self.parse_type())
        } else {
            None
        };

        Some(Type::FunctionType(Box::new(FunctionType {
            parameters,
            return_type,
            attributes,
        })))
    }

    fn parse_opaque_type(&mut self) -> Option<Type> {
        // TODO: Cleanup

        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Opaque));
        self.lexer.skip_tok();

        if !self.lexer.skip_if(&Token::Punct(Punct::LeftParen)) {
            error!(
                "[P0???]: opaque type: expected '(' after 'opaque'\n--> {}",
                self.lexer.position()
            );
            info!("[P0???]: opaque type: syntax hint: opaque(<string>)");

            return None;
        }

        let Some(opaque_identity) = self.lexer.next_if_string() else {
            error!(
                "[P0???]: opaque type: expected string literal after 'opaque('\n--> {}",
                self.lexer.position()
            );
            info!("[P0???]: opaque type: syntax hint: opaque(<string>)");

            return None;
        };

        if !self.lexer.skip_if(&Token::Punct(Punct::RightParen)) {
            error!(
                "[P0???]: opaque type: expected ')' to close\n--> {}",
                self.lexer.position()
            );
            info!("[P0???]: opaque type: syntax hint: opaque(<string>)");

            return None;
        }

        Some(Type::OpaqueType(opaque_identity))
    }

    fn parse_type_primary(&mut self) -> Option<Type> {
        // TODO: Cleanup

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
                Some(Type::Bool)
            }

            Token::Keyword(Keyword::U8) => {
                self.lexer.skip_tok();
                Some(Type::UInt8)
            }

            Token::Keyword(Keyword::U16) => {
                self.lexer.skip_tok();
                Some(Type::UInt16)
            }

            Token::Keyword(Keyword::U32) => {
                self.lexer.skip_tok();
                Some(Type::UInt32)
            }

            Token::Keyword(Keyword::U64) => {
                self.lexer.skip_tok();
                Some(Type::UInt64)
            }

            Token::Keyword(Keyword::U128) => {
                self.lexer.skip_tok();
                Some(Type::UInt128)
            }

            Token::Keyword(Keyword::I8) => {
                self.lexer.skip_tok();
                Some(Type::Int8)
            }

            Token::Keyword(Keyword::I16) => {
                self.lexer.skip_tok();
                Some(Type::Int16)
            }

            Token::Keyword(Keyword::I32) => {
                self.lexer.skip_tok();
                Some(Type::Int32)
            }

            Token::Keyword(Keyword::I64) => {
                self.lexer.skip_tok();
                Some(Type::Int64)
            }

            Token::Keyword(Keyword::I128) => {
                self.lexer.skip_tok();
                Some(Type::Int128)
            }

            Token::Keyword(Keyword::F8) => {
                self.lexer.skip_tok();
                Some(Type::Float8)
            }

            Token::Keyword(Keyword::F16) => {
                self.lexer.skip_tok();
                Some(Type::Float16)
            }

            Token::Keyword(Keyword::F32) => {
                self.lexer.skip_tok();
                Some(Type::Float32)
            }

            Token::Keyword(Keyword::F64) => {
                self.lexer.skip_tok();
                Some(Type::Float64)
            }

            Token::Keyword(Keyword::F128) => {
                self.lexer.skip_tok();
                Some(Type::Float128)
            }

            Token::Name(_) | Token::Op(Op::Scope) => self.parse_type_name(),
            Token::Punct(Punct::LeftBracket) => self.parse_array_or_slice_or_map(),
            Token::Op(Op::BitAnd) => self.parse_reference_type(),
            Token::Op(Op::Mul) => self.parse_pointer_type(),

            Token::Punct(Punct::LeftBrace) => {
                let block = self.parse_block();
                Some(Type::LatentType(Box::new(block)))
            }

            Token::Keyword(Keyword::Fn) => self.parse_function_type(),
            Token::Keyword(Keyword::Opaque) => self.parse_opaque_type(),

            Token::Integer(int) => {
                self.lexer.skip_tok();
                error!("[P0???]: type: unexpected integer '{int}'\n--> {current_pos}");

                None
            }

            Token::Float(float) => {
                self.lexer.skip_tok();
                error!("[P0???]: type: unexpected float '{float}'\n--> {current_pos}");

                None
            }

            Token::Keyword(func) => {
                self.lexer.skip_tok();
                error!("[P0???]: type: unexpected keyword '{func}'\n--> {current_pos}");

                None
            }

            Token::String(string) => {
                self.lexer.skip_tok();
                error!("[P0???]: type: unexpected string '{string}'\n--> {current_pos}");

                None
            }

            Token::BString(bstring) => {
                self.lexer.skip_tok();
                error!("[P0???]: type: unexpected bstring '{bstring:?}'\n--> {current_pos}");

                None
            }

            Token::Punct(punc) => {
                self.lexer.skip_tok();
                error!("[P0???]: type: unexpected punctuation '{punc}'\n--> {current_pos}");

                None
            }

            Token::Op(op) => {
                self.lexer.skip_tok();
                error!("[P0???]: type: unexpected operator '{op}'\n--> {current_pos}");

                None
            }

            Token::Comment(_) => {
                self.lexer.skip_tok();
                error!("[P0???]: type: unexpected comment\n--> {current_pos}");

                None
            }

            Token::Eof => {
                self.lexer.skip_tok();
                error!("[P0???]: type: unexpected end of file\n--> {current_pos}");

                None
            }
        };

        if must_preserve_generic_depth {
            self.generic_type_depth = old_generic_type_depth;
        }

        result
    }

    pub(crate) fn parse_type(&mut self) -> Type {
        // TODO: Cleanup

        if self.lexer.skip_if(&Token::Punct(Punct::LeftParen)) {
            if self.lexer.skip_if(&Token::Punct(Punct::RightParen)) {
                return Type::TupleType(Box::new(TupleType {
                    element_types: Vec::new(),
                }));
            }

            let inner = self.parse_type();

            let result = match self.lexer.next_t() {
                Token::Punct(Punct::RightParen) => Type::Parentheses(Box::new(inner)),

                Token::Punct(Punct::Comma) => {
                    let mut element_types = Vec::from([inner]);

                    loop {
                        if self.lexer.skip_if(&Token::Punct(Punct::RightParen)) {
                            break;
                        }

                        let element = self.parse_type();
                        element_types.push(element);

                        if !self.lexer.skip_if(&Token::Punct(Punct::Comma)) {
                            if self.lexer.skip_if(&Token::Punct(Punct::RightParen)) {
                                break;
                            }

                            error!(
                                "[P0???]: tuple type: expected ',' or ')' after element type\n--> {}",
                                self.lexer.position()
                            );
                            info!("[P0???]: tuple type: syntax hint: (<type1>, <type2>, ... )");

                            return Type::SyntaxError;
                        }
                    }

                    Type::TupleType(Box::new(TupleType { element_types }))
                }

                _ => {
                    self.set_failed_bit();
                    error!(
                        "[P0???]: type: expected ')' or ',' after type \n--> {}",
                        self.lexer.position()
                    );

                    Type::SyntaxError
                }
            };

            return result;
        }

        let Some(the_type) = self.parse_type_primary() else {
            self.set_failed_bit();
            return Type::SyntaxError;
        };

        let Some(refinement) = self.parse_refinement_options() else {
            self.set_failed_bit();
            return Type::SyntaxError;
        };

        if refinement.has_any() {
            return Type::RefinementType(Box::new(RefinementType {
                basis_type: the_type,
                width: refinement.width,
                minimum: refinement.minimum,
                maximum: refinement.maximum,
            }));
        }

        the_type
    }
}
