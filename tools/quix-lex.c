#include <nitrate/code.h>
#include <stdio.h>

int main() {
  const char *options[] = {"lex", "-fuse-msgpack", NULL};

  quix_stream_t *fp = quix_from(stdin, false);
  if (!quix_cc(fp, stdout, quix_diag_stderr, 0, options)) {
    quix_fclose(fp);
    return 1;
  }

  quix_fclose(fp);

  quix_deinit();

  return 0;
}
