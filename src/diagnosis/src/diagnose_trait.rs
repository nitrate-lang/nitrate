use crate::DiagnosticDrain;
use nitrate_structure::SourceModel;

pub trait Diagnose {
    fn diagnose(&self, subject: &SourceModel, drain: &DiagnosticDrain);
}
