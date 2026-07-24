use inkwell::module::Module;
use inkwell::passes::{PassManager, PassManagerBuilder};

use crate::LLVMContext;

pub enum OptLevel {
    None,
    Default,
    Aggressive,
}

impl LLVMContext {
    pub fn optimize_module(&self, module: &mut Module) {
        let pass_manager_builder = PassManagerBuilder::create();
        pass_manager_builder.set_optimization_level(self.opt_level);

        let module_pass_manager = PassManager::create(());
        pass_manager_builder.populate_module_pass_manager(&module_pass_manager);
        module_pass_manager.run_on(module);
    }
}
