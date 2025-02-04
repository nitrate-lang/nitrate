#include <gtest/gtest.h>

#include <pipeline/libnitrate-lexer/LexicalCase.hh>

using namespace ncc::lex;

///============================================================================///
/// BUILTIN PUNCTUATORS
TEST_CASE(Punctor, Symbol, 0, "(", {PuncLPar, Token()});
TEST_CASE(Punctor, Symbol, 1, ")", {PuncRPar, Token()});
TEST_CASE(Punctor, Symbol, 2, "[", {PuncLBrk, Token()});
TEST_CASE(Punctor, Symbol, 3, "]", {PuncRBrk, Token()});
TEST_CASE(Punctor, Symbol, 4, "{", {PuncLCur, Token()});
TEST_CASE(Punctor, Symbol, 5, "}", {PuncRCur, Token()});
TEST_CASE(Punctor, Symbol, 6, ",", {PuncComa, Token()});
TEST_CASE(Punctor, Symbol, 7, ":", {PuncColn, Token()});
TEST_CASE(Punctor, Symbol, 8, ";", {PuncSemi, Token()});
TEST_CASE(Punctor, Symbol, 9, "::", {PuncScope, Token()});

///============================================================================///
/// ADJACENT PUNCTUATORS
TEST_CASE(Punctor, Adjacent, 0, "()", {PuncLPar, PuncRPar, Token()});
TEST_CASE(Punctor, Adjacent, 1, "[]", {PuncLBrk, PuncRBrk, Token()});
TEST_CASE(Punctor, Adjacent, 2, "{}", {PuncLCur, PuncRCur, Token()});
TEST_CASE(Punctor, Adjacent, 3, "(}]({[", {PuncLPar, PuncRCur, PuncRBrk, PuncLPar, PuncLCur, PuncLBrk, Token()});
TEST_CASE(Punctor, Adjacent, 4, ":: :::", {PuncScope, PuncScope, PuncColn, Token()});
TEST_CASE(Punctor, Adjacent, 5, "::::", {PuncScope, PuncScope, Token()});
TEST_CASE(Punctor, Adjacent, 6, ":::::", {PuncScope, PuncScope, PuncColn, Token()});
TEST_CASE(Punctor, Adjacent, 7, ")::(", {PuncRPar, PuncScope, PuncLPar, Token()});
TEST_CASE(Punctor, Adjacent, 8, "):(", {PuncRPar, PuncColn, PuncLPar, Token()});
TEST_CASE(Punctor, Adjacent, 9, ":#:", {PuncColn, Token(Note, ":"), Token()});
TEST_CASE(Punctor, Adjacent, 10, "let := 1", {Token(Let), PuncColn, OpSet, Token(1UL), Token()});
