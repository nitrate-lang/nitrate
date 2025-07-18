// TODO: Develop nitrate abstract syntax tree (AST) data structures

use crate::lexer::*;

use super::array_type::ArrayType;
use super::binary_op::BinaryExpr;
use super::block::Block;
use super::character::CharLit;
use super::function_type::FunctionType;
use super::list::List;
use super::number::{FloatLit, IntegerLit};
use super::object::Object;
use super::statement::Statement;
use super::string::StringLit;
use super::struct_type::StructType;
use super::tuple_type::TupleType;
use super::types::{InnerType, Type};
use super::unary_op::UnaryExpr;

#[derive(Debug, Default, Clone, Copy, PartialEq, PartialOrd, Hash)]
pub struct OriginTag {
    offset: u32,
}

impl OriginTag {
    pub fn new(offset: u32) -> Self {
        OriginTag { offset }
    }
}

#[derive(Debug, Default, Clone, PartialEq, PartialOrd, Hash)]
struct UncommonMetadata<'a> {
    has_parenthesis: bool,
    comments: Vec<&'a str>,
}

#[derive(Debug, Default, Clone, PartialEq, PartialOrd, Hash)]
pub struct Metadata<'a> {
    origin: OriginTag,
    optional: Option<Box<UncommonMetadata<'a>>>,
}

impl<'a> Metadata<'a> {
    pub fn new(origin: OriginTag, has_parenthesis: bool, comments: Option<Vec<&'a str>>) -> Self {
        let requires_extra = comments.is_some() || has_parenthesis;
        if requires_extra {
            Metadata {
                origin,
                optional: Some(Box::new(UncommonMetadata {
                    has_parenthesis,
                    comments: comments.unwrap_or_default(),
                })),
            }
        } else {
            Metadata {
                origin,
                optional: None,
            }
        }
    }

    pub fn origin(&self) -> OriginTag {
        self.origin
    }

    pub fn set_origin(&mut self, origin: OriginTag) {
        self.origin = origin;
    }

    pub fn has_parenthesis(&self) -> bool {
        self.optional
            .as_ref()
            .map_or(false, |opt| opt.has_parenthesis)
    }

    pub fn set_has_parenthesis(&mut self, has_parenthesis: bool) {
        if let Some(opt) = &mut self.optional {
            opt.has_parenthesis = has_parenthesis;
        } else {
            self.optional = Some(Box::new(UncommonMetadata {
                has_parenthesis,
                comments: Vec::new(),
            }));
        }
    }

    pub fn comments(&self) -> &[&'a str] {
        self.optional.as_ref().map_or(&[], |opt| &opt.comments)
    }

    pub fn comments_mut(&mut self) -> &mut Vec<&'a str> {
        if let None = self.optional {
            self.optional = Some(Box::new(UncommonMetadata::default()));
        }

        &mut self.optional.as_mut().unwrap().comments
    }
}

#[derive(Debug, Clone, PartialEq, PartialOrd, Hash)]
pub enum InnerExpr<'a> {
    Discard,

    /* Primitive Expressions */
    Integer(IntegerLit),
    Float(FloatLit),
    String(StringLit<'a>),
    Char(CharLit),
    List(List<'a>),
    Object(Object<'a>),

    /* Compound Expressions */
    Block(Block<'a>),
    Statement(Statement<'a>),
    BinaryOp(BinaryExpr<'a>),
    UnaryOp(UnaryExpr<'a>),

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
pub struct Expr<'a> {
    expr: InnerExpr<'a>,
    metadata: Metadata<'a>,
}

impl<'a> Expr<'a> {
    pub fn new(expr: InnerExpr<'a>, metadata: Metadata<'a>) -> Self {
        Expr { expr, metadata }
    }

    pub fn into_inner(self) -> InnerExpr<'a> {
        self.expr
    }

    pub fn get(&self) -> &InnerExpr<'a> {
        &self.expr
    }

    pub fn get_mut(&mut self) -> &mut InnerExpr<'a> {
        &mut self.expr
    }

    pub fn discard(&mut self) {
        self.expr = InnerExpr::Discard;
    }

    pub fn is_discarded(&self) -> bool {
        matches!(self.expr, InnerExpr::Discard)
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

    pub fn into_type(self) -> Option<Type<'a>> {
        let type_maybe = match self.expr {
            InnerExpr::Discard => Some(InnerType::Discard),

            InnerExpr::Integer(_) => None,
            InnerExpr::Float(_) => None,
            InnerExpr::String(_) => None,
            InnerExpr::Char(_) => None,
            InnerExpr::List(_) => None,
            InnerExpr::Object(_) => None,

            InnerExpr::Block(_) => None,
            InnerExpr::Statement(_) => None,
            InnerExpr::BinaryOp(_) => None,
            InnerExpr::UnaryOp(_) => None,

            InnerExpr::UInt1 => Some(InnerType::UInt1),
            InnerExpr::UInt8 => Some(InnerType::UInt8),
            InnerExpr::UInt16 => Some(InnerType::UInt16),
            InnerExpr::UInt32 => Some(InnerType::UInt32),
            InnerExpr::UInt64 => Some(InnerType::UInt64),
            InnerExpr::UInt128 => Some(InnerType::UInt128),
            InnerExpr::Int8 => Some(InnerType::Int8),
            InnerExpr::Int16 => Some(InnerType::Int16),
            InnerExpr::Int32 => Some(InnerType::Int32),
            InnerExpr::Int64 => Some(InnerType::Int64),
            InnerExpr::Int128 => Some(InnerType::Int128),
            InnerExpr::Float16 => Some(InnerType::Float16),
            InnerExpr::Float32 => Some(InnerType::Float32),
            InnerExpr::Float64 => Some(InnerType::Float64),
            InnerExpr::Float128 => Some(InnerType::Float128),

            InnerExpr::InferType => Some(InnerType::InferType),
            InnerExpr::TupleType(tuple) => Some(InnerType::TupleType(tuple)),
            InnerExpr::StructType(struct_type) => Some(InnerType::StructType(struct_type)),
            InnerExpr::ArrayType(array) => Some(InnerType::ArrayType(array)),
            InnerExpr::FunctionType(function) => Some(InnerType::FunctionType(function)),
        };

        type_maybe.map(|inner| Type::new(inner, self.metadata))
    }

    pub fn is_lit(&self) -> bool {
        match &self.expr {
            InnerExpr::Discard => false,

            InnerExpr::Integer(_) => true,
            InnerExpr::Float(_) => true,
            InnerExpr::String(_) => true,
            InnerExpr::Char(_) => true,
            InnerExpr::List(list) => list.elements().iter().all(|item| item.is_lit()),
            InnerExpr::Object(map) => map.get().iter().all(|(_, value)| value.is_lit()),

            InnerExpr::Block(_) => false,
            InnerExpr::Statement(_) => false,
            InnerExpr::BinaryOp(_) => false,
            InnerExpr::UnaryOp(_) => false,

            InnerExpr::UInt1 => true,
            InnerExpr::UInt8 => true,
            InnerExpr::UInt16 => true,
            InnerExpr::UInt32 => true,
            InnerExpr::UInt64 => true,
            InnerExpr::UInt128 => true,
            InnerExpr::Int8 => true,
            InnerExpr::Int16 => true,
            InnerExpr::Int32 => true,
            InnerExpr::Int64 => true,
            InnerExpr::Int128 => true,
            InnerExpr::Float16 => true,
            InnerExpr::Float32 => true,
            InnerExpr::Float64 => true,
            InnerExpr::Float128 => true,

            InnerExpr::InferType => false,
            InnerExpr::TupleType(tuple) => tuple.elements().iter().all(|item| item.is_lit()),
            InnerExpr::StructType(_struct) => {
                _struct.fields().iter().all(|(_, field)| field.is_lit())
            }
            InnerExpr::ArrayType(array) => array.element_ty().is_lit() && array.count().is_lit(),
            InnerExpr::FunctionType(function) => {
                function.parameters().iter().all(|(_, ty, default)| {
                    ty.is_lit() && default.as_ref().map_or(true, |d| d.is_lit())
                }) && function.return_type().map_or(true, |ty| ty.is_lit())
                    && function.attributes().iter().all(|attr| attr.is_lit())
            }
        }
    }

    pub fn is_type(&self) -> bool {
        match &self.expr {
            InnerExpr::Discard => false,

            InnerExpr::Integer(_) => false,
            InnerExpr::Float(_) => false,
            InnerExpr::String(_) => false,
            InnerExpr::Char(_) => false,
            InnerExpr::List(_) => false,
            InnerExpr::Object(_) => false,

            InnerExpr::Block(_) => false,
            InnerExpr::Statement(_) => false,
            InnerExpr::BinaryOp(_) => false,
            InnerExpr::UnaryOp(_) => false,

            InnerExpr::UInt1 => true,
            InnerExpr::UInt8 => true,
            InnerExpr::UInt16 => true,
            InnerExpr::UInt32 => true,
            InnerExpr::UInt64 => true,
            InnerExpr::UInt128 => true,
            InnerExpr::Int8 => true,
            InnerExpr::Int16 => true,
            InnerExpr::Int32 => true,
            InnerExpr::Int64 => true,
            InnerExpr::Int128 => true,
            InnerExpr::Float16 => true,
            InnerExpr::Float32 => true,
            InnerExpr::Float64 => true,
            InnerExpr::Float128 => true,

            InnerExpr::InferType => true,
            InnerExpr::TupleType(_) => true,
            InnerExpr::StructType(_) => true,
            InnerExpr::ArrayType(_) => true,
            InnerExpr::FunctionType(_) => true,
        }
    }
}

#[derive(Debug, Default, Clone, PartialEq, PartialOrd, Hash)]
pub struct CodeFormat {}

pub trait ToCode<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat);
}

impl<'a> ToCode<'a> for Expr<'a> {
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
            InnerExpr::Discard => {}

            InnerExpr::Integer(e) => e.to_code(tokens, options),
            InnerExpr::Float(e) => e.to_code(tokens, options),
            InnerExpr::String(e) => e.to_code(tokens, options),
            InnerExpr::Char(e) => e.to_code(tokens, options),
            InnerExpr::List(e) => e.to_code(tokens, options),
            InnerExpr::Object(e) => e.to_code(tokens, options),

            InnerExpr::Block(e) => e.to_code(tokens, options),
            InnerExpr::Statement(e) => e.to_code(tokens, options),
            InnerExpr::BinaryOp(e) => e.to_code(tokens, options),
            InnerExpr::UnaryOp(e) => e.to_code(tokens, options),

            InnerExpr::UInt1 => tokens.push(Token::Identifier(Identifier::new("bool"))),
            InnerExpr::UInt8 => tokens.push(Token::Identifier(Identifier::new("u8"))),
            InnerExpr::UInt16 => tokens.push(Token::Identifier(Identifier::new("u16"))),
            InnerExpr::UInt32 => tokens.push(Token::Identifier(Identifier::new("u32"))),
            InnerExpr::UInt64 => tokens.push(Token::Identifier(Identifier::new("u64"))),
            InnerExpr::UInt128 => tokens.push(Token::Identifier(Identifier::new("u128"))),
            InnerExpr::Int8 => tokens.push(Token::Identifier(Identifier::new("i8"))),
            InnerExpr::Int16 => tokens.push(Token::Identifier(Identifier::new("i16"))),
            InnerExpr::Int32 => tokens.push(Token::Identifier(Identifier::new("i32"))),
            InnerExpr::Int64 => tokens.push(Token::Identifier(Identifier::new("i64"))),
            InnerExpr::Int128 => tokens.push(Token::Identifier(Identifier::new("i128"))),
            InnerExpr::Float16 => tokens.push(Token::Identifier(Identifier::new("f16"))),
            InnerExpr::Float32 => tokens.push(Token::Identifier(Identifier::new("f32"))),
            InnerExpr::Float64 => tokens.push(Token::Identifier(Identifier::new("f64"))),
            InnerExpr::Float128 => tokens.push(Token::Identifier(Identifier::new("f128"))),

            InnerExpr::InferType => tokens.push(Token::Operator(Operator::Question)),
            InnerExpr::TupleType(e) => e.to_code(tokens, options),
            InnerExpr::StructType(e) => e.to_code(tokens, options),
            InnerExpr::ArrayType(e) => e.to_code(tokens, options),
            InnerExpr::FunctionType(e) => e.to_code(tokens, options),
        }

        if self.has_parenthesis() {
            tokens.push(Token::Punctuation(Punctuation::RightParenthesis));
        }
    }
}

impl<'a> std::ops::Deref for Expr<'a> {
    type Target = InnerExpr<'a>;

    fn deref(&self) -> &Self::Target {
        &self.expr
    }
}

impl<'a> std::ops::DerefMut for Expr<'a> {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.expr
    }
}
