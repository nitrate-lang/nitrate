#include <gtest/gtest.h>

#include <pipeline/libnitrate-lexer/LexicalCase.hh>

using namespace ncc::lex;

///============================================================================///
/// BUILTIN PUNCTUATORS
TEST_CASE(Punctor, Symbol, 0, "(", {PuncLPar});
TEST_CASE(Punctor, Symbol, 1, ")", {PuncRPar});
TEST_CASE(Punctor, Symbol, 2, "[", {PuncLBrk});
TEST_CASE(Punctor, Symbol, 3, "]", {PuncRBrk});
TEST_CASE(Punctor, Symbol, 4, "{", {PuncLCur});
TEST_CASE(Punctor, Symbol, 5, "}", {PuncRCur});
TEST_CASE(Punctor, Symbol, 6, ",", {PuncComa});
TEST_CASE(Punctor, Symbol, 7, ":", {PuncColn});
TEST_CASE(Punctor, Symbol, 8, ";", {PuncSemi});
TEST_CASE(Punctor, Symbol, 9, "::", {PuncScope});

///============================================================================///
/// ADJACENT PUNCTUATORS
TEST_CASE(Punctor, Adjacent, 0, "()", {PuncLPar, PuncRPar});
TEST_CASE(Punctor, Adjacent, 1, "[]", {PuncLBrk, PuncRBrk});
TEST_CASE(Punctor, Adjacent, 2, "{}", {PuncLCur, PuncRCur});
TEST_CASE(Punctor, Adjacent, 3, "(}]({[", {PuncLPar, PuncRCur, PuncRBrk, PuncLPar, PuncLCur, PuncLBrk});
TEST_CASE(Punctor, Adjacent, 4, ":: :::", {PuncScope, PuncScope, PuncColn});
TEST_CASE(Punctor, Adjacent, 5, "::::", {PuncScope, PuncScope});
TEST_CASE(Punctor, Adjacent, 6, ":::::", {PuncScope, PuncScope, PuncColn});
TEST_CASE(Punctor, Adjacent, 7, ")::(", {PuncRPar, PuncScope, PuncLPar});
TEST_CASE(Punctor, Adjacent, 8, "):(", {PuncRPar, PuncColn, PuncLPar});
TEST_CASE(Punctor, Adjacent, 9, ":#:", {PuncColn, Token(Note, ":")});
TEST_CASE(Punctor, Adjacent, 10, "let := 1", {Token(Let), PuncColn, OpSet, Token(1UL)});
