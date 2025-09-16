mod collector;
mod diagnostic;
mod file_id;

pub use collector::DiagnosticCollector;
pub use diagnostic::{
    DiagnosticGroupId, DiagnosticId, DiagnosticInfo, FormattableDiagnosticGroup, Origin,
    SourcePosition, Span,
};
pub use file_id::{FileId, get_or_create_file_id};
