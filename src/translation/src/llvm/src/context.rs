use std::ops::{Deref, DerefMut};

use inkwell::context::Context;

pub struct LLVMContext {
    pub context: Context,
}

impl LLVMContext {
    pub fn new() -> Self {
        LLVMContext {
            context: Context::create(),
        }
    }
}

impl Deref for LLVMContext {
    type Target = Context;

    fn deref(&self) -> &Self::Target {
        &self.context
    }
}

impl DerefMut for LLVMContext {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.context
    }
}
