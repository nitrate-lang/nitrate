#include <quix-core/Classes.hh>
#include <quix-prep/Classes.hh>

__attribute__((visibility("default"))) extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data,
                                                                             size_t Size) {
  if (Size == 0) {
    return 0;
  }

  qcore_fuzzing = false;

  FILE *fp = fmemopen((void *)Data, Size, "r");
  if (fp == NULL) {
    return 0;
  }

  qcore_env env;
  qprep lex(fp, nullptr, env.get());

  while (qlex_next(lex.get()).ty != qEofF);

  fclose(fp);

  return 0;
}
