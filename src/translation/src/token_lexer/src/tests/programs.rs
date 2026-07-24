//! Tests for lexing small programs.

use crate::tests::tokens_skipping_trivia;
use nitrate_token::{Integer, IntegerKind, Token};

#[test]
fn test_small_program_hello() {
    let source = "fn main() {\n    println(\"Hello, World!\");\n}";
    let toks = tokens_skipping_trivia(source);
    let expected = [
        Token::Fn,
        Token::Name("main".into()),
        Token::OpenParen,
        Token::CloseParen,
        Token::OpenBrace,
        Token::Name("println".into()),
        Token::OpenParen,
        Token::String("Hello, World!".into()),
        Token::CloseParen,
        Token::Semi,
        Token::CloseBrace,
    ];
    assert_eq!(toks.len(), expected.len());
    for (i, tok) in toks.iter().enumerate() {
        assert_eq!(tok.token, expected[i], "token {i} mismatch");
    }
}

#[test]
fn test_small_program_let_decl() {
    let source = "let x: i32 = 42;";
    let toks = tokens_skipping_trivia(source);
    let expected = [
        Token::Let,
        Token::Name("x".into()),
        Token::Colon,
        Token::I32,
        Token::Eq,
        Token::Integer(Integer::new(42, IntegerKind::Dec)),
        Token::Semi,
    ];
    assert_eq!(toks.len(), expected.len());
    for (i, tok) in toks.iter().enumerate() {
        assert_eq!(tok.token, expected[i], "token {i} mismatch");
    }
}

#[test]
fn test_small_program_with_comments() {
    let source = "# entry point\nfn main() { }";
    let toks = tokens_skipping_trivia(source);
    assert_eq!(toks.len(), 6);
    assert_eq!(toks[0].token, Token::Fn);
    assert_eq!(toks[1].token, Token::Name("main".into()));
    assert_eq!(toks[2].token, Token::OpenParen);
    assert_eq!(toks[3].token, Token::CloseParen);
    assert_eq!(toks[4].token, Token::OpenBrace);
    assert_eq!(toks[5].token, Token::CloseBrace);
}

#[test]
fn test_small_program_with_utf8_identifiers() {
    let source = "fn 計算(x: i32) -> i32 { ret x + 1; }";
    let toks = tokens_skipping_trivia(source);
    let mut it = toks.iter().map(|t| &t.token);
    assert_eq!(it.next(), Some(&Token::Fn));
    assert_eq!(it.next(), Some(&Token::Name("計算".into())));
    assert_eq!(it.next(), Some(&Token::OpenParen));
    assert_eq!(it.next(), Some(&Token::Name("x".into())));
    assert_eq!(it.next(), Some(&Token::Colon));
    assert_eq!(it.next(), Some(&Token::I32));
    assert_eq!(it.next(), Some(&Token::CloseParen));
    assert_eq!(it.next(), Some(&Token::Minus));
    assert_eq!(it.next(), Some(&Token::Gt));
    assert_eq!(it.next(), Some(&Token::I32));
    assert_eq!(it.next(), Some(&Token::OpenBrace));
    assert_eq!(it.next(), Some(&Token::Ret));
    assert_eq!(it.next(), Some(&Token::Name("x".into())));
    assert_eq!(it.next(), Some(&Token::Plus));
    assert_eq!(it.next(), Some(&Token::Integer(Integer::new(1, IntegerKind::Dec))));
    assert_eq!(it.next(), Some(&Token::Semi));
    assert_eq!(it.next(), Some(&Token::CloseBrace));
    assert_eq!(it.next(), None);
}

#[test]
fn test_small_program_with_all_literal_types() {
    let source = r#"let a = 42;
let b = 0xFF;
let c = 3.14;
let d = "hello";
let e = "\x00\xff";
let f = null;
let g = true;
let h = `keyword let`;
"#;
    let toks = tokens_skipping_trivia(source);
    assert!(toks.len() > 20, "expected many tokens from a multi-line program");
    assert_eq!(toks[0].token, Token::Let);
}
