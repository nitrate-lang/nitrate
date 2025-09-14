use crate::DiagnosticDrain;
use nitrate_parsetree::kind::Package;

pub trait Diagnose {
    fn diagnose(&self, package: &Package, drain: &DiagnosticDrain);
}
