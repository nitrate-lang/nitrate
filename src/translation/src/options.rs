use nitrate_diagnosis::{Diagnose, DiagnosticDrain};
use nitrate_optimization::FunctionOptimization;

#[derive(Default)]
pub struct OptimizationOptions {}

#[derive(Default)]
pub struct TranslationOptions {
    pub source_name_for_debug_messages: String,
    pub drain: DiagnosticDrain,
    pub thread_count: usize,
    pub diagnostics: Vec<Box<dyn Diagnose + Sync>>,
    pub function_optimizations: Vec<Box<dyn FunctionOptimization + Sync>>,
}
