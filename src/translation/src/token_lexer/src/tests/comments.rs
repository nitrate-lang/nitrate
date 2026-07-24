//! Tests for comment lexing.

use crate::tests::{all_tokens, tokens_skipping_trivia};
use nitrate_token::{Comment, CommentKind, Integer, IntegerKind, Token};

#[test]
fn test_comment_basic() {
    let source = "# This is a comment";
    let toks = all_tokens(source);
    assert_eq!(toks.len(), 1);
    assert_eq!(
        toks[0].token,
        Token::Comment(Comment::new("# This is a comment".into(), CommentKind::SingleLine)),
    );
}

#[test]
fn test_comment_empty() {
    let source = "#";
    let toks = all_tokens(source);
    assert_eq!(toks.len(), 1);
    assert_eq!(
        toks[0].token,
        Token::Comment(Comment::new("#".into(), CommentKind::SingleLine)),
    );
}

#[test]
fn test_comment_with_carriage_return() {
    let source = "# comment\r\n";
    let toks = all_tokens(source);
    assert_eq!(toks.len(), 2);
    assert_eq!(
        toks[0].token,
        Token::Comment(Comment::new("# comment".into(), CommentKind::SingleLine)),
    );
    assert_eq!(toks[1].token, Token::NewLine);
}

#[test]
fn test_comment_after_code() {
    let toks = tokens_skipping_trivia("let x = 5; # some comment");
    assert_eq!(toks.len(), 5);
    assert_eq!(toks[0].token, Token::Let);
    assert_eq!(toks[1].token, Token::Name("x".into()));
    assert_eq!(toks[2].token, Token::Eq);
    assert_eq!(toks[3].token, Token::Integer(Integer::new(5, IntegerKind::Dec)));
    assert_eq!(toks[4].token, Token::Semi);
}

#[test]
fn test_multiple_comments_separated_by_newlines() {
    let toks = tokens_skipping_trivia("# a\n# b\n");
    assert!(toks.is_empty());
}
