use super::parse::Parser;
use crate::bugs::SyntaxBug;

use interned_string::IString;
use nitrate_parsetree::kind::{
    ArrayType, Expr, FunctionType, FunctionTypeParameter, Lifetime, Path, ReferenceType,
    RefinementType, SliceType, TupleType, Type,
};
use nitrate_tokenize::{Keyword, Token};

#[allow(unused_imports)]
use nitrate_tokenize::Lexer;
use smallvec::SmallVec;

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
                    let bug = SyntaxBug::ExpectedColon(this.lexer.peek_pos());
                    this.bugs.push(&bug);
                }

                minimum_bound = Some(minimum);
            }

            if !this.lexer.skip_if(&Token::CloseBracket) {
                let maximum = this.parse_expression();

                if !this.lexer.skip_if(&Token::CloseBracket) {
                    let bug = SyntaxBug::ExpectedCloseBracket(this.lexer.peek_pos());
                    this.bugs.push(&bug);
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
            let bug = SyntaxBug::ExpectedOpenBracket(self.lexer.peek_pos());
            self.bugs.push(&bug);

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
            let bug = SyntaxBug::ExpectedSemicolon(self.lexer.peek_pos());
            self.bugs.push(&bug);
        }

        let len = self.parse_expression();

        if !self.lexer.skip_if(&Token::CloseBracket) {
            let bug = SyntaxBug::ExpectedCloseBracket(self.lexer.peek_pos());
            self.bugs.push(&bug);

            self.lexer.skip_while(&Token::CloseBracket);
        }

        Type::ArrayType(Box::new(ArrayType { element_type, len }))
    }

    fn parse_reference_type(&mut self) -> ReferenceType {
        assert!(self.lexer.peek_t() == Token::And);
        self.lexer.skip_tok();

        let mut lifetime = None;
        let mut exclusive = None;
        let mut mutability = None;

        if self.lexer.skip_if(&Token::SingleQuote) {
            if self.lexer.skip_if(&Token::Keyword(Keyword::Static)) {
                lifetime = Some(Lifetime::Static);
            } else if self.lexer.skip_if(&Token::Name(IString::from("thread"))) {
                lifetime = Some(Lifetime::Thread);
            } else if self.lexer.skip_if(&Token::Name(IString::from("task"))) {
                lifetime = Some(Lifetime::Task);
            } else if self.lexer.skip_if(&Token::Name(IString::from("gc"))) {
                lifetime = Some(Lifetime::GarbageCollected);
            } else if let Some(name) = self.lexer.next_if_name() {
                lifetime = Some(Lifetime::Other { name });
            } else {
                let bug = SyntaxBug::ReferenceTypeExpectedLifetimeName(self.lexer.peek_pos());
                self.bugs.push(&bug);
            }
        }

        if self.lexer.skip_if(&Token::Keyword(Keyword::Poly)) {
            exclusive = Some(false);
        } else if self.lexer.skip_if(&Token::Keyword(Keyword::Iso)) {
            exclusive = Some(true);
        }

        if self.lexer.skip_if(&Token::Keyword(Keyword::Mut)) {
            mutability = Some(true);
        } else if self.lexer.skip_if(&Token::Keyword(Keyword::Const)) {
            mutability = Some(false);
        }

        let to = self.parse_type();

        ReferenceType {
            lifetime,
            exclusive,
            mutability,
            to,
        }
    }

    fn parse_pointer_type(&mut self) -> ReferenceType {
        assert!(self.lexer.peek_t() == Token::Star);
        self.lexer.skip_tok();

        let mut exclusive = None;
        let mut mutability = None;

        if self.lexer.skip_if(&Token::Keyword(Keyword::Poly)) {
            exclusive = Some(false);
        } else if self.lexer.skip_if(&Token::Keyword(Keyword::Iso)) {
            exclusive = Some(true);
        }

        if self.lexer.skip_if(&Token::Keyword(Keyword::Mut)) {
            mutability = Some(true);
        } else if self.lexer.skip_if(&Token::Keyword(Keyword::Const)) {
            mutability = Some(false);
        }

        let to = self.parse_type();

        ReferenceType {
            lifetime: Some(Lifetime::Manual),
            exclusive,
            mutability,
            to,
        }
    }

    fn parse_function_type_parameters(&mut self) -> Vec<FunctionTypeParameter> {
        fn parse_function_type_parameter(this: &mut Parser) -> FunctionTypeParameter {
            let attributes = this.parse_attributes();

            let mut name = None;

            let rewind_pos = this.lexer.current_pos();
            if let Some(param_name) = this.lexer.next_if_name() {
                if this.lexer.skip_if(&Token::Colon) {
                    name = Some(param_name);
                } else {
                    this.lexer.rewind(rewind_pos);
                }
            }

            let param_type = this.parse_type();

            let default = if this.lexer.skip_if(&Token::Eq) {
                Some(this.parse_expression())
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

        if !self.lexer.skip_if(&Token::OpenParen) {
            let bug = SyntaxBug::ExpectedOpenParen(self.lexer.peek_pos());
            self.bugs.push(&bug);
        }

        self.lexer.skip_if(&Token::Comma);

        let mut params = Vec::new();
        let mut already_reported_too_many_parameters = false;

        while !self.lexer.skip_if(&Token::CloseParen) {
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

            let param = parse_function_type_parameter(self);
            params.push(param);

            if !self.lexer.skip_if(&Token::Comma) && !self.lexer.next_is(&Token::CloseParen) {
                let bug = SyntaxBug::FunctionParametersExpectedEnd(self.lexer.peek_pos());
                self.bugs.push(&bug);

                self.lexer.skip_while(&Token::CloseParen);
                break;
            }
        }

        params
    }

    fn parse_function_type(&mut self) -> FunctionType {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Fn));
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();
        let parameters = self.parse_function_type_parameters();

        let return_type = if self.lexer.skip_if(&Token::Minus) {
            if !self.lexer.skip_if(&Token::Lt) {
                let bug = SyntaxBug::ExpectedArrow(self.lexer.peek_pos());
                self.bugs.push(&bug);
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
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Opaque));
        self.lexer.skip_tok();

        if !self.lexer.skip_if(&Token::OpenParen) {
            let bug = SyntaxBug::ExpectedOpenParen(self.lexer.peek_pos());
            self.bugs.push(&bug);
        }

        let opaque_identity = self.lexer.next_if_string().unwrap_or_else(|| {
            let bug = SyntaxBug::OpaqueTypeMissingName(self.lexer.peek_pos());
            self.bugs.push(&bug);
            "".into()
        });

        if !self.lexer.skip_if(&Token::CloseParen) {
            let bug = SyntaxBug::ExpectedCloseParen(self.lexer.peek_pos());
            self.bugs.push(&bug);
        }

        Type::OpaqueType(opaque_identity)
    }

    fn parse_type_primitive(&mut self) -> Type {
        let keyword = self.lexer.next_if_keyword().unwrap();

        match keyword {
            Keyword::Bool => Type::Bool,
            Keyword::U8 => Type::UInt8,
            Keyword::U16 => Type::UInt16,
            Keyword::U32 => Type::UInt32,
            Keyword::U64 => Type::UInt64,
            Keyword::U128 => Type::UInt128,
            Keyword::I8 => Type::Int8,
            Keyword::I16 => Type::Int16,
            Keyword::I32 => Type::Int32,
            Keyword::I64 => Type::Int64,
            Keyword::I128 => Type::Int128,
            Keyword::F8 => Type::Float8,
            Keyword::F16 => Type::Float16,
            Keyword::F32 => Type::Float32,
            Keyword::F64 => Type::Float64,
            Keyword::F128 => Type::Float128,

            _ => unreachable!("not a primitive type keyword"),
        }
    }

    pub(crate) fn parse_type_path(&mut self) -> Path {
        assert!(matches!(self.lexer.peek_t(), Token::Name(_) | Token::Colon));

        let mut path = SmallVec::new();
        let mut last_was_scope = false;

        loop {
            match self.lexer.peek_t() {
                Token::Name(name) => {
                    if !last_was_scope && !path.is_empty() {
                        break;
                    }

                    self.lexer.skip_tok();

                    path.push(name);
                    last_was_scope = false;
                }

                Token::Colon => {
                    self.lexer.skip_tok();
                    if !self.lexer.skip_if(&Token::Colon) {
                        last_was_scope = false;
                        break;
                    }

                    if last_was_scope {
                        let bug = SyntaxBug::PathUnexpectedScopeSeparator(self.lexer.peek_pos());
                        self.bugs.push(&bug);
                        break;
                    }

                    if path.is_empty() {
                        path.push(IString::from(""));
                    }

                    last_was_scope = true;
                }

                _ => break,
            }
        }

        if path.is_empty() {
            let bug = SyntaxBug::PathIsEmpty(self.lexer.peek_pos());
            self.bugs.push(&bug);
        }

        if last_was_scope {
            let bug = SyntaxBug::PathTrailingScopeSeparator(self.lexer.peek_pos());
            self.bugs.push(&bug);
        }

        let type_arguments = self.parse_generic_arguments();

        Path {
            path,
            type_arguments,
        }
    }

    fn parse_type_primary(&mut self) -> Type {
        let current_pos = self.lexer.current_pos();

        match self.lexer.peek_t() {
            Token::Keyword(Keyword::Bool)
            | Token::Keyword(Keyword::U8)
            | Token::Keyword(Keyword::U16)
            | Token::Keyword(Keyword::U32)
            | Token::Keyword(Keyword::U64)
            | Token::Keyword(Keyword::U128)
            | Token::Keyword(Keyword::I8)
            | Token::Keyword(Keyword::I16)
            | Token::Keyword(Keyword::I32)
            | Token::Keyword(Keyword::I64)
            | Token::Keyword(Keyword::I128)
            | Token::Keyword(Keyword::F8)
            | Token::Keyword(Keyword::F16)
            | Token::Keyword(Keyword::F32)
            | Token::Keyword(Keyword::F64)
            | Token::Keyword(Keyword::F128) => self.parse_type_primitive(),

            Token::Name(_) | Token::Colon => Type::TypeName(Box::new(self.parse_type_path())),
            Token::OpenBracket => self.parse_array_or_slice(),
            Token::And => Type::ReferenceType(Box::new(self.parse_reference_type())),
            Token::Star => Type::ReferenceType(Box::new(self.parse_pointer_type())),
            Token::OpenBrace => Type::LatentType(Box::new(self.parse_block())),
            Token::Keyword(Keyword::Fn) => Type::FunctionType(Box::new(self.parse_function_type())),
            Token::Keyword(Keyword::Opaque) => self.parse_opaque_type(),

            _ => {
                self.lexer.skip_tok();

                let log = SyntaxBug::ExpectedType(current_pos);
                self.bugs.push(&log);

                Type::SyntaxError
            }
        }
    }

    fn parse_rest_of_tuple(&mut self, first_element: Type) -> TupleType {
        let mut element_types = Vec::from([first_element]);
        let mut already_reported_too_many_elements = false;

        while !self.lexer.skip_if(&Token::CloseParen) {
            if self.lexer.is_eof() {
                let bug = SyntaxBug::TupleTypeExpectedEnd(self.lexer.peek_pos());
                self.bugs.push(&bug);
                break;
            }

            const MAX_TUPLE_ELEMENTS: usize = 65_536;

            if !already_reported_too_many_elements && element_types.len() >= MAX_TUPLE_ELEMENTS {
                already_reported_too_many_elements = true;

                let bug = SyntaxBug::TupleTypeElementLimit(self.lexer.peek_pos());
                self.bugs.push(&bug);
            }

            let element = self.parse_type();
            element_types.push(element);

            if !self.lexer.skip_if(&Token::Comma) && !self.lexer.next_is(&Token::CloseParen) {
                let bug = SyntaxBug::TupleTypeExpectedEnd(self.lexer.peek_pos());
                self.bugs.push(&bug);

                self.lexer.skip_while(&Token::CloseParen);
                break;
            }
        }

        TupleType { element_types }
    }

    pub(crate) fn parse_type(&mut self) -> Type {
        if self.lexer.skip_if(&Token::OpenParen) {
            if self.lexer.skip_if(&Token::CloseParen) {
                return Type::TupleType(Box::new(TupleType {
                    element_types: Vec::new(),
                }));
            }

            let inner = self.parse_type();

            let result = match self.lexer.next_t() {
                Token::CloseParen => Type::Parentheses(Box::new(inner)),

                Token::Comma => {
                    let tuple = self.parse_rest_of_tuple(inner);
                    Type::TupleType(Box::new(tuple))
                }

                _ => {
                    let bug = SyntaxBug::TupleTypeExpectedEnd(self.lexer.peek_pos());
                    self.bugs.push(&bug);

                    Type::SyntaxError
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
