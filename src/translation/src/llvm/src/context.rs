use inkwell::{context::Context, targets::TargetData};
use std::ops::{Deref, DerefMut};

pub struct LLVMContext {
    pub context: Context,
    pub target_data: TargetData,
}

impl LLVMContext {
    pub fn new() -> Self {
        LLVMContext {
            context: Context::create(),
            target_data: TargetData::create(""),
        }
    }

    pub fn target_data(&self) -> &TargetData {
        &self.target_data
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
