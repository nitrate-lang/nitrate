#![forbid(unsafe_code)]
#![warn(clippy::pedantic)]

mod expr;
mod expr_iter;
mod expr_iter_mut;
mod id_store;
mod item;
mod item_iter;
mod item_iter_mut;
mod iter;
mod iter_mut;
mod ty;
mod ty_iter;
mod ty_iter_mut;

pub mod kind {
    pub use super::expr::*;
    pub use super::ty::*;
    pub use crate::item::*;
}

pub mod tag {
    pub use crate::id_store::*;
}

pub use iter::{Order, ParseTreeIter, RefNode};
pub use iter_mut::{ParseTreeIterMut, RefNodeMut};
