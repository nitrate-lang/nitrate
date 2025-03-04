#include <gtest/gtest.h>

#include <nitrate-lexer/Init.hh>

using namespace ncc;
using namespace ncc::lex;

TEST(Lexer, Init_Init) {
  if (!LexerLibrary.IsInitialized()) {
    EXPECT_FALSE(CoreLibrary.IsInitialized());
    EXPECT_FALSE(LexerLibrary.IsInitialized());

    if (auto lib_rc = LexerLibrary.GetRC()) {
      EXPECT_TRUE(CoreLibrary.IsInitialized());
      EXPECT_TRUE(LexerLibrary.IsInitialized());
    }

    EXPECT_FALSE(CoreLibrary.IsInitialized());
    EXPECT_FALSE(LexerLibrary.IsInitialized());
  }
}
