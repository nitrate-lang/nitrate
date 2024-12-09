#include <nitrate/code.h>
#include <stdio.h>

int main() {
  const char *options[] = {"lex", "-fuse-msgpack", NULL};

  nit_stream_t *in = nit_from(stdin, false);
  nit_stream_t *out = nit_from(stdout, false);
  bool ok = nit_cc(in, out, nit_diag_stderr, 0, options);

  nit_fclose(in);
  nit_fclose(out);

  return ok ? 0 : 1;
}
