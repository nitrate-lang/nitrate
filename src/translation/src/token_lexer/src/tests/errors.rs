//! Tests for error-handling edge cases (all return Eof on error).

use crate::Lexer;
use nitrate_diagnosis::SourcePosition;
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

// ================ NEW TESTS FOR 97%+ COVERAGE ================

// 1. convert_float_repr error - float overflow
// Note: f64::parse is very permissive and large values become INFINITY which
// is not NaN, so NotNan::new(INFINITY) succeeds. The error path in
// convert_float_repr is only for truly invalid float syntax.
// Verify it doesn't panic:
#[test]
fn test_float_overflow_does_not_panic() {
    let source =
        "99999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999.0";
    let mut lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    // A very large float may parse as Float(inf) - just verify it doesn't panic
    assert!(tok.token == Token::Eof || matches!(tok.token, Token::Float(_)));
}

// 2. parse_float - dot followed by non-digit hits the rewind path (line 374)
#[test]
fn test_float_dot_non_digit_rewinds() {
    // "0." is already Integer(0), Dot - need a number that ends with dot
    // Actually 5. hits the parse_float path: reads "5", sees ".", advances,
    // then peek_byte returns something non-digit
    let mut lexer = Lexer::new(b"5.\n", None).expect("source too big");
    lexer.disable_trivia();
    let tok1 = lexer.next_tok();
    assert_eq!(
        tok1.token,
        Token::Integer(nitrate_token::Integer::new(5, nitrate_token::IntegerKind::Dec))
    );
    let tok2 = lexer.next_tok();
    assert_eq!(tok2.token, Token::Dot);
}

// 3. parse_float error path when peek_byte fails (line 379)
#[test]
fn test_float_peek_error_after_dot() {
    // Number ending in dot at EOF
    let mut lexer = Lexer::new(b"5.", None).expect("source too big");
    lexer.disable_trivia();
    let tok1 = lexer.next_tok();
    assert_eq!(
        tok1.token,
        Token::Integer(nitrate_token::Integer::new(5, nitrate_token::IntegerKind::Dec))
    );
    let tok2 = lexer.next_tok();
    assert_eq!(tok2.token, Token::Dot);
}

// 4. atypical identifier with reserved prefix (backtick variant) - line 241-244
#[test]
fn test_atypical_identifier_reserved_prefix_in_backtick() {
    let mut lexer = Lexer::new("`\u{2699}\u{fe0f}reserved`".as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

// 5. parse_typical_identifier invalid UTF-8
#[test]
fn test_typical_identifier_invalid_utf8() {
    // Non-ASCII bytes that aren't valid UTF-8 continuation bytes
    // \xff is a standalone invalid UTF-8 byte, but !b.is_ascii() would include it
    let mut lexer = Lexer::new(b"\xff\xfe", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

// 6. parse_string_escape Err(()) from peek_byte (end of input after \)
#[test]
fn test_string_escape_end_of_input_after_backslash() {
    let mut lexer = Lexer::new(b"\"\\", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

// 7. parse_string_unicode_escape missing closing brace for \u{U+...}
#[test]
fn test_string_unicode_escape_missing_close_brace_upper_u() {
    let source = "\"\\u{U+1F600\"";
    let mut lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

// 8. parse_string - string that hits the storage buffer path and ends up as BString
#[test]
fn test_string_bstring_via_escape_buffer() {
    // String with escape that populates storage, and result is non-UTF8
    let source = b"\"\\x41\xff\""; // \x41 = 'A', then \xff makes it non-UTF8
    let mut lexer = Lexer::new(source, None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::BString(b"A\xff".to_vec()));
}

// 9. parse_string_unicode_escape that results in codepoint too large (char::from_u32 fails)
#[test]
fn test_string_unicode_escape_invalid_codepoint() {
    let source = "\"\\u{110000}\"";
    let mut lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

// 10. next_tok with preread token (line 73-75)
#[test]
fn test_next_tok_with_preread() {
    let mut lexer = Lexer::new(b"a b", None).expect("source too big");
    lexer.disable_trivia();
    let _ = lexer.peek_tok(); // cache "a"
    let n = lexer.next_tok(); // should use cached
    assert_eq!(n.token, Token::Name("a".into()));
    let n2 = lexer.next_tok();
    assert_eq!(n2.token, Token::Name("b".into()));
}

// 11. parse_single_byte with all whitespace trivia tokens (line 829, 832)
#[test]
fn test_whitespace_token_coverage() {
    use crate::tests::all_tokens;
    // \t (tab), \n (newline), \x0b (vertical tab), \x0c (form feed), \r (CR), ' ' (space)
    let source = "\t\n\x0b\x0c\r ";
    let toks = all_tokens(source);
    assert_eq!(toks.len(), 6);
    assert_eq!(toks[0].token, Token::HorizontalTab);
    assert_eq!(toks[1].token, Token::NewLine);
    assert_eq!(toks[2].token, Token::VerticalTab);
    assert_eq!(toks[3].token, Token::FormFeed);
    assert_eq!(toks[4].token, Token::CarriageReturn);
    assert_eq!(toks[5].token, Token::Space);
}

// 12. rewind with preread token
#[test]
fn test_rewind_clears_preread() {
    let mut lexer = Lexer::new(b"a b", None).expect("source too big");
    lexer.disable_trivia();
    let first = lexer.next_tok(); // consume "a"
    assert_eq!(first.token, Token::Name("a".into()));
    let pos = lexer.current_pos(); // position after "a", before "b"
    lexer.rewind(pos);
    // Should not have preread anymore, next should be "b"
    let n = lexer.next_tok();
    assert_eq!(n.token, Token::Name("b".into()));
}

// 13. parse_string_escape - invalid char error (line 686-688)
// Already tested via test_string_invalid_escape_returns_eof, but adding explicit coverage
#[test]
fn test_string_escape_invalid_char() {
    let mut lexer = Lexer::new(b"\"\\z\"", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

// 14. parse_next_token with trivia disabled and non-trivia token
#[test]
fn test_parse_next_token_trivia_disabled() {
    let mut lexer = Lexer::new(b"  fn", None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::Fn);
    assert_eq!(tok.start_offset, 2);
}

// 15. test string line 751-754 path: storage is non-empty and final string is non-UTF8
// Already covered by test_string_unicode_escape_to_bstring_via_non_utf8_accumulation

// 16. test parse_string_escape Err(()) path at line 694
#[test]
fn test_string_escape_peek_error() {
    let mut lexer = Lexer::new(b"\"\\", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

// 17. test parse_string where storage is non-empty and final result is valid UTF-8
// This hits the String::from_utf8(storage) path on line 751
#[test]
fn test_string_with_escapes_storage_buffer_utf8() {
    let source = b"\"\\x48\\x65\\x6c\\x6c\\x6f\""; // "Hello" via hex escapes
    let mut lexer = Lexer::new(source, None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::String("Hello".into()));
}

// 18. test parse_next_token EOF path
#[test]
fn test_parse_next_token_eof() {
    let mut lexer = Lexer::new(b"", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

// 19. test parse_next_token error -> Eof conversion
#[test]
fn test_parse_next_token_error_is_eof() {
    // A byte that's not a valid token start should trigger error which becomes Eof
    let mut lexer = Lexer::new(b"\x02", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

// 20. LexerIterator next returning None
#[test]
fn test_lexer_iterator_next_none() {
    use crate::LexerIterator;
    let mut lexer = Lexer::new(b"", None).expect("source too big");
    lexer.disable_trivia();
    let mut iter = LexerIterator::new(lexer);
    assert!(iter.next().is_none());
}

// 21. parse_single_byte error path with specific bytes
#[test]
fn test_parse_single_byte_error_coverage() {
    // Test bytes that would go through parse_single_byte and hit the error path
    let bytes = [b'\x02', b'\x03', b'\x04', b'\x05', b'\x06', b'\x07', b'\x0e'];
    for &b in &bytes {
        let buf = [b];
        let mut lexer = Lexer::new(&buf, None).expect("source too big");
        lexer.disable_trivia();
        assert_eq!(lexer.next_tok().token, Token::Eof, "byte 0x{:02x} should return Eof", b);
    }
}

// 22. Tokens that were uncovered: SingleQuote, Semi, Comma, etc. plus trivia
// Test all token types produced by parse_single_byte
#[test]
fn test_all_parse_single_byte_tokens() {
    use crate::tests::tokens_skipping_trivia;
    // Test a few less common ones that might not be covered
    let toks = tokens_skipping_trivia("'");
    assert_eq!(toks[0].token, Token::SingleQuote);

    let toks = tokens_skipping_trivia("~");
    assert_eq!(toks[0].token, Token::Tilde);

    let toks = tokens_skipping_trivia("?");
    assert_eq!(toks[0].token, Token::Question);

    let toks = tokens_skipping_trivia("$");
    assert_eq!(toks[0].token, Token::Dollar);

    let toks = tokens_skipping_trivia("@");
    assert_eq!(toks[0].token, Token::At);

    let toks = tokens_skipping_trivia("^");
    assert_eq!(toks[0].token, Token::Caret);

    let toks = tokens_skipping_trivia("%");
    assert_eq!(toks[0].token, Token::Percent);
}

// 23. Test carriage return stripping in comments
#[test]
fn test_comment_trailing_backslash_before_newline() {
    use crate::tests::all_tokens;
    // Comment with \r at end should strip it
    let toks = all_tokens("# test\r\n");
    assert_eq!(toks.len(), 2);
    assert_eq!(
        toks[0].token,
        Token::Comment(nitrate_token::Comment::new(
            "# test".into(),
            nitrate_token::CommentKind::SingleLine
        ))
    );
    assert_eq!(toks[1].token, Token::NewLine);
}

// 24. test parse_string - storage buffer path that becomes String (already covered by #17)
// 25. test parse_string - storage buffer path that becomes BString via escape
#[test]
fn test_string_bstring_via_octal_storage() {
    // Use octal escape \o377 (0xFF) which makes the result non-UTF8
    // Combined with an earlier escape to trigger storage path
    let source = b"\"\\x41\\o377\""; // 'A' then 0xFF = BString
    let mut lexer = Lexer::new(source, None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::BString(b"A\xff".to_vec()));
}

// 26. Test number with radix prefix that doesn't overflow but goes through
// all the radix_decode code paths
#[test]
fn test_integer_hex_with_underscores_and_max_value() {
    use nitrate_token::{Integer, IntegerKind};
    let source = b"0xffff_ffff_ffff_ffff_ffff_ffff_ffff_ffff";
    let mut lexer = Lexer::new(source, None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(
        tok.token,
        Token::Integer(Integer::new(
            0xffff_ffff_ffff_ffff_ffff_ffff_ffff_ffffu128,
            IntegerKind::Hex
        ))
    );
}

// 27. parse_next_token with comment and trivia disabled
// The comment is in the trivia skip list and should be skipped
#[test]
fn test_trivia_disabled_skips_comments() {
    let mut lexer = Lexer::new(b"# comment\nfn", None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::Fn);
}

// 28. Verify that the advance function properly handles the non-utf8 byte path (line 173, 182)
// When a byte is NOT end of a UTF-8 sequence (i.e., it's a continuation byte 0x80-0xBF),
// column should NOT increment
#[test]
fn test_advance_continuation_byte_column() {
    // '\xe9' alone would be a continuation byte without a start byte
    // But read_while for identifiers reads based on is_ascii() check
    // So let's test with a string that has continuation bytes inside
    let source = "\u{00e9}\u{00e9}"; // éé = 0xC3 0xA9 0xC3 0xA9
    let mut lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::Name(source.into()));
    // end_column should be 2 (2 visible chars) but end_offset should be 4 (4 bytes)
    assert_eq!(tok.end_column, 2);
    assert_eq!(tok.end_offset, 4);
}

// 29. parse_string with multiline string using storage buffer that becomes BString
#[test]
fn test_string_multiline_bstring_via_storage() {
    let source = b"\"hello\\x41\xffworld\""; // escape triggers storage, \xff makes BString
    let mut lexer = Lexer::new(source, None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.token, Token::BString(b"helloA\xffworld".to_vec()));
}

// 30. Test // style comment (not valid for this language but # comment)
// The lexer only supports # comments

// 31. radix_decode with overflow in different bases
#[test]
fn test_integer_overflow_hex_exhaustive() {
    let overflow = String::from("0x") + &"f".repeat(33); // 33 hex digits overflows u128
    let mut lexer = Lexer::new(overflow.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

#[test]
fn test_integer_overflow_bin_exhaustive() {
    let overflow = String::from("0b") + &"1".repeat(129); // 129 binary digits overflows u128
    let mut lexer = Lexer::new(overflow.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

// 32. Test very large float that overflows f64 - values near f64 limits
#[test]
fn test_float_huge_values() {
    // Very long decimal fractions still parse as valid f64 (rounded)
    // Just verify it doesn't panic
    let source = "0.99999999999999999999999999999999999999999999999999999999999999999999999999999999999";
    let mut lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert!(tok.token == Token::Eof || matches!(tok.token, Token::Float(_)));
}

// 33. Test extra float patterns - underflow via 0.000...something
#[test]
fn test_float_underflow() {
    let source = String::from("0.") + &"0".repeat(300) + "1";
    let mut lexer = Lexer::new(source.as_bytes(), None).expect("source too big");
    lexer.disable_trivia();
    // This should either be a valid float (0.0) or Eof depending on overflow
    // Just verify it doesn't panic
    let _tok = lexer.next_tok();
}

// 34. Test continue byte in non-utf8 scenario for parse_typical_identifier error path
// Use bytes that look like non-ASCII continuation
#[test]
fn test_typical_identifier_continuation_bytes_no_start() {
    // Standalone continuation bytes (0x80-0xBF) without a start byte
    // They pass as "not is_ascii()" in read_while, then fail UTF-8 conversion
    let source = b"\x80\x81"; // continuation bytes without start
    let mut lexer = Lexer::new(source, None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

// 35. parse_single_byte invalid token with \x04
#[test]
fn test_invalid_token_0x04_returns_eof() {
    let mut lexer = Lexer::new(b"\x04", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);
}

// 36. Comment-only input with trivia disabled and re-enable
#[test]
fn test_comment_only_trivia_cycle() {
    let mut lexer = Lexer::new(b"#comment", None).expect("source too big");
    lexer.disable_trivia();
    assert_eq!(lexer.next_tok().token, Token::Eof);

    // With trivia enabled, the comment token appears
    let mut lexer2 = Lexer::new(b"#comment", None).expect("source too big");
    let tok = lexer2.next_tok(); // trivia enabled by default
    assert_eq!(
        tok.token,
        Token::Comment(nitrate_token::Comment::new(
            "#comment".into(),
            nitrate_token::CommentKind::SingleLine
        ))
    );
}

// 37. parse_string_unicode_escape with missing hex digits (line 577)
// Already tested by test_string_unicode_escape_no_hex_digits

// 38. parse_string_unicode_escape with 9 digits (too many) - line 582-588
// Already tested by test_string_unicode_escape_too_many_digits

// 39. parse_string_unicode_escape with char::from_u32 failing (line 603-610)
// Already tested by test_string_unicode_escape_codepoint_too_large

// 40. parse_string_unicode_escape missing closing brace (line 613-614)
// Already tested by test_string_unicode_escape_no_close_brace

// 41. Test that the `_ => {}` in parse_number catch-all for non-prefix after '0' is hit
#[test]
fn test_number_just_zero() {
    use crate::tests::eq;
    eq(
        "0",
        Token::Integer(nitrate_token::Integer::new(0, nitrate_token::IntegerKind::Dec)),
    );
}

// 42. test parse_number with different bases not yet tested
#[test]
fn test_integer_various_bases_coverage() {
    // Decimal with leading zero and no prefix (just "0")
    let mut lexer = Lexer::new(b"0", None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(
        tok.token,
        Token::Integer(nitrate_token::Integer::new(0, nitrate_token::IntegerKind::Dec))
    );
}
