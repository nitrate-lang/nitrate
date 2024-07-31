#include <quix-lexer/Lib.h>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  FILE *fp = fmemopen((void *)Data, Size, "r");
  if (fp == NULL) {
    return 0;
  }

  qlex_t *lex = qlex_new(fp);

  while (qlex_next(lex).ty != qEofF);

  qlex_free(lex);

  fclose(fp);
  return 0;
}