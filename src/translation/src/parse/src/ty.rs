use super::parse::Parser;
use crate::diagnosis::SyntaxErr;

use nitrate_parsetree::{
    kind::{
        ArrayType, Bool, Exclusivity, Expr, Float8, Float16, Float32, Float64, Float128,
        FunctionType, Int8, Int16, Int32, Int64, Int128, LatentType, Lifetime, Mutability,
        OpaqueType, ReferenceType, RefinementType, SliceType, TupleType, Type, TypeParentheses,
        TypePath, TypePathSegment, TypeSyntaxError, UInt8, UInt16, UInt32, UInt64, UInt128,
    },
    tag::{intern_lifetime_name, intern_opaque_type_name},
};
use nitrate_tokenize::Token;

#[allow(unused_imports)]
use nitrate_tokenize::Lexer;

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
            assert!(this.lexer.peek_t() == Token::OpenBracket);
            this.lexer.skip_tok();

            let mut minimum_bound = None;
            let mut maximum_bound = None;

            if !this.lexer.skip_if(&Token::Colon) {
                let minimum = this.parse_expression();

                if !this.lexer.skip_if(&Token::Colon) {
                    let bug = SyntaxErr::ExpectedColon(this.lexer.peek_pos());
                    this.log.report(&bug);
                }

                minimum_bound = Some(minimum);
            }

            if !this.lexer.skip_if(&Token::CloseBracket) {
                let maximum = this.parse_expression();

                if !this.lexer.skip_if(&Token::CloseBracket) {
                    let bug = SyntaxErr::ExpectedCloseBracket(this.lexer.peek_pos());
                    this.log.report(&bug);
                }

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
        assert!(self.lexer.peek_t() == Token::OpenBracket);
        self.lexer.skip_tok();

        let element_type = self.parse_type();

        if self.lexer.skip_if(&Token::CloseBracket) {
            return Type::SliceType(Box::new(SliceType { element_type }));
        }

        if !self.lexer.skip_if(&Token::Semi) {
            let bug = SyntaxErr::ExpectedSemicolon(self.lexer.peek_pos());
            self.log.report(&bug);
        }

        let len = self.parse_expression();

        if !self.lexer.skip_if(&Token::CloseBracket) {
            let bug = SyntaxErr::ExpectedCloseBracket(self.lexer.peek_pos());
            self.log.report(&bug);

            self.lexer.skip_while(&Token::CloseBracket);
        }

        Type::ArrayType(Box::new(ArrayType { element_type, len }))
    }

    fn parse_reference_type(&mut self) -> ReferenceType {
        assert!(self.lexer.peek_t() == Token::And);
        self.lexer.skip_tok();

        let mut exclusive = None;
        let mut mutability = None;

        let lifetime = if self.lexer.next_is(&Token::SingleQuote) {
            Some(self.parse_lifetime())
        } else {
            None
        };

        if self.lexer.skip_if(&Token::Poly) {
            exclusive = Some(Exclusivity::Poly);
        } else if self.lexer.skip_if(&Token::Iso) {
            exclusive = Some(Exclusivity::Iso);
        }

        if self.lexer.skip_if(&Token::Mut) {
            mutability = Some(Mutability::Mut);
        } else if self.lexer.skip_if(&Token::Const) {
            mutability = Some(Mutability::Const);
        }

        let to = self.parse_type();

        ReferenceType {
            lifetime,
            exclusivity: exclusive,
            mutability,
            to,
        }
    }

    fn parse_function_type(&mut self) -> FunctionType {
        assert!(self.lexer.peek_t() == Token::Fn);
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();
        let parameters = self.parse_function_parameters();

        let return_type = if self.lexer.skip_if(&Token::Minus) {
            if !self.lexer.skip_if(&Token::Gt) {
                let bug = SyntaxErr::ExpectedArrow(self.lexer.peek_pos());
                self.log.report(&bug);
            }

            Some(self.parse_type())
        } else {
            None
        };

        FunctionType {
            parameters,
            return_type,
            attributes,
        }
    }

    fn parse_opaque_type(&mut self) -> Type {
        assert!(self.lexer.peek_t() == Token::Opaque);
        self.lexer.skip_tok();

        if !self.lexer.skip_if(&Token::OpenParen) {
            let bug = SyntaxErr::ExpectedOpenParen(self.lexer.peek_pos());
            self.log.report(&bug);
        }

        let opaque_identity = self.lexer.next_if_string().unwrap_or_else(|| {
            let bug = SyntaxErr::OpaqueTypeMissingName(self.lexer.peek_pos());
            self.log.report(&bug);
            "".into()
        });

        let name = intern_opaque_type_name(opaque_identity);

        if !self.lexer.skip_if(&Token::CloseParen) {
            let bug = SyntaxErr::ExpectedCloseParen(self.lexer.peek_pos());
            self.log.report(&bug);
        }

        Type::OpaqueType(OpaqueType { name })
    }

    fn parse_lifetime(&mut self) -> Lifetime {
        assert!(self.lexer.peek_t() == Token::SingleQuote);
        self.lexer.skip_tok();

        if self.lexer.skip_if(&Token::Static) {
            return Lifetime {
                name: intern_lifetime_name("static".to_string()),
            };
        }

        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxErr::ReferenceTypeExpectedLifetimeName(self.lexer.peek_pos());
            self.log.report(&bug);
            "".into()
        });

        Lifetime {
            name: intern_lifetime_name(name),
        }
    }

    fn parse_type_primitive(&mut self) -> Type {
        match self.lexer.next_t() {
            Token::Bool => Type::Bool(Bool),
            Token::U8 => Type::UInt8(UInt8),
            Token::U16 => Type::UInt16(UInt16),
            Token::U32 => Type::UInt32(UInt32),
            Token::U64 => Type::UInt64(UInt64),
            Token::U128 => Type::UInt128(UInt128),
            Token::I8 => Type::Int8(Int8),
            Token::I16 => Type::Int16(Int16),
            Token::I32 => Type::Int32(Int32),
            Token::I64 => Type::Int64(Int64),
            Token::I128 => Type::Int128(Int128),
            Token::F8 => Type::Float8(Float8),
            Token::F16 => Type::Float16(Float16),
            Token::F32 => Type::Float32(Float32),
            Token::F64 => Type::Float64(Float64),
            Token::F128 => Type::Float128(Float128),

            _ => Type::SyntaxError(TypeSyntaxError),
        }
    }

    pub(crate) fn parse_type_path(&mut self) -> TypePath {
        fn parse_double_colon(this: &mut Parser) -> bool {
            if !this.lexer.skip_if(&Token::Colon) {
                return false;
            }

            if !this.lexer.skip_if(&Token::Colon) {
                let bug = SyntaxErr::ExpectedColon(this.lexer.peek_pos());
                this.log.report(&bug);
                return false;
            }

            true
        }

        assert!(matches!(self.lexer.peek_t(), Token::Name(_) | Token::Colon));

        let mut segments = Vec::new();
        let mut prev_scope = false;
        let mut already_reported_too_many_segments = false;

        if parse_double_colon(self) {
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

            prev_scope = parse_double_colon(self);
        }

        assert_ne!(segments.len(), 0);

        TypePath {
            segments,
            resolved: None,
        }
    }

    fn parse_type_primary(&mut self) -> Type {
        let current_pos = self.lexer.current_pos();

        match self.lexer.peek_t() {
            Token::Bool
            | Token::U8
            | Token::U16
            | Token::U32
            | Token::U64
            | Token::U128
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

            Token::Name(_) | Token::Colon => Type::TypePath(Box::new(self.parse_type_path())),

            Token::OpenBracket => self.parse_array_or_slice(),
            Token::And => Type::ReferenceType(Box::new(self.parse_reference_type())),
            Token::Fn => Type::FunctionType(Box::new(self.parse_function_type())),
            Token::Opaque => self.parse_opaque_type(),

            Token::OpenBrace | Token::Unsafe | Token::Safe => {
                Type::LatentType(Box::new(LatentType {
                    body: self.parse_block(),
                }))
            }

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
        let mut already_reported_too_many_elements = false;

        while !self.lexer.skip_if(&Token::CloseParen) {
            if self.lexer.is_eof() {
                let bug = SyntaxErr::TupleTypeExpectedEnd(self.lexer.peek_pos());
                self.log.report(&bug);
                break;
            }

            const MAX_TUPLE_ELEMENTS: usize = 65_536;

            if !already_reported_too_many_elements && element_types.len() >= MAX_TUPLE_ELEMENTS {
                already_reported_too_many_elements = true;

                let bug = SyntaxErr::TupleTypeElementLimit(self.lexer.peek_pos());
                self.log.report(&bug);
            }

            let element = self.parse_type();
            element_types.push(element);

            if !self.lexer.skip_if(&Token::Comma) && !self.lexer.next_is(&Token::CloseParen) {
                let bug = SyntaxErr::TupleTypeExpectedEnd(self.lexer.peek_pos());
                self.log.report(&bug);

                self.lexer.skip_while(&Token::CloseParen);
                break;
            }
        }

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

            let result = match self.lexer.next_t() {
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
