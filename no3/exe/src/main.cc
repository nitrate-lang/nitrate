#include <no3/Interpreter.hh>

int main(int argc, char* argv[]) {
  auto _ = no3::OpenLibrary();

  std::vector<std::string_view> args(argv, argv + argc);
  return no3::Interpreter().Execute(args) ? 0 : 1;
}
