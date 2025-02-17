#pragma once

#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <nitrate-lexer/Init.hh>
#include <nitrate-lexer/Lexer.hh>

static inline std::pair<std::vector<ncc::lex::Token>, ncc::lex::Tokenizer> LexString(std::string_view text) {
  auto env = std::make_shared<ncc::Environment>();
  boost::iostreams::stream<boost::iostreams::array_source> ss(text.data(), text.size());
  ncc::lex::Tokenizer tokenizer(ss, env);
  std::vector<ncc::lex::Token> tokens;
  while (auto token = tokenizer.Next()) {
    tokens.push_back(token);
  }

  return {std::move(tokens), std::move(tokenizer)};
}

#define TEST_CASE(__DOMAIN, __GROUP, __TEST_NUMBER, __TEXT, ...) \
  TEST(Lexer, Parse_##__DOMAIN##_##__GROUP##_##__TEST_NUMBER) {  \
    if (auto lib_rc = ncc::lex::LexerLibrary.GetRC()) {          \
      std::vector<ncc::lex::Token> expected = __VA_ARGS__;       \
      EXPECT_EQ(expected, LexString(__TEXT).first);              \
    }                                                            \
  }
