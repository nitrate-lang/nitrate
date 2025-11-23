use inkwell::{module::Module, targets::FileType};

use crate::LLVMContext;

impl LLVMContext {
    pub fn write_asm(
        &self,
        module: &mut Module,
        output: &mut dyn std::io::Write,
    ) -> Result<(), String> {
        let buffer = self
            .target_machine
            .write_to_memory_buffer(module, FileType::Assembly)
            .map_err(|e| format!("Failed to write module to assembly memory buffer: {}", e))?;

        output
            .write_all(buffer.as_slice())
            .map_err(|e| format!("Failed to write assembly to output: {}", e))
    }
}
