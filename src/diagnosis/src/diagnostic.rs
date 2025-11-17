use std::ops::Deref;

use serde::{Deserialize, Serialize};

use crate::FileId;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct SourcePosition {
    pub line: u32,
    pub column: u32,
    pub offset: u32,
    pub fileid: Option<FileId>,
}

impl std::fmt::Display for SourcePosition {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "{}:{}:{}",
            self.fileid.as_ref().map_or("???", |fid| fid.deref()),
            self.line + 1,
            self.column + 1
        )
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
    Scan = 0,
    Token = 1,
    Syntax = 2,
    Resolve = 3,
    Hir = 4,
    Type = 5,
}

impl std::fmt::Display for DiagnosticGroupId {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            DiagnosticGroupId::Scan => write!(f, "Scanner"),
            DiagnosticGroupId::Token => write!(f, "Lexical"),
            DiagnosticGroupId::Syntax => write!(f, "Syntax"),
            DiagnosticGroupId::Resolve => write!(f, "Resolution"),
            DiagnosticGroupId::Hir => write!(f, "HIR"),
            DiagnosticGroupId::Type => write!(f, "Type"),
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
