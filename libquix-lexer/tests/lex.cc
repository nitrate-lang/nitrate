#include <quix-lexer/Lib.h>

#include <chrono>
#include <filesystem>
#include <iostream>
#include <string_view>
#include <unordered_map>
#include <vector>

static void print_token(qlex_t *lexer, qlex_tok_t tok) {
  switch (tok.ty) {
    case qEofF:
      std::cout << "EOF()";
      break;
    case qErro:
      std::cout << "ERRO()";
      break;
    case qKeyW:
      std::cout << "KEYW(" << qlex_kwstr(tok.v.key) << ")";
      break;
    case qOper:
      std::cout << "OPER(" << qlex_opstr(tok.v.op) << ")";
      break;
    case qPunc:
      std::cout << "PUNC(" << qlex_punctstr(tok.v.punc) << ")";
      break;
    case qName:
      std::cout << "NAME(" << qlex_str(lexer, &tok) << ")";
      break;
    case qIntL:
      std::cout << "INTL(" << qlex_str(lexer, &tok) << ")";
      break;
    case qNumL:
      std::cout << "NUML(" << qlex_str(lexer, &tok) << ")";
      break;
    case qText:
      std::cout << "TEXT(" << qlex_str(lexer, &tok) << ")";
      break;
    case qChar:
      std::cout << "CHAR(" << qlex_str(lexer, &tok) << ")";
      break;
    case qMacB:
      std::cout << "MACB(" << qlex_str(lexer, &tok) << ")";
      break;
    case qMacr:
      std::cout << "MACR(" << qlex_str(lexer, &tok) << ")";
      break;
    case qNote:
      std::cout << "NOTE(" << qlex_str(lexer, &tok) << ")";
      break;
  }

  std::cout << "\n";
}

void do_lex(FILE *file) {
  qlex_t *lexer = qlex_new(file);

  qlex_tok_t tok;

  while ((tok = qlex_next(lexer)).ty != qEofF) {
    print_token(lexer, tok);
  }

  qlex_free(lexer);
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

  do_lex(file);

  fclose(file);

  qlex_lib_deinit();
}