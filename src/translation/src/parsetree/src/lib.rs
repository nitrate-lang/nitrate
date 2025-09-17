#![forbid(unsafe_code)]
#![warn(clippy::pedantic)]

mod expr;
mod id_store;
mod item;
mod ty;

pub mod kind {
    pub use super::expr::*;
    pub use super::ty::*;
    pub use crate::item::*;
}

pub mod tag {
    pub use crate::id_store::*;
}
