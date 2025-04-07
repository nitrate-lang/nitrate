#include <gtest/gtest.h>

#include <pipeline/libnitrate-lexer/LexicalCase.hh>

using namespace ncc::lex;

///============================================================================///
/// SIMPLE CHARACTERS
TEST_CASE(Char, Simple, 0, R"('a')", {Token(Char, "a")});

///============================================================================///
/// ASCII ESCAPE SEQUENCES
TEST_CASE(Char, AsciiEscape, 0, R"('\a')", {Token(Char, "\a")});
TEST_CASE(Char, AsciiEscape, 1, R"('\b')", {Token(Char, "\b")});
TEST_CASE(Char, AsciiEscape, 2, R"('\t')", {Token(Char, "\t")});
TEST_CASE(Char, AsciiEscape, 3, R"('\n')", {Token(Char, "\n")});
TEST_CASE(Char, AsciiEscape, 4, R"('\v')", {Token(Char, "\v")});
TEST_CASE(Char, AsciiEscape, 5, R"('\f')", {Token(Char, "\f")});
TEST_CASE(Char, AsciiEscape, 6, R"('\r')", {Token(Char, "\r")});
TEST_CASE(Char, AsciiEscape, 7, R"('\0')", {Token(Char, 1, "\0")});

///============================================================================///
/// DOUBLE ASCII ESCAPE SEQUENCES
TEST_CASE(Char, DoubleAscii, 0, R"('\a\a')", {Token(Text, "\a\a")});
TEST_CASE(Char, DoubleAscii, 1, R"('\b\b')", {Token(Text, "\b\b")});
TEST_CASE(Char, DoubleAscii, 2, R"('\t\t')", {Token(Text, "\t\t")});
TEST_CASE(Char, DoubleAscii, 3, R"('\n\n')", {Token(Text, "\n\n")});
TEST_CASE(Char, DoubleAscii, 4, R"('\v\v')", {Token(Text, "\v\v")});
TEST_CASE(Char, DoubleAscii, 5, R"('\f\f')", {Token(Text, "\f\f")});
TEST_CASE(Char, DoubleAscii, 6, R"('\r\r')", {Token(Text, "\r\r")});
TEST_CASE(Char, DoubleAscii, 7, R"('\0\0')", {Token(Text, 2, "\0\0")});

///============================================================================///
/// HEXADECIMAL ESCAPE SEQUENCES
TEST_CASE(Char, Hex, 0, R"('\x89')", {Token(Char, "\x89")});
TEST_CASE(Char, Hex, 1, R"('\x0A')", {Token(Char, "\n")});
TEST_CASE(Char, Hex, 2, R"('\xff')", {Token(Char, "\xff")});
TEST_CASE(Char, Hex, 3, R"('\x00')", {Token(Char, 1, "\0")});
TEST_CASE(Char, Hex, 4, R"('\x80')", {Token(Char, "\x80")});
TEST_CASE(Char, Hex, 5, R"('\xAG')", {});
TEST_CASE(Char, Hex, 6, R"('\xGA')", {});
TEST_CASE(Char, Hex, 10, R"('\X89')", {Token(Char, "\x89")});
TEST_CASE(Char, Hex, 11, R"('\X0A')", {Token(Char, "\n")});
TEST_CASE(Char, Hex, 12, R"('\Xff')", {Token(Char, "\xff")});
TEST_CASE(Char, Hex, 13, R"('\X00')", {Token(Char, 1, "\0")});
TEST_CASE(Char, Hex, 14, R"('\X80')", {Token(Char, "\x80")});
TEST_CASE(Char, Hex, 15, R"('\XAG')", {});
TEST_CASE(Char, Hex, 16, R"('\XGA')", {});

///============================================================================///
/// UNICODE ESCAPE SEQUENCES
TEST_CASE(Char, Unicode, 0, R"('\u{1F525}')", {Token(Text, "🔥")});
TEST_CASE(Char, Unicode, 1, R"('\u1F525}')", {});
TEST_CASE(Char, Unicode, 2, R"('\u{1F525')", {});
TEST_CASE(Char, Unicode, 3, R"('\u{ZZZ}')", {});
TEST_CASE(Char, Unicode, 4, R"('\u{FFFFFFFF}')", {});
TEST_CASE(Char, Unicode, 5, R"('\u{FFFFFFFFF}')", {});
TEST_CASE(Char, Unicode, 6, R"('\u{41}')", {Token(Char, "A")});
TEST_CASE(Char, Unicode, 7, R"('\u{100}')", {Token(Text, "Ā")});
TEST_CASE(Char, Unicode, 8, R"('🍉')", {Token(Text, "🍉")});
TEST_CASE(Char, Unicode, 9, R"('\u{D800}')", {Token(Text, "\xed\xa0\x80")});
TEST_CASE(Char, Unicode, 10, R"('\U{1F525}')", {Token(Text, "🔥")});
TEST_CASE(Char, Unicode, 11, R"('\U1F525}')", {});
TEST_CASE(Char, Unicode, 12, R"('\U{1F525')", {});
TEST_CASE(Char, Unicode, 13, R"('\U{ZZZ}')", {});
TEST_CASE(Char, Unicode, 14, R"('\U{FFFFFFFF}')", {});
TEST_CASE(Char, Unicode, 15, R"('\U{FFFFFFFFF}')", {});
TEST_CASE(Char, Unicode, 16, R"('\U{41}')", {Token(Char, "A")});
TEST_CASE(Char, Unicode, 17, R"('\U{100}')", {Token(Text, "Ā")});
TEST_CASE(Char, Unicode, 18, R"('🍉')", {Token(Text, "🍉")});
TEST_CASE(Char, Unicode, 19, R"('\U{D800}')", {Token(Text, "\xed\xa0\x80")});

///============================================================================///
/// OCTAL ESCAPE SEQUENCES
TEST_CASE(Char, Octal, 0, R"('\o000')", {Token(Char, 1, "\0")});
TEST_CASE(Char, Octal, 1, R"('\o377')", {Token(Char, "\xff")});
TEST_CASE(Char, Octal, 2, R"('\o400')", {});
TEST_CASE(Char, Octal, 3, R"('\o777')", {});
TEST_CASE(Char, Octal, 10, R"('\O000')", {Token(Char, 1, "\0")});
TEST_CASE(Char, Octal, 11, R"('\O377')", {Token(Char, "\xff")});
TEST_CASE(Char, Octal, 12, R"('\O400')", {});
TEST_CASE(Char, Octal, 13, R"('\O777')", {});

///============================================================================///
/// MALFORMED QUOTES
TEST_CASE(Char, Malformed, 0, R"(')", {});
TEST_CASE(Char, Malformed, 1, R"('\)", {});
TEST_CASE(Char, Malformed, 2, R"('")", {});
TEST_CASE(Char, Malformed, 3, R"("')", {});
TEST_CASE(Char, Malformed, 4, R"('\')", {});
