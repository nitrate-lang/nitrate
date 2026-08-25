use crate::span::SrcSpan;
use serde::{Deserialize, Serialize};

/// Represents a range of trivia tokens (whitespace, comments, etc.)
/// in the original source. Stores a SrcSpan pointing into the source bytes.
/// For lazy relexing, re-lex the byte range to get the individual trivia tokens.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub struct Trivia {
    /// Source span of this trivia range in the original source.
    pub span: SrcSpan,
}

impl Trivia {
    /// Create a new trivia span.
    pub const fn new(span: SrcSpan) -> Self {
        Trivia { span }
    }

    /// Returns true if this trivia range is empty.
    pub fn is_empty(&self) -> bool {
        self.span.is_empty()
    }

    /// Extract the source bytes for this trivia range.
    pub fn extract<'a>(&self, source: &'a [u8]) -> &'a [u8] {
        self.span.extract(source)
    }

    /// Extract the source text for this trivia range as a string.
    pub fn extract_str<'a>(&self, source: &'a [u8]) -> &'a str {
        self.span.extract_str(source)
    }
}

impl From<SrcSpan> for Trivia {
    fn from(span: SrcSpan) -> Self {
        Trivia { span }
    }
}

/// A reference to a trivia range, used during parsing to record trivia positions.
/// Stores just a u32 offset for memory efficiency; the end is determined by context.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct TriviaRef {
    /// Offset into the source where this trivia range starts.
    pub offset: u32,
}

impl TriviaRef {
    /// Create a new trivia reference.
    pub const fn new(offset: u32) -> Self {
        TriviaRef { offset }
    }
}
