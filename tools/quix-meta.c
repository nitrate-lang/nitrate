#include <quix/code.h>
#include <stdio.h>

int main() {
  const char *options[] = {"meta", "-fuse-msgpack", NULL};

  if (!quix_cc(stdin, stdout, quix_diag_stderr, 0, options)) {
    return 1;
  }

  quix_deinit();

  return 0;
}
