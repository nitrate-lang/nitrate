use crate::DiagnosticDrain;
use nitrate_structure::kind::Expr;

pub trait Diagnose {
    fn diagnose(&self, subject: &Expr, drain: &DiagnosticDrain);
}
