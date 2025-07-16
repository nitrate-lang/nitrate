// TODO: Develop nitrate abstract syntax tree (AST) data structures

use super::character::CharLit;
use super::list::ListLit;
use super::number::NumberLit;
use super::string::StringLit;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum ExprLit<'a> {
    Number(NumberLit),
    String(StringLit<'a>),
    Char(CharLit),
    List(ListLit<'a>),
}

impl<'a> ExprLit<'a> {
    pub fn into_expr(self) -> Expr<'a> {
        Expr::ExprLit(self)
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum Expr<'a> {
    ExprLit(ExprLit<'a>),

    Other, // Placeholder for other expression types
}

impl<'a> Expr<'a> {
    pub fn is_lit(&self) -> bool {
        matches!(self, Expr::ExprLit(_))
    }

    pub fn into_expr_lit(self) -> Option<ExprLit<'a>> {
        if let Expr::ExprLit(lit) = self {
            Some(lit)
        } else {
            None
        }
    }
}
