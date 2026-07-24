//! Tests for LexerIterator integration.

use crate::{Lexer, LexerIterator};

#[test]
fn test_lexer_iterator_smoke() {
    let source = "fn main() { }";
    let toks: Vec<_> = LexerIterator::new(Lexer::new(source.as_bytes(), None).unwrap()).collect();
    assert!(!toks.is_empty(), "should produce tokens");
}

#[test]
fn test_lexer_iterator_empty_source() {
    let toks: Vec<_> = LexerIterator::new(Lexer::new(b"", None).unwrap()).collect();
    assert!(toks.is_empty(), "should produce no tokens");
}

#[test]
fn test_lexer_iterator_yields_all_tokens_including_trivia() {
    use crate::tests::all_tokens;
    let toks = all_tokens("a b");
    assert_eq!(toks.len(), 3);
}

#[test]
fn test_lexer_iterator_with_trivia_disabled_skips_whitespace() {
    let mut lexer = Lexer::new(b"a b", None).expect("source too big");
    lexer.disable_trivia();
    let iter = LexerIterator::new(lexer);
    let toks: Vec<_> = iter.collect();
    assert_eq!(toks.len(), 2);
    assert_eq!(toks[0].token, nitrate_token::Token::Name("a".into()));
    assert_eq!(toks[1].token, nitrate_token::Token::Name("b".into()));
}

#[test]
fn test_lexer_iterator_yields_no_eof() {
    use nitrate_token::Token;
    let mut lexer = Lexer::new(b"a b", None).expect("source too big");
    lexer.disable_trivia();
    let toks: Vec<_> = LexerIterator::new(lexer).collect();
    for t in &toks {
        assert_ne!(t.token, Token::Eof, "LexerIterator should not yield Eof");
    }
}
