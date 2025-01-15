// Transform source code into a JSON encoded array of tokens.
// No preprocessing is performed.

#include <nitrate/code.hh>

int main() {
  const std::string source = R"(
    pub fn main(): i32 {
      ret 42;
    }
  )";

  std::string tokens_json;
  if (auto future = nitrate::Pipeline(source, tokens_json, {"lex"}).Get()) {
    std::cout << tokens_json;

    return 0;
  } else {
    std::cerr << "Compilation failed" << std::endl;

    return 1;
  }
}
