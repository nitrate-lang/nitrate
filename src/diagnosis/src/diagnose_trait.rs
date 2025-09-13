use crate::DiagnosticDrain;
use nitrate_parsetree::kind::Expr;

pub trait Diagnose {
    fn diagnose(&self, subject: &Expr, drain: &DiagnosticDrain);
}
