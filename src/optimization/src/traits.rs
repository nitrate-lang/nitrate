use nitrate_diagnosis::CompilerLog;
use nitrate_parsetree::kind::Function;

pub trait FunctionOptimization {
    fn optimize_function(&self, subject: &mut Function, bugs: &CompilerLog);
}
