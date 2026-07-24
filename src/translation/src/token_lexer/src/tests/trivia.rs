//! Tests for whitespace trivia tokens.

use crate::tests::all_tokens;
use crate::tests::tokens_skipping_trivia;
use nitrate_token::Token;

#[test]
fn test_trivia_space() {
    let toks = all_tokens(" ");
    assert_eq!(toks.len(), 1);
    assert_eq!(toks[0].token, Token::Space);
}

#[test]
fn test_trivia_horizontal_tab() {
    let toks = all_tokens("\t");
    assert_eq!(toks.len(), 1);
    assert_eq!(toks[0].token, Token::HorizontalTab);
}

#[test]
fn test_trivia_newline() {
    let toks = all_tokens("\n");
    assert_eq!(toks.len(), 1);
    assert_eq!(toks[0].token, Token::NewLine);
}

#[test]
fn test_trivia_vertical_tab() {
    let toks = all_tokens("\x0b");
    assert_eq!(toks.len(), 1);
    assert_eq!(toks[0].token, Token::VerticalTab);
}

#[test]
fn test_trivia_form_feed() {
    let toks = all_tokens("\x0c");
    assert_eq!(toks.len(), 1);
    assert_eq!(toks[0].token, Token::FormFeed);
}

#[test]
fn test_trivia_carriage_return() {
    let toks = all_tokens("\r");
    assert_eq!(toks.len(), 1);
    assert_eq!(toks[0].token, Token::CarriageReturn);
}

#[test]
fn test_trivia_disabled_skips_all_whitespace() {
    let toks = tokens_skipping_trivia("   \n\t\x0b\x0c\r  ");
    assert!(
        toks.is_empty(),
        "all whitespace should be skipped when trivia is disabled"
    );
}

#[test]
fn test_trivia_disabled_preserves_non_trivia() {
    let toks = tokens_skipping_trivia("fn \n main");
    assert_eq!(toks.len(), 2);
    assert_eq!(toks[0].token, Token::Fn);
    assert_eq!(toks[1].token, Token::Name("main".into()));
}

#[test]
fn test_trivia_enabled_yields_whitespace_tokens() {
    let toks = all_tokens("a b");
    assert_eq!(toks.len(), 3);
    assert_eq!(toks[0].token, Token::Name("a".into()));
    assert_eq!(toks[1].token, Token::Space);
    assert_eq!(toks[2].token, Token::Name("b".into()));
}

#[test]
fn test_mixed_trivia_types() {
    let toks = all_tokens(" \t\n\x0b\x0c\r ");
    assert_eq!(toks.len(), 7);
    assert_eq!(toks[0].token, Token::Space);
    assert_eq!(toks[1].token, Token::HorizontalTab);
    assert_eq!(toks[2].token, Token::NewLine);
    assert_eq!(toks[3].token, Token::VerticalTab);
    assert_eq!(toks[4].token, Token::FormFeed);
    assert_eq!(toks[5].token, Token::CarriageReturn);
    assert_eq!(toks[6].token, Token::Space);
}

#[test]
fn test_multiple_newlines_produce_multiple_newline_tokens() {
    let toks = all_tokens("\n\n\n");
    assert_eq!(toks.len(), 3);
    for tok in &toks {
        assert_eq!(tok.token, Token::NewLine);
    }
}

#[test]
fn test_toggling_trivia() {
    use crate::Lexer;
    let mut lexer = Lexer::new(b"a b", None).expect("source too big");
    lexer.disable_trivia();
    let tok1 = lexer.next_tok();
    assert_eq!(tok1.token, Token::Name("a".into()));

    lexer.enable_trivia();
    let tok2 = lexer.next_tok();
    assert_eq!(tok2.token, Token::Space);

    lexer.disable_trivia();
    let tok3 = lexer.next_tok();
    assert_eq!(tok3.token, Token::Name("b".into()));
}
