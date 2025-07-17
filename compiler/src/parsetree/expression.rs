// TODO: Develop nitrate abstract syntax tree (AST) data structures

use crate::lexer::*;

use super::binary_op::BinaryExpr;
use super::block::Block;
use super::character::CharLit;
use super::list::List;
use super::number::{FloatLit, IntegerLit};
use super::object::Object;
use super::statement::Statement;
use super::string::StringLit;
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

    /* Control Flow */
    If,
    For,
    Foreach,
    While,
    Switch,
    Return,
    Continue,
    Break,

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
    Unit,

    /* Compound Types */
    ArrayTy,
    FunctionTy,
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

    pub fn is_lit(&self) -> bool {
        match &self.expr {
            InnerExpr::Float(_)
            | InnerExpr::Integer(_)
            | InnerExpr::String(_)
            | InnerExpr::Char(_) => true,

            InnerExpr::List(list) => list.iter().all(|item| item.is_lit()),
            InnerExpr::Object(map) => map.iter().all(|(_, value)| value.is_lit()),

            InnerExpr::UInt1
            | InnerExpr::UInt8
            | InnerExpr::UInt16
            | InnerExpr::UInt32
            | InnerExpr::UInt64
            | InnerExpr::UInt128
            | InnerExpr::Int8
            | InnerExpr::Int16
            | InnerExpr::Int32
            | InnerExpr::Int64
            | InnerExpr::Int128
            | InnerExpr::Float16
            | InnerExpr::Float32
            | InnerExpr::Float64
            | InnerExpr::Float128 => true,

            // FIXME: Verify recursively that components of ArrayTy and FunctionTy are literals
            InnerExpr::ArrayTy | InnerExpr::FunctionTy => true,

            _ => false,
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

            InnerExpr::If => {}
            InnerExpr::For => {}
            InnerExpr::Foreach => {}
            InnerExpr::While => {}
            InnerExpr::Switch => {}
            InnerExpr::Return => {}
            InnerExpr::Continue => {}
            InnerExpr::Break => {}

            InnerExpr::UInt1 => {}
            InnerExpr::UInt8 => {}
            InnerExpr::UInt16 => {}
            InnerExpr::UInt32 => {}
            InnerExpr::UInt64 => {}
            InnerExpr::UInt128 => {}
            InnerExpr::Int8 => {}
            InnerExpr::Int16 => {}
            InnerExpr::Int32 => {}
            InnerExpr::Int64 => {}
            InnerExpr::Int128 => {}
            InnerExpr::Float16 => {}
            InnerExpr::Float32 => {}
            InnerExpr::Float64 => {}
            InnerExpr::Float128 => {}
            InnerExpr::Unit => {}

            InnerExpr::ArrayTy => {}
            InnerExpr::FunctionTy => {}
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
