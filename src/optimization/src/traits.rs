use nitrate_diagnosis::DiagnosticDrain;
use nitrate_structure::kind::Function;

pub trait FunctionOptimization {
    fn optimize_function(&self, subject: &mut Function, drain: &DiagnosticDrain);
}
