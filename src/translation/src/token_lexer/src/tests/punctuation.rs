//! Tests for single-byte punctuation tokens.

use crate::tests::eq;
use nitrate_token::Token;

#[test]
fn test_single_quote() {
    eq("'", Token::SingleQuote);
}
#[test]
fn test_semi() {
    eq(";", Token::Semi);
}
#[test]
fn test_comma() {
    eq(",", Token::Comma);
}
#[test]
fn test_dot() {
    eq(".", Token::Dot);
}
#[test]
fn test_open_paren() {
    eq("(", Token::OpenParen);
}
#[test]
fn test_close_paren() {
    eq(")", Token::CloseParen);
}
#[test]
fn test_open_brace() {
    eq("{", Token::OpenBrace);
}
#[test]
fn test_close_brace() {
    eq("}", Token::CloseBrace);
}
#[test]
fn test_open_bracket() {
    eq("[", Token::OpenBracket);
}
#[test]
fn test_close_bracket() {
    eq("]", Token::CloseBracket);
}
#[test]
fn test_at() {
    eq("@", Token::At);
}
#[test]
fn test_tilde() {
    eq("~", Token::Tilde);
}
#[test]
fn test_question() {
    eq("?", Token::Question);
}
#[test]
fn test_colon() {
    eq(":", Token::Colon);
}
#[test]
fn test_dollar() {
    eq("$", Token::Dollar);
}
#[test]
fn test_eq() {
    eq("=", Token::Eq);
}
#[test]
fn test_bang() {
    eq("!", Token::Bang);
}
#[test]
fn test_lt() {
    eq("<", Token::Lt);
}
#[test]
fn test_gt() {
    eq(">", Token::Gt);
}
#[test]
fn test_minus() {
    eq("-", Token::Minus);
}
#[test]
fn test_and() {
    eq("&", Token::And);
}
#[test]
fn test_or() {
    eq("|", Token::Or);
}
#[test]
fn test_plus() {
    eq("+", Token::Plus);
}
#[test]
fn test_star() {
    eq("*", Token::Star);
}
#[test]
fn test_slash() {
    eq("/", Token::Slash);
}
#[test]
fn test_caret() {
    eq("^", Token::Caret);
}
#[test]
fn test_percent() {
    eq("%", Token::Percent);
}

#[test]
fn test_multiple_punctuations() {
    use crate::tests::tokens_skipping_trivia;
    let toks = tokens_skipping_trivia("+-*/;");
    assert_eq!(toks.len(), 5);
    assert_eq!(toks[0].token, Token::Plus);
    assert_eq!(toks[1].token, Token::Minus);
    assert_eq!(toks[2].token, Token::Star);
    assert_eq!(toks[3].token, Token::Slash);
    assert_eq!(toks[4].token, Token::Semi);
}

#[test]
fn test_consecutive_dots() {
    use crate::tests::tokens_skipping_trivia;
    let toks = tokens_skipping_trivia("...");
    assert_eq!(toks.len(), 3);
    for tok in &toks {
        assert_eq!(tok.token, Token::Dot);
    }
}
