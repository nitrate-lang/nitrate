use crate::DiagnosticDrain;

pub trait Diagnose<Subject> {
    fn diagnose(&mut self, subject: &Subject, drain: &DiagnosticDrain);
}
