#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <nitrate-core/Environment.hh>
#include <nitrate-lexer/Lexer.hh>
#include <nitrate-parser/ASTBase.hh>
#include <nitrate-parser/ASTExpr.hh>
#include <nitrate-parser/ASTStmt.hh>
#include <nitrate-parser/Algorithm.hh>
#include <nitrate-parser/CodeWriter.hh>
#include <nitrate-parser/Context.hh>
#include <nitrate-parser/Init.hh>

using namespace ncc;
using namespace ncc::parse;

int main() {
  auto _ = ParseLibrary.GetRC();

  auto env = std::make_shared<Environment>();
  auto scanner = lex::Tokenizer(std::cin, env);

  const auto ast_root = GeneralParser::Create(scanner, env)->Parse();
  if (!ast_root.Check()) {
    return 1;
  }

  for_each<Function>(ast_root.Get(), [](auto node) {
    auto function = node.template As<Function>();
    function->SetBody(nullptr);
  });

  for_each<Variable>(ast_root.Get(), [](auto node) {
    auto variable = node.template As<Variable>();
    /// FIXME: TODO: Fix this

    variable->SetInitializer(nullptr);
  });

  auto writer = CodeWriterFactory::Create(std::cout);
  ast_root.Get()->Accept(*writer);

  return 0;
}
