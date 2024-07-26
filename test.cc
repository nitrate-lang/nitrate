#include <quix-lexer/Lib.h>

#include <chrono>
#include <filesystem>
#include <iostream>
#include <string_view>
#include <vector>

using timepoint_t = std::chrono::time_point<std::chrono::high_resolution_clock>;

void print_results(timepoint_t start, timepoint_t end, size_t size, size_t tok_count) {
  std::chrono::nanoseconds elapsed = end - start;
  std::cout << "Elapsed time: " << elapsed.count() << " ns" << std::endl;
  std::cout << "File size: " << size << " bytes" << std::endl;
  std::cout << "Token count: " << tok_count << std::endl;

  // double throughput = size / elapsed.count();
  // double M = 1000000;
  // std::cout << "Throughput: " << throughput / M << " MB/s" << std::endl;

  double tokens_per_sec = tok_count / (elapsed.count() / 1e9);
  std::cout << "Tokens per second: " << tokens_per_sec << std::endl;

  double mbps = size / (elapsed.count() / 1e9) / 1e6;
  std::cout << "MB/s: " << mbps << std::endl;

  double ns_per_tok = elapsed.count() / tok_count;
  std::cout << "ns per token: " << ns_per_tok << std::endl;

  double avg_tok_size = size / tok_count;
  std::cout << "Average token size: " << avg_tok_size << " bytes" << std::endl;

  double ns_per_byte = elapsed.count() / size;
  std::cout << "ns per byte: " << ns_per_byte << std::endl;
}

static void format_the_print(qlex_ty_t ty, std::string_view str) {
  switch (ty) {
    case qEofF:
      printf("EOFF()\n");
      break;
    case qErro:
      printf("ERRO()\n");
      break;
    case qKeyW:
      printf("KEYW(%.*s)\n", (int)str.size(), str.data());
      break;
    case qOper:
      printf("OPER(%.*s)\n", (int)str.size(), str.data());
      break;
    case qPunc:
      printf("PUNC(%.*s)\n", (int)str.size(), str.data());
      break;
    case qName:
      printf("NAME(%.*s)\n", (int)str.size(), str.data());
      break;
    case qIntL:
      printf("INTL(%.*s)\n", (int)str.size(), str.data());
      break;
    case qNumL:
      printf("NUML(%.*s)\n", (int)str.size(), str.data());
      break;
    case qText:
      printf("TEXT(%.*s)\n", (int)str.size(), str.data());
      break;
    case qChar:
      printf("CHAR(%.*s)\n", (int)str.size(), str.data());
      break;
    case qMacB:
      printf("MACB(%.*s)\n", (int)str.size(), str.data());
      break;
    case qMacr:
      printf("MACR(%.*s)\n", (int)str.size(), str.data());
      break;
    case qNote:
      printf("NOTE(%.*s)\n", (int)str.size(), str.data());
      break;
  }
}

static void print_token(qlex_t *lexer, qlex_tok_t *tok) {
  size_t sz = qlex_tok_size(lexer, tok);
  static char buf[1024];

  if (sz < sizeof(buf) - 1) {
    sz = qlex_tok_write(lexer, tok, buf, sizeof(buf));
    format_the_print(tok->ty, std::string_view(buf, sz));
  } else {
    char *tmp = (char *)malloc(sz + 1);
    sz = qlex_tok_write(lexer, tok, tmp, sz);
    format_the_print(tok->ty, std::string_view(tmp, sz));
    free(tmp);
  }
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
      print_token(lexer, &tok);
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