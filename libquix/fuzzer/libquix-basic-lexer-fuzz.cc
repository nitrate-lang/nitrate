#include <quix/code.h>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  if (Size == 0) {
    return 0;
  }

  quix_stream_t *fp = quix_from(fmemopen((void *)Data, Size, "rb"), true);
  if (fp == NULL) {
    return 0;
  }

  FILE *out = tmpfile();

  const char *options[] = {"lex", "-fuse-msgpack", NULL};

  if (!quix_cc(fp, out, quix_diag_stderr, 0, options)) {
    quix_fclose(fp);
    fclose(out);
    return 0;
  }

  quix_fclose(fp);
  fclose(out);

  return 0;
}