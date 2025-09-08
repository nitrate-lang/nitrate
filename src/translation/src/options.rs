use std::num::NonZero;

use nitrate_diagnosis::{Diagnose, DiagnosticDrain};
use nitrate_optimization::FunctionOptimization;

#[derive(Default)]
pub struct OptimizationOptions {}

pub struct TranslationOptions {
    pub source_name_for_debug_messages: String,
    pub drain: DiagnosticDrain,
    pub thread_count: NonZero<usize>,
    pub diagnostic_passes: Vec<Box<dyn Diagnose + Sync>>,
    pub function_optimizations: Vec<Box<dyn FunctionOptimization + Sync>>,
}

impl Default for TranslationOptions {
    fn default() -> Self {
        Self {
            source_name_for_debug_messages: String::default(),
            drain: DiagnosticDrain::default(),
            thread_count: NonZero::new(1).unwrap(),
            diagnostic_passes: Vec::new(),
            function_optimizations: Vec::new(),
        }
    }
}
