mod collector;
mod diagnostic;
mod file_id;

pub use collector::CompilerLog;
pub use diagnostic::{
    DiagnosticGroupId, DiagnosticId, DiagnosticInfo, FormattableDiagnosticGroup, Origin,
    SourcePosition, Span,
};
pub use file_id::{FileId, intern_file_id};
