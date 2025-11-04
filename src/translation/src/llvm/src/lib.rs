#![forbid(unsafe_code)]

mod asm;
mod context;
mod obj;
mod opt;

pub use context::LLVMContext;
pub use opt::OptLevel;
