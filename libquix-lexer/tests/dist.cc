#include <quix-lexer/Lib.h>

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <quix-core/Classes.hh>
#include <quix-lexer/Classes.hh>
#include <string_view>
#include <unordered_map>
#include <vector>

struct Dist {
  std::unordered_map<qlex_ty_t, size_t> counts;
};

void do_dist(FILE *file) {
  fseek(file, 0, SEEK_END);
  fseek(file, 0, SEEK_SET);

  Dist dist;

  {
    qcore_env env;
    qlex lexer(file, nullptr, env.get());

    qlex_tok_t tok;
    while ((tok = qlex_next(lexer.get())).ty != qEofF) {
      ++dist.counts[tok.ty];
    }
  }

  std::cout << "Token distribution:" << std::endl;
  std::vector<std::pair<qlex_ty_t, size_t>> sorted;
  for (const auto &[ty, count] : dist.counts) {
    sorted.push_back({ty, count});
  }
  std::sort(sorted.begin(), sorted.end(),
            [](const auto &a, const auto &b) { return a.second > b.second; });
  for (const auto &[ty, count] : sorted) {
    std::cout << "  " << qlex_ty_str(ty) << ": " << count << std::endl;
  }
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

  FILE *file = fopen(args[1].data(), "rb");
  if (!file) {
    std::cerr << "Failed to open file: " << args[1] << std::endl;
    return 1;
  }

  do_dist(file);

  fclose(file);

  qlex_lib_deinit();
}