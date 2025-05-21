// #include <gtest/gtest.h>

// #include <array>
// #include <nitrate-lexer/Grammar.hh>
// #include <nitrate-lexer/Token.hh>
// #include <sstream>

// using namespace ncc::lex;

// TEST(Lexer, GetOperatorPrecedence) {
//   struct W {
//     int m_precedence;
//     Operator m_op;
//     OpMode m_mode;

//     constexpr W(int precedence, Operator op, OpMode mode) : m_precedence(precedence), m_op(op), m_mode(mode) {}
//   };

//   static constexpr auto kOperatorPrecedenceDefinition = []() {
//     std::array map = {
//         W{190, OpDot, OpMode::Binary},
//         W{180, OpInc, OpMode::PostUnary},
//         W{180, OpDec, OpMode::PostUnary},
//         W{170, OpPlus, OpMode::PreUnary},
//         W{170, OpMinus, OpMode::PreUnary},
//         W{170, OpTimes, OpMode::PreUnary},
//         W{170, OpSlash, OpMode::PreUnary},
//         W{170, OpPercent, OpMode::PreUnary},
//         W{170, OpBitAnd, OpMode::PreUnary},
//         W{170, OpBitOr, OpMode::PreUnary},
//         W{170, OpBitXor, OpMode::PreUnary},
//         W{170, OpBitNot, OpMode::PreUnary},
//         W{170, OpLShift, OpMode::PreUnary},
//         W{170, OpRShift, OpMode::PreUnary},
//         W{170, OpROTL, OpMode::PreUnary},
//         W{170, OpROTR, OpMode::PreUnary},
//         W{170, OpLogicAnd, OpMode::PreUnary},
//         W{170, OpLogicOr, OpMode::PreUnary},
//         W{170, OpLogicXor, OpMode::PreUnary},
//         W{170, OpLogicNot, OpMode::PreUnary},
//         W{170, OpLT, OpMode::PreUnary},
//         W{170, OpGT, OpMode::PreUnary},
//         W{170, OpLE, OpMode::PreUnary},
//         W{170, OpGE, OpMode::PreUnary},
//         W{170, OpEq, OpMode::PreUnary},
//         W{170, OpNE, OpMode::PreUnary},
//         W{170, OpSet, OpMode::PreUnary},
//         W{170, OpPlusSet, OpMode::PreUnary},
//         W{170, OpMinusSet, OpMode::PreUnary},
//         W{170, OpTimesSet, OpMode::PreUnary},
//         W{170, OpSlashSet, OpMode::PreUnary},
//         W{170, OpPercentSet, OpMode::PreUnary},
//         W{170, OpBitAndSet, OpMode::PreUnary},
//         W{170, OpBitOrSet, OpMode::PreUnary},
//         W{170, OpBitXorSet, OpMode::PreUnary},
//         W{170, OpLogicAndSet, OpMode::PreUnary},
//         W{170, OpLogicOrSet, OpMode::PreUnary},
//         W{170, OpLogicXorSet, OpMode::PreUnary},
//         W{170, OpLShiftSet, OpMode::PreUnary},
//         W{170, OpRShiftSet, OpMode::PreUnary},
//         W{170, OpROTLSet, OpMode::PreUnary},
//         W{170, OpROTRSet, OpMode::PreUnary},
//         W{170, OpInc, OpMode::PreUnary},
//         W{170, OpDec, OpMode::PreUnary},
//         W{170, OpAs, OpMode::PreUnary},
//         W{170, OpBitcastAs, OpMode::PreUnary},
//         W{170, OpIn, OpMode::PreUnary},
//         W{170, OpOut, OpMode::PreUnary},
//         W{170, OpSizeof, OpMode::PreUnary},
//         W{170, OpBitsizeof, OpMode::PreUnary},
//         W{170, OpAlignof, OpMode::PreUnary},
//         W{170, OpTypeof, OpMode::PreUnary},
//         W{170, OpComptime, OpMode::PreUnary},
//         W{170, OpDot, OpMode::PreUnary},
//         W{170, OpRange, OpMode::PreUnary},
//         W{170, OpEllipsis, OpMode::PreUnary},
//         W{170, OpArrow, OpMode::PreUnary},
//         W{170, OpTernary, OpMode::PreUnary},
//         W{160, OpAs, OpMode::Binary},
//         W{160, OpBitcastAs, OpMode::Binary},
//         W{150, OpTimes, OpMode::Binary},
//         W{150, OpSlash, OpMode::Binary},
//         W{150, OpPercent, OpMode::Binary},
//         W{140, OpPlus, OpMode::Binary},
//         W{140, OpMinus, OpMode::Binary},
//         W{130, OpLShift, OpMode::Binary},
//         W{130, OpRShift, OpMode::Binary},
//         W{130, OpROTL, OpMode::Binary},
//         W{130, OpROTR, OpMode::Binary},
//         W{120, OpBitAnd, OpMode::Binary},
//         W{110, OpBitXor, OpMode::Binary},
//         W{100, OpBitOr, OpMode::Binary},
//         W{80, OpEq, OpMode::Binary},
//         W{80, OpNE, OpMode::Binary},
//         W{80, OpLT, OpMode::Binary},
//         W{80, OpGT, OpMode::Binary},
//         W{80, OpLE, OpMode::Binary},
//         W{80, OpGE, OpMode::Binary},
//         W{70, OpLogicAnd, OpMode::Binary},
//         W{60, OpLogicOr, OpMode::Binary},
//         W{50, OpLogicXor, OpMode::Binary},
//         W{40, OpTernary, OpMode::Ternary},
//         W{30, OpIn, OpMode::Binary},
//         W{30, OpOut, OpMode::Binary},
//         W{20, OpRange, OpMode::Binary},
//         W{20, OpArrow, OpMode::Binary},
//         W{10, OpSet, OpMode::Binary},
//         W{10, OpPlusSet, OpMode::Binary},
//         W{10, OpMinusSet, OpMode::Binary},
//         W{10, OpTimesSet, OpMode::Binary},
//         W{10, OpSlashSet, OpMode::Binary},
//         W{10, OpPercentSet, OpMode::Binary},
//         W{10, OpBitAndSet, OpMode::Binary},
//         W{10, OpBitOrSet, OpMode::Binary},
//         W{10, OpBitXorSet, OpMode::Binary},
//         W{10, OpLogicAndSet, OpMode::Binary},
//         W{10, OpLogicOrSet, OpMode::Binary},
//         W{10, OpLogicXorSet, OpMode::Binary},
//         W{10, OpLShiftSet, OpMode::Binary},
//         W{10, OpRShiftSet, OpMode::Binary},
//         W{10, OpROTLSet, OpMode::Binary},
//         W{10, OpROTRSet, OpMode::Binary},
//     };

//     static_assert(map.size() == 102);

//     return map;
//   }();

//   for (auto w : kOperatorPrecedenceDefinition) {
//     EXPECT_EQ(GetOperatorPrecedence(w.m_op, w.m_mode), w.m_precedence);
//   }
// }

// TEST(Lexer, GetOperatorPrecedence_BadInput) { EXPECT_EQ(GetOperatorPrecedence(OpAs, OpMode::Ternary), -1); }

// TEST(Lexer, GetOperatorAssociativity) {
//   struct W {
//     Operator m_op;
//     OpMode m_mode;
//     Associativity m_assoc;

//     constexpr W(Operator op, OpMode mode, Associativity assoc) : m_op(op), m_mode(mode), m_assoc(assoc) {}
//   };

//   static constexpr auto kOperatorAssociativityDefinition = []() {
//     std::array<W, 102> map = {
//         W{OpDot, OpMode::Binary, Left},
//         W{OpInc, OpMode::PostUnary, Left},
//         W{OpDec, OpMode::PostUnary, Left},
//         W{OpPlus, OpMode::PreUnary, Right},
//         W{OpMinus, OpMode::PreUnary, Right},
//         W{OpTimes, OpMode::PreUnary, Right},
//         W{OpSlash, OpMode::PreUnary, Right},
//         W{OpPercent, OpMode::PreUnary, Right},
//         W{OpBitAnd, OpMode::PreUnary, Right},
//         W{OpBitOr, OpMode::PreUnary, Right},
//         W{OpBitXor, OpMode::PreUnary, Right},
//         W{OpBitNot, OpMode::PreUnary, Right},
//         W{OpLShift, OpMode::PreUnary, Right},
//         W{OpRShift, OpMode::PreUnary, Right},
//         W{OpROTL, OpMode::PreUnary, Right},
//         W{OpROTR, OpMode::PreUnary, Right},
//         W{OpLogicAnd, OpMode::PreUnary, Right},
//         W{OpLogicOr, OpMode::PreUnary, Right},
//         W{OpLogicXor, OpMode::PreUnary, Right},
//         W{OpLogicNot, OpMode::PreUnary, Right},
//         W{OpLT, OpMode::PreUnary, Right},
//         W{OpGT, OpMode::PreUnary, Right},
//         W{OpLE, OpMode::PreUnary, Right},
//         W{OpGE, OpMode::PreUnary, Right},
//         W{OpEq, OpMode::PreUnary, Right},
//         W{OpNE, OpMode::PreUnary, Right},
//         W{OpSet, OpMode::PreUnary, Right},
//         W{OpPlusSet, OpMode::PreUnary, Right},
//         W{OpMinusSet, OpMode::PreUnary, Right},
//         W{OpTimesSet, OpMode::PreUnary, Right},
//         W{OpSlashSet, OpMode::PreUnary, Right},
//         W{OpPercentSet, OpMode::PreUnary, Right},
//         W{OpBitAndSet, OpMode::PreUnary, Right},
//         W{OpBitOrSet, OpMode::PreUnary, Right},
//         W{OpBitXorSet, OpMode::PreUnary, Right},
//         W{OpLogicAndSet, OpMode::PreUnary, Right},
//         W{OpLogicOrSet, OpMode::PreUnary, Right},
//         W{OpLogicXorSet, OpMode::PreUnary, Right},
//         W{OpLShiftSet, OpMode::PreUnary, Right},
//         W{OpRShiftSet, OpMode::PreUnary, Right},
//         W{OpROTLSet, OpMode::PreUnary, Right},
//         W{OpROTRSet, OpMode::PreUnary, Right},
//         W{OpInc, OpMode::PreUnary, Right},
//         W{OpDec, OpMode::PreUnary, Right},
//         W{OpAs, OpMode::PreUnary, Right},
//         W{OpBitcastAs, OpMode::PreUnary, Right},
//         W{OpIn, OpMode::PreUnary, Right},
//         W{OpOut, OpMode::PreUnary, Right},
//         W{OpSizeof, OpMode::PreUnary, Right},
//         W{OpBitsizeof, OpMode::PreUnary, Right},
//         W{OpAlignof, OpMode::PreUnary, Right},
//         W{OpTypeof, OpMode::PreUnary, Right},
//         W{OpComptime, OpMode::PreUnary, Right},
//         W{OpDot, OpMode::PreUnary, Right},
//         W{OpRange, OpMode::PreUnary, Right},
//         W{OpEllipsis, OpMode::PreUnary, Right},
//         W{OpArrow, OpMode::PreUnary, Right},
//         W{OpTernary, OpMode::PreUnary, Right},
//         W{OpAs, OpMode::Binary, Left},
//         W{OpBitcastAs, OpMode::Binary, Left},
//         W{OpTimes, OpMode::Binary, Left},
//         W{OpSlash, OpMode::Binary, Left},
//         W{OpPercent, OpMode::Binary, Left},
//         W{OpPlus, OpMode::Binary, Left},
//         W{OpMinus, OpMode::Binary, Left},
//         W{OpLShift, OpMode::Binary, Left},
//         W{OpRShift, OpMode::Binary, Left},
//         W{OpROTL, OpMode::Binary, Left},
//         W{OpROTR, OpMode::Binary, Left},
//         W{OpBitAnd, OpMode::Binary, Left},
//         W{OpBitXor, OpMode::Binary, Left},
//         W{OpBitOr, OpMode::Binary, Left},
//         W{OpEq, OpMode::Binary, Left},
//         W{OpNE, OpMode::Binary, Left},
//         W{OpLT, OpMode::Binary, Left},
//         W{OpGT, OpMode::Binary, Left},
//         W{OpLE, OpMode::Binary, Left},
//         W{OpGE, OpMode::Binary, Left},
//         W{OpLogicAnd, OpMode::Binary, Left},
//         W{OpLogicOr, OpMode::Binary, Left},
//         W{OpLogicXor, OpMode::Binary, Left},
//         W{OpTernary, OpMode::Ternary, Right},
//         W{OpIn, OpMode::Binary, Left},
//         W{OpOut, OpMode::Binary, Left},
//         W{OpRange, OpMode::Binary, Left},
//         W{OpArrow, OpMode::Binary, Left},
//         W{OpSet, OpMode::Binary, Right},
//         W{OpPlusSet, OpMode::Binary, Right},
//         W{OpMinusSet, OpMode::Binary, Right},
//         W{OpTimesSet, OpMode::Binary, Right},
//         W{OpSlashSet, OpMode::Binary, Right},
//         W{OpPercentSet, OpMode::Binary, Right},
//         W{OpBitAndSet, OpMode::Binary, Right},
//         W{OpBitOrSet, OpMode::Binary, Right},
//         W{OpBitXorSet, OpMode::Binary, Right},
//         W{OpLogicAndSet, OpMode::Binary, Right},
//         W{OpLogicOrSet, OpMode::Binary, Right},
//         W{OpLogicXorSet, OpMode::Binary, Right},
//         W{OpLShiftSet, OpMode::Binary, Right},
//         W{OpRShiftSet, OpMode::Binary, Right},
//         W{OpROTLSet, OpMode::Binary, Right},
//         W{OpROTRSet, OpMode::Binary, Right},
//     };

//     static_assert(map.size() == 102);

//     return map;
//   }();

//   for (auto w : kOperatorAssociativityDefinition) {
//     EXPECT_EQ(GetOperatorAssociativity(w.m_op, w.m_mode), w.m_assoc);
//   }
// }

// TEST(Lexer, GetOperatorAssociativity_BadInput) { EXPECT_EQ(GetOperatorAssociativity(OpAs, OpMode::Ternary), Left); }

// TEST(Lexer, Token_ToString) {
//   EXPECT_EQ(ToString(EofF, TokenData("")), "");
//   EXPECT_EQ(ToString(EofF, TokenData(OpPlus)), "");

//   EXPECT_EQ(ToString(KeyW, TokenData(Scope)), "scope");
//   EXPECT_EQ(ToString(KeyW, TokenData(Struct)), "struct");

//   EXPECT_EQ(ToString(Oper, TokenData(OpPlus)), "+");
//   EXPECT_EQ(ToString(Oper, TokenData(OpLogicAndSet)), "&&=");

//   EXPECT_EQ(ToString(Punc, TokenData(PuncLPar)), "(");
//   EXPECT_EQ(ToString(Punc, TokenData(PuncScope)), "::");

//   EXPECT_EQ(ToString(Name, TokenData("hello_world")), "hello_world");
//   EXPECT_EQ(ToString(Name, TokenData("")), "");
//   EXPECT_EQ(ToString(Name, TokenData("in valid$!()@")), "in valid$!()@");

//   EXPECT_EQ(ToString(IntL, TokenData("123")), "123");
//   EXPECT_EQ(ToString(IntL, TokenData("")), "");
//   EXPECT_EQ(ToString(IntL, TokenData("in valid$!()@")), "in valid$!()@");

//   EXPECT_EQ(ToString(NumL, TokenData("123.456")), "123.456");
//   EXPECT_EQ(ToString(NumL, TokenData("")), "");
//   EXPECT_EQ(ToString(NumL, TokenData("in valid$!()@")), "in valid$!()@");

//   EXPECT_EQ(ToString(Text, TokenData("hello world")), "hello world");
//   EXPECT_EQ(ToString(Text, TokenData("")), "");

//   EXPECT_EQ(ToString(Char, TokenData("a")), "a");
//   EXPECT_EQ(ToString(Char, TokenData("")), "");
//   EXPECT_EQ(ToString(Char, TokenData("in valid$!()@")), "in valid$!()@");

//   EXPECT_EQ(ToString(MacB, TokenData("")), "");
//   EXPECT_EQ(ToString(MacB, TokenData("in valid$!()@")), "in valid$!()@");

//   EXPECT_EQ(ToString(Macr, TokenData("")), "");
//   EXPECT_EQ(ToString(Macr, TokenData("in valid$!()@")), "in valid$!()@");

//   EXPECT_EQ(ToString(Note, TokenData("")), "");
//   EXPECT_EQ(ToString(Note, TokenData("this is a note")), "this is a note");
// }

// TEST(Lexer, TokenType_ToString) {
//   EXPECT_EQ(ToString(EofF), "eof");
//   EXPECT_EQ(ToString(KeyW), "key");
//   EXPECT_EQ(ToString(Oper), "op");
//   EXPECT_EQ(ToString(Punc), "sym");
//   EXPECT_EQ(ToString(Name), "name");
//   EXPECT_EQ(ToString(IntL), "int");
//   EXPECT_EQ(ToString(NumL), "num");
//   EXPECT_EQ(ToString(Text), "str");
//   EXPECT_EQ(ToString(Char), "char");
//   EXPECT_EQ(ToString(MacB), "macb");
//   EXPECT_EQ(ToString(Macr), "macr");
//   EXPECT_EQ(ToString(Note), "note");
// }

// TEST(Lexer, Token_OStream) {
//   static std::unordered_map<Token, std::string_view> test_vectors = {
//       {Token(EofF, TokenData::GetDefault(EofF)), R"($TOKEN{21{"pos":null,"type":0}})"},
//       {Token(EofF, TokenData::GetDefault(EofF), LocationID(3434)), R"($TOKEN{21{"pos":3434,"type":0}})"},

//       {Token(KeyW, TokenData::GetDefault(KeyW)), R"($TOKEN{21{"pos":null,"type":1}})"},
//       {Token(KeyW, TokenData::GetDefault(KeyW), LocationID(3434)), R"($TOKEN{21{"pos":3434,"type":1}})"},

//       {Token(Oper, TokenData::GetDefault(Oper)), R"($TOKEN{21{"pos":null,"type":2}})"},
//       {Token(Oper, TokenData::GetDefault(Oper), LocationID(3434)), R"($TOKEN{21{"pos":3434,"type":2}})"},

//       {Token(Punc, TokenData::GetDefault(Punc)), R"($TOKEN{21{"pos":null,"type":3}})"},
//       {Token(Punc, TokenData::GetDefault(Punc), LocationID(3434)), R"($TOKEN{21{"pos":3434,"type":3}})"},

//       {Token(Name, TokenData::GetDefault(Name)), R"($TOKEN{21{"pos":null,"type":4}})"},
//       {Token(Name, TokenData::GetDefault(Name), LocationID(3434)), R"($TOKEN{21{"pos":3434,"type":4}})"},

//       {Token(IntL, TokenData::GetDefault(IntL)), R"($TOKEN{21{"pos":null,"type":5}})"},
//       {Token(IntL, TokenData::GetDefault(IntL), LocationID(3434)), R"($TOKEN{21{"pos":3434,"type":5}})"},

//       {Token(NumL, TokenData::GetDefault(NumL)), R"($TOKEN{21{"pos":null,"type":6}})"},
//       {Token(NumL, TokenData::GetDefault(NumL), LocationID(3434)), R"($TOKEN{21{"pos":3434,"type":6}})"},

//       {Token(Text, TokenData::GetDefault(Text)), R"($TOKEN{21{"pos":null,"type":7}})"},
//       {Token(Text, TokenData::GetDefault(Text), LocationID(3434)), R"($TOKEN{21{"pos":3434,"type":7}})"},

//       {Token(Char, TokenData::GetDefault(Char)), R"($TOKEN{21{"pos":null,"type":8}})"},
//       {Token(Char, TokenData::GetDefault(Char), LocationID(3434)), R"($TOKEN{21{"pos":3434,"type":8}})"},

//       {Token(MacB, TokenData::GetDefault(MacB)), R"($TOKEN{21{"pos":null,"type":9}})"},
//       {Token(MacB, TokenData::GetDefault(MacB), LocationID(3434)), R"($TOKEN{21{"pos":3434,"type":9}})"},

//       {Token(Macr, TokenData::GetDefault(Macr)), R"($TOKEN{22{"pos":null,"type":10}})"},
//       {Token(Macr, TokenData::GetDefault(Macr), LocationID(3434)), R"($TOKEN{22{"pos":3434,"type":10}})"},

//       {Token(Note, TokenData::GetDefault(Note)), R"($TOKEN{22{"pos":null,"type":11}})"},
//       {Token(Note, TokenData::GetDefault(Note), LocationID(3434)), R"($TOKEN{22{"pos":3434,"type":11}})"},
//   };

//   for (const auto &[token, expected_output] : test_vectors) {
//     std::stringstream ss;
//     ss << token;
//     EXPECT_EQ(ss.str(), expected_output);
//   }
// }
