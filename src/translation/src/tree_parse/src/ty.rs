use super::parse::Parser;
use crate::diagnosis::SyntaxErr;
use crate::helper::MAX_LIMIT;
use nitrate_tree::ByteSpan;

use nitrate_nstring::NString;
use nitrate_token::Token;
use nitrate_tree::ast::{
    ArrayType, Bool, Expr, Float32, Float64, FuncTypeParam, FuncTypeParams, FunctionType, Int8, Int16, Int32, Int64,
    Int128, Lifetime, PointerType, ReferenceType, RefinementType, SliceType, TupleType, Type, TypeParentheses,
    TypePath, TypePathSegment, TypePotential, TypeSyntaxError, UInt8, UInt16, UInt32, UInt64, UInt128, USize,
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
        let bracket_start = self.lexer.peek_pos().offset;
        assert!(self.lexer.peek_tok().token == Token::OpenBracket);
        self.lexer.skip_tok();

        let element_type = self.parse_type();

        if self.lexer.skip_if(&Token::CloseBracket) {
            return Type::SliceType(Box::new(SliceType {
                span: ByteSpan::new(bracket_start, self.lexer.current_pos().offset),
                element_type,
            }));
        }

        self.expect_semicolon();

        let len = self.parse_expression();

        self.expect_close_bracket();

        Type::ArrayType(Box::new(ArrayType {
            span: ByteSpan::new(bracket_start, self.lexer.current_pos().offset),
            element_type,
            len,
        }))
    }

    fn parse_reference_type(&mut self) -> ReferenceType {
        let ref_start = self.lexer.peek_pos().offset;
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
            span: ByteSpan::new(ref_start, self.lexer.current_pos().offset),
            lifetime,
            exclusivity,
            mutability,
            to,
        }
    }

    fn parse_pointer_type(&mut self) -> PointerType {
        let ptr_start = self.lexer.peek_pos().offset;
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
            span: ByteSpan::new(ptr_start, self.lexer.current_pos().offset),
            lifetime,
            exclusivity,
            mutability,
            to,
        }
    }

    fn parse_function_type_parameters(&mut self) -> FuncTypeParams {
        self.expect_open_paren();

        let eof = SyntaxErr::FunctionParametersExpectedEnd(self.lexer.peek_pos());
        let limit = SyntaxErr::FunctionParameterLimit(self.lexer.peek_pos());
        let end = SyntaxErr::FunctionParametersExpectedEnd(self.lexer.peek_pos());

        let params = self.parse_comma_separated_list(&Token::CloseParen, MAX_LIMIT, true, eof, limit, end, |this| {
            let param_start = this.lexer.peek_pos().offset;
            let param = this.parse_common_func_param(false);
            FuncTypeParam {
                span: ByteSpan::new(param_start, this.lexer.current_pos().offset),
                attributes: param.attributes,
                name: param.name,
                ty: param.ty,
            }
        });

        params
    }

    fn parse_function_type(&mut self) -> FunctionType {
        let fn_start = self.lexer.peek_pos().offset;
        assert!(self.lexer.peek_tok().token == Token::Fn);
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();
        let parameters = self.parse_function_type_parameters();

        let return_type = self.parse_return_type_arrow();

        FunctionType {
            span: ByteSpan::new(fn_start, self.lexer.current_pos().offset),
            parameters,
            return_type,
            attributes,
        }
    }

    fn parse_lifetime(&mut self) -> Lifetime {
        let lt_start = self.lexer.peek_pos().offset;
        assert!(self.lexer.peek_tok().token == Token::SingleQuote);
        self.lexer.skip_tok();

        if self.lexer.skip_if(&Token::Static) {
            return Lifetime {
                span: ByteSpan::new(lt_start, self.lexer.current_pos().offset),
                name: "static".into(),
            };
        }

        let err = SyntaxErr::ReferenceTypeExpectedLifetimeName(self.lexer.peek_pos());
        let name = self.parse_string_name(err);

        Lifetime {
            span: ByteSpan::new(lt_start, self.lexer.current_pos().offset),
            name: name.into(),
        }
    }

    pub(crate) fn create_type_path(&mut self, segment_name: String) -> TypePath {
        TypePath {
            span: ByteSpan::new(0, 0),
            segments: Vec::from([TypePathSegment {
                span: ByteSpan::new(0, 0),
                name: segment_name,
                type_arguments: None,
            }]),
            resolved_path: None,
        }
    }

    fn parse_type_primitive(&mut self) -> Type {
        let type_start = self.lexer.peek_pos().offset;
        let result = match self.lexer.next_tok().token {
            Token::Bool => Type::Bool(Bool::default()),
            Token::U8 => Type::UInt8(UInt8::default()),
            Token::U16 => Type::UInt16(UInt16::default()),
            Token::U32 => Type::UInt32(UInt32::default()),
            Token::U64 => Type::UInt64(UInt64::default()),
            Token::U128 => Type::UInt128(UInt128::default()),
            Token::USize => Type::USize(USize::default()),
            Token::I8 => Type::Int8(Int8::default()),
            Token::I16 => Type::Int16(Int16::default()),
            Token::I32 => Type::Int32(Int32::default()),
            Token::I64 => Type::Int64(Int64::default()),
            Token::I128 => Type::Int128(Int128::default()),
            Token::F8 => Type::TypePath(Box::new(self.create_type_path("f8".to_string()))),
            Token::F16 => Type::TypePath(Box::new(self.create_type_path("f16".to_string()))),
            Token::F32 => Type::Float32(Float32::default()),
            Token::F64 => Type::Float64(Float64::default()),
            Token::F128 => Type::TypePath(Box::new(self.create_type_path("f128".to_string()))),

            _ => Type::SyntaxError(TypeSyntaxError::default()),
        };
        let type_end = self.lexer.current_pos().offset;
        // Apply span to result
        set_type_span(result, type_start, type_end)
    }

    pub(crate) fn parse_type_path(&mut self) -> TypePath {
        assert!(matches!(
            self.lexer.peek_tok().token,
            Token::Name(_) | Token::Colon | Token::SelfType | Token::SelfKeyword
        ));

        let mut segments = Vec::new();
        let mut prev_scope = false;
        let mut already_reported_too_many_segments = false;

        if self.parse_double_colon() {
            prev_scope = true;

            segments.push(TypePathSegment {
                span: ByteSpan::new(0, self.lexer.current_pos().offset),
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

            if !already_reported_too_many_segments && segments.len() >= MAX_LIMIT {
                already_reported_too_many_segments = true;

                let bug = SyntaxErr::PathSegmentLimit(self.lexer.peek_pos());
                self.log.report(&bug);
            }

            let name_start = self.lexer.peek_pos().offset;
            let Some(identifier) = self.lexer.next_if_name() else {
                let bug = SyntaxErr::PathExpectedName(self.lexer.peek_pos());
                self.log.report(&bug);
                break;
            };
            let name_end = self.lexer.current_pos().offset;

            let type_arguments = self.parse_generic_arguments();

            segments.push(TypePathSegment {
                span: ByteSpan::new(name_start, name_end),
                name: identifier,
                type_arguments,
            });

            prev_scope = self.parse_double_colon();
        }

        assert_ne!(segments.len(), 0);

        let path_start = segments.first().map(|s| s.span.start).unwrap_or(0);
        let path_end = segments.last().map(|s| s.span.end).unwrap_or(0);

        TypePath {
            span: ByteSpan::new(path_start, path_end),
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

            Token::OpenBrace | Token::Unsafe | Token::Safe => {
                let block_start = self.lexer.peek_pos().offset;
                let body = self.parse_block();
                Type::TypePotential(Box::new(TypePotential {
                    span: ByteSpan::new(block_start, self.lexer.current_pos().offset),
                    body,
                }))
            }

            _ => {
                let err_start = self.lexer.peek_pos().offset;
                self.lexer.skip_tok();

                let log = SyntaxErr::ExpectedType(current_pos);
                self.log.report(&log);

                Type::SyntaxError(TypeSyntaxError {
                    span: ByteSpan::new(err_start, self.lexer.current_pos().offset),
                })
            }
        }
    }

    fn parse_rest_of_tuple(&mut self, first_element: Type, paren_start: u32) -> TupleType {
        let mut element_types = Vec::from([first_element]);

        let eof = SyntaxErr::TupleTypeExpectedEnd(self.lexer.peek_pos());
        let limit = SyntaxErr::TupleTypeElementLimit(self.lexer.peek_pos());
        let end = SyntaxErr::TupleTypeExpectedEnd(self.lexer.peek_pos());

        let rest = self.parse_comma_separated_list(&Token::CloseParen, MAX_LIMIT, false, eof, limit, end, |this| {
            this.parse_type()
        });

        element_types.extend(rest);

        TupleType {
            span: ByteSpan::new(paren_start, self.lexer.current_pos().offset),
            element_types,
        }
    }

    pub fn parse_type(&mut self) -> Type {
        if self.lexer.skip_if(&Token::OpenParen) {
            let paren_start = self.lexer.current_pos().offset;
            if self.lexer.skip_if(&Token::CloseParen) {
                let paren_end = self.lexer.current_pos().offset;
                return Type::TupleType(Box::new(TupleType {
                    span: ByteSpan::new(paren_start, paren_end),
                    element_types: Vec::new(),
                }));
            }

            let inner = self.parse_type();

            let result = match self.lexer.next_tok().token {
                Token::CloseParen => {
                    let paren_end = self.lexer.current_pos().offset;
                    Type::Parentheses(Box::new(TypeParentheses {
                        span: ByteSpan::new(paren_start, paren_end),
                        inner,
                    }))
                }

                Token::Comma => {
                    let tuple = self.parse_rest_of_tuple(inner, paren_start);
                    Type::TupleType(Box::new(tuple))
                }

                _ => {
                    let bug = SyntaxErr::TupleTypeExpectedEnd(self.lexer.peek_pos());
                    self.log.report(&bug);

                    Type::SyntaxError(TypeSyntaxError {
                        span: ByteSpan::new(paren_start, self.lexer.current_pos().offset),
                    })
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
            span: ByteSpan::new(basis_type.span().start, self.lexer.current_pos().offset),
            basis_type,
            width: refine_options.width,
            minimum: refine_options.minimum,
            maximum: refine_options.maximum,
        }))
    }
}

/// Helper to set the span on a primitive type that was created via Default.
fn set_type_span(ty: Type, start: u32, end: u32) -> Type {
    use Type::*;
    let span = ByteSpan::new(start, end);
    match ty {
        Bool(mut b) => {
            b.span = span;
            Bool(b)
        }
        UInt8(mut u) => {
            u.span = span;
            UInt8(u)
        }
        UInt16(mut u) => {
            u.span = span;
            UInt16(u)
        }
        UInt32(mut u) => {
            u.span = span;
            UInt32(u)
        }
        UInt64(mut u) => {
            u.span = span;
            UInt64(u)
        }
        UInt128(mut u) => {
            u.span = span;
            UInt128(u)
        }
        USize(mut u) => {
            u.span = span;
            USize(u)
        }
        Int8(mut i) => {
            i.span = span;
            Int8(i)
        }
        Int16(mut i) => {
            i.span = span;
            Int16(i)
        }
        Int32(mut i) => {
            i.span = span;
            Int32(i)
        }
        Int64(mut i) => {
            i.span = span;
            Int64(i)
        }
        Int128(mut i) => {
            i.span = span;
            Int128(i)
        }
        Float32(mut f) => {
            f.span = span;
            Float32(f)
        }
        Float64(mut f) => {
            f.span = span;
            Float64(f)
        }
        TypePath(mut tp) => {
            tp.span = span;
            if let Some(seg) = tp.segments.first_mut() {
                seg.span = span;
            }
            TypePath(tp)
        }
        SyntaxError(mut e) => {
            e.span = span;
            SyntaxError(e)
        }
        other => other,
    }
}
