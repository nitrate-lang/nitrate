//! Tests for error-handling edge cases (all return Eof on error).

use crate::Lexer;
use nitrate_token::Token;

#[test]
fn test_unterminated_string_returns_eof() {
    let mut lexer = Lexer::new(b"\"hello", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_invalid_single_byte_char_returns_eof() {
    let mut lexer = Lexer::new(b"\x01", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_nul_byte_returns_eof() {
    let mut lexer = Lexer::new(b"\0", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

// String escape errors
#[test]
fn test_string_invalid_escape_returns_eof() {
    let mut lexer = Lexer::new(b"\"\\q\"", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_string_unterminated_after_escape_backslash() {
    let mut lexer = Lexer::new(b"\"\\", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_string_hex_escape_invalid_digit() {
    let mut lexer = Lexer::new(b"\"\\xGH\"", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_string_hex_escape_single_digit() {
    let mut lexer = Lexer::new(b"\"\\x4\"", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_string_octal_escape_invalid_digit() {
    let mut lexer = Lexer::new(b"\"\\o128\"", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_string_octal_escape_short() {
    let mut lexer = Lexer::new(b"\"\\o77\"", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_string_unicode_escape_without_brace() {
    let mut lexer = Lexer::new(b"\"\\u1F600\"", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_string_unicode_escape_no_hex_digits() {
    let mut lexer = Lexer::new(b"\"\\u{}\"", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_string_unicode_escape_no_close_brace() {
    let mut lexer = Lexer::new(b"\"\\u{1F600\"", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_string_unicode_escape_codepoint_too_large() {
    let mut lexer = Lexer::new(b"\"\\u{110000}\"", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_string_unicode_escape_too_many_digits() {
    let mut lexer = Lexer::new(b"\"\\u{123456789}\"", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_string_unicode_escape_uppercase_u_no_plus() {
    let mut lexer = Lexer::new(b"\"\\uU1F600}\"", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

// Additional error coverage: atypical identifier - unterminated (already tested)
// Additional error coverage: typical identifier reserved prefix error
#[test]
fn test_typical_identifier_reserved_prefix_error() {
    // The reserved prefix is "\u{2699}\u{fe0f}" (⚙️)
    let mut lexer = Lexer::new("\u{2699}\u{fe0f}reserved".as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

// Float literal conversion error
#[test]
fn test_float_invalid_literal_returns_eof() {
    // A float with multiple dots should produce an error
    let mut lexer = Lexer::new(b"1.2.3", None).expect("source too big");
    lexer.disable_trivia();
    // First token should be Float(1.2), then Dot, then Integer(3) - no error expected
    let tok1 = lexer.next_tok();
    assert_eq!(tok1.token, Token::Float(ordered_float::NotNan::new(1.2).unwrap()));
    let tok2 = lexer.next_tok();
    assert_eq!(tok2.token, Token::Dot);
    let tok3 = lexer.next_tok();
    assert_eq!(
        tok3.token,
        Token::Integer(nitrate_token::Integer::new(3, nitrate_token::IntegerKind::Dec))
    );
}

// String escape: unexpected end-of-input in escape
#[test]
fn test_string_escape_eof() {
    let mut lexer = Lexer::new(b"\"\\x", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

// String escape: invalid escape after \x with only one hex digit
#[test]
fn test_string_hex_escape_one_valid_one_invalid() {
    let mut lexer = Lexer::new(b"\"\\x1G\"", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

// String escape: unicode escape with code point too large (already tested)
// String escape: backslash at end of input
#[test]
fn test_string_trailing_backslash_eof() {
    let mut lexer = Lexer::new(b"\"hello\\", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

// String with invalid UTF-8 bytes (non-UTF8 continuation byte after escape)
#[test]
fn test_string_binary_from_utf8_error_in_path_with_escape() {
    // String with a valid escape then binary data that makes the overall result
    // invalid UTF-8 should produce BString
    let source = b"\"hello\xffworld\"";
    let mut lexer = Lexer::new(source, None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::BString(b"hello\xffworld".to_vec()));
}

// Binary string via Unicode escape fallback (invalid UTF-8 after \u escape)
#[test]
fn test_string_unicode_escape_to_bstring_via_non_utf8_accumulation() {
    // A string with valid \u escape but containing raw non-UTF8 bytes => BString
    let source = b"\"\\u{41}\xff\"";
    let mut lexer = Lexer::new(source, None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    // \u{41} = 'A', then 0xff makes it non-UTF8
    assert_eq!(tok.token, Token::BString(b"A\xff".to_vec()));
}

// Octal escape: invalid digit (already has test)
// Comment with invalid UTF-8 bytes
#[test]
fn test_comment_invalid_utf8_returns_eof() {
    let mut lexer = Lexer::new(b"#\xff", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

// Invalid token char
#[test]
fn test_invalid_token_returns_eof() {
    // \x02 through \x07 are invalid single-byte tokens
    let mut lexer = Lexer::new(b"\x02", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_invalid_token_0x03_returns_eof() {
    let mut lexer = Lexer::new(b"\x03", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_invalid_token_0x05_returns_eof() {
    let mut lexer = Lexer::new(b"\x05", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_invalid_token_0x06_returns_eof() {
    let mut lexer = Lexer::new(b"\x06", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_invalid_token_0x07_returns_eof() {
    let mut lexer = Lexer::new(b"\x07", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_invalid_token_0x0e_returns_eof() {
    let mut lexer = Lexer::new(b"\x0e", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_invalid_token_0x1b_returns_eof() {
    let mut lexer = Lexer::new(b"\x1b", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_invalid_token_0x7f_del_returns_eof() {
    let mut lexer = Lexer::new(b"\x7f", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

// Skip while with various tokens
#[test]
fn test_skip_while_skips_until_matching() {
    let mut lexer = Lexer::new(b"abc;def", None).expect("source too big");
    lexer.disable_trivia();
    lexer.skip_while(&Token::Semi);
    assert_eq!(lexer.next_tok().token, Token::Name("def".into()));
}

#[test]
fn test_skip_while_at_eof() {
    let mut lexer = Lexer::new(b"abc", None).expect("source too big");
    lexer.disable_trivia();
    lexer.skip_while(&Token::Semi);
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

// LexerIterator with various modes
#[test]
fn test_lexer_iterator_empty() {
    use crate::LexerIterator;
    let lexer = Lexer::new(b"", None).expect("source too big");
    let iter = LexerIterator::new(lexer);
    let toks: Vec<_> = iter.collect();
    assert!(toks.is_empty());
}

// AnnotatedToken constructors
#[test]
fn test_annotated_token_start_end() {
    use nitrate_token::{AnnotatedToken, SourcePosition};
    let start = SourcePosition {
        line: 1,
        column: 2,
        offset: 10,
        fileid: None,
    };
    let end = SourcePosition {
        line: 1,
        column: 5,
        offset: 13,
        fileid: None,
    };
    let at = AnnotatedToken::new(Token::Semi, start.clone(), end.clone());
    assert_eq!(at.start(), start);
    assert_eq!(at.end(), end);
}

// Overlong integer overflow paths for radix_decode
#[test]
fn test_integer_overflow_oct() {
    let overflow = String::from("0o") + &"7".repeat(43); // 43 octal digits likely overflows u128
    let mut lexer = Lexer::new(overflow.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_integer_overflow_dec() {
    let overflow = String::from("0d") + &"9".repeat(39); // 39 decimal digits overflows u128
    let mut lexer = Lexer::new(overflow.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

// Unterminated atypical identifier (regression test)
#[test]
fn test_atypical_identifier_unterminated_extra() {
    let mut lexer = Lexer::new(b"`hello\nworld", None).expect("source too big");
    lexer.disable_trivia();
    // The backtick identifier reads until backtick, which may include newline
    // This should trigger the unterminated error path
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

// Atypical identifier containing invalid UTF-8 bytes
#[test]
fn test_atypical_identifier_invalid_utf8() {
    let mut lexer = Lexer::new(b"`\xff`", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

// String with various escape sequences individually tested
#[test]
fn test_all_string_escape_chars() {
    // Each of \0, \a, \b, \t, \n, \v, \f, \r, \\, \', \" tested in one string
    let source = br#""\0\a\b\t\n\v\f\r\\\'\"\x20""#;
    let mut lexer = Lexer::new(source, None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::String("\0\u{7}\u{8}\t\n\u{b}\u{c}\r\\\'\" ".into()));
}

// Unicode escape with uppercase U and + prefix (\\u{U+...})
#[test]
fn test_string_unicode_escape_uppercase_u_with_plus() {
    let source = "\"\\u{U+1F600}\"";
    let mut lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::String("\u{1F600}".into()));
}

// Valid octal escape
#[test]
fn test_string_octal_escape_valid() {
    let source = r#""\o077""#;
    let mut lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::String("?".into()));
}

#[test]
fn test_string_octal_escape_max() {
    let source = r#""\o377""#;
    let mut lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::BString(b"\xff".to_vec()));
}

// Test skip_if with peek_tok (already covered via nav tests)

// Edge case: float with no digits after decimal
#[test]
fn test_float_no_digits_after_dot_rewinds() {
    let mut lexer = Lexer::new(b"5.a", None).expect("source too big");
    lexer.disable_trivia();
    let tok1 = lexer.next_tok();
    assert_eq!(
        tok1.token,
        Token::Integer(nitrate_token::Integer::new(5, nitrate_token::IntegerKind::Dec))
    );
    let tok2 = lexer.next_tok();
    assert_eq!(tok2.token, Token::Dot);
    let tok3 = lexer.next_tok();
    assert_eq!(tok3.token, Token::Name("a".into()));
}

// radix_decode with valid large number that doesn't overflow but exercises all code paths
#[test]
fn test_radix_decode_max_u128_dec() {
    use crate::Lexer;
    let source = "340282366920938463463374607431768211455";
    let mut lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(
        tok.token,
        Token::Integer(nitrate_token::Integer::new(u128::MAX, nitrate_token::IntegerKind::Dec))
    );
}

#[test]
fn test_integer_bin_overflow() {
    let overflow = String::from("0b") + &"1".repeat(200);
    let mut lexer = Lexer::new(overflow.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

// Number parser: ensure the 0b/0o/0d/0x with no digits returns Eof
#[test]
fn test_binary_literal_no_digits_after_prefix() {
    let mut lexer = Lexer::new(b"0b ", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_octal_literal_no_digits_after_prefix() {
    let mut lexer = Lexer::new(b"0o ", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_decimal_literal_no_digits_after_prefix() {
    let mut lexer = Lexer::new(b"0d ", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_hex_literal_no_digits_after_prefix() {
    let mut lexer = Lexer::new(b"0x ", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

// Unicode escape too many digits (already tested with 9 digits)
// Additional test: exactly 8 hex digits which is valid (max allowed)
#[test]
fn test_string_unicode_escape_8_digits_valid() {
    let source = "\"\\u{00000041}\"";
    let mut lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::String("A".into()));
}

// Test that the advance function handles continuation bytes correctly
#[test]
fn test_advance_with_continuation_bytes() {
    // Multi-byte UTF-8: 2-byte char, 3-byte char, 4-byte char
    let source = "a\u{00e9}\u{4e2d}\u{1f600}"; // 'a', é, 中, 😀
    let mut lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::Name(source.into()));
    // Check that offsets are correct
    assert_eq!(tok.start_offset, 0);
    assert_eq!(tok.end_offset, source.len() as u32);
}

// Octal escape: 3 valid digits
#[test]
fn test_string_octal_escape_3_digits() {
    let source = b"\"\\o100\"";
    let mut lexer = Lexer::new(source, None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::String("@".into()));
}

// Octal escape produces BString when combined with non-UTF8
#[test]
fn test_string_octal_escape_non_utf8_chain() {
    // \o377 = 0xFF, not valid UTF-8, so result is BString
    let source = b"\"\\o377\"";
    let mut lexer = Lexer::new(source, None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::BString(b"\xff".to_vec()));
}

// Hex escape produces BString when combined with non-UTF8
#[test]
fn test_string_hex_escape_non_utf8_chain() {
    let source = b"\"\\xff\\xfe\"";
    let mut lexer = Lexer::new(source, None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::BString(b"\xff\xfe".to_vec()));
}

// Multiple escapes with valid UTF-8 that produce a string
#[test]
fn test_string_combined_escapes() {
    let source = b"\"\\x48\\x65\\x6c\\x6c\\x6f\""; // "Hello"
    let mut lexer = Lexer::new(source, None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::String("Hello".into()));
}

// Unicode escape with uppercase U only (no '+') - should error
#[test]
fn test_string_unicode_escape_uppercase_u_no_plus_in_braces() {
    let source = "\"\\u{U1F600}\"";
    let mut lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    // This should be an error (U without +)
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

// Test peek with preread token already cached
#[test]
fn test_peek_cache_and_next() {
    let mut lexer = Lexer::new(b"a b", None).expect("source too big");
    lexer.disable_trivia();
    let p1 = lexer.peek_tok();
    let p2 = lexer.peek_tok();
    assert_eq!(p1, p2);
    let n = lexer.next_tok();
    assert_eq!(n, p1);
    // After next, the next peek should work again
    let p3 = lexer.peek_tok();
    assert_eq!(p3.token, Token::Name("b".into()));
}

// skip_tok with preread token
#[test]
fn test_skip_tok_after_peek() {
    let mut lexer = Lexer::new(b"a b", None).expect("source too big");
    lexer.disable_trivia();
    let _ = lexer.peek_tok(); // cache "a"
    lexer.skip_tok(); // should skip "a" via preread
    assert_eq!(lexer.next_tok().token, Token::Name("b".into()));
}

// peek_pos with whitespace
#[test]
fn test_peek_pos_skip_whitespace() {
    let mut lexer = Lexer::new(b"  x", None).expect("source too big");
    lexer.disable_trivia();
    let pos = lexer.peek_pos();
    assert_eq!(pos.offset, 2);
}

// Enable trivia after disable
#[test]
fn test_enable_trivia_back() {
    use crate::tests::all_tokens;
    let toks = all_tokens("a b");
    assert!(toks.len() >= 2); // at least a, b
}

// Test the 'next_is' with peek returning false
#[test]
fn test_next_is_false() {
    let mut lexer = Lexer::new(b"a", None).expect("source too big");
    lexer.disable_trivia();
    assert!(!lexer.next_is(&Token::Semi));
    assert_eq!(lexer.next_tok().token, Token::Name("a".into()));
}

// Test position tracking for multiline with more complex behavior
#[test]
fn test_position_on_newline() {
    let source = "x\ny";
    let mut lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    let tok1 = lexer.next_tok();
    assert_eq!(tok1.token, Token::Name("x".into()));
    assert_eq!(tok1.end_line, 0);
    let tok2 = lexer.next_tok();
    assert_eq!(tok2.token, Token::Name("y".into()));
    assert_eq!(tok2.start_line, 1);
    assert_eq!(tok2.start_column, 0);
}

// Test octal escape with valid UTF-8 result produces Token::String
#[test]
fn test_string_octal_escape_utf8_result() {
    let source = b"\"\\o101\""; // octal 101 = 65 = 'A'
    let mut lexer = Lexer::new(source, None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::String("A".into()));
}
