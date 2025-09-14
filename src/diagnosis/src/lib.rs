mod collector;
mod diagnostic;

pub use collector::DiagnosticCollector;
pub use diagnostic::{
    DiagnosticGroupId, DiagnosticId, DiagnosticInfo, FormattableDiagnosticGroup, Origin,
    SourcePosition, Span,
};
