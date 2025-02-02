#include <gtest/gtest.h>

#include <pipeline/libnitrate-lexer/LexicalCase.hh>

using namespace ncc::lex;

///============================================================================///
/// SIMPLE CHARACTERS
TEST_CASE(Char, Simple, 0, R"('a')", {Token(Char, "a"), Token()});

///============================================================================///
/// ASCII ESCAPE SEQUENCES
TEST_CASE(Char, AsciiEscape, 100, R"('\a')", {Token(Char, "\a"), Token()});
TEST_CASE(Char, AsciiEscape, 101, R"('\b')", {Token(Char, "\b"), Token()});
TEST_CASE(Char, AsciiEscape, 102, R"('\t')", {Token(Char, "\t"), Token()});
TEST_CASE(Char, AsciiEscape, 103, R"('\n')", {Token(Char, "\n"), Token()});
TEST_CASE(Char, AsciiEscape, 104, R"('\v')", {Token(Char, "\v"), Token()});
TEST_CASE(Char, AsciiEscape, 105, R"('\f')", {Token(Char, "\f"), Token()});
TEST_CASE(Char, AsciiEscape, 106, R"('\r')", {Token(Char, "\r"), Token()});
TEST_CASE(Char, AsciiEscape, 107, R"('\0')", {Token(Char, 1, "\0"), Token()});

///============================================================================///
/// DOUBLE ASCII ESCAPE SEQUENCES
TEST_CASE(Char, DoubleAscii, 110, R"('\a\a')", {Token(Text, "\a\a"), Token()});
TEST_CASE(Char, DoubleAscii, 111, R"('\b\b')", {Token(Text, "\b\b"), Token()});
TEST_CASE(Char, DoubleAscii, 112, R"('\t\t')", {Token(Text, "\t\t"), Token()});
TEST_CASE(Char, DoubleAscii, 113, R"('\n\n')", {Token(Text, "\n\n"), Token()});
TEST_CASE(Char, DoubleAscii, 114, R"('\v\v')", {Token(Text, "\v\v"), Token()});
TEST_CASE(Char, DoubleAscii, 115, R"('\f\f')", {Token(Text, "\f\f"), Token()});
TEST_CASE(Char, DoubleAscii, 116, R"('\r\r')", {Token(Text, "\r\r"), Token()});
TEST_CASE(Char, DoubleAscii, 117, R"('\0\0')",
          {Token(Text, 2, "\0\0"), Token()});

///============================================================================///
/// HEXADECIMAL ESCAPE SEQUENCES
TEST_CASE(Char, Hex, 200, R"('\x89')", {Token(Char, "\x89"), Token()});
TEST_CASE(Char, Hex, 201, R"('\x0A')", {Token(Char, "\n"), Token()});
TEST_CASE(Char, Hex, 202, R"('\xff')", {Token(Char, "\xff"), Token()});
TEST_CASE(Char, Hex, 203, R"('\x00')", {Token(Char, 1, "\0"), Token()});
TEST_CASE(Char, Hex, 204, R"('\x80')", {Token(Char, "\x80"), Token()});
TEST_CASE(Char, Hex, 205, R"('\xAG')", {Token()});
TEST_CASE(Char, Hex, 206, R"('\xGA')", {Token()});

///============================================================================///
/// UNICODE ESCAPE SEQUENCES
TEST_CASE(Char, Unicode, 300, R"('\u{1F525}')", {Token(Text, "🔥"), Token()});
TEST_CASE(Char, Unicode, 301, R"('\u1F525}')", {Token()});
TEST_CASE(Char, Unicode, 302, R"('\u{1F525')", {Token()});
TEST_CASE(Char, Unicode, 303, R"('\u{ZZZ}')", {Token()});
TEST_CASE(Char, Unicode, 304, R"('\u{FFFFFFFF}')", {Token()});
TEST_CASE(Char, Unicode, 305, R"('\u{FFFFFFFFF}')", {Token()});
TEST_CASE(Char, Unicode, 306, R"('\u{41}')", {Token(Char, "A"), Token()});
TEST_CASE(Char, Unicode, 307, R"('\u{100}')", {Token(Text, "Ā"), Token()});
TEST_CASE(Char, Unicode, 308, R"('🍉')", {Token(Text, "🍉"), Token()});

///============================================================================///
/// OCTAL ESCAPE SEQUENCES
TEST_CASE(Char, Octal, 400, R"('\o000')", {Token(Char, 1, "\0"), Token()});
TEST_CASE(Char, Octal, 401, R"('\o377')", {Token(Char, "\xff"), Token()});
TEST_CASE(Char, Octal, 402, R"('\o400')", {Token()});
TEST_CASE(Char, Octal, 403, R"('\o777')", {Token()});

///============================================================================///
/// MALFORMED QUOTES
TEST_CASE(Char, Malformed, 500, R"(')", {Token()});
TEST_CASE(Char, Malformed, 501, R"('\)", {Token()});
TEST_CASE(Char, Malformed, 502, R"('")", {Token()});
TEST_CASE(Char, Malformed, 503, R"("')", {Token()});
TEST_CASE(Char, Malformed, 504, R"('\')", {Token()});
