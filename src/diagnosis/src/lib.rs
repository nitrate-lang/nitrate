mod diagnostic;
mod drain;

pub use diagnostic::{
    DiagnosticGroupId, DiagnosticId, DiagnosticInfo, FormattableDiagnosticGroup, Origin,
    SourcePosition, Span,
};
pub use drain::DiagnosticDrain;
