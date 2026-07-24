//! Tests for string and binary string literal lexing.

use crate::Lexer;
use crate::tests::eq;
use nitrate_token::Token;

#[test]
fn test_string_empty() {
    eq("\"\"", Token::String("".into()));
}
#[test]
fn test_string_ascii() {
    eq("\"Hello, World!\"", Token::String("Hello, World!".into()));
}

#[test]
fn test_string_utf8() {
    let source = "\"π_ρογρ🎄αμματι\t\nσμός🔥\"";
    let mut lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::String("π_ρογρ🎄αμματι\t\nσμός🔥".into()));
    assert_eq!(tok.start_offset, 0);
    assert_eq!(tok.end_offset, 43);
    assert_eq!(tok.end_line, 1);
}

#[test]
fn test_string_escape_sequences() {
    let source = r#""\0\a\b\t\n\v\f\r\\\'\"\x41\o453\u{1F600}""#;
    let mut lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::String("\0\u{7}\u{8}\t\n\u{b}\u{c}\r\\'\"A+😀".into()));
    assert_eq!(tok.end_offset, 42);
}

#[test]
fn test_string_unicode_escape_maximal() {
    let source = "\"\\u{10FFFF}\"";
    let mut lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::String("\u{10FFFF}".into()));
}

#[test]
fn test_string_unicode_escape_boundary() {
    let source = "\"\\u{0}\"";
    let mut lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::String("\0".into()));
}

#[test]
fn test_string_multiple_escapes() {
    let source = r#""\n\t\r\\""#;
    let mut lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::String("\n\t\r\\".into()));
}

#[test]
fn test_string_only_escapes_empty_result() {
    let source = r#""\0\0""#;
    let mut lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::String("\0\0".into()));
}

#[test]
fn test_string_with_raw_tab_and_newline_inside() {
    let source = "\"hello\tworld\n\"";
    let mut lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::String("hello\tworld\n".into()));
}

#[test]
fn test_string_crlf_is_not_stripped() {
    let source = "\"a\r\nb\"";
    let mut lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::String("a\r\nb".into()));
}

#[test]
fn test_string_spanning_lines_then_newline() {
    use crate::tests::all_tokens;
    let source = "\"hello\nworld\"\n";
    let toks = all_tokens(source);
    assert_eq!(toks.len(), 2);
    assert_eq!(toks[0].token, Token::String("hello\nworld".into()));
    assert_eq!(toks[1].token, Token::NewLine);
}

// Binary strings
#[test]
fn test_bstring_via_hex_escapes() {
    let source = r#""\x9b\xeb\xdd\x44\xde\x13\xfd\x17\x97\xac\xf9\xe5\x4d\xb2\x78\xcd""#;
    let mut lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(
        tok.token,
        Token::BString(b"\x9b\xeb\xdd\x44\xde\x13\xfd\x17\x97\xac\xf9\xe5\x4d\xb2\x78\xcd".into())
    );
}

#[test]
fn test_bstring_via_octal_escapes() {
    let source = r#""\o077\o377""#;
    let mut lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::BString(b"\x3f\xff".into()));
}

#[test]
fn test_bstring_mixed_valid_and_invalid_utf8() {
    let source = r#""Hello\xfeWorld""#;
    let mut lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::BString(b"Hello\xfeWorld".into()));
}

#[test]
fn test_bstring_from_invalid_utf8_no_escape() {
    let source = b"\"\xff\"";
    let mut lexer = Lexer::new(source, None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::BString(b"\xff".to_vec()));
}
