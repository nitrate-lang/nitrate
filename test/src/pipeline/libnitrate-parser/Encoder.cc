#include <gtest/gtest.h>

#include <nitrate-core/Environment.hh>
#include <nitrate-lexer/Lexer.hh>
#include <nitrate-parser/ASTBase.hh>
#include <nitrate-parser/ASTReader.hh>
#include <nitrate-parser/ASTWriter.hh>
#include <nitrate-parser/Context.hh>
#include <nitrate-parser/Init.hh>
#include <static-data/SourceSample_01.hh>

using namespace ncc::parse;

TEST(AST, Encoder) {
  if (auto lib_rc = ncc::parse::ParseLibrary.GetRC()) {
    auto subid = ncc::Log.Subscribe([](const std::string &msg, ncc::Sev sev, const ncc::ECBase &ec) {
      std::cout << ec.Format(msg, sev) << std::endl;
    });

    auto env = std::make_shared<ncc::Environment>();
    auto original = Parser::FromString<ncc::lex::Tokenizer>(test::vector::SOURCE_SAMPLE_01, env)->Parse();
    EXPECT_TRUE(original.Check());

    auto serialized = original.Get()->Serialize();
    auto decoded = AstReader(serialized).Get();
    ASSERT_TRUE(decoded.has_value());

    EXPECT_TRUE(original.Get()->IsEq(decoded.value().Get()));
    EXPECT_TRUE(serialized.size() > 100);

    ncc::Log.Unsubscribe(subid);
  }
}
