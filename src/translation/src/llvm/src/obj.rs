use inkwell::{module::Module, targets::FileType};

use crate::LLVMContext;

impl LLVMContext {
    pub fn write_object_file(
        &self,
        module: &mut Module,
        output_path: &std::path::Path,
    ) -> Result<(), String> {
        self.target_machine
            .write_to_file(&module, FileType::Object, output_path)
            .map_err(|e| format!("Failed to write object code to file: {}", e))
    }
}
