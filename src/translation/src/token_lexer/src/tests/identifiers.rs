//! Tests for identifier lexing (typical and atypical/backtick).

use crate::Lexer;
use crate::tests::eq;
use crate::tests::tokens_skipping_trivia;
use nitrate_token::Token;

// Typical identifiers
#[test]
fn test_identifier_ascii() {
    eq("helloWorld_42", Token::Name("helloWorld_42".into()));
}
#[test]
fn test_identifier_starts_with_underscore() {
    eq("_private", Token::Name("_private".into()));
}
#[test]
fn test_identifier_starts_with_underscore_digits() {
    eq("_42", Token::Name("_42".into()));
}
#[test]
fn test_identifier_single_character() {
    eq("x", Token::Name("x".into()));
}

#[test]
fn test_identifier_utf8() {
    let toks = tokens_skipping_trivia("π_ρογρ🎄αμματισμός🔥");
    assert_eq!(toks.len(), 1);
    assert_eq!(toks[0].token, Token::Name("π_ρογρ🎄αμματισμός🔥".into()));
    assert_eq!(toks[0].start_offset, 0);
    assert_eq!(toks[0].end_offset, 39);
}

#[test]
fn test_identifier_multi_byte_offset() {
    let source = "abcλπ";
    let mut lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::Name("abcλπ".into()));
    assert_eq!(tok.start_offset, 0);
    assert_eq!(tok.end_offset, 7);
}

#[test]
fn test_identifier_emoji_only() {
    let source = "🔥";
    let mut lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::Name("🔥".into()));
    assert_eq!(tok.start_offset, 0);
    assert_eq!(tok.end_offset, 4);
}

#[test]
fn test_identifier_hindi() {
    let source = "नमस्ते";
    let mut lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::Name("नमस्ते".into()));
    assert_eq!(tok.start_offset, 0);
    assert_eq!(tok.end_offset, 18);
}

#[test]
fn test_identifier_chinese() {
    let source = "你好世界";
    let mut lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::Name("你好世界".into()));
    assert_eq!(tok.start_offset, 0);
    assert_eq!(tok.end_offset, 12);
}

#[test]
fn test_digit_starts_number_not_identifier() {
    let toks = tokens_skipping_trivia("42abc");
    assert_eq!(toks.len(), 2);
    assert_eq!(
        toks[0].token,
        Token::Integer(nitrate_token::Integer::new(42, nitrate_token::IntegerKind::Dec))
    );
    assert_eq!(toks[1].token, Token::Name("abc".into()));
}

// Atypical (backtick) identifiers
#[test]
fn test_atypical_identifier_basic() {
    eq("`hello world`", Token::Name("hello world".into()));
}
#[test]
fn test_atypical_identifier_contains_keyword() {
    eq("`let`", Token::Name("let".into()));
}
#[test]
fn test_atypical_identifier_empty() {
    eq("``", Token::Name("".into()));
}

#[test]
fn test_atypical_identifier_utf8() {
    let source = "`🔥`";
    let mut lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::Name("🔥".into()));
    assert_eq!(tok.start_offset, 0);
    assert_eq!(tok.end_offset, 6);
}

#[test]
fn test_atypical_identifier_multiline() {
    let source = "`π_ρ \\n \\0ο🎄αμ\nματι σς🔥`";
    let mut lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::Name("π_ρ \\n \\0ο🎄αμ\nματι σς🔥".into()));
    assert_eq!(tok.start_offset, 0);
    assert_eq!(tok.end_offset, 41);
    assert_eq!(tok.start_line, 0);
    assert_eq!(tok.end_line, 1);
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

// Reserved prefix edge cases
#[test]
fn test_typical_identifier_reserved_prefix() {
    let mut lexer = Lexer::new("\u{2699}\u{fe0f}reserved".as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::Eof);
}

#[test]
fn test_atypical_identifier_reserved_prefix() {
    let mut lexer = Lexer::new("`\u{2699}\u{fe0f}reserved`".as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::Eof);
}

#[test]
fn test_atypical_identifier_unterminated() {
    let mut lexer = Lexer::new(b"`hello", None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::Eof);
}

#[test]
fn test_atypical_identifier_containing_keyword_returns_name() {
    let mut lexer = Lexer::new(b"`impl`", None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::Name("impl".into()));
}
