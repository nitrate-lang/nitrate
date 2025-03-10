#pragma once

#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <nitrate-core/Environment.hh>
#include <nitrate-lexer/Grammar.hh>
#include <nitrate-lexer/Init.hh>
#include <nitrate-lexer/Lexer.hh>
#include <nitrate-parser/ASTExpr.hh>
#include <nitrate-parser/ASTStmt.hh>
#include <nitrate-parser/ASTType.hh>
#include <nitrate-parser/Context.hh>
#include <nitrate-parser/Init.hh>
#include <nitrate-seq/Sequencer.hh>

namespace detail {
  static inline std::optional<std::string> ParseString(std::string_view text) {
    auto env = std::make_shared<ncc::Environment>();
    boost::iostreams::stream<boost::iostreams::array_source> ss(text.data(), text.size());
    auto tokenizer = ncc::seq::Sequencer(ss, env);
    auto my_pool = ncc::DynamicArena();
    auto ast = ncc::parse::GeneralParser::Create(tokenizer, env, my_pool)->Parse();
    if (!ast.Check()) {
      return std::nullopt;
    }

    return ast.Get()->PrettyPrint();
  }

  static inline auto NormalizeInput(std::string_view text) {
    std::string result(text);
    std::erase_if(result, isspace);
    return result;
  }
}  // namespace detail

#define TEST_CASE(__DOMAIN, __GROUP, __TEST_NUMBER, __TEXT, ...)   \
  TEST(AST, Parse_##__DOMAIN##_##__GROUP##_##__TEST_NUMBER) {      \
    if (auto lib_rc = ncc::parse::ParseLibrary.GetRC()) {          \
      const auto expected = detail::NormalizeInput(#__VA_ARGS__);  \
      const auto result = detail::ParseString(__TEXT);             \
      if (!result.has_value()) {                                   \
        if (expected != "nullptr") {                               \
          FAIL() << "Failed to parse the input: " << __TEXT;       \
        }                                                          \
        return;                                                    \
      }                                                            \
      EXPECT_EQ(expected, detail::NormalizeInput(result.value())); \
    }                                                              \
  }
