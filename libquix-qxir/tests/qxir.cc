
#include <errno.h>
#include <quix-lexer/Lib.h>
#include <quix-qxir/Lib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
  FILE *source = NULL;
  qlex_t *lexer = NULL;
  qxir_conf_t *conf = NULL;
  qxir_t *qxir = NULL;
  int ret = 0;

  if (argc < 2) {
    printf("Usage: %s <path>\n", argv[0]);
    return -1;
  }

  const char *path = argv[1];

  if (!qxir_lib_init()) {
    printf("Error: %s\n", qxir_strerror());

    /* Handle qxir library initialization error */
    return -5;
  }

  if ((source = fopen(path, "r")) == NULL) {
    printf("Error: %s\n", strerror(errno));

    /* Handle file open error */

    ret = -1;
    goto cleanup;
  }

  if ((lexer = qlex_new(source, nullptr)) == NULL) {
    printf("Error: %s\n", qlex_strerror());

    /* Handle lexer creation error */
    ret = -2;
    goto cleanup;
  }

  if ((conf = qxir_conf_new(true)) == NULL) {
    printf("Error: %s\n", qxir_strerror());

    /* Handle qxir configuration creation error */
    ret = -3;
    goto cleanup;
  }

  // if ((qxir = qxir_new(lexer, conf)) == NULL) {
  //   printf("Error: %s\n", qxir_strerror());

  //   /* Handle qxir creation error */
  //   ret = -4;
  //   goto cleanup;
  // }

  /* Use qxir */
  qxir_and_dump(qxir, stdout, NULL, NULL);

cleanup:
  qxir_free(qxir);
  qxir_conf_free(conf);
  qlex_free(lexer);

  if (source) fclose(source);

  qxir_lib_deinit();

  return ret;
}