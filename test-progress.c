
#include <errno.h>
#include <quix-lexer/Lib.h>
#include <quix-parser/Lib.h>
#include <stdio.h>
#include <string.h>

void dump_callback(const char *msg, size_t len, uintptr_t userdata) {
  printf("%.*s\n", (int)len, msg);
}

void do_parse(qparse_t *parser) {
  qcore_arena_t arena;
  qcore_arena_open(&arena);

  qparse_node_t *node = NULL;
  bool ok = qparse_do(parser, &arena, &node);

  if (!ok) {
    printf("Error: %s\n", qparse_strerror());
    qcore_arena_close(&arena);

    qparse_dumps(parser, false, dump_callback, 0);
    return;
  }

  size_t outlen;
  char *serial = qparse_ast_repr(node, false, 2, &arena, &outlen);

  fwrite(serial, 1, outlen, stdout);
  qcore_arena_close(&arena);
}

int example_parser(const char *path) {
  FILE *source = NULL;
  qlex_t *lexer = NULL;
  qparse_conf_t *conf = NULL;
  qparse_t *parser = NULL;
  int ret = 0;

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

  if ((lexer = qlex_new(source, path)) == NULL) {
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

  do_parse(parser);

cleanup:
  qparse_free(parser);
  qparse_conf_free(conf);
  qlex_free(lexer);

  if (source) fclose(source);

  qparse_lib_deinit();

  return ret;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: %s <path>\n", argv[0]);
    return 1;
  }

  example_parser(argv[1]);

  return 0;
}