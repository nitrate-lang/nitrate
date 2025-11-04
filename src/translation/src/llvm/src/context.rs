use inkwell::{
    OptimizationLevel,
    context::Context,
    targets::{
        CodeModel, InitializationConfig, RelocMode, Target, TargetData, TargetMachine, TargetTriple,
    },
};

use crate::OptLevel;
use std::ops::{Deref, DerefMut};

fn initialize() {
    let config = InitializationConfig {
        asm_parser: true,
        asm_printer: true,
        disassembler: true,
        base: true,
        info: true,
        machine_code: true,
    };

    Target::initialize_all(&config);
}

static INIT_ALREADY: std::sync::Once = std::sync::Once::new();

pub struct LLVMContext {
    pub context: Context,
    pub target: Target,
    pub target_data: TargetData,
    pub target_machine: TargetMachine,
    pub opt_level: OptimizationLevel,
}

impl LLVMContext {
    pub fn new(target_triple: &str, opt_level: OptLevel) -> Result<Self, String> {
        INIT_ALREADY.call_once(|| {
            initialize();
        });

        let opt_level = match opt_level {
            OptLevel::None => OptimizationLevel::None,
            OptLevel::Default => OptimizationLevel::Default,
            OptLevel::Aggressive => OptimizationLevel::Aggressive,
        };

        let cpu_name = "generic";
        let features = "";

        let triple = TargetTriple::create(target_triple);
        let target = Target::from_triple(&triple)
            .map_err(|e| format!("Failed to get target from triple: {}", e))?;

        let target_machine = target
            .create_target_machine(
                &triple,
                cpu_name,
                features,
                opt_level,
                RelocMode::PIC,
                CodeModel::Default,
            )
            .ok_or_else(|| "Failed to create target machine".to_string())?;

        Ok(LLVMContext {
            context: Context::create(),
            target,
            target_data: target_machine.get_target_data(),
            target_machine,
            opt_level,
        })
    }

    pub fn default_target_triple() -> String {
        TargetMachine::get_default_triple()
            .as_str()
            .to_str()
            .unwrap()
            .to_string()
    }

    pub fn target_data(&self) -> &TargetData {
        &self.target_data
    }

    pub fn ptr_size(&self) -> u32 {
        self.target_data.get_pointer_byte_size(None)
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
