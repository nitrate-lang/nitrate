use nitrate_diagnosis::DiagnosticCollector;
use nitrate_parsetree::kind::NamedFunction;

pub trait FunctionOptimization {
    fn optimize_function(&self, subject: &mut NamedFunction, bugs: &DiagnosticCollector);
}
