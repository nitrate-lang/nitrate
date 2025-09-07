use crate::DiagnosticDrain;
use nitrate_structure::SourceModel;

pub trait Diagnose {
    fn diagnose<'a>(&self, subject: &SourceModel<'a>, drain: &DiagnosticDrain);
}
