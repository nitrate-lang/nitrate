#include <gtest/gtest.h>

#include <pipeline/libnitrate-lexer/LexicalCase.hh>

using namespace ncc::lex;

///============================================================================///
/// MACRO BODY
TEST_CASE(MacroCall, Symbol, 0, R"(@hello)", {Token(Macr, "hello")})
TEST_CASE(MacroCall, Symbol, 1, R"(@hello())", {Token(Macr, "hello"), Token(PuncLPar), Token(PuncRPar)})
TEST_CASE(MacroCall, Symbol, 2, R"(@hello(abc))",
          {Token(Macr, "hello"), Token(PuncLPar), Token("abc"), Token(PuncRPar)})
TEST_CASE(MacroCall, Symbol, 3, R"(@🍉🍉🍉)", {Token(Macr, "🍉🍉🍉")});
TEST_CASE(MacroCall, Symbol, 4, R"(@foo::🍉🍉🍉)", {Token(Macr, "foo::🍉🍉🍉")});
TEST_CASE(MacroCall, Symbol, 5, R"(@foo:::🍉🍉🍉)", {Token(Macr, "foo:::🍉🍉🍉")});
TEST_CASE(MacroCall, Symbol, 6, R"(@foo:::🍉;🍉🍉)", {Token(Macr, "foo:::🍉"), PuncSemi, Token("🍉🍉")});

///============================================================================///
/// MACRO INVALID
TEST_CASE(MacroCall, Invalid, 0, R"(@)", {Token(Macr, "")})
TEST_CASE(MacroCall, Invalid, 1, "@abc\x45\x89\xd0", {});
