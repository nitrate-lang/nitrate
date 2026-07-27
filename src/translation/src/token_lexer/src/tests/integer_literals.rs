//! Tests for integer literal lexing.

use crate::tests::eq;
use nitrate_token::{Integer, IntegerKind, Token};

macro_rules! int_eq {
    ($src:expr, $val:expr, $kind:ident) => {
        eq($src, Token::Integer(Integer::new($val, IntegerKind::$kind)));
    };
}

#[test]
fn test_integer_dec() {
    int_eq!("42", 42, Dec);
}
#[test]
fn test_integer_dec_zero() {
    int_eq!("0", 0, Dec);
}
#[test]
fn test_integer_dec_max() {
    int_eq!("340282366920938463463374607431768211455", u128::MAX, Dec);
}

#[test]
fn test_integer_bin() {
    int_eq!("0b101010", 0b10_1010, Bin);
}
#[test]
fn test_integer_bin_max() {
    int_eq!(
        "0b11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111",
        u128::MAX,
        Bin
    );
}
#[test]
fn test_integer_bin_single_digit() {
    int_eq!("0b0", 0, Bin);
    int_eq!("0b1", 1, Bin);
}

#[test]
fn test_integer_oct() {
    int_eq!("0o30071", 0o30071, Oct);
}
#[test]
fn test_integer_oct_max() {
    int_eq!("0o3777777777777777777777777777777777777777777", u128::MAX, Oct);
}

#[test]
fn test_integer_hex() {
    int_eq!("0x12345abcdef", 0x0123_45ab_cdef, Hex);
}
#[test]
fn test_integer_hex_max() {
    int_eq!("0xffffffffffffffffffffffffffffffff", u128::MAX, Hex);
}
#[test]
fn test_integer_hex_uppercase() {
    int_eq!("0xABCDEF", 0xAB_CD_EF, Hex);
}

#[test]
fn test_integer_dec_with_underscores() {
    int_eq!("1_000_000", 1_000_000u128, Dec);
}
#[test]
fn test_integer_bin_with_underscores() {
    int_eq!("0b1010_1011_1100", 0b1010_1011_1100u128, Bin);
}
#[test]
fn test_integer_oct_with_underscores() {
    int_eq!("0o777_000", 0o777_000u128, Oct);
}
#[test]
fn test_integer_hex_with_underscores() {
    int_eq!("0xdead_beef", 0xdead_beefu128, Hex);
}

#[test]
fn test_integer_dec_prefix() {
    int_eq!("0d12345", 12345, Dec);
}
#[test]
fn test_integer_dec_prefix_zero() {
    int_eq!("0d0", 0, Dec);
}

// Error edge cases: empty prefixes
use crate::Lexer;
#[test]
fn test_binary_literal_no_digits() {
    let mut lexer = Lexer::new(b"0b", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}
#[test]
fn test_octal_literal_no_digits() {
    let mut lexer = Lexer::new(b"0o", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}
#[test]
fn test_decimal_literal_no_digits() {
    let mut lexer = Lexer::new(b"0d", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}
#[test]
fn test_hex_literal_no_digits() {
    let mut lexer = Lexer::new(b"0x", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

// Overflow edge cases
#[test]
fn test_integer_overflow_bin() {
    let overflow = String::from("0b") + &"1".repeat(129);
    let mut lexer = Lexer::new(overflow.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_integer_overflow_hex() {
    let overflow = String::from("0x") + &"f".repeat(33);
    let mut lexer = Lexer::new(overflow.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_large_integer_does_not_panic() {
    let digits = "9".repeat(38);
    let mut lexer = Lexer::new(digits.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(
        tok.token,
        Token::Integer(Integer::new(digits.parse().unwrap(), IntegerKind::Dec))
    );
}
