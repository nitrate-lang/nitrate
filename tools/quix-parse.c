#include <quix/code.h>
#include <stdio.h>

int main() {
  const char *options[] = {"parse", "-fuse-msgpack", NULL};

  if (!quix_cc(stdin, stdout, quix_diag_stderr, 0, options)) {
    printf("Failed to transform input.\n");
    return 1;
  }

  quix_deinit();

  return 0;
}
