// TODO: Develop nitrate abstract syntax tree (AST) data structures

use crate::lexer::{Comment, CommentKind, Token};

use super::binary_op::BinaryExpr;
use super::character::CharLit;
use super::list::List;
use super::number::NumberLit;
use super::string::StringLit;

#[derive(Debug, Default, Clone, Copy, PartialEq, Eq, Hash)]
pub struct OriginTag {
    offset: u32,
}

impl OriginTag {
    pub fn new(offset: u32) -> Self {
        OriginTag { offset }
    }
}

#[derive(Debug, Default, Clone, PartialEq, Eq, Hash)]
pub struct ExprMeta {
    origin: OriginTag,
    has_parenthesis: bool,
}

impl ExprMeta {
    pub fn new(has_parenthesis: bool, origin: OriginTag) -> Self {
        ExprMeta {
            has_parenthesis,
            origin,
        }
    }

    pub fn has_parenthesis(&self) -> bool {
        self.has_parenthesis
    }

    pub fn origin(&self) -> OriginTag {
        self.origin
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum InnerExpr<'a> {
    Discard,

    Number(NumberLit),
    String(StringLit<'a>),
    Char(CharLit),
    List(List<'a>),

    BinaryOp(BinaryExpr<'a>),
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct Expr<'a> {
    expr: InnerExpr<'a>,
    metadata: ExprMeta,
}

impl<'a> Expr<'a> {
    pub fn new(expr: InnerExpr<'a>, metadata: ExprMeta) -> Self {
        Expr { expr, metadata }
    }

    pub fn is_lit(&self) -> bool {
        match &self.expr {
            InnerExpr::Number(_) | InnerExpr::String(_) | InnerExpr::Char(_) => true,
            InnerExpr::List(list) => list.iter().all(|item| item.is_lit()),
            _ => false,
        }
    }

    pub fn expr(&self) -> &InnerExpr<'a> {
        &self.expr
    }

    pub fn expr_mut(&mut self) -> &mut InnerExpr<'a> {
        &mut self.expr
    }

    pub fn into_inner(self) -> InnerExpr<'a> {
        self.expr
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
        self.metadata.has_parenthesis = has_parenthesis;
    }

    pub fn origin(&self) -> OriginTag {
        self.metadata.origin()
    }

    pub fn set_origin(&mut self, origin: OriginTag) {
        self.metadata.origin = origin;
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

pub trait ToCode<'a> {
    fn to_code(&self) -> Vec<Token<'a>>;
}

impl<'a> ToCode<'a> for Expr<'a> {
    fn to_code(&self) -> Vec<Token<'a>> {
        let mut tokens = Vec::new();

        match &self.expr {
            InnerExpr::String(string) => tokens.push(Token::Comment(Comment::new(
                string,
                CommentKind::SingleLine,
            ))),
            _ => {}
        }

        tokens
    }
}
