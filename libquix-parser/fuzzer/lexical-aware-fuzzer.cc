
#define __QUIX_IMPL__

#include <quix-parser/Lib.h>
#include <quix-parser/Parser.h>
#include <stdlib.h>

#include <cstdint>
#include <iostream>
#include <quix-core/Classes.hh>
#include <quix-lexer/Base.hh>
#include <quix-parser/Classes.hh>
#include <random>
#include <string_view>

#define FEEDBACK_INTERVAL 1
#define BYTESTR_SIZE 4
#define ASCII_SIZE 4

static std::mt19937 g_gen;

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

static std::string_view random_ascii_stringid() {
  static char buf[ASCII_SIZE];
  for (size_t i = 0; i < sizeof(buf); i++) {
    buf[i] = static_cast<char>(std::uniform_int_distribution<unsigned>(0x41, 0x5A)(g_gen));
  }

  return std::string_view(buf, sizeof(buf));
}

struct random_token_generator : public qlex_t {
  size_t count;

  virtual qlex_tok_t next_impl() override {
    qlex_tok_t tok{};

    if (count > 256) {
      tok.ty = qEofF;
      return tok;
    }

  begin_gen:
    tok.ty = static_cast<qlex_ty_t>(std::uniform_int_distribution<unsigned>(qEofF, qNote)(g_gen));

    switch (tok.ty) {
      case qEofF: {
        break;
      }

      case qErro: {
        goto begin_gen;
        break;
      }

      case qKeyW: {
        tok.v.key = static_cast<qlex_key_t>(
            std::uniform_int_distribution<unsigned>(qKSubsystem, qKFalse)(g_gen));
        break;
      }

      case qOper: {
        tok.v.op = static_cast<qlex_op_t>(
            std::uniform_int_distribution<unsigned>(qOpTernary, qOpOut)(g_gen));
        break;
      }

      case qPunc: {
        tok.v.punc = static_cast<qlex_punc_t>(
            std::uniform_int_distribution<unsigned>(qPuncLPar, qPuncSemi)(g_gen));
        break;
      }

      case qName: {
        tok.v.str_idx = put_string(random_ascii_stringid());
        break;
      }

      case qIntL: {
        uint64_t num = std::uniform_int_distribution<uint64_t>(
            std::numeric_limits<uint64_t>::min(), std::numeric_limits<uint64_t>::max())(g_gen);
        tok.v.str_idx = put_string(std::to_string(num));
        break;
      }

      case qNumL: {
        double num = std::uniform_real_distribution<double>(
            std::numeric_limits<double>::min(), std::numeric_limits<double>::max())(g_gen);
        tok.v.str_idx = put_string(std::to_string(num));
        break;
      }

      case qText: {
        tok.v.str_idx = put_string(random_ascii_stringid());
        break;
      }

      case qChar: {
        tok.v.str_idx = put_string(std::to_string(static_cast<char32_t>(
            std::uniform_int_distribution<unsigned>(std::numeric_limits<char32_t>::min(),
                                                    std::numeric_limits<char32_t>::max())(g_gen))));
        break;
      }

      case qMacB: {
        goto begin_gen;
        break;
      }

      case qMacr: {
        goto begin_gen;
        break;
      }

      case qNote: {
        tok.v.str_idx = put_string(random_ascii_stringid());
        break;
      }
    }

    count++;

    return tok;
  }

public:
  random_token_generator(uint64_t seed, qcore_env_t env) : qlex_t(nullptr, nullptr, false, env) {
    g_gen.seed(seed);
    count = 0;
  }
  virtual ~random_token_generator() {}
};

void fuzz_round(uint64_t seed) {
  qcore_env env;
  qlex_t *lex = new random_token_generator(seed, env.get());

  qparse_conf conf;
  qparser parse(lex, conf.get(), env.get());
  qcore_arena arena;
  qparse_node_t *node = nullptr;

  qparse_do(parse.get(), arena.get(), &node);
  qlex_free(lex);
}

int main(int argc, char **argv) {
  if (argc == 3 && strcmp(argv[1], "--dump") == 0) {
    uint64_t seed;
    sscanf(argv[2], "0x%016lx", &seed);

    qcore_env env;
    qlex_t *lex = new random_token_generator(seed, env.get());

    qlex_tok_t tok;
    while ((tok = lex->next()).ty != qEofF) {
      print_token(lex, tok, std::cout);
    }

    qlex_free(lex);

    return 0;
  } else if (argc != 2) {
    printf("Usage: %s <uint64_t seed>\n", argv[0]);
    return 1;
  }

  qcore_fuzzing = true;

  uint64_t seed;
  sscanf(argv[1], "0x%016lx", &seed);

  while (true) {
    fuzz_round(seed);

    if ((seed % FEEDBACK_INTERVAL) == 0) {
      printf("\rLast: 0x%016lx, Next: 0x%016lx ", seed, seed + 1);
      fflush(stdout);
    }

    seed++;
  }
}