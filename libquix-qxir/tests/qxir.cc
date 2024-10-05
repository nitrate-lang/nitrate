
#include <errno.h>
#include <quix-lexer/Lib.h>
#include <quix-parser/Lib.h>
#include <quix-prep/Lib.h>
#include <quix-qxir/Lib.h>
#include <stdio.h>
#include <string.h>

#include <quix-core/Classes.hh>
#include <quix-parser/Classes.hh>
#include <quix-prep/Classes.hh>

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: %s <path>\n", argv[0]);
    return -1;
  }

  const char *path = argv[1];

  qlex_lib_init();
  qprep_lib_init();
  qparse_lib_init();
  qxir_lib_init();

  FILE *source = NULL;
  if ((source = fopen(path, "r")) == NULL) {
    printf("Error: %s\n", strerror(errno));

    return -1;
  }

  qcore_env env;
  qprep lexer(source, nullptr, env.get());

  qparse_conf parconf;
  qparser parser(lexer.get(), parconf.get(), env.get());

  qparse_node_t *node = NULL;
  if (!qparse_do(parser.get(), &node)) {
    printf("Error: %s\n", qparse_strerror());

    return -5;
  }

  qxir_justprint(node, stdout, QXIR_SERIAL_CODE, NULL, 0);

  fclose(source);

  qxir_lib_deinit();
  qparse_lib_deinit();
  qprep_lib_deinit();
  qlex_lib_deinit();

  return 0;
}
