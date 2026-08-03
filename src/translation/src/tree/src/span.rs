use nitrate_diagnosis::FileId;
use serde::{Deserialize, Serialize};
use std::format;

/// A compact source position storing file, line, column, and byte offset.
/// Total size: 8 bytes.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Default, Serialize, Deserialize)]
pub struct SrcPos {
    /// File identifier (None = unknown).
    pub fileid: Option<FileId>,
    /// 0-based column number (clamped to 255).
    pub column: u8,
    /// 0-based line number (clamped to 65535).
    pub line: u16,
    /// Byte offset from start of file (clamped to 24 bits: 16_777_215).
    pub offset: u32,
}

impl SrcPos {
    /// Maximum value for the 24-bit offset field.
    pub const MAX_OFFSET: u32 = 0xFF_FF_FF;

    /// Create a new source position. Values exceeding field widths are clamped.
    #[must_use]
    pub fn new(fileid: Option<FileId>, line: u32, column: u32, offset: u32) -> Self {
        SrcPos {
            fileid,
            line: (line as u64).min(u16::MAX as u64) as u16,
            column: (column as u64).min(u8::MAX as u64) as u8,
            offset: offset.min(Self::MAX_OFFSET),
        }
    }

    /// Create a source position with just an offset.
    #[must_use]
    pub fn at_offset(offset: u32) -> Self {
        SrcPos {
            fileid: None,
            column: 0,
            line: 0,
            offset: if offset > Self::MAX_OFFSET {
                Self::MAX_OFFSET
            } else {
                offset
            },
        }
    }

    /// Returns true if the position is empty (zero offset, no file).
    #[must_use]
    pub fn is_empty(self) -> bool {
        self.offset == 0 && self.fileid == None && self.line == 0 && self.column == 0
    }

    /// Format the position as `file:line:col`.
    #[must_use]
    pub fn display(&self, file: &str) -> String {
        if self.fileid == None {
            format!("?:{}:{}", self.line + 1, self.column + 1)
        } else {
            format!("{}:{}:{}", file, self.line + 1, self.column + 1)
        }
    }

    /// Format a span from `self` to `end`.
    #[must_use]
    pub fn display_span(&self, end: SrcPos, file: &str) -> String {
        format!("{}: {}", self.display(file), end.display(file))
    }
}

/// A source span storing start and end positions.
/// Total size: 16 bytes (two SrcPos values).
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Default, Serialize, Deserialize)]
pub struct SrcSpan {
    pub start: SrcPos,
    pub end: SrcPos,
}

impl SrcSpan {
    /// Create a new span from start and end byte offsets (backward compatible).
    #[must_use]
    pub const fn new(start: u32, end: u32) -> Self {
        SrcSpan {
            start: SrcPos {
                fileid: None,
                column: 0,
                line: 0,
                offset: start,
            },
            end: SrcPos {
                fileid: None,
                column: 0,
                line: 0,
                offset: end,
            },
        }
    }

    /// Create a new span from SrcPos values (new API).
    #[must_use]
    pub const fn from_parts(start: SrcPos, end: SrcPos) -> Self {
        SrcSpan { start, end }
    }

    /// Returns true if the span has zero length.
    #[must_use]
    pub fn is_empty(self) -> bool {
        self.start.offset == self.end.offset && self.start.line == self.end.line && self.start.column == self.end.column
    }

    /// Returns the length of the span in bytes.
    #[must_use]
    pub fn len(self) -> u32 {
        self.end.offset.saturating_sub(self.start.offset)
    }

    /// Extract the source text for this span from the given source bytes.
    #[must_use]
    pub fn extract<'a>(self, source: &'a [u8]) -> &'a [u8] {
        &source[self.start.offset as usize..self.end.offset as usize]
    }

    /// Extract the source text as a string.
    #[must_use]
    pub fn extract_str<'a>(self, source: &'a [u8]) -> &'a str {
        std::str::from_utf8(self.extract(source)).expect("Source text should be valid UTF-8")
    }

    /// Format as `file:line:col` for error messages.
    #[must_use]
    pub fn display(&self, file: &str) -> String {
        self.start.display(file)
    }
}

impl From<(SrcPos, SrcPos)> for SrcSpan {
    fn from((start, end): (SrcPos, SrcPos)) -> Self {
        SrcSpan::from_parts(start, end)
    }
}

/// Convert a span to its start position (lossy).
impl From<SrcSpan> for SrcPos {
    fn from(span: SrcSpan) -> Self {
        span.start
    }
}

/// Convert a position to a zero-length span.
impl From<SrcPos> for SrcSpan {
    fn from(pos: SrcPos) -> Self {
        SrcSpan::from_parts(pos, pos)
    }
}
