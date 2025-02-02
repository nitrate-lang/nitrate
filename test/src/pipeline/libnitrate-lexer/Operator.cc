#include <gtest/gtest.h>

#include <pipeline/libnitrate-lexer/LexicalCase.hh>

#include "nitrate-lexer/Token.hh"

using namespace ncc::lex;

///============================================================================///
/// BUILTIN OPERATORS
TEST_CASE(Operator, Symbol, 0, "+", {OpPlus, Token()});
TEST_CASE(Operator, Symbol, 1, "-", {OpMinus, Token()});
TEST_CASE(Operator, Symbol, 2, "*", {OpTimes, Token()});
TEST_CASE(Operator, Symbol, 3, "/", {OpSlash, Token()});
TEST_CASE(Operator, Symbol, 4, "%", {OpPercent, Token()});
TEST_CASE(Operator, Symbol, 5, "&", {OpBitAnd, Token()});
TEST_CASE(Operator, Symbol, 6, "|", {OpBitOr, Token()});
TEST_CASE(Operator, Symbol, 7, "^", {OpBitXor, Token()});
TEST_CASE(Operator, Symbol, 8, "~", {OpBitNot, Token()});
TEST_CASE(Operator, Symbol, 9, "<<", {OpLShift, Token()});
TEST_CASE(Operator, Symbol, 10, ">>", {OpRShift, Token()});
TEST_CASE(Operator, Symbol, 12, "<<<", {OpROTL, Token()});
TEST_CASE(Operator, Symbol, 13, ">>>", {OpROTR, Token()});
TEST_CASE(Operator, Symbol, 14, "&&", {OpLogicAnd, Token()});
TEST_CASE(Operator, Symbol, 15, "||", {OpLogicOr, Token()});
TEST_CASE(Operator, Symbol, 16, "^^", {OpLogicXor, Token()});
TEST_CASE(Operator, Symbol, 17, "!", {OpLogicNot, Token()});
TEST_CASE(Operator, Symbol, 18, "<", {OpLT, Token()});
TEST_CASE(Operator, Symbol, 19, ">", {OpGT, Token()});
TEST_CASE(Operator, Symbol, 20, "<=", {OpLE, Token()});
TEST_CASE(Operator, Symbol, 21, ">=", {OpGE, Token()});
TEST_CASE(Operator, Symbol, 22, "==", {OpEq, Token()});
TEST_CASE(Operator, Symbol, 23, "!=", {OpNE, Token()});
TEST_CASE(Operator, Symbol, 24, "=", {OpSet, Token()});
TEST_CASE(Operator, Symbol, 25, "+=", {OpPlusSet, Token()});
TEST_CASE(Operator, Symbol, 26, "-=", {OpMinusSet, Token()});
TEST_CASE(Operator, Symbol, 27, "*=", {OpTimesSet, Token()});
TEST_CASE(Operator, Symbol, 28, "/=", {OpSlashSet, Token()});
TEST_CASE(Operator, Symbol, 29, "%=", {OpPercentSet, Token()});
TEST_CASE(Operator, Symbol, 30, "&=", {OpBitAndSet, Token()});
TEST_CASE(Operator, Symbol, 31, "|=", {OpBitOrSet, Token()});
TEST_CASE(Operator, Symbol, 32, "^=", {OpBitXorSet, Token()});
TEST_CASE(Operator, Symbol, 33, "&&=", {OpLogicAndSet, Token()});
TEST_CASE(Operator, Symbol, 34, "||=", {OpLogicOrSet, Token()});
TEST_CASE(Operator, Symbol, 35, "^^=", {OpLogicXorSet, Token()});
TEST_CASE(Operator, Symbol, 36, "<<=", {OpLShiftSet, Token()});
TEST_CASE(Operator, Symbol, 37, ">>=", {OpRShiftSet, Token()});
TEST_CASE(Operator, Symbol, 38, "<<<=", {OpROTLSet, Token()});
TEST_CASE(Operator, Symbol, 39, ">>>=", {OpROTRSet, Token()});
TEST_CASE(Operator, Symbol, 40, "++", {OpInc, Token()});
TEST_CASE(Operator, Symbol, 41, "--", {OpDec, Token()});
TEST_CASE(Operator, Symbol, 42, "as", {OpAs, Token()});
TEST_CASE(Operator, Symbol, 43, "bitcast_as", {OpBitcastAs, Token()});
TEST_CASE(Operator, Symbol, 44, "in", {OpIn, Token()});
TEST_CASE(Operator, Symbol, 45, "out", {OpOut, Token()});
TEST_CASE(Operator, Symbol, 46, "sizeof", {OpSizeof, Token()});
TEST_CASE(Operator, Symbol, 47, "bitsizeof", {OpBitsizeof, Token()});
TEST_CASE(Operator, Symbol, 48, "alignof", {OpAlignof, Token()});
TEST_CASE(Operator, Symbol, 49, "typeof", {OpTypeof, Token()});
TEST_CASE(Operator, Symbol, 50, "comptime", {OpComptime, Token()});
TEST_CASE(Operator, Symbol, 51, ".", {OpDot, Token()});
TEST_CASE(Operator, Symbol, 52, "..", {OpRange, Token()});
TEST_CASE(Operator, Symbol, 53, "...", {OpEllipsis, Token()});
TEST_CASE(Operator, Symbol, 54, "=>", {OpArrow, Token()});
TEST_CASE(Operator, Symbol, 55, "?", {OpTernary, Token()});

///============================================================================///
/// ADJACENT OPERATORS
TEST_CASE(Operator, Adjacent, 200, "++", {OpInc, Token()});
TEST_CASE(Operator, Adjacent, 201, "--", {OpDec, Token()});
TEST_CASE(Operator, Adjacent, 202, "**", {OpTimes, OpTimes, Token()});
TEST_CASE(Operator, Adjacent, 203, "//", {Token(Note, ""), Token()});
TEST_CASE(Operator, Adjacent, 204, "%%", {OpPercent, OpPercent, Token()});
TEST_CASE(Operator, Adjacent, 205, "&&", {OpLogicAnd, Token()});
TEST_CASE(Operator, Adjacent, 206, "||", {OpLogicOr, Token()});
TEST_CASE(Operator, Adjacent, 207, "^^", {OpLogicXor, Token()});
TEST_CASE(Operator, Adjacent, 208, "~~", {OpBitNot, OpBitNot, Token()});
TEST_CASE(Operator, Adjacent, 209, "<<<<", {OpROTL, OpLT, Token()});
TEST_CASE(Operator, Adjacent, 210, ">>>>", {OpROTR, OpGT, Token()});
TEST_CASE(Operator, Adjacent, 212, "<<<<<<", {OpROTL, OpROTL, Token()});
TEST_CASE(Operator, Adjacent, 213, ">>>>>>", {OpROTR, OpROTR, Token()});
TEST_CASE(Operator, Adjacent, 214, "&&&&", {OpLogicAnd, OpLogicAnd, Token()});
TEST_CASE(Operator, Adjacent, 215, "||||", {OpLogicOr, OpLogicOr, Token()});
TEST_CASE(Operator, Adjacent, 216, "^^^^", {OpLogicXor, OpLogicXor, Token()});
TEST_CASE(Operator, Adjacent, 217, "!!", {OpLogicNot, OpLogicNot, Token()});
TEST_CASE(Operator, Adjacent, 218, "<<", {OpLShift, Token()});
TEST_CASE(Operator, Adjacent, 219, ">>", {OpRShift, Token()});
TEST_CASE(Operator, Adjacent, 220, "<=<=", {OpLE, OpLE, Token()});
TEST_CASE(Operator, Adjacent, 221, ">=>=", {OpGE, OpGE, Token()});
TEST_CASE(Operator, Adjacent, 222, "====", {OpEq, OpEq, Token()});
TEST_CASE(Operator, Adjacent, 223, "!=!=", {OpNE, OpNE, Token()});
TEST_CASE(Operator, Adjacent, 224, "==", {OpEq, Token()});
TEST_CASE(Operator, Adjacent, 225, "+=+=", {OpPlusSet, OpPlusSet, Token()});
TEST_CASE(Operator, Adjacent, 226, "-=-=", {OpMinusSet, OpMinusSet, Token()});
TEST_CASE(Operator, Adjacent, 227, "*=*=", {OpTimesSet, OpTimesSet, Token()});
TEST_CASE(Operator, Adjacent, 228, "/=/=", {OpSlashSet, OpSlashSet, Token()});
TEST_CASE(Operator, Adjacent, 229, "%=%=", {OpPercentSet, OpPercentSet, Token()});
TEST_CASE(Operator, Adjacent, 230, "&=&=", {OpBitAndSet, OpBitAndSet, Token()});
TEST_CASE(Operator, Adjacent, 231, "|=|=", {OpBitOrSet, OpBitOrSet, Token()});
TEST_CASE(Operator, Adjacent, 232, "^=^=", {OpBitXorSet, OpBitXorSet, Token()});
TEST_CASE(Operator, Adjacent, 233, "&&=&&=", {OpLogicAndSet, OpLogicAndSet, Token()});
TEST_CASE(Operator, Adjacent, 234, "||=||=", {OpLogicOrSet, OpLogicOrSet, Token()});
TEST_CASE(Operator, Adjacent, 235, "^^=^^=", {OpLogicXorSet, OpLogicXorSet, Token()});
TEST_CASE(Operator, Adjacent, 236, "<<=<<=", {OpLShiftSet, OpLShiftSet, Token()});
TEST_CASE(Operator, Adjacent, 237, ">>=>>=", {OpRShiftSet, OpRShiftSet, Token()});
TEST_CASE(Operator, Adjacent, 238, "<<<=<<<=", {OpROTLSet, OpROTLSet, Token()});
TEST_CASE(Operator, Adjacent, 239, ">>>=>>>=", {OpROTRSet, OpROTRSet, Token()});
TEST_CASE(Operator, Adjacent, 240, "++++", {OpInc, OpInc, Token()});
TEST_CASE(Operator, Adjacent, 241, "----", {OpDec, OpDec, Token()});
TEST_CASE(Operator, Adjacent, 242, "asas", {Token(Name, "asas"), Token()});
TEST_CASE(Operator, Adjacent, 243, "bitcast_asbitcast_as", {Token(Name, "bitcast_asbitcast_as"), Token()});
TEST_CASE(Operator, Adjacent, 244, "inin", {Token(Name, "inin"), Token()});
TEST_CASE(Operator, Adjacent, 245, "outout", {Token(Name, "outout"), Token()});
TEST_CASE(Operator, Adjacent, 246, "sizeofsizeof", {Token(Name, "sizeofsizeof"), Token()});
TEST_CASE(Operator, Adjacent, 247, "bitsizeofbitsizeof", {Token(Name, "bitsizeofbitsizeof"), Token()});
TEST_CASE(Operator, Adjacent, 248, "alignofalignof", {Token(Name, "alignofalignof"), Token()});
TEST_CASE(Operator, Adjacent, 249, "typeoftypeof", {Token(Name, "typeoftypeof"), Token()});
TEST_CASE(Operator, Adjacent, 250, "comptimecomptime", {Token(Name, "comptimecomptime"), Token()});
TEST_CASE(Operator, Adjacent, 251, "..", {OpRange, Token()});
TEST_CASE(Operator, Adjacent, 252, "....", {OpEllipsis, OpDot, Token()});
TEST_CASE(Operator, Adjacent, 253, "......", {OpEllipsis, OpEllipsis, Token()});
TEST_CASE(Operator, Adjacent, 254, "=>=>", {OpArrow, OpArrow, Token()});
TEST_CASE(Operator, Adjacent, 255, "??", {OpTernary, OpTernary, Token()});