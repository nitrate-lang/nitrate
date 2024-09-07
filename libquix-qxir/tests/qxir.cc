
#include <errno.h>
#include <quix-lexer/Lib.h>
#include <quix-parser/Lib.h>
#include <quix-prep/Lib.h>
#include <quix-qxir/Lib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
  FILE *source = NULL;
  qlex_t *lexer = NULL;
  qparse_conf_t *parconf = NULL;
  qparse_t *parser = NULL;
  qparse_node_t *node = NULL;
  qcore_arena_t arena;
  int ret = 0;

  if (argc < 2) {
    printf("Usage: %s <path>\n", argv[0]);
    return -1;
  }

  const char *path = argv[1];

  qcore_arena_open(&arena);

  if (!qlex_lib_init()) {
    printf("Error: %s\n", qlex_strerror());

    /* Handle lexer library initialization error */
    return -2;
  }

  if (!qprep_lib_init()) {
    printf("Error: %s\n", qprep_strerror());

    /* Handle preprocessor library initialization error */
    return -3;
  }

  if (!qparse_lib_init()) {
    printf("Error: %s\n", qparse_strerror());

    /* Handle parser library initialization error */
    return -5;
  }

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

  if ((lexer = qprep_new(source, nullptr)) == NULL) {
    printf("Error: %s\n", qprep_strerror());

    /* Handle lexer creation error */
    ret = -2;
    goto cleanup;
  }

  if ((parconf = qparse_conf_new(true)) == NULL) {
    printf("Error: %s\n", qparse_strerror());

    /* Handle parser config error */
    ret = -3;
    goto cleanup;
  }

  if ((parser = qparse_new(lexer, parconf)) == NULL) {
    printf("Error: %s\n", qparse_strerror());

    /* Handle parser creation error */
    ret = -4;
    goto cleanup;
  }

  if (!qparse_do(parser, &arena, &node)) {
    printf("Error: %s\n", qparse_strerror());

    /* Handle parsing error */
    ret = -5;
    goto cleanup;
  }

  qxir_justprint(node, stdout, QXIR_SERIAL_CODE, NULL, 0);

cleanup:
  qparse_free(parser);
  qparse_conf_free(parconf);
  qlex_free(lexer);
  if (source) fclose(source);
  qcore_arena_close(&arena);

  qxir_lib_deinit();
  qparse_lib_deinit();
  qprep_lib_deinit();
  qlex_lib_deinit();

  return ret;
}
