
#include <errno.h>
#include <quix-lexer/Lib.h>
#include <quix-parser/Lib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char**argv) {
  FILE *source = NULL;
  qlex_t *lexer = NULL;
  qparse_conf_t *conf = NULL;
  qparse_t *parser = NULL;
  int ret = 0;

  if (argc < 2) {
    printf("Usage: %s <path>\n", argv[0]);
    return -1;
  }

  const char *path = argv[1];

  if (!qparse_lib_init()) {
    printf("Error: %s\n", qparse_strerror());

    /* Handle parser library initialization error */
    return -5;
  }

  if ((source = fopen(path, "r")) == NULL) {
    printf("Error: %s\n", strerror(errno));

    /* Handle file open error */

    ret = -1;
    goto cleanup;
  }

  if ((lexer = qlex_new(source)) == NULL) {
    printf("Error: %s\n", qlex_strerror());

    /* Handle lexer creation error */
    ret = -2;
    goto cleanup;
  }

  if ((conf = qparse_conf_new(true)) == NULL) {
    printf("Error: %s\n", qparse_strerror());

    /* Handle parser configuration creation error */
    ret = -3;
    goto cleanup;
  }

  if ((parser = qparse_new(lexer, conf)) == NULL) {
    printf("Error: %s\n", qparse_strerror());

    /* Handle parser creation error */
    ret = -4;
    goto cleanup;
  }

  /* Use parser */
  qparse_and_dump(parser, stdout, NULL, NULL);

cleanup:
  qparse_free(parser);
  qparse_conf_free(conf);
  qlex_free(lexer);

  if (source) fclose(source);

  qparse_lib_deinit();

  return ret;
}