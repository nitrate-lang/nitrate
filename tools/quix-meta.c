#include <nitrate/code.h>
#include <stdio.h>

int main() {
  const char *options[] = {"meta", "-fuse-json", NULL};

  nit_stream_t *fp = nit_from(stdin, false);
  if (!nit_cc(fp, stdout, nit_diag_stderr, 0, options)) {
    nit_fclose(fp);
    return 1;
  }

  nit_fclose(fp);

  nit_deinit();

  return 0;
}
