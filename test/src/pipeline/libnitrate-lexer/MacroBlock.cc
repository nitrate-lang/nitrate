#include <gtest/gtest.h>

#include <pipeline/libnitrate-lexer/LexicalCase.hh>

using namespace ncc::lex;

///============================================================================///
/// MACRO BODY
TEST_CASE(MacroBlock, Body, 0, R"(@())", {Token(MacB, "")})
TEST_CASE(MacroBlock, Body, 1, R"(@(abc def))", {Token(MacB, "abc def")})
TEST_CASE(MacroBlock, Body, 2, R"(@((abc def)))", {Token(MacB, "(abc def)")})
TEST_CASE(MacroBlock, Body, 3, R"(@(a(b)c d(e)f))", {Token(MacB, "a(b)c d(e)f")})
TEST_CASE(MacroBlock, Body, 4, R"(@(fn main() {}))", {Token(MacB, "fn main() {}")})
TEST_CASE(MacroBlock, Body, 5, R"(@(print(🔥 + 🍉);))", {Token(MacB, "print(🔥 + 🍉);")});
TEST_CASE(MacroBlock, Body, 6, R"(-@(12):@(34)/)", {OpMinus, Token(MacB, "12"), PuncColn, Token(MacB, "34"), OpSlash})

///============================================================================///
/// MACRO INVALID
TEST_CASE(MacroBlock, Invalid, 0, R"(@)", {Token(Macr, "")})
TEST_CASE(MacroBlock, Invalid, 1, R"(@()", {})
TEST_CASE(MacroBlock, Invalid, 2, R"(@(abc)", {})
TEST_CASE(MacroBlock, Invalid, 3, R"( @(abc def 🍉🍉🍉)", {})
TEST_CASE(MacroBlock, Invalid, 4, R"(@((()", {})
TEST_CASE(MacroBlock, Invalid, 5, R"( @(print(')))')) )", {Token(MacB, R"(print('))"), PuncRPar})
