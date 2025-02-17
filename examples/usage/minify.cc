#include <nitrate-core/Environment.hh>
#include <nitrate-parser/CodeWriter.hh>
#include <nitrate-parser/Context.hh>
#include <nitrate-seq/Sequencer.hh>

using namespace ncc;

int main() {
  auto env = std::make_shared<Environment>();
  auto scanner = seq::Sequencer(std::cin, env);
  auto parser = parse::Parser::Create(scanner, env);
  auto ast_maybe = parser->Parse();
  if (!ast_maybe.Check()) {
    return 1;
  }

  auto minifier = parse::CodeWriterFactory::Create(std::cout);
  ast_maybe.Get()->Accept(*minifier);
  return 0;
}
