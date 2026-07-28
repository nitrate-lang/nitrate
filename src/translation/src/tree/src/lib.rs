#![forbid(unsafe_code)]

mod expr;
mod expr_iter;
mod expr_iter_mut;
mod item;
mod item_iter;
mod item_iter_mut;
mod iter;
mod iter_mut;
mod pretty_print;
mod span;
mod trivia;
mod ty;
mod ty_iter;
mod ty_iter_mut;

pub use span::ByteSpan;
pub use trivia::Trivia;

pub mod ast {
    pub use super::expr::*;
    pub use super::span::*;
    pub use super::trivia::*;
    pub use super::ty::*;
    pub use crate::item::*;
}

pub use iter::{Order, ParseTreeIter, RefNode};
pub use iter_mut::{ParseTreeIterMut, RefNodeMut};
pub use pretty_print::{PrettyPrint, PrintContext};

pub mod prelude {
    pub use super::ast::*;
    pub use super::iter::{Order, ParseTreeIter, RefNode};
    pub use super::iter_mut::{ParseTreeIterMut, RefNodeMut};
    pub use super::pretty_print::{PrettyPrint, PrintContext};
    pub use super::span::*;
    pub use super::trivia::*;
}
