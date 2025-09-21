#![forbid(unsafe_code)]
#![warn(clippy::pedantic)]

mod expr;
mod expr_iter;
mod id_store;
mod item;
mod item_iter;
mod iter_mut;
mod ty;
mod ty_iter;

pub mod kind {
    pub use super::expr::*;
    pub use super::ty::*;
    pub use crate::item::*;
}

pub mod tag {
    pub use crate::id_store::*;
}

pub use iter_mut::{Order, ParseTreeIterMut, RefNodeMut};
