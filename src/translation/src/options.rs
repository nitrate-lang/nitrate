use nitrate_diagnosis::{Diagnose, DiagnosticDrain};

#[derive(Default)]
pub struct OptimizationOptions {}

#[derive(Default)]
pub struct TranslationOptions {
    pub source_name_for_debug_messages: String,
    pub drain: DiagnosticDrain,
    pub diagnostic_passes: Vec<Box<dyn Diagnose + Sync>>,
    pub optimization: OptimizationOptions,
}
