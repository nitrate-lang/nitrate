// #include <gtest/gtest.h>

// TODO: Reenable testing

// #include <nitrate-lexer/Init.hh>
// #include <nitrate-lexer/Lexer.hh>
// #include <pipeline/libnitrate-lexer/LexicalCase.hh>

// using namespace ncc;
// using namespace ncc::lex;

// #define SOURCE_LOCATION_TEST(__TESTNUM, __SOURCE_CODE, __LINE, __COLUMN, __OFFSET) \
//   TEST(Lexer, SourceLocation_##__TESTNUM) {                                        \
//     if (auto lib_rc = LexerLibrary.GetRC()) {                                      \
//       auto result = LexString(__SOURCE_CODE);                                      \
//       auto loc = result.first.at(0).GetStart().Get(result.second);                 \
//       EXPECT_EQ(loc.GetRow(), __LINE);                                             \
//       EXPECT_EQ(loc.GetCol(), __COLUMN);                                           \
//       EXPECT_EQ(loc.GetOffset(), __OFFSET);                                        \
//     }                                                                              \
//   }

// /// TODO: Test cases for source location
