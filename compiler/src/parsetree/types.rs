use super::array_type::ArrayType;
use super::expression::{CodeFormat, Expr, InnerExpr, Metadata, OriginTag, ToCode};
use super::function_type::FunctionType;
use super::struct_type::StructType;
use super::tuple_type::TupleType;
use crate::lexer::{Comment, CommentKind, Identifier, Operator, Punctuation, Token};

#[derive(Debug, Clone, PartialEq, PartialOrd, Hash)]
pub enum InnerType<'a> {
    Discard,

    /* Primitive Types */
    UInt1,
    UInt8,
    UInt16,
    UInt32,
    UInt64,
    UInt128,
    Int8,
    Int16,
    Int32,
    Int64,
    Int128,
    Float16,
    Float32,
    Float64,
    Float128,

    /* Compound Types */
    InferType,
    TupleType(TupleType<'a>),
    StructType(StructType<'a>),
    ArrayType(ArrayType<'a>),
    FunctionType(FunctionType<'a>),
}

#[derive(Debug, Clone, PartialEq, PartialOrd, Hash)]
pub struct Type<'a> {
    expr: InnerType<'a>,
    metadata: Metadata<'a>,
}

impl<'a> Type<'a> {
    pub fn new(expr: InnerType<'a>, metadata: Metadata<'a>) -> Self {
        Type { expr, metadata }
    }

    pub fn into_inner(self) -> InnerType<'a> {
        self.expr
    }

    pub fn get(&self) -> &InnerType<'a> {
        &self.expr
    }

    pub fn get_mut(&mut self) -> &mut InnerType<'a> {
        &mut self.expr
    }

    pub fn discard(&mut self) {
        self.expr = InnerType::Discard;
    }

    pub fn is_discarded(&self) -> bool {
        matches!(self.expr, InnerType::Discard)
    }

    pub fn has_parenthesis(&self) -> bool {
        self.metadata.has_parenthesis()
    }

    pub fn set_has_parenthesis(&mut self, has_parenthesis: bool) {
        self.metadata.set_has_parenthesis(has_parenthesis);
    }

    pub fn origin(&self) -> OriginTag {
        self.metadata.origin()
    }

    pub fn set_origin(&mut self, origin: OriginTag) {
        self.metadata.set_origin(origin);
    }

    pub fn comments(&self) -> &[&'a str] {
        &self.metadata.comments()
    }

    pub fn add_comment(&mut self, comment: &'a str) {
        self.metadata.comments_mut().push(comment);
    }

    pub fn into_expr(self) -> Expr<'a> {
        let expr = match self.expr {
            InnerType::Discard => InnerExpr::Discard,

            InnerType::UInt1 => InnerExpr::UInt1,
            InnerType::UInt8 => InnerExpr::UInt8,
            InnerType::UInt16 => InnerExpr::UInt16,
            InnerType::UInt32 => InnerExpr::UInt32,
            InnerType::UInt64 => InnerExpr::UInt64,
            InnerType::UInt128 => InnerExpr::UInt128,
            InnerType::Int8 => InnerExpr::Int8,
            InnerType::Int16 => InnerExpr::Int16,
            InnerType::Int32 => InnerExpr::Int32,
            InnerType::Int64 => InnerExpr::Int64,
            InnerType::Int128 => InnerExpr::Int128,
            InnerType::Float16 => InnerExpr::Float16,
            InnerType::Float32 => InnerExpr::Float32,
            InnerType::Float64 => InnerExpr::Float64,
            InnerType::Float128 => InnerExpr::Float128,

            InnerType::InferType => InnerExpr::InferType,
            InnerType::TupleType(tuple) => InnerExpr::TupleType(tuple),
            InnerType::StructType(struct_type) => InnerExpr::StructType(struct_type),
            InnerType::ArrayType(array) => InnerExpr::ArrayType(array),
            InnerType::FunctionType(function) => InnerExpr::FunctionType(function),
        };

        Expr::new(expr, self.metadata)
    }

    pub fn is_lit(&self) -> bool {
        match &self.expr {
            InnerType::Discard => false,

            InnerType::UInt1 => true,
            InnerType::UInt8 => true,
            InnerType::UInt16 => true,
            InnerType::UInt32 => true,
            InnerType::UInt64 => true,
            InnerType::UInt128 => true,
            InnerType::Int8 => true,
            InnerType::Int16 => true,
            InnerType::Int32 => true,
            InnerType::Int64 => true,
            InnerType::Int128 => true,
            InnerType::Float16 => true,
            InnerType::Float32 => true,
            InnerType::Float64 => true,
            InnerType::Float128 => true,

            InnerType::InferType => false,
            InnerType::TupleType(tuple) => tuple.elements().iter().all(|item| item.is_lit()),
            InnerType::StructType(_struct) => _struct
                .fields()
                .iter()
                .all(|(_, field_ty)| field_ty.is_lit()),
            InnerType::ArrayType(array) => array.element_ty().is_lit() && array.count().is_lit(),
            InnerType::FunctionType(function) => {
                function.parameters().iter().all(|(_, ty, default)| {
                    ty.is_lit() && default.as_ref().map_or(true, |d| d.is_lit())
                }) && function.return_type().map_or(true, |ty| ty.is_lit())
                    && function.attributes().iter().all(|attr| attr.is_lit())
            }
        }
    }
}

impl<'a> ToCode<'a> for Type<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        if self.is_discarded() {
            return;
        }

        for comment in self.comments() {
            tokens.push(Token::Comment(Comment::new(
                comment,
                CommentKind::SingleLine,
            )));
        }

        if self.has_parenthesis() {
            tokens.push(Token::Punctuation(Punctuation::LeftParenthesis));
        }

        match &self.expr {
            InnerType::Discard => {}

            InnerType::UInt1 => tokens.push(Token::Identifier(Identifier::new("bool"))),
            InnerType::UInt8 => tokens.push(Token::Identifier(Identifier::new("u8"))),
            InnerType::UInt16 => tokens.push(Token::Identifier(Identifier::new("u16"))),
            InnerType::UInt32 => tokens.push(Token::Identifier(Identifier::new("u32"))),
            InnerType::UInt64 => tokens.push(Token::Identifier(Identifier::new("u64"))),
            InnerType::UInt128 => tokens.push(Token::Identifier(Identifier::new("u128"))),
            InnerType::Int8 => tokens.push(Token::Identifier(Identifier::new("i8"))),
            InnerType::Int16 => tokens.push(Token::Identifier(Identifier::new("i16"))),
            InnerType::Int32 => tokens.push(Token::Identifier(Identifier::new("i32"))),
            InnerType::Int64 => tokens.push(Token::Identifier(Identifier::new("i64"))),
            InnerType::Int128 => tokens.push(Token::Identifier(Identifier::new("i128"))),
            InnerType::Float16 => tokens.push(Token::Identifier(Identifier::new("f16"))),
            InnerType::Float32 => tokens.push(Token::Identifier(Identifier::new("f32"))),
            InnerType::Float64 => tokens.push(Token::Identifier(Identifier::new("f64"))),
            InnerType::Float128 => tokens.push(Token::Identifier(Identifier::new("f128"))),

            InnerType::InferType => tokens.push(Token::Operator(Operator::Question)),
            InnerType::TupleType(e) => e.to_code(tokens, options),
            InnerType::StructType(e) => e.to_code(tokens, options),
            InnerType::ArrayType(e) => e.to_code(tokens, options),
            InnerType::FunctionType(e) => e.to_code(tokens, options),
        }

        if self.has_parenthesis() {
            tokens.push(Token::Punctuation(Punctuation::RightParenthesis));
        }
    }
}

impl<'a> std::ops::Deref for Type<'a> {
    type Target = InnerType<'a>;

    fn deref(&self) -> &Self::Target {
        &self.expr
    }
}

impl<'a> std::ops::DerefMut for Type<'a> {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.expr
    }
}
