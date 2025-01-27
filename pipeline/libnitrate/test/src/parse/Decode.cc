#include <gtest/gtest.h>

#include <nitrate-lexer/Lexer.hh>
#include <nitrate-parser/ASTReader.hh>
#include <nitrate-parser/ASTWriter.hh>
#include <nitrate-parser/Context.hh>

#include "test-vectors.hh"

using namespace ncc::parse;

TEST(AST, FromJson) {
  auto env = std::make_shared<ncc::Environment>();
  auto original =
      Parser::FromString<ncc::lex::Tokenizer>(test::vector::kAstExecise, env)
          ->Parse();
  ASSERT_TRUE(original.Check());

  auto serialized = original.Get()->ToJson();
  auto decoded = AstReader::FromString(serialized);
  ASSERT_TRUE(decoded.has_value());

  EXPECT_TRUE(original.Get()->IsEq(decoded.value()));
}
