//! Tests for float literal lexing.

use crate::Lexer;
use crate::tests::eq;
use crate::tests::tokens_skipping_trivia;
use nitrate_token::{Integer, IntegerKind, Token};
use ordered_float::NotNan;

macro_rules! float_eq {
    ($src:expr, $val:expr) => {
        eq($src, Token::Float(NotNan::new($val).unwrap()));
    };
}

#[test]
fn test_float_basic() {
    float_eq!("123.456", 123.456);
}
#[test]
fn test_float_zero_point_zero() {
    float_eq!("0.0", 0.0);
}
#[test]
fn test_float_from_zero_prefix() {
    float_eq!("0.5", 0.5);
}
#[test]
fn test_float_zero_prefix_with_underscore() {
    float_eq!("0_0.5", 0.5);
}
#[test]
fn test_float_with_underscores() {
    float_eq!("1_234.567_89", 1234.56789);
}

#[test]
fn test_float_leading_dot_produces_dot_then_integer() {
    let toks = tokens_skipping_trivia(".5");
    assert_eq!(toks.len(), 2);
    assert_eq!(toks[0].token, Token::Dot);
    assert_eq!(toks[1].token, Token::Integer(Integer::new(5, IntegerKind::Dec)));
}

#[test]
fn test_float_trailing_dot_produces_integer_then_dot() {
    let toks = tokens_skipping_trivia("5.");
    assert_eq!(toks.len(), 2);
    assert_eq!(toks[0].token, Token::Integer(Integer::new(5, IntegerKind::Dec)));
    assert_eq!(toks[1].token, Token::Dot);
}

#[test]
fn test_float_scientific_notation_tokens() {
    // The lexer does NOT parse scientific notation as a single float.
    let toks = tokens_skipping_trivia("3.4028235e+38");
    assert_eq!(toks.len(), 4);
    assert_eq!(toks[0].token, Token::Float(NotNan::new(3.4028235).unwrap()));
    assert_eq!(toks[1].token, Token::Name("e".into()));
    assert_eq!(toks[2].token, Token::Plus);
    assert_eq!(toks[3].token, Token::Integer(Integer::new(38, IntegerKind::Dec)));
}

#[test]
fn test_float_with_underscores_detailed() {
    let source = "1_234.567_89";
    let mut lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::Float(NotNan::new(1234.56789).unwrap()));
}
