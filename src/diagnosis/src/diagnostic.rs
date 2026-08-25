use crate::FileId;
use serde::{Deserialize, Serialize};
use std::write;

#[derive(Debug, Clone, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub struct SourcePosition {
    pub line: u32,
    pub column: u32,
    pub offset: u32,
    pub fileid: Option<FileId>,
}

impl SourcePosition {
    #[must_use]
    pub const fn new(line: u32, column: u32, offset: u32, fileid: Option<FileId>) -> Self {
        SourcePosition {
            line,
            column,
            offset,
            fileid,
        }
    }
}

impl std::fmt::Display for SourcePosition {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "{}:{}:{}",
            self.fileid.as_ref().map_or("unknown", |id| &**id),
            self.line + 1,
            self.column + 1
        )
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct Span {
    pub start: SourcePosition,
    pub end: SourcePosition,
}

#[derive(Debug, Clone)]
pub enum Origin {
    Point(SourcePosition),
    Span(Span),
    None,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u8)]
pub enum DiagnosticGroupId {
    Scan = 0,
    Lex = 1,
    Parse = 2,
    Resolve = 3,
    Hir = 4,
    Type = 5,
    Semantic = 6,
    BorrowCheck = 7,
}

impl std::fmt::Display for DiagnosticGroupId {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            DiagnosticGroupId::Scan => write!(f, "Scanner"),
            DiagnosticGroupId::Lex => write!(f, "Lexical"),
            DiagnosticGroupId::Parse => write!(f, "Syntax"),
            DiagnosticGroupId::Resolve => write!(f, "Resolution"),
            DiagnosticGroupId::Hir => write!(f, "Hir"),
            DiagnosticGroupId::Type => write!(f, "Type"),
            DiagnosticGroupId::Semantic => write!(f, "Semantic"),
            DiagnosticGroupId::BorrowCheck => write!(f, "BorrowCheck"),
        }
    }
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

/// Formats the error code string (e.g. `E27D2`) for the given diagnostic
/// group and variant.
///
/// The returned string matches the `error[EXXXX]` prefix emitted by
/// [`crate::CompilerLog`], so it can be passed directly to `no3 --explain`.
#[must_use]
pub fn diagnostic_code(group_id: DiagnosticGroupId, variant: u16) -> String {
    let id = DiagnosticId::new(group_id, variant).unwrap_or(DiagnosticId::UNKNOWN);
    format!("E{:04X}", id.0)
}

/// A static, human-readable explanation for a single diagnostic error code.
///
/// Each compilation stage registers one entry per error variant. The `no3`
/// driver aggregates every stage's entries (through
/// `nitrate_translation::diagnostic_explanations`) and serves them to
/// `no3 --explain <CODE>`.
///
/// The `variant_id` of an entry must match the `FormattableDiagnosticGroup::variant_id()`
/// of the corresponding error, so that `code()` yields the same string the
/// compiler prints.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct DiagnosticExplanation {
    /// The diagnostic group the error belongs to.
    pub group_id: DiagnosticGroupId,
    /// The variant ID within the group.
    pub variant_id: u16,
    /// The human-readable explanation shown by `no3 --explain <CODE>`.
    pub explanation: &'static str,
}

impl DiagnosticExplanation {
    /// Returns the error code as printed by the compiler, e.g. `E27D2`.
    #[must_use]
    pub fn code(&self) -> String {
        diagnostic_code(self.group_id, self.variant_id)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn diagnostic_code_matches_printed_format() {
        // Parser group (2), variant 2002 -> (2 << 12) | 2002 = 0x27D2.
        assert_eq!(diagnostic_code(DiagnosticGroupId::Parse, 2002), "E27D2");
        // Type group (5), variant 0 -> 0x5000.
        assert_eq!(diagnostic_code(DiagnosticGroupId::Type, 0), "E5000");
        // BorrowCheck group (7), variant 0x10C -> 0x710C.
        assert_eq!(diagnostic_code(DiagnosticGroupId::BorrowCheck, 0x10C), "E710C");
    }

    #[test]
    fn explanation_code_matches_diagnostic_id() {
        let explanation = DiagnosticExplanation {
            group_id: DiagnosticGroupId::Lex,
            variant_id: 0x7FF,
            explanation: "test",
        };
        assert_eq!(explanation.code(), "E17FF");
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
