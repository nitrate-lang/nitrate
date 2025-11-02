use nitrate_diagnosis::CompilerLog;
use nitrate_tree::ast::Function;

pub trait FunctionOptimization {
    fn optimize_function(&self, subject: &mut Function, log: &CompilerLog);
}
