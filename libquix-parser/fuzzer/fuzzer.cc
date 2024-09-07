#include <quix-prep/Lib.h>
#include <quix-parser/Lib.h>

#include <stdexcept>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  if (Size == 0) {
    return 0;
  }

  FILE *fp = fmemopen((void *)Data, Size, "r");
  if (fp == NULL) {
    throw std::runtime_error("fmemopen failed");
  }

  qlex_t *lex = qprep_new(fp, nullptr);
  if (!lex) {
    fclose(fp);
    throw std::runtime_error("qlex_new failed");
  }

  qparse_conf_t *conf = qparse_conf_new(true);
  if (!conf) {
    qlex_free(lex);
    fclose(fp);
    throw std::runtime_error("qparse_conf_new failed");
  }

  qparse_t *parse = qparse_new(lex, conf);
  if (!parse) {
    qparse_conf_free(conf);
    qlex_free(lex);
    fclose(fp);
    throw std::runtime_error("qparse_new failed");
  }

  qcore_arena_t arena;
  qcore_arena_open(&arena);
  qparse_node_t *node = nullptr;

  qparse_do(parse, &arena, &node);

  qcore_arena_close(&arena);

  qparse_free(parse);
  qparse_conf_free(conf);
  qlex_free(lex);
  fclose(fp);

  return 0;
}