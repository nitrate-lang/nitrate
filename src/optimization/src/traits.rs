use nitrate_diagnosis::CompilerLog;
use nitrate_parsetree::ast::Function;

pub trait FunctionOptimization {
    fn optimize_function(&self, subject: &mut Function, log: &CompilerLog);
}
