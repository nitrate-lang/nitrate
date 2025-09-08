use nitrate_diagnosis::DiagnosticDrain;
use nitrate_structure::kind::Function;

pub trait FunctionOptimization {
    fn optimize_function<'a>(&self, subject: &mut Function<'a>, drain: &DiagnosticDrain);
}
