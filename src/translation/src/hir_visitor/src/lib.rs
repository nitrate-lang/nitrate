#![forbid(unsafe_code)]

mod expr;
mod item;
mod ty;

pub use expr::*;
pub use item::*;
pub use ty::*;

pub trait HirVisitor<T>: HirTypeVisitor<T> + HirValueVisitor<T> + HirItemVisitor<T> {}
