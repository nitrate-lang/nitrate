//! Tests for error-handling edge cases (all return Eof on error).

use crate::Lexer;
use nitrate_token::Token;

#[test]
fn test_unterminated_string_returns_eof() {
    let mut lexer = Lexer::new(b"\"hello", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_invalid_single_byte_char_returns_eof() {
    let mut lexer = Lexer::new(b"\x01", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_nul_byte_returns_eof() {
    let mut lexer = Lexer::new(b"\0", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

// String escape errors
#[test]
fn test_string_invalid_escape_returns_eof() {
    let mut lexer = Lexer::new(b"\"\\q\"", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_string_unterminated_after_escape_backslash() {
    let mut lexer = Lexer::new(b"\"\\", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_string_hex_escape_invalid_digit() {
    let mut lexer = Lexer::new(b"\"\\xGH\"", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_string_hex_escape_single_digit() {
    let mut lexer = Lexer::new(b"\"\\x4\"", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_string_octal_escape_invalid_digit() {
    let mut lexer = Lexer::new(b"\"\\o128\"", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_string_octal_escape_short() {
    let mut lexer = Lexer::new(b"\"\\o77\"", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_string_unicode_escape_without_brace() {
    let mut lexer = Lexer::new(b"\"\\u1F600\"", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_string_unicode_escape_no_hex_digits() {
    let mut lexer = Lexer::new(b"\"\\u{}\"", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_string_unicode_escape_no_close_brace() {
    let mut lexer = Lexer::new(b"\"\\u{1F600\"", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_string_unicode_escape_codepoint_too_large() {
    let mut lexer = Lexer::new(b"\"\\u{110000}\"", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_string_unicode_escape_too_many_digits() {
    let mut lexer = Lexer::new(b"\"\\u{123456789}\"", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_string_unicode_escape_uppercase_u_no_plus() {
    let mut lexer = Lexer::new(b"\"\\uU1F600}\"", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}
