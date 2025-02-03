#include <gtest/gtest.h>

#include <pipeline/libnitrate-lexer/LexicalCase.hh>

using namespace ncc::lex;

///============================================================================///
/// ASCII IDENTIFIERS
TEST_CASE(Identifier, ASCII, 0, "a", {Token("a"), Token()});
TEST_CASE(Identifier, ASCII, 1, "abc", {Token("abc"), Token()});
TEST_CASE(Identifier, ASCII, 2, " abc ", {Token("abc"), Token()});
TEST_CASE(Identifier, ASCII, 3, " abc def ", {Token("abc"), Token("def"), Token()});
TEST_CASE(Identifier, ASCII, 4, " abc::def ", {Token("abc"), Token(PuncScope), Token("def"), Token()});
TEST_CASE(Identifier, ASCII, 5, " abc+def ", {Token("abc"), Token(OpPlus), Token("def"), Token()});
TEST_CASE(Identifier, ASCII, 6, " 123i32 ", {Token(123UL), Token("i32"), Token()});
TEST_CASE(Identifier, ASCII, 7, " 123.2i32 ", {Token(123.2), Token("i32"), Token()});
TEST_CASE(Identifier, ASCII, 8, " 123.6i32/1 ", {Token(123.6), Token("i32"), Token(OpSlash), Token(1UL), Token()});

///============================================================================///
/// UNICODE IDENTIFIERS
TEST_CASE(Identifier, Unicode, 0, "🔥", {Token("🔥"), Token()});
TEST_CASE(Identifier, Unicode, 1, "🔥🍉", {Token("🔥🍉"), Token()});
TEST_CASE(Identifier, Unicode, 2, "🔥 🍉", {Token("🔥"), Token("🍉"), Token()});
