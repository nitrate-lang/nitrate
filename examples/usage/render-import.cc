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
#include <nitrate-parser/Utility.hh>

using namespace ncc;
using namespace ncc::parse;

int main() {
  auto env = std::make_shared<Environment>();
  auto scanner = lex::Tokenizer(std::cin, env);

  const auto ast_root = Parser::Create(scanner, env)->Parse();
  if (!ast_root.Check()) {
    return 1;
  }

  for_each<Function>(ast_root.Get(), [](auto node) {
    auto function = node.template As<Function>();
    function->SetBody(nullptr);
  });

  /// FIXME: Fix this
  for_each<Variable>(ast_root.Get(), [](auto node) {
    auto variable = node.template As<Variable>();

    {
      auto attributes = ExpressionList(variable->GetAttributes().begin(), variable->GetAttributes().end());
      attributes.push_back(CreateNode<Identifier>("extern")());
      variable->SetAttributes(attributes);
    }

    variable->SetInitializer(nullptr);
  });

  auto writer = CodeWriterFactory::Create(std::cout);
  ast_root.Get()->Accept(*writer);

  return 0;
}
