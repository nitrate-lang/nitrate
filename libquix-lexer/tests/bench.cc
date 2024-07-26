#include <quix-lexer/Lib.h>

#include <chrono>
#include <filesystem>
#include <iostream>
#include <string_view>
#include <unordered_map>
#include <vector>

using timepoint_t = std::chrono::time_point<std::chrono::high_resolution_clock>;

void print_results(timepoint_t start, timepoint_t end, size_t size, size_t tok_count) {
  std::chrono::nanoseconds elapsed = end - start;
  std::cout << "Elapsed time: " << elapsed.count() << " ns" << std::endl;
  std::cout << "File size: " << size << " bytes" << std::endl;
  std::cout << "Token count: " << tok_count << std::endl;

  if (tok_count == 0) {
    std::cout << "No tokens found" << std::endl;
    return;
  }

  double tokens_per_sec = tok_count / (elapsed.count() / 1e9);
  std::cout << "Tokens per second: " << tokens_per_sec << std::endl;

  double mbps = size / (elapsed.count() / 1e9) / 1e6;
  std::cout << "MB/s: " << mbps << std::endl;

  double ns_per_tok = elapsed.count() / (double)tok_count;
  std::cout << "ns per token: " << ns_per_tok << std::endl;

  double avg_tok_size = size / (double)tok_count;
  std::cout << "Average token size: " << avg_tok_size << " bytes" << std::endl;

  double ns_per_byte = elapsed.count() / (double)size;
  std::cout << "ns per byte: " << ns_per_byte << std::endl;
}

void do_benchmark(FILE *file) {
  fseek(file, 0, SEEK_END);
  size_t size = ftell(file);
  fseek(file, 0, SEEK_SET);

  timepoint_t start, end;
  size_t tok_count = 0;

  start = std::chrono::high_resolution_clock::now();
  {
    qlex_t *lexer = qlex_new(file);

    qlex_tok_t tok;
    while ((tok = qlex_next(lexer)).ty != qEofF) {
      ++tok_count;
    }

    qlex_delete(lexer);
  }
  end = std::chrono::high_resolution_clock::now();

  print_results(start, end, size, tok_count);
}

int main(int argc, char **argv) {
  qlex_lib_init();

  std::vector<std::string_view> args(argv, argv + argc);

  if (args.size() < 2) {
    std::cerr << "Usage: " << args[0] << " <input-file>" << std::endl;
    return 1;
  }

  if (!std::filesystem::exists(args[1])) {
    std::cerr << "File not found: " << args[1] << std::endl;
    return 1;
  }

  FILE *file = fopen(args[1].data(), "r");
  if (!file) {
    std::cerr << "Failed to open file: " << args[1] << std::endl;
    return 1;
  }

  do_benchmark(file);

  fclose(file);

  qlex_lib_deinit();
}