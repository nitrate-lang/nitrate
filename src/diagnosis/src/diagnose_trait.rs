use crate::DiagnosticDrain;
use nitrate_parsetree::kind::Module;

pub trait Diagnose {
    fn diagnose(&self, module: &Module, drain: &DiagnosticDrain);
}
