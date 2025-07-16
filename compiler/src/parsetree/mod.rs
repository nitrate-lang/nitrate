// TODO: Develop nitrate abstract syntax tree (AST) data structures

mod number;
mod origin;

use number::Number;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum Expr {
    Number(Number),
}
