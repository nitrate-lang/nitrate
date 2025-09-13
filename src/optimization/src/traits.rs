use nitrate_diagnosis::DiagnosticDrain;
use nitrate_parsetree::kind::NamedFunction;

pub trait FunctionOptimization {
    fn optimize_function(&self, subject: &mut NamedFunction, drain: &DiagnosticDrain);
}
