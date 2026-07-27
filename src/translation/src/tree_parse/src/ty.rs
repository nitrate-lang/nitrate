use super::parse::Parser;
use crate::diagnosis::SyntaxErr;

use nitrate_nstring::NString;
use nitrate_token::Token;
use nitrate_tree::ast::{
    ArrayType, Bool, Expr, Float32, Float64, FuncTypeParam, FuncTypeParams, FunctionType, Int8, Int16, Int32, Int64,
    Int128, LatentType, Lifetime, PointerType, ReferenceType, RefinementType, SliceType, TupleType, Type,
    TypeParentheses, TypePath, TypePathSegment, TypeSyntaxError, UInt8, UInt16, UInt32, UInt64, UInt128, USize,
};

#[derive(Default)]
struct RefinementOptions {
    minimum: Option<Expr>,
    maximum: Option<Expr>,
    width: Option<Expr>,
}

impl RefinementOptions {
    fn is_none(&self) -> bool {
        self.minimum.is_none() && self.maximum.is_none() && self.width.is_none()
    }
}

impl Parser<'_, '_> {
    fn parse_refinement_options(&mut self) -> RefinementOptions {
        fn parse_refinement_range(this: &mut Parser) -> (Option<Expr>, Option<Expr>) {
            assert!(this.lexer.peek_tok().token == Token::OpenBracket);
            this.lexer.skip_tok();

            let mut minimum_bound = None;
            let mut maximum_bound = None;

            if !this.lexer.skip_if(&Token::Colon) {
                let minimum = this.parse_expression();

                this.expect_colon();

                minimum_bound = Some(minimum);
            }

            if !this.lexer.skip_if(&Token::CloseBracket) {
                let maximum = this.parse_expression();

                this.expect_close_bracket();

                maximum_bound = Some(maximum);
            }

            (minimum_bound, maximum_bound)
        }

        if !self.lexer.skip_if(&Token::Colon) {
            return RefinementOptions::default();
        }

        if self.lexer.next_is(&Token::OpenBracket) {
            let (minimum, maximum) = parse_refinement_range(self);

            return RefinementOptions {
                minimum,
                maximum,
                width: None,
            };
        }

        let width = self.parse_expression();

        if !self.lexer.skip_if(&Token::Colon) {
            return RefinementOptions {
                width: Some(width),
                minimum: None,
                maximum: None,
            };
        }

        if !self.lexer.next_is(&Token::OpenBracket) {
            let bug = SyntaxErr::ExpectedOpenBracket(self.lexer.peek_pos());
            self.log.report(&bug);

            self.parse_expression(); // Skip

            return RefinementOptions {
                width: Some(width),
                minimum: None,
                maximum: None,
            };
        }

        let (minimum, maximum) = parse_refinement_range(self);

        RefinementOptions {
            width: Some(width),
            minimum,
            maximum,
        }
    }

    fn parse_array_or_slice(&mut self) -> Type {
        assert!(self.lexer.peek_tok().token == Token::OpenBracket);
        self.lexer.skip_tok();

        let element_type = self.parse_type();

        if self.lexer.skip_if(&Token::CloseBracket) {
            return Type::SliceType(Box::new(SliceType { element_type }));
        }

        self.expect_semicolon();

        let len = self.parse_expression();

        self.expect_close_bracket();

        Type::ArrayType(Box::new(ArrayType { element_type, len }))
    }

    fn parse_reference_type(&mut self) -> ReferenceType {
        assert!(self.lexer.peek_tok().token == Token::And);
        self.lexer.skip_tok();

        let lifetime = if self.lexer.next_is(&Token::SingleQuote) {
            Some(self.parse_lifetime())
        } else {
            None
        };

        let exclusivity = self.parse_exclusivity();
        let mutability = self.parse_mutability();

        let to = self.parse_type();

        ReferenceType {
            lifetime,
            exclusivity,
            mutability,
            to,
        }
    }

    fn parse_pointer_type(&mut self) -> PointerType {
        assert!(self.lexer.peek_tok().token == Token::Star);
        self.lexer.skip_tok();

        let lifetime = if self.lexer.next_is(&Token::SingleQuote) {
            Some(self.parse_lifetime())
        } else {
            None
        };

        let exclusivity = self.parse_exclusivity();
        let mutability = self.parse_mutability();

        let to = self.parse_type();

        PointerType {
            lifetime,
            exclusivity,
            mutability,
            to,
        }
    }

    fn parse_function_type_parameters(&mut self) -> FuncTypeParams {
        fn parse_function_parameter(this: &mut Parser) -> FuncTypeParam {
            let attributes = this.parse_attributes();

            let name = this.lexer.next_if_name().unwrap_or_else(|| {
                let bug = SyntaxErr::FunctionParameterMissingName(this.lexer.peek_pos());
                this.log.report(&bug);
                "".into()
            });

            let name = NString::from(name);

            this.expect_colon();

            let ty = this.parse_type();

            FuncTypeParam { attributes, name, ty }
        }

        self.expect_open_paren();

        let eof = SyntaxErr::FunctionParametersExpectedEnd(self.lexer.peek_pos());
        let limit = SyntaxErr::FunctionParameterLimit(self.lexer.peek_pos());
        let end = SyntaxErr::FunctionParametersExpectedEnd(self.lexer.peek_pos());

        let params = self.parse_comma_separated_list(
            &Token::CloseParen,
            65_536,
            true,
            eof,
            limit,
            end,
            parse_function_parameter,
        );

        params
    }

    fn parse_function_type(&mut self) -> FunctionType {
        assert!(self.lexer.peek_tok().token == Token::Fn);
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();
        let parameters = self.parse_function_type_parameters();

        let return_type = self.parse_return_type_arrow();

        FunctionType {
            parameters,
            return_type,
            attributes,
        }
    }

    fn parse_lifetime(&mut self) -> Lifetime {
        assert!(self.lexer.peek_tok().token == Token::SingleQuote);
        self.lexer.skip_tok();

        if self.lexer.skip_if(&Token::Static) {
            return Lifetime {
                name: NString::from("static".to_string()),
            };
        }

        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxErr::ReferenceTypeExpectedLifetimeName(self.lexer.peek_pos());
            self.log.report(&bug);
            "".into()
        });

        Lifetime {
            name: NString::from(name),
        }
    }

    pub(crate) fn create_type_path(&mut self, segment_name: String) -> TypePath {
        TypePath {
            segments: Vec::from([TypePathSegment {
                name: segment_name,
                type_arguments: None,
            }]),
            resolved_path: None,
        }
    }

    fn parse_type_primitive(&mut self) -> Type {
        match self.lexer.next_tok().token {
            Token::Bool => Type::Bool(Bool),
            Token::U8 => Type::UInt8(UInt8),
            Token::U16 => Type::UInt16(UInt16),
            Token::U32 => Type::UInt32(UInt32),
            Token::U64 => Type::UInt64(UInt64),
            Token::U128 => Type::UInt128(UInt128),
            Token::USize => Type::USize(USize),
            Token::I8 => Type::Int8(Int8),
            Token::I16 => Type::Int16(Int16),
            Token::I32 => Type::Int32(Int32),
            Token::I64 => Type::Int64(Int64),
            Token::I128 => Type::Int128(Int128),
            Token::F8 => Type::TypePath(Box::new(self.create_type_path("f8".to_string()))),
            Token::F16 => Type::TypePath(Box::new(self.create_type_path("f16".to_string()))),
            Token::F32 => Type::Float32(Float32),
            Token::F64 => Type::Float64(Float64),
            Token::F128 => Type::TypePath(Box::new(self.create_type_path("f128".to_string()))),

            _ => Type::SyntaxError(TypeSyntaxError),
        }
    }

    pub(crate) fn parse_type_path(&mut self) -> TypePath {
        assert!(matches!(
            self.lexer.peek_tok().token,
            Token::Name(_) | Token::Colon | Token::SelfType
        ));

        let mut segments = Vec::new();
        let mut prev_scope = false;
        let mut already_reported_too_many_segments = false;

        if self.parse_double_colon() {
            prev_scope = true;

            segments.push(TypePathSegment {
                name: "".into(),
                type_arguments: None,
            });
        }

        while prev_scope || segments.is_empty() {
            if self.lexer.is_eof() {
                let bug = SyntaxErr::PathExpectedNameOrSeparator(self.lexer.peek_pos());
                self.log.report(&bug);
                break;
            }

            const MAX_PATH_SEGMENTS: usize = 65_536;
            if !already_reported_too_many_segments && segments.len() >= MAX_PATH_SEGMENTS {
                already_reported_too_many_segments = true;

                let bug = SyntaxErr::PathSegmentLimit(self.lexer.peek_pos());
                self.log.report(&bug);
            }

            let Some(identifier) = self.lexer.next_if_name() else {
                let bug = SyntaxErr::PathExpectedName(self.lexer.peek_pos());
                self.log.report(&bug);
                break;
            };

            let type_arguments = self.parse_generic_arguments();

            segments.push(TypePathSegment {
                name: identifier,
                type_arguments,
            });

            prev_scope = self.parse_double_colon();
        }

        assert_ne!(segments.len(), 0);

        TypePath {
            segments,
            resolved_path: None,
        }
    }

    fn parse_type_primary(&mut self) -> Type {
        let current_pos = self.lexer.current_pos();

        match self.lexer.peek_tok().token {
            Token::Bool
            | Token::U8
            | Token::U16
            | Token::U32
            | Token::U64
            | Token::U128
            | Token::USize
            | Token::I8
            | Token::I16
            | Token::I32
            | Token::I64
            | Token::I128
            | Token::F8
            | Token::F16
            | Token::F32
            | Token::F64
            | Token::F128 => self.parse_type_primitive(),

            Token::SingleQuote => Type::Lifetime(Box::new(self.parse_lifetime())),

            Token::Name(_) | Token::Colon | Token::SelfType => Type::TypePath(Box::new(self.parse_type_path())),

            Token::OpenBracket => self.parse_array_or_slice(),
            Token::And => Type::ReferenceType(Box::new(self.parse_reference_type())),
            Token::Star => Type::PointerType(Box::new(self.parse_pointer_type())),
            Token::Fn => Type::FunctionType(Box::new(self.parse_function_type())),

            Token::OpenBrace | Token::Unsafe | Token::Safe => Type::LatentType(Box::new(LatentType {
                body: self.parse_block(),
            })),

            _ => {
                self.lexer.skip_tok();

                let log = SyntaxErr::ExpectedType(current_pos);
                self.log.report(&log);

                Type::SyntaxError(TypeSyntaxError)
            }
        }
    }

    fn parse_rest_of_tuple(&mut self, first_element: Type) -> TupleType {
        let mut element_types = Vec::from([first_element]);

        let eof = SyntaxErr::TupleTypeExpectedEnd(self.lexer.peek_pos());
        let limit = SyntaxErr::TupleTypeElementLimit(self.lexer.peek_pos());
        let end = SyntaxErr::TupleTypeExpectedEnd(self.lexer.peek_pos());

        let rest = self.parse_comma_separated_list(&Token::CloseParen, 65_536, false, eof, limit, end, |this| {
            this.parse_type()
        });

        element_types.extend(rest);

        TupleType { element_types }
    }

    pub fn parse_type(&mut self) -> Type {
        if self.lexer.skip_if(&Token::OpenParen) {
            if self.lexer.skip_if(&Token::CloseParen) {
                return Type::TupleType(Box::new(TupleType {
                    element_types: Vec::new(),
                }));
            }

            let inner = self.parse_type();

            let result = match self.lexer.next_tok().token {
                Token::CloseParen => Type::Parentheses(Box::new(TypeParentheses { inner })),

                Token::Comma => {
                    let tuple = self.parse_rest_of_tuple(inner);
                    Type::TupleType(Box::new(tuple))
                }

                _ => {
                    let bug = SyntaxErr::TupleTypeExpectedEnd(self.lexer.peek_pos());
                    self.log.report(&bug);

                    Type::SyntaxError(TypeSyntaxError)
                }
            };

            return result;
        }

        let basis_type = self.parse_type_primary();

        let refine_options = self.parse_refinement_options();
        if refine_options.is_none() {
            return basis_type;
        }

        Type::RefinementType(Box::new(RefinementType {
            basis_type,
            width: refine_options.width,
            minimum: refine_options.minimum,
            maximum: refine_options.maximum,
        }))
    }
}
