use interned_string::IString;
use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct SourcePosition {
    pub line: u32,
    pub column: u32,
    pub offset: u32,
    pub filename: IString,
}

impl std::fmt::Display for SourcePosition {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}:{}:{}", self.filename, self.line + 1, self.column + 1)
    }
}

#[derive(Debug, Clone)]
pub struct Span {
    pub start: SourcePosition,
    pub end: SourcePosition,
}

#[derive(Debug, Clone)]
pub enum Origin {
    Point(SourcePosition),
    Span(Span),
    Unknown,
    None,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u8)]
pub enum DiagnosticGroupId {
    ScanError = 0,
    TokenError = 1,
    SyntaxError = 2,
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct DiagnosticId(pub(crate) u16);

impl DiagnosticId {
    pub const UNKNOWN: Self = DiagnosticId(0xFFFF);

    pub fn new(group_id: DiagnosticGroupId, variant: u16) -> Option<Self> {
        /*
         * GGGG VVVV VVVV VVVV
         */

        if variant > 0x0FFF || (group_id as u32) > 0x0F {
            return None;
        }

        let group_id_4bit = (group_id as u32) & 0x0F;
        let variant_12bit = (variant as u32) & 0xFFF;
        let id = (group_id_4bit << 12) | (variant_12bit);

        Some(DiagnosticId(id as u16))
    }
}

#[derive(Debug, Clone)]
pub struct DiagnosticInfo {
    pub origin: Origin,
    pub message: String,
}

pub trait FormattableDiagnosticGroup {
    fn group_id(&self) -> DiagnosticGroupId;

    /// Returns a per-variant unique ID within the diagnostic group.
    fn variant_id(&self) -> u16;

    fn format(&self) -> DiagnosticInfo;
}
