#include <gtest/gtest.h>

#include <pipeline/libnitrate-lexer/LexicalCase.hh>

using namespace ncc::lex;

///============================================================================///
/// ASCII IDENTIFIERS
TEST_CASE(Identifier, ASCII, 0, "a", {Token("a")});
TEST_CASE(Identifier, ASCII, 1, "abc", {Token("abc")});
TEST_CASE(Identifier, ASCII, 2, " abc ", {Token("abc")});
TEST_CASE(Identifier, ASCII, 3, " abc def ", {Token("abc"), Token("def")});
TEST_CASE(Identifier, ASCII, 4, " abc::def ", {Token("abc"), Token(PuncScope), Token("def")});
TEST_CASE(Identifier, ASCII, 5, " abc+def ", {Token("abc"), Token(OpPlus), Token("def")});
TEST_CASE(Identifier, ASCII, 6, " 123i32 ", {Token(123UL), Token("i32")});
TEST_CASE(Identifier, ASCII, 7, " 123.2i32 ", {Token(NumL, "123.2"), Token("i32")});
TEST_CASE(Identifier, ASCII, 8, " 123.6i32/1 ", {Token(NumL, "123.6"), Token("i32"), Token(OpSlash), Token(1UL)});

///============================================================================///
/// UNICODE IDENTIFIERS
TEST_CASE(Identifier, Unicode, 0, "🔥", {Token("🔥")});
TEST_CASE(Identifier, Unicode, 1, "🔥🍉", {Token("🔥🍉")});
TEST_CASE(Identifier, Unicode, 2, "🔥 🍉", {Token("🔥"), Token("🍉")});
TEST_CASE(Identifier, Unicode, 3, "1🔥", {Token(1UL), Token("🔥")});
TEST_CASE(Identifier, Unicode, 4, "🔥1", {Token("🔥1")});
TEST_CASE(Identifier, Unicode, 5, "+שלום_עולם+", {Token(OpPlus), Token("שלום_עולם"), Token(OpPlus)});

///============================================================================///
/// MIXED IDENTIFIERS
TEST_CASE(Identifier, Mixed, 0, "2a", {Token(2UL), Token("a")});
TEST_CASE(Identifier, Mixed, 1, "a2", {Token("a2")});
TEST_CASE(Identifier, Mixed, 2, "_2a", {Token("_2a")});

///============================================================================///
/// OTHER STUFF
TEST_CASE(Identifier, Invalid, 0, "2", {Token(2UL)});
TEST_CASE(Identifier, Invalid, 1, "f'abc'", {__FString, Token(Text, "abc")});
TEST_CASE(Identifier, Invalid, 2, "f\"abc\"", {__FString, Token(Text, "abc")});
TEST_CASE(Identifier, Invalid, 3, "abc\x45\x89\xd0", {});
