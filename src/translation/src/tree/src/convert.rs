use crate::span::{SrcPos, SrcSpan};
use nitrate_diagnosis::FileId;
use nitrate_token::AnnotatedToken;

/// Convert an AnnotatedToken's start position to a SrcPos.
#[must_use]
pub fn tok_to_srcpos_start(tok: &AnnotatedToken) -> SrcPos {
    SrcPos {
        fileid: tok.fileid,
        line: tok.start_line,
        column: tok.start_column,
        offset: tok.start_offset,
    }
}

/// Convert an AnnotatedToken's end position to a SrcPos.
#[must_use]
pub fn tok_to_srcpos_end(tok: &AnnotatedToken) -> SrcPos {
    SrcPos {
        fileid: tok.fileid,
        line: tok.end_line,
        column: tok.end_column,
        offset: tok.end_offset,
    }
}

/// Convert an AnnotatedToken's range to a SrcSpan.
#[must_use]
pub fn tok_to_srcspan(tok: &AnnotatedToken) -> SrcSpan {
    SrcSpan {
        start: tok_to_srcpos_start(tok),
        end: tok_to_srcpos_end(tok),
    }
}

/// Convert a `(fileid, line, col, offset)` tuple to a SrcPos.
#[must_use]
pub fn raw_to_srcpos((fileid, line, column, offset): (Option<FileId>, u16, u8, u32)) -> SrcPos {
    SrcPos {
        fileid,
        line,
        column,
        offset,
    }
}

/// Create a SrcSpan from raw byte offsets (for constructing spans without
/// line/column info).
#[must_use]
pub fn span_from_offsets(start: u32, end: u32) -> SrcSpan {
    SrcSpan {
        start: SrcPos {
            fileid: None,
            line: 0,
            column: 0,
            offset: start,
        },
        end: SrcPos {
            fileid: None,
            line: 0,
            column: 0,
            offset: end,
        },
    }
}

/// Create a SrcSpan from two `(fileid, line, col, offset)` tuples.
#[must_use]
pub fn span_from_raw(start: (Option<FileId>, u16, u8, u32), end: (Option<FileId>, u16, u8, u32)) -> SrcSpan {
    SrcSpan {
        start: raw_to_srcpos(start),
        end: raw_to_srcpos(end),
    }
}
