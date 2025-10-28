use std::num::NonZero;

use nitrate_diagnosis::CompilerLog;
use nitrate_optimization::FunctionOptimization;
use nitrate_source::ast::Module;

pub trait Diagnose {
    fn diagnose(&self, module: &Module, log: &CompilerLog);
}

#[derive(Default)]
pub struct OptimizationOptions {}

pub struct TranslationOptions {
    pub package_name: String,
    pub source_name_for_debug_messages: String,
    pub source_path: Option<std::path::PathBuf>,
    pub log: CompilerLog,
    pub thread_count: NonZero<usize>,
    pub diagnostic_passes: Vec<Box<dyn Diagnose + Sync>>,
    pub function_optimizations: Vec<Box<dyn FunctionOptimization + Sync>>,
}

impl Default for TranslationOptions {
    fn default() -> Self {
        Self {
            package_name: String::default(),
            source_name_for_debug_messages: String::default(),
            source_path: None,
            log: CompilerLog::default(),
            thread_count: NonZero::new(1).unwrap(),
            diagnostic_passes: Vec::new(),
            function_optimizations: Vec::new(),
        }
    }
}
