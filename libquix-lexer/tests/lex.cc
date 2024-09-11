#include <quix-lexer/Lib.h>

#include <filesystem>
#include <iostream>
#include <quix-core/Classes.hh>
#include <quix-lexer/Classes.hh>
#include <vector>

static void print_token(qlex_t *lexer, qlex_tok_t tok, std::ostream &out) {
  size_t len;

  switch (tok.ty) {
    case qEofF:
      out << "EOF()";
      break;
    case qErro:
      out << "ERRO()";
      break;
    case qKeyW:
      out << "KEYW(" << qlex_kwstr(tok.v.key) << ")";
      break;
    case qOper:
      out << "OPER(" << qlex_opstr(tok.v.op) << ")";
      break;
    case qPunc:
      out << "PUNC(" << qlex_punctstr(tok.v.punc) << ")";
      break;
    case qName:
      out << "NAME(" << std::string_view(qlex_str(lexer, &tok, &len), len) << ")";
      break;
    case qIntL:
      out << "INTL(" << std::string_view(qlex_str(lexer, &tok, &len), len) << ")";
      break;
    case qNumL:
      out << "NUML(" << std::string_view(qlex_str(lexer, &tok, &len), len) << ")";
      break;
    case qText:
      out << "TEXT(" << std::string_view(qlex_str(lexer, &tok, &len), len) << ")";
      break;
    case qChar:
      out << "CHAR(" << std::string_view(qlex_str(lexer, &tok, &len), len) << ")";
      break;
    case qMacB:
      out << "MACB(" << std::string_view(qlex_str(lexer, &tok, &len), len) << ")";
      break;
    case qMacr:
      out << "MACR(" << std::string_view(qlex_str(lexer, &tok, &len), len) << ")";
      break;
    case qNote:
      out << "NOTE(" << std::string_view(qlex_str(lexer, &tok, &len), len) << ")";
      break;

    default:
      out << "UNKNOWN(" << tok.ty << ")";
      break;
  }

  out << "\n";
}

void do_lex(FILE *file) {
  qcore_env env;
  qlex lexer(file, nullptr, env.get());

  qlex_tok_t tok;

  while (true) {
    tok = qlex_next(lexer.get());
    if (tok.ty == qEofF) {
      break;
    }
    print_token(lexer.get(), tok, std::cout);
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

  FILE *file = fopen(args[1].data(), "r");
  if (!file) {
    std::cerr << "Failed to open file: " << args[1] << std::endl;
    return 1;
  }

  do_lex(file);

  fclose(file);

  qlex_lib_deinit();
}