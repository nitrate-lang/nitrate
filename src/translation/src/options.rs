use nitrate_diagnosis::Diagnose;
use nitrate_parse::SourceModel;

type SourceModelDiagnostic<'a> = Box<dyn Diagnose<SourceModel<'a>>>;

#[derive(Default)]
pub struct DiagnosticOptions<'a> {
    pub source_model_diagnostics: Vec<SourceModelDiagnostic<'a>>,
}

#[derive(Default)]
pub struct OptimizationOptions {}

#[derive(Default)]
pub struct TranslationOptions<'a> {
    pub source_name_for_debug_messages: String,
    pub diagnostic: DiagnosticOptions<'a>,
    pub optimization: OptimizationOptions,
}
