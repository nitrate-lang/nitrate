use std::num::NonZero;

use interned_string::IString;
use nitrate_diagnosis::DiagnosticDrain;
use nitrate_optimization::FunctionOptimization;
use nitrate_parsetree::kind::Package;

pub trait Diagnose {
    fn diagnose(&self, package: &Package, drain: &DiagnosticDrain);
}

#[derive(Default)]
pub struct OptimizationOptions {}

pub struct TranslationOptions {
    pub crate_name: IString,
    pub source_name_for_debug_messages: String,
    pub drain: DiagnosticDrain,
    pub thread_count: NonZero<usize>,
    pub diagnostic_passes: Vec<Box<dyn Diagnose + Sync>>,
    pub function_optimizations: Vec<Box<dyn FunctionOptimization + Sync>>,
}

impl Default for TranslationOptions {
    fn default() -> Self {
        Self {
            crate_name: IString::default(),
            source_name_for_debug_messages: String::default(),
            drain: DiagnosticDrain::default(),
            thread_count: NonZero::new(1).unwrap(),
            diagnostic_passes: Vec::new(),
            function_optimizations: Vec::new(),
        }
    }
}
