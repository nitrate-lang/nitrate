#include <gtest/gtest.h>

#include <nitrate-parser/ASTReader.hh>
#include <nitrate-parser/ASTWriter.hh>
#include <nitrate-parser/Context.hh>
#include <sstream>

#include "test-vectors.hh"

using namespace ncc::parse;

TEST(AST, FromJson) {
  auto env = std::make_shared<ncc::Environment>();
  auto original = Parser::FromString(test::vector::ASTExecise, env)->parse();
  ASSERT_TRUE(original.check());

  std::stringstream serialized;
  AST_JsonWriter writer(serialized);
  original.get()->Accept(writer);

  auto decoded = AST_JsonReader::FromString(serialized.str());
  ASSERT_TRUE(decoded.has_value());

  EXPECT_TRUE(original.get()->isSame(decoded.value()));
}

TEST(AST, FromMsgPack) {
  auto env = std::make_shared<ncc::Environment>();
  auto original = Parser::FromString(test::vector::ASTExecise, env)->parse();
  ASSERT_TRUE(original.check());

  std::stringstream serialized;
  AST_MsgPackWriter writer(serialized);
  original.get()->Accept(writer);

  auto decoded = AST_MsgPackReader::FromString(serialized.str());
  ASSERT_TRUE(decoded.has_value());

  EXPECT_TRUE(original.get()->isSame(decoded.value()));
}
