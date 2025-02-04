#include <gtest/gtest.h>

#include <pipeline/libnitrate-lexer/LexicalCase.hh>

using namespace ncc::lex;

///============================================================================///
/// SIMPLE CHARACTERS
TEST_CASE(Char, Simple, 0, R"('a')", {Token(Char, "a"), Token()});

///============================================================================///
/// ASCII ESCAPE SEQUENCES
TEST_CASE(Char, AsciiEscape, 0, R"('\a')", {Token(Char, "\a"), Token()});
TEST_CASE(Char, AsciiEscape, 1, R"('\b')", {Token(Char, "\b"), Token()});
TEST_CASE(Char, AsciiEscape, 2, R"('\t')", {Token(Char, "\t"), Token()});
TEST_CASE(Char, AsciiEscape, 3, R"('\n')", {Token(Char, "\n"), Token()});
TEST_CASE(Char, AsciiEscape, 4, R"('\v')", {Token(Char, "\v"), Token()});
TEST_CASE(Char, AsciiEscape, 5, R"('\f')", {Token(Char, "\f"), Token()});
TEST_CASE(Char, AsciiEscape, 6, R"('\r')", {Token(Char, "\r"), Token()});
TEST_CASE(Char, AsciiEscape, 7, R"('\0')", {Token(Char, 1, "\0"), Token()});

///============================================================================///
/// DOUBLE ASCII ESCAPE SEQUENCES
TEST_CASE(Char, DoubleAscii, 0, R"('\a\a')", {Token(Text, "\a\a"), Token()});
TEST_CASE(Char, DoubleAscii, 1, R"('\b\b')", {Token(Text, "\b\b"), Token()});
TEST_CASE(Char, DoubleAscii, 2, R"('\t\t')", {Token(Text, "\t\t"), Token()});
TEST_CASE(Char, DoubleAscii, 3, R"('\n\n')", {Token(Text, "\n\n"), Token()});
TEST_CASE(Char, DoubleAscii, 4, R"('\v\v')", {Token(Text, "\v\v"), Token()});
TEST_CASE(Char, DoubleAscii, 5, R"('\f\f')", {Token(Text, "\f\f"), Token()});
TEST_CASE(Char, DoubleAscii, 6, R"('\r\r')", {Token(Text, "\r\r"), Token()});
TEST_CASE(Char, DoubleAscii, 7, R"('\0\0')", {Token(Text, 2, "\0\0"), Token()});

///============================================================================///
/// HEXADECIMAL ESCAPE SEQUENCES
TEST_CASE(Char, Hex, 0, R"('\x89')", {Token(Char, "\x89"), Token()});
TEST_CASE(Char, Hex, 1, R"('\x0A')", {Token(Char, "\n"), Token()});
TEST_CASE(Char, Hex, 2, R"('\xff')", {Token(Char, "\xff"), Token()});
TEST_CASE(Char, Hex, 3, R"('\x00')", {Token(Char, 1, "\0"), Token()});
TEST_CASE(Char, Hex, 4, R"('\x80')", {Token(Char, "\x80"), Token()});
TEST_CASE(Char, Hex, 5, R"('\xAG')", {Token()});
TEST_CASE(Char, Hex, 6, R"('\xGA')", {Token()});

///============================================================================///
/// UNICODE ESCAPE SEQUENCES
TEST_CASE(Char, Unicode, 0, R"('\u{1F525}')", {Token(Text, "🔥"), Token()});
TEST_CASE(Char, Unicode, 1, R"('\u1F525}')", {Token()});
TEST_CASE(Char, Unicode, 2, R"('\u{1F525')", {Token()});
TEST_CASE(Char, Unicode, 3, R"('\u{ZZZ}')", {Token()});
TEST_CASE(Char, Unicode, 4, R"('\u{FFFFFFFF}')", {Token()});
TEST_CASE(Char, Unicode, 5, R"('\u{FFFFFFFFF}')", {Token()});
TEST_CASE(Char, Unicode, 6, R"('\u{41}')", {Token(Char, "A"), Token()});
TEST_CASE(Char, Unicode, 7, R"('\u{100}')", {Token(Text, "Ā"), Token()});
TEST_CASE(Char, Unicode, 8, R"('🍉')", {Token(Text, "🍉"), Token()});
TEST_CASE(Char, Unicode, 9, R"('\u{D800}')", {Token(Text, "\xed\xa0\x80"), Token()});

///============================================================================///
/// OCTAL ESCAPE SEQUENCES
TEST_CASE(Char, Octal, 0, R"('\o000')", {Token(Char, 1, "\0"), Token()});
TEST_CASE(Char, Octal, 1, R"('\o377')", {Token(Char, "\xff"), Token()});
TEST_CASE(Char, Octal, 2, R"('\o400')", {Token()});
TEST_CASE(Char, Octal, 3, R"('\o777')", {Token()});

///============================================================================///
/// MALFORMED QUOTES
TEST_CASE(Char, Malformed, 0, R"(')", {Token()});
TEST_CASE(Char, Malformed, 1, R"('\)", {Token()});
TEST_CASE(Char, Malformed, 2, R"('")", {Token()});
TEST_CASE(Char, Malformed, 3, R"("')", {Token()});
TEST_CASE(Char, Malformed, 4, R"('\')", {Token()});
