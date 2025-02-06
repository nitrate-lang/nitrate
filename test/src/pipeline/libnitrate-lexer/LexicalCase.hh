#pragma once

#include <nitrate-lexer/Init.hh>
#include <nitrate-lexer/Lexer.hh>

#define TEST_CASE(__DOMAIN, __GROUP, __TEST_NUMBER, __TEXT, ...) \
  TEST(Lexer, Parse_##__DOMAIN##_##__GROUP##_##__TEST_NUMBER) {  \
    if (auto lib_rc = ncc::lex::LexerLibrary.GetRC()) {          \
      std::stringstream ss(__TEXT);                              \
      auto env = std::make_shared<ncc::Environment>();           \
      ncc::lex::Tokenizer tokenizer(ss, env);                    \
      std::vector<ncc::lex::Token> expected = __VA_ARGS__;       \
      std::vector<ncc::lex::Token> actual;                       \
      while (auto token = tokenizer.Next()) {                    \
        actual.push_back(token);                                 \
      }                                                          \
      EXPECT_EQ(expected, actual);                               \
    }                                                            \
  }
