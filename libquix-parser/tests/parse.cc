
#include <errno.h>
#include <quix-lexer/Lib.h>
#include <quix-parser/Lib.h>
#include <quix-prep/Lib.h>
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

  FILE *source = NULL;
  if ((source = fopen(path, "rb")) == NULL) {
    printf("Error: %s\n", strerror(errno));
    return -1;
  }

  qprep_lib_init();
  qparse_lib_init();

  qcore_env env;
  qprep lexer(source, nullptr, env.get());

  qparse_conf conf;
  qparser parser(lexer.get(), conf.get(), env.get());

  qparse_and_dump(parser.get(), stdout, NULL, NULL);

  fclose(source);

  return 0;
}