//! Tests for source-size limits (MAX_SOURCE_SIZE = 4096 in test cfg).

use crate::Lexer;

#[test]
fn test_source_too_big_triggers_error() {
    let big = vec![b' '; 4097];
    assert!(matches!(Lexer::new(&big, None), Err(crate::LexerError::SourceTooBig),));
}

#[test]
fn test_source_at_max_size_is_ok() {
    let big = vec![b'x'; 4096];
    assert!(Lexer::new(&big, None).is_ok());
}
