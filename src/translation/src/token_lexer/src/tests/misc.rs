//! Miscellaneous tests (AnnotatedToken, position tracking, large inputs, etc.).

use crate::Lexer;
use crate::tests::all_tokens;
use crate::tests::tokens_skipping_trivia;
use nitrate_token::{AnnotatedToken, Integer, IntegerKind, SourcePosition, Token};
use ordered_float::NotNan;

// AnnotatedToken constructors
#[test]
fn test_annotated_token_constructors() {
    let start = SourcePosition {
        line: 0,
        column: 0,
        offset: 0,
        fileid: None,
    };
    let end = SourcePosition {
        line: 0,
        column: 3,
        offset: 3,
        fileid: None,
    };
    let at = AnnotatedToken::new(Token::Fn, start.clone(), end.clone());
    assert_eq!(at.start(), start);
    assert_eq!(at.end(), end);
    assert_eq!(at.range(), (start, end));
}

#[test]
fn test_annotated_token_token_field() {
    let at = AnnotatedToken::new(
        Token::Integer(Integer::new(42, IntegerKind::Dec)),
        SourcePosition {
            line: 0,
            column: 0,
            offset: 0,
            fileid: None,
        },
        SourcePosition {
            line: 0,
            column: 2,
            offset: 2,
            fileid: None,
        },
    );
    assert_eq!(at.token, Token::Integer(Integer::new(42, IntegerKind::Dec)));
}

// Position tracking
#[test]
fn test_position_tracking_integer_dec() {
    let source = "42";
    let mut lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.start_line, 0);
    assert_eq!(tok.start_column, 0);
    assert_eq!(tok.start_offset, 0);
    assert_eq!(tok.end_line, 0);
    assert_eq!(tok.end_column, 2);
    assert_eq!(tok.end_offset, 2);
}

#[test]
fn test_position_tracking_multiline() {
    let source = "a\nb";
    let mut lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();

    let tok_a = lexer.next_tok();
    assert_eq!(tok_a.token, Token::Name("a".into()));
    assert_eq!(tok_a.start_line, 0);
    assert_eq!(tok_a.start_column, 0);
    assert_eq!(tok_a.start_offset, 0);
    assert_eq!(tok_a.end_line, 0);
    assert_eq!(tok_a.end_offset, 1);

    let tok_b = lexer.next_tok();
    assert_eq!(tok_b.token, Token::Name("b".into()));
    assert_eq!(tok_b.start_line, 1);
    assert_eq!(tok_b.start_column, 0);
    assert_eq!(tok_b.start_offset, 2);
    assert_eq!(tok_b.end_line, 1);
    assert_eq!(tok_b.end_offset, 3);
}

// Large input stress tests
#[test]
fn test_large_identifier_does_not_panic() {
    let long_id = "a".repeat(1000);
    let mut lexer = Lexer::new(long_id.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::Name(long_id.clone().into()));
    assert_eq!(tok.end_offset, 1000);
}

#[test]
fn test_empty_source_trivia_disabled() {
    let toks = tokens_skipping_trivia("");
    assert!(toks.is_empty());
}

#[test]
fn test_empty_source_yields_no_tokens() {
    let toks = all_tokens("");
    assert!(toks.is_empty());
}

#[test]
fn test_trailing_dot() {
    use crate::tests::eq;
    eq(".", Token::Dot);
}

#[test]
fn test_trivia_disabled_eof_after_whitespace_only() {
    let mut lexer = Lexer::new(b"   ", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}
