#include <nitrate/code.h>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  if (Size == 0) {
    return 0;
  }

  nit_stream_t *fp = nit_from(fmemopen((void *)Data, Size, "rb"), true);
  if (fp == NULL) {
    return 0;
  }

  FILE *out = tmpfile();

  const char *options[] = {"lex", "-fuse-msgpack", NULL};

  if (!nit_cc(fp, out, nit_diag_stderr, 0, options)) {
    nit_fclose(fp);
    fclose(out);
    return 0;
  }

  nit_fclose(fp);
  fclose(out);

  return 0;
}