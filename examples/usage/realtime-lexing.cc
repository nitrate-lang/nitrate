// Read code from stdin and write it to a file in real-time.

#include <fstream>
#include <nitrate/code.hh>

int main() {
  std::fstream output_file("tokens.json", std::ios::out);
  if (!output_file.is_open()) {
    std::cerr << "Failed to open tokens.json" << std::endl;

    return 1;
  }

  if (auto future = nitrate::pipeline(std::cin, output_file, {"lex"}).get()) {
    std::cout << "Tokens written to tokens.json" << std::endl;

    return 0;
  } else {
    std::cerr << "Compilation failed" << std::endl;

    return 1;
  }
}
