//! Shared helper functions used across all lexer test modules.

use crate::{Lexer, LexerIterator};
use nitrate_token::{AnnotatedToken, Token};

/// Assert that `source` lexes to exactly one token (no trivia) with the
/// given value. Checks only the token value and offset bounds.
pub fn eq(source: &str, token: Token) {
    let mut lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    let end_off = source.len() as u32;
    assert_eq!(tok.token, token, "'{source}' did not produce the expected token",);
    assert_eq!(tok.start_offset, 0, "'{source}' start_offset should be 0",);
    assert_eq!(
        tok.end_offset, end_off,
        "'{source}' end_offset should be {} but got {}",
        end_off, tok.end_offset,
    );
    assert_eq!(lexer.next_tok().token, Token::Eof, "expected Eof after '{source}'");
}

/// Collect *all* tokens (including trivia) into a Vec.
pub fn all_tokens(source: &str) -> Vec<AnnotatedToken> {
    let lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    LexerIterator::new(lexer).collect()
}

/// Collect tokens with trivia disabled.
pub fn tokens_skipping_trivia(source: &str) -> Vec<AnnotatedToken> {
    let mut lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    LexerIterator::new(lexer).collect()
}
