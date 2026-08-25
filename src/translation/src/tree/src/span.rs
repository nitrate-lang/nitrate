use nitrate_diagnosis::FileId;
use nitrate_token::LexPos;
use serde::{Deserialize, Serialize};
use std::format;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Default, Serialize, Deserialize)]
#[repr(C, packed)]
pub struct U24([u8; 3]);

impl U24 {
    pub const MIN: Self = Self([0x00, 0x00, 0x00]);
    pub const MAX: Self = Self([0xFF, 0xFF, 0xFF]);

    /// Creates a U24 from a u32, truncating the top byte.
    pub fn from_u32(val: u32) -> Self {
        let bytes = val.to_le_bytes(); // Little-endian format
        Self([bytes[0], bytes[1], bytes[2]])
    }

    /// Converts the U24 into a native u32.
    pub fn to_u32(self) -> u32 {
        u32::from_le_bytes([self.0[0], self.0[1], self.0[2], 0])
    }
}

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
    pub offset: U24,
}

impl SrcPos {
    /// Maximum value for the 24-bit offset field.
    pub const MAX_OFFSET: u32 = 0xFF_FF_FF;

    /// Create a new source position. Values exceeding field widths are clamped.
    #[must_use]
    pub fn new(fileid: Option<FileId>, line: u16, column: u8, offset: u32) -> Self {
        SrcPos {
            fileid,
            line,
            column,
            offset: U24::from_u32(offset.min(Self::MAX_OFFSET)),
        }
    }

    /// Returns true if the position is empty (zero offset, no file).
    #[must_use]
    pub fn is_empty(self) -> bool {
        self.offset.to_u32() == 0 && self.fileid == None && self.line == 0 && self.column == 0
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

impl From<LexPos> for SrcPos {
    fn from(lp: LexPos) -> Self {
        SrcPos::new(lp.fileid, lp.line, lp.column, lp.offset)
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
    pub fn new<T: Into<SrcPos>, U: Into<SrcPos>>(start: T, end: U) -> Self {
        SrcSpan {
            start: start.into(),
            end: end.into(),
        }
    }

    /// Returns true if the span has zero length.
    #[must_use]
    pub fn is_empty(self) -> bool {
        self.start.offset.to_u32() == self.end.offset.to_u32()
            && self.start.line == self.end.line
            && self.start.column == self.end.column
    }

    /// Returns the length of the span in bytes.
    #[must_use]
    pub fn len(self) -> u32 {
        self.end.offset.to_u32().saturating_sub(self.start.offset.to_u32())
    }

    /// Extract the source text for this span from the given source bytes.
    #[must_use]
    pub fn extract<'a>(self, source: &'a [u8]) -> &'a [u8] {
        &source[self.start.offset.to_u32() as usize..self.end.offset.to_u32() as usize]
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
