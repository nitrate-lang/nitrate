use std::format;

use serde::{Deserialize, Serialize};

/// A byte span in the source file.
/// Stored as two u32 values for memory efficiency.
/// Since source files are limited to u32::MAX bytes, u32 is sufficient.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Default, Serialize, Deserialize)]
pub struct ByteSpan {
    /// The byte offset of the start of the span (inclusive).
    #[serde(default)]
    pub start: u32,
    /// The byte offset of the end of the span (exclusive).
    #[serde(default)]
    pub end: u32,
}

impl ByteSpan {
    #[must_use]
    pub const fn new(start: u32, end: u32) -> Self {
        ByteSpan { start, end }
    }

    /// Create an empty span at the given position.
    #[must_use]
    pub const fn empty_at(pos: u32) -> Self {
        ByteSpan { start: pos, end: pos }
    }

    /// Returns true if the span is empty (start == end).
    #[must_use]
    pub fn is_empty(self) -> bool {
        self.start == self.end
    }

    /// Returns the length of the span in bytes.
    #[must_use]
    pub fn len(self) -> u32 {
        self.end - self.start
    }

    /// Extract the source text for this span from the given source bytes.
    #[must_use]
    pub fn extract<'a>(self, source: &'a [u8]) -> &'a [u8] {
        &source[self.start as usize..self.end as usize]
    }

    /// Extract the source text as a string from the given source bytes.
    #[must_use]
    pub fn extract_str<'a>(self, source: &'a [u8]) -> &'a str {
        std::str::from_utf8(self.extract(source)).expect("Source text should be valid UTF-8")
    }
}

/// A packed source position storing line and column in 32 bits.
/// 23 bits for line number, 9 bits for column number.
/// This allows lines up to ~8 million and columns up to 511.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub struct TextPos(u32);

impl TextPos {
    /// Maximum column value (9 bits).
    pub const MAX_COL: u32 = 0x1FF;
    /// Maximum line value (23 bits).
    pub const MAX_LINE: u32 = 0x7F_FF_FF;

    /// Pack line and column into a single u32.
    #[must_use]
    pub fn pack(line: u32, column: u32) -> Self {
        assert!(
            line <= Self::MAX_LINE,
            "Line number too large: {line} (max: {})",
            Self::MAX_LINE
        );
        assert!(
            column <= Self::MAX_COL,
            "Column number too large: {column} (max: {})",
            Self::MAX_COL
        );
        TextPos((line & Self::MAX_LINE) << 9 | (column & Self::MAX_COL))
    }

    /// Get the line number (0-based).
    #[must_use]
    pub fn line(self) -> u32 {
        self.0 >> 9
    }

    /// Get the column number (0-based).
    #[must_use]
    pub fn column(self) -> u32 {
        self.0 & Self::MAX_COL
    }

    /// Get the raw packed value.
    #[must_use]
    pub const fn raw(self) -> u32 {
        self.0
    }
}

impl From<(u32, u32)> for TextPos {
    fn from((line, column): (u32, u32)) -> Self {
        TextPos::pack(line, column)
    }
}

/// A source span storing start and end positions.
/// Each position stores line and column in a packed 32-bit format.
/// Total size: 8 bytes (start + end as u32 each).
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub struct Span {
    start: u32,
    end: u32,
}

impl Span {
    /// Create a new span from start and end positions.
    #[must_use]
    pub const fn new(start: TextPos, end: TextPos) -> Self {
        Span {
            start: start.raw(),
            end: end.raw(),
        }
    }

    /// Create a span from raw packed values.
    #[must_use]
    pub const fn from_raw(start: u32, end: u32) -> Self {
        Span { start, end }
    }

    /// Get the start position.
    #[must_use]
    pub fn start(self) -> TextPos {
        TextPos(self.start)
    }

    /// Get the end position.
    #[must_use]
    pub fn end(self) -> TextPos {
        TextPos(self.end)
    }

    /// Get the start line.
    #[must_use]
    pub fn start_line(self) -> u32 {
        self.start >> 9
    }

    /// Get the start column.
    #[must_use]
    pub fn start_column(self) -> u32 {
        self.start & TextPos::MAX_COL
    }

    /// Get the end line.
    #[must_use]
    pub fn end_line(self) -> u32 {
        self.end >> 9
    }

    /// Get the end column.
    #[must_use]
    pub fn end_column(self) -> u32 {
        self.end & TextPos::MAX_COL
    }

    /// Format as `file:line:col` for error messages.
    /// `file` should be the filename, or "???" if unknown.
    #[must_use]
    pub fn display(&self, file: &str) -> String {
        format!("{}:{}:{}", file, self.start_line() + 1, self.start_column() + 1)
    }
}

impl From<(TextPos, TextPos)> for Span {
    fn from((start, end): (TextPos, TextPos)) -> Self {
        Span::new(start, end)
    }
}
