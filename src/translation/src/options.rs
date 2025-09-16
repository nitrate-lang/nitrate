use std::num::NonZero;

use nitrate_diagnosis::DiagnosticCollector;
use nitrate_optimization::FunctionOptimization;
use nitrate_parsetree::kind::Package;

pub trait Diagnose {
    fn diagnose(&self, package: &Package, bugs: &DiagnosticCollector);
}

#[derive(Default)]
pub struct OptimizationOptions {}

pub struct TranslationOptions {
    pub crate_name: String,
    pub source_name_for_debug_messages: String,
    pub bugs: DiagnosticCollector,
    pub thread_count: NonZero<usize>,
    pub diagnostic_passes: Vec<Box<dyn Diagnose + Sync>>,
    pub function_optimizations: Vec<Box<dyn FunctionOptimization + Sync>>,
}

impl Default for TranslationOptions {
    fn default() -> Self {
        Self {
            crate_name: String::default(),
            source_name_for_debug_messages: String::default(),
            bugs: DiagnosticCollector::default(),
            thread_count: NonZero::new(1).unwrap(),
            diagnostic_passes: Vec::new(),
            function_optimizations: Vec::new(),
        }
    }
}
