// TODO: Develop nitrate abstract syntax tree (AST) data structures

mod character;
mod number;
mod origin;
mod string;

use character::CharLit;
use number::NumberLit;
use string::StringLit;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum Expr<'a> {
    Number(NumberLit),
    String(StringLit<'a>),
    Char(CharLit),
}
