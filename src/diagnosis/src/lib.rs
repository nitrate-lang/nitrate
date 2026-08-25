mod collector;
mod diagnostic;
mod file_id;

pub use collector::CompilerLog;
pub use diagnostic::{
    DiagnosticExplanation, DiagnosticGroupId, DiagnosticId, DiagnosticInfo, FormattableDiagnosticGroup, Origin,
    SourcePosition, Span, diagnostic_code,
};
pub use file_id::{FileId, intern_file_id};
