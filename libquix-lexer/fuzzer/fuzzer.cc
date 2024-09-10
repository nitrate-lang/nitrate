#include <quix-lexer/Lib.h>

#include <quix-core/Classes.hh>
#include <quix-lexer/Classes.hh>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  FILE *fp = fmemopen((void *)Data, Size, "r");
  if (fp == NULL) {
    return 0;
  }

  qcore_env env;
  qlex lex(fp, nullptr, env.get());

  while (qlex_next(lex.get()).ty != qEofF);

  fclose(fp);

  return 0;
}