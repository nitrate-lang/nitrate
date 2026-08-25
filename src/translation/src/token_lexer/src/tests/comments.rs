//! Tests for comment lexing.
//!
//! Comment text includes the delimiter markers so the LSP can distinguish
//! comment kinds by inspecting the stored text:
//! - `#` comments store the full text including `#`.
//! - `//` comments store the full text including `//`.
//! - `/* */` comments store the full text including `/*` and `*/`.

use crate::tests::{all_tokens, tokens_skipping_trivia};
use nitrate_token::{Comment, CommentKind, Integer, IntegerKind, Token};

#[test]
fn test_hash_comment_basic() {
    let source = "# This is a comment";
    let toks = all_tokens(source);
    assert_eq!(toks.len(), 1);
    assert_eq!(
        toks[0].token,
        Token::Comment(Comment::new("# This is a comment".into(), CommentKind::SingleLine)),
    );
}

#[test]
fn test_hash_comment_empty() {
    let source = "#";
    let toks = all_tokens(source);
    assert_eq!(toks.len(), 1);
    assert_eq!(
        toks[0].token,
        Token::Comment(Comment::new("#".into(), CommentKind::SingleLine)),
    );
}

#[test]
fn test_hash_comment_with_carriage_return() {
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
fn test_hash_comment_after_code() {
    let toks = tokens_skipping_trivia("let x = 5; # some comment");
    assert_eq!(toks.len(), 5);
    assert_eq!(toks[0].token, Token::Let);
    assert_eq!(toks[1].token, Token::Name("x".into()));
    assert_eq!(toks[2].token, Token::Eq);
    assert_eq!(toks[3].token, Token::Integer(Integer::new(5, IntegerKind::Dec)));
    assert_eq!(toks[4].token, Token::Semi);
}

#[test]
fn test_multiple_hash_comments_separated_by_newlines() {
    let toks = tokens_skipping_trivia("# a\n# b\n");
    assert!(toks.is_empty());
}

// --- `//` line comment tests ---

#[test]
fn test_slash_slash_comment_basic() {
    let source = "// This is a comment";
    let toks = all_tokens(source);
    assert_eq!(toks.len(), 1);
    assert_eq!(
        toks[0].token,
        Token::Comment(Comment::new("// This is a comment".into(), CommentKind::SingleLine)),
    );
}

#[test]
fn test_slash_slash_comment_empty() {
    let source = "//";
    let toks = all_tokens(source);
    assert_eq!(toks.len(), 1);
    assert_eq!(
        toks[0].token,
        Token::Comment(Comment::new("//".into(), CommentKind::SingleLine)),
    );
}

#[test]
fn test_slash_slash_comment_with_carriage_return() {
    let source = "// comment\r\n";
    let toks = all_tokens(source);
    assert_eq!(toks.len(), 2);
    assert_eq!(
        toks[0].token,
        Token::Comment(Comment::new("// comment".into(), CommentKind::SingleLine)),
    );
    assert_eq!(toks[1].token, Token::NewLine);
}

#[test]
fn test_slash_slash_comment_after_code() {
    let toks = tokens_skipping_trivia("let x = 5; // some comment");
    assert_eq!(toks.len(), 5);
    assert_eq!(toks[0].token, Token::Let);
    assert_eq!(toks[1].token, Token::Name("x".into()));
    assert_eq!(toks[2].token, Token::Eq);
    assert_eq!(toks[3].token, Token::Integer(Integer::new(5, IntegerKind::Dec)));
    assert_eq!(toks[4].token, Token::Semi);
}

#[test]
fn test_multiple_slash_slash_comments_separated_by_newlines() {
    let toks = tokens_skipping_trivia("// a\n// b\n");
    assert!(toks.is_empty());
}

#[test]
fn test_slash_slash_comment_with_special_chars() {
    let source = "// comment with $pecial @#$% characters!";
    let toks = all_tokens(source);
    assert_eq!(toks.len(), 1);
    assert_eq!(
        toks[0].token,
        Token::Comment(Comment::new(
            "// comment with $pecial @#$% characters!".into(),
            CommentKind::SingleLine
        )),
    );
}

#[test]
fn test_slash_slash_comment_immediately_followed_by_code_on_next_line() {
    let toks = tokens_skipping_trivia("// comment\nlet x = 1;");
    assert_eq!(toks.len(), 5);
    assert_eq!(toks[0].token, Token::Let);
    assert_eq!(toks[1].token, Token::Name("x".into()));
    assert_eq!(toks[2].token, Token::Eq);
    assert_eq!(toks[3].token, Token::Integer(Integer::new(1, IntegerKind::Dec)));
    assert_eq!(toks[4].token, Token::Semi);
}

#[test]
fn test_slash_slash_comment_eof_no_newline() {
    let source = "// comment at end of file";
    let toks = all_tokens(source);
    assert_eq!(toks.len(), 1);
    assert_eq!(
        toks[0].token,
        Token::Comment(Comment::new(
            "// comment at end of file".into(),
            CommentKind::SingleLine
        )),
    );
}

// --- `/* */` block comment tests ---
// Block comments store the full text including /* and */ delimiters.

#[test]
fn test_block_comment_basic() {
    let source = "/* block comment */";
    let toks = all_tokens(source);
    assert_eq!(toks.len(), 1);
    assert_eq!(
        toks[0].token,
        Token::Comment(Comment::new("/* block comment */".into(), CommentKind::MultiLine)),
    );
}

#[test]
fn test_block_comment_empty() {
    let source = "/**/";
    let toks = all_tokens(source);
    assert_eq!(toks.len(), 1);
    assert_eq!(
        toks[0].token,
        Token::Comment(Comment::new("/**/".into(), CommentKind::MultiLine)),
    );
}

#[test]
fn test_block_comment_multiline() {
    let source = "/* line1\nline2\nline3 */";
    let toks = all_tokens(source);
    assert_eq!(toks.len(), 1);
    assert_eq!(
        toks[0].token,
        Token::Comment(Comment::new("/* line1\nline2\nline3 */".into(), CommentKind::MultiLine)),
    );
}

#[test]
fn test_block_comment_after_code() {
    let toks = tokens_skipping_trivia("let x = 5; /* block */");
    assert_eq!(toks.len(), 5);
    assert_eq!(toks[0].token, Token::Let);
    assert_eq!(toks[1].token, Token::Name("x".into()));
    assert_eq!(toks[2].token, Token::Eq);
    assert_eq!(toks[3].token, Token::Integer(Integer::new(5, IntegerKind::Dec)));
    assert_eq!(toks[4].token, Token::Semi);
}

#[test]
fn test_block_comment_before_code() {
    let toks = tokens_skipping_trivia("/* before */ let x = 1;");
    assert_eq!(toks.len(), 5);
    assert_eq!(toks[0].token, Token::Let);
    assert_eq!(toks[1].token, Token::Name("x".into()));
    assert_eq!(toks[2].token, Token::Eq);
    assert_eq!(toks[3].token, Token::Integer(Integer::new(1, IntegerKind::Dec)));
    assert_eq!(toks[4].token, Token::Semi);
}

#[test]
fn test_block_comment_in_the_middle_of_code() {
    let toks = tokens_skipping_trivia("let /* comment */ x = 1;");
    assert_eq!(toks.len(), 5);
    assert_eq!(toks[0].token, Token::Let);
    assert_eq!(toks[1].token, Token::Name("x".into()));
    assert_eq!(toks[2].token, Token::Eq);
    assert_eq!(toks[3].token, Token::Integer(Integer::new(1, IntegerKind::Dec)));
    assert_eq!(toks[4].token, Token::Semi);
}

#[test]
fn test_block_comment_with_nested_looking_content() {
    let source = "/* contains /* nested looking */";
    let toks = all_tokens(source);
    // The first `*/` terminates the comment — the stored text includes delimiters
    assert_eq!(
        toks[0].token,
        Token::Comment(Comment::new(
            "/* contains /* nested looking */".into(),
            CommentKind::MultiLine,
        ))
    );
}

#[test]
fn test_block_comment_multiple_stars_before_close() {
    let source = "/**/";
    let toks = all_tokens(source);
    assert_eq!(
        toks[0].token,
        Token::Comment(Comment::new("/**/".into(), CommentKind::MultiLine))
    );
}

#[test]
fn test_block_comment_many_stars() {
    let source = "/*****/";
    let toks = all_tokens(source);
    assert_eq!(
        toks[0].token,
        Token::Comment(Comment::new("/*****/".into(), CommentKind::MultiLine)),
    );
}

#[test]
fn test_block_comment_with_newlines_inside() {
    let source = "/* line1\nline2\nline3\n*/";
    let toks = all_tokens(source);
    assert_eq!(
        toks[0].token,
        Token::Comment(Comment::new(
            "/* line1\nline2\nline3\n*/".into(),
            CommentKind::MultiLine
        )),
    );
}

#[test]
fn test_block_comment_with_carriage_returns() {
    let source = "/* line1\r\nline2\r\n*/";
    let toks = all_tokens(source);
    assert_eq!(
        toks[0].token,
        Token::Comment(Comment::new("/* line1\r\nline2\r\n*/".into(), CommentKind::MultiLine)),
    );
}

#[test]
fn test_block_comment_with_special_chars() {
    let source = "/* special: $%^&*()_+-=[]{}|;:',.<>?/~` */";
    let toks = all_tokens(source);
    assert_eq!(
        toks[0].token,
        Token::Comment(Comment::new(
            "/* special: $%^&*()_+-=[]{}|;:',.<>?/~` */".into(),
            CommentKind::MultiLine
        )),
    );
}

#[test]
fn test_block_comment_eof_without_close_is_error() {
    // When errors occur, the lexer returns Token::Eof
    let toks = tokens_skipping_trivia("/* unterminated");
    assert!(toks.is_empty());
}

#[test]
fn test_block_comment_eof_newline_then_eof() {
    let toks = tokens_skipping_trivia("/* unterminated\n");
    assert!(toks.is_empty());
}

#[test]
fn test_block_comment_with_trailing_code_on_same_line() {
    let toks = tokens_skipping_trivia("/* comment */ let x = 1;");
    assert_eq!(toks.len(), 5);
    assert_eq!(toks[0].token, Token::Let);
    assert_eq!(toks[1].token, Token::Name("x".into()));
    assert_eq!(toks[2].token, Token::Eq);
    assert_eq!(toks[3].token, Token::Integer(Integer::new(1, IntegerKind::Dec)));
    assert_eq!(toks[4].token, Token::Semi);
}

#[test]
fn test_block_comment_exact_three_char() {
    let source = "/*a*/";
    let toks = all_tokens(source);
    assert_eq!(
        toks[0].token,
        Token::Comment(Comment::new("/*a*/".into(), CommentKind::MultiLine)),
    );
}

// --- Mixed comment types ---

#[test]
fn test_mixed_hash_and_slash_slash_comments() {
    let toks = tokens_skipping_trivia("# hash comment\n// slash comment");
    assert!(toks.is_empty());
}

#[test]
fn test_mixed_hash_and_block_comments() {
    let toks = tokens_skipping_trivia("# hash comment\n/* block */");
    assert!(toks.is_empty());
}

#[test]
fn test_slash_is_still_division_operator_when_alone() {
    let toks = tokens_skipping_trivia("a / b");
    assert_eq!(toks.len(), 3);
    assert_eq!(toks[0].token, Token::Name("a".into()));
    assert_eq!(toks[1].token, Token::Slash);
    assert_eq!(toks[2].token, Token::Name("b".into()));
}

#[test]
fn test_slash_followed_by_non_slash_or_star_is_division() {
    let toks = tokens_skipping_trivia("a /= b");
    assert_eq!(toks.len(), 4);
    assert_eq!(toks[0].token, Token::Name("a".into()));
    assert_eq!(toks[1].token, Token::Slash);
    assert_eq!(toks[2].token, Token::Eq);
    assert_eq!(toks[3].token, Token::Name("b".into()));
}

#[test]
fn test_block_comment_with_newlines_skipped_as_trivia() {
    // When skipping trivia, a multi-line block comment should be invisible
    let toks = tokens_skipping_trivia("let x = 1; /* line1\nline2\nline3 */ let y = 2;");
    assert_eq!(toks.len(), 10);
    assert_eq!(toks[0].token, Token::Let);
    assert_eq!(toks[1].token, Token::Name("x".into()));
    assert_eq!(toks[2].token, Token::Eq);
    assert_eq!(toks[3].token, Token::Integer(Integer::new(1, IntegerKind::Dec)));
    assert_eq!(toks[4].token, Token::Semi);
    assert_eq!(toks[5].token, Token::Let);
    assert_eq!(toks[6].token, Token::Name("y".into()));
    assert_eq!(toks[7].token, Token::Eq);
    assert_eq!(toks[8].token, Token::Integer(Integer::new(2, IntegerKind::Dec)));
    assert_eq!(toks[9].token, Token::Semi);
}

#[test]
fn test_all_three_comment_types_interleaved() {
    let toks = tokens_skipping_trivia(
        "# hash\n\
         // slash\n\
         /* block */\n\
         let x = 1;",
    );
    assert_eq!(toks.len(), 5);
    assert_eq!(toks[0].token, Token::Let);
    assert_eq!(toks[1].token, Token::Name("x".into()));
    assert_eq!(toks[2].token, Token::Eq);
    assert_eq!(toks[3].token, Token::Integer(Integer::new(1, IntegerKind::Dec)));
    assert_eq!(toks[4].token, Token::Semi);
}

#[test]
fn test_block_comment_adjacent_to_hash_line_comment() {
    let toks = tokens_skipping_trivia("/* block */# hash comment\nlet x = 1;");
    assert_eq!(toks.len(), 5);
    assert_eq!(toks[0].token, Token::Let);
    assert_eq!(toks[1].token, Token::Name("x".into()));
    assert_eq!(toks[2].token, Token::Eq);
    assert_eq!(toks[3].token, Token::Integer(Integer::new(1, IntegerKind::Dec)));
    assert_eq!(toks[4].token, Token::Semi);
}

#[test]
fn test_block_comment_at_end_of_file_no_newline() {
    let source = "123 /* trailing block */";
    let toks = all_tokens(source);
    assert_eq!(toks.len(), 3);
    assert_eq!(toks[0].token, Token::Integer(Integer::new(123, IntegerKind::Dec)));
    assert_eq!(toks[1].token, Token::Space);
    assert_eq!(
        toks[2].token,
        Token::Comment(Comment::new("/* trailing block */".into(), CommentKind::MultiLine)),
    );
}

#[test]
fn test_block_comment_star_in_middle_but_no_slash() {
    // "/* a * b */" — the '*' before 'b' should be part of comment content
    let source = "/* a * b */";
    let toks = all_tokens(source);
    assert_eq!(
        toks[0].token,
        Token::Comment(Comment::new("/* a * b */".into(), CommentKind::MultiLine)),
    );
}

#[test]
fn test_block_comment_multiple_consecutive_block_comments() {
    let toks = tokens_skipping_trivia("/* first *//* second */ let x = 1;");
    assert_eq!(toks.len(), 5);
    assert_eq!(toks[0].token, Token::Let);
    assert_eq!(toks[1].token, Token::Name("x".into()));
    assert_eq!(toks[2].token, Token::Eq);
    assert_eq!(toks[3].token, Token::Integer(Integer::new(1, IntegerKind::Dec)));
    assert_eq!(toks[4].token, Token::Semi);
}

#[test]
fn test_slash_slash_comment_adjacent_to_block_comment() {
    let toks = tokens_skipping_trivia("// line\n/* block */\nlet x = 1;");
    assert_eq!(toks.len(), 5);
    assert_eq!(toks[0].token, Token::Let);
    assert_eq!(toks[1].token, Token::Name("x".into()));
    assert_eq!(toks[2].token, Token::Eq);
    assert_eq!(toks[3].token, Token::Integer(Integer::new(1, IntegerKind::Dec)));
    assert_eq!(toks[4].token, Token::Semi);
}
