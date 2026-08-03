//! Tests for token navigation: peek, next, skip, rewind, is_eof, etc.

use crate::Lexer;
use nitrate_token::{Integer, IntegerKind, Token};

#[test]
fn test_peek_does_not_advance() {
    let mut lexer = Lexer::new(b"a", None).expect("source too big");
    lexer.disable_trivia();
    let p1 = lexer.peek_tok();
    let p2 = lexer.peek_tok();
    assert_eq!(p1, p2, "peek should be idempotent");
    let n = lexer.next_tok();
    assert_eq!(n, p1);
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_skip_tok_skips_next_non_trivia_token() {
    let mut lexer = Lexer::new(b"hello world", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Name("hello".into()));
    lexer.skip_tok();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_next_is() {
    let mut lexer = Lexer::new(b"fn", None).expect("source too big");
    lexer.disable_trivia();
    assert!(lexer.next_is(&Token::Fn));
    assert!(lexer.next_is(&Token::Fn), "next_is should not consume");
    assert_eq!(lexer.next_tok().token, Token::Fn);
}

#[test]
fn test_skip_if_matching() {
    let mut lexer = Lexer::new(b"fn", None).expect("source too big");
    lexer.disable_trivia();
    assert!(lexer.skip_if(&Token::Fn));
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_skip_if_not_matching() {
    let mut lexer = Lexer::new(b"fn", None).expect("source too big");
    lexer.disable_trivia();
    assert!(!lexer.skip_if(&Token::Let));
    assert_eq!(lexer.next_tok().token, Token::Fn);
}

#[test]
fn test_next_if_name() {
    let mut lexer = Lexer::new(b"hello", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_if_name(), Some("hello".into()));
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_next_if_name_not_name() {
    let mut lexer = Lexer::new(b"123", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_if_name(), None);
    assert_eq!(
        lexer.next_tok().token,
        Token::Integer(Integer::new(123, IntegerKind::Dec))
    );
}

#[test]
fn test_is_eof() {
    let mut lexer = Lexer::new(b"a", None).expect("source too big");
    lexer.disable_trivia();
    assert!(!lexer.is_eof());
    lexer.next_tok();
    assert!(lexer.is_eof());
}

#[test]
fn test_rewind_resets_state() {
    let mut lexer = Lexer::new(b"a b", None).expect("source too big");
    lexer.disable_trivia();

    let _a = lexer.next_tok();
    let pos = lexer.current_pos();
    lexer.next_tok();
    lexer.rewind(pos.clone());
    let pos2 = lexer.current_pos();
    assert_eq!(pos2, pos);
    assert_eq!(lexer.next_tok().token, Token::Name("b".into()));
}

#[test]
fn test_rewind_to_beginning() {
    let mut lexer = Lexer::new(b"abc", None).expect("source too big");
    lexer.disable_trivia();

    lexer.next_tok();
    lexer.rewind_raw((None, 0, 0, 0));
    assert_eq!(lexer.next_tok().token, Token::Name("abc".into()));
}

#[test]
fn test_current_pos_after_eof() {
    let mut lexer = Lexer::new(b"x", None).expect("source too big");
    lexer.disable_trivia();
    lexer.next_tok();
    let pos = lexer.current_pos();
    assert_eq!(pos.offset, 1);
    assert_eq!(pos.line, 0);
    assert_eq!(pos.column, 1);
}
