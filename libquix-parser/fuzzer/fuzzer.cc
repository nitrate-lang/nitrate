#include <quix-parser/Lib.h>
#include <quix-prep/Lib.h>

#include <quix-core/Classes.hh>
#include <quix-parser/Classes.hh>
#include <quix-prep/Classes.hh>
#include <stdexcept>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  if (Size == 0) {
    return 0;
  }

  FILE *fp = fmemopen((void *)Data, Size, "r");
  if (fp == NULL) {
    throw std::runtime_error("fmemopen failed");
  }

  qcore_env env;

  qprep lex(fp, nullptr, env.get());
  qparse_conf conf;
  qparser parse(lex.get(), conf.get(), env.get());

  qcore_arena arena;
  qparse_node_t *node = nullptr;

  qparse_do(parse.get(), arena.get(), &node);

  fclose(fp);

  return 0;
}