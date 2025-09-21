#![forbid(unsafe_code)]
#![warn(clippy::pedantic)]

mod expr;
mod id_store;
mod item;
mod iter_mut;
mod ty;

pub mod kind {
    pub use super::expr::*;
    pub use super::ty::*;
    pub use crate::item::*;
}

pub mod tag {
    pub use crate::id_store::*;
}

pub use iter_mut::{
    Order, RefNodeMut, expr_depth_first_iter_mut, item_depth_first_iter_mut,
    type_depth_first_iter_mut,
};
