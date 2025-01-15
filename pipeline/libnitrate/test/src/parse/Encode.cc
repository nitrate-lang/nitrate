#include <gtest/gtest.h>

#include <nitrate/code.hh>
#include <nlohmann/json.hpp>

#include "test-vectors.hh"

using namespace nitrate;

TEST(AST, ToJson) {
  std::string output;
  ASSERT_TRUE(Chain(test::vector::kAstExecise, output,
                    {
                        {"lex"},
                        {"parse", "-fuse-json"},
                    })
                  .Get());

  ASSERT_FALSE(nlohmann::json::parse(output, nullptr, false).is_discarded());
}

TEST(AST, ToMsgPack) {
  std::string output;
  ASSERT_TRUE(Chain(test::vector::kAstExecise, output,
                    {
                        {"lex"},
                        {"parse", "-fuse-msgpack"},
                    })
                  .Get());

  ASSERT_FALSE(
      nlohmann::json::from_msgpack(output, true, false).is_discarded());
}
