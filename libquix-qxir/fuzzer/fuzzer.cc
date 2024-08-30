#include <quix-lexer/Lib.h>
#include <quix-parser/Lib.h>
#include <quix-qxir/Lib.h>

#include <iostream>
#include <optional>
#include <stdexcept>

extern "C" const char *__asan_default_options() { return "detect_leaks=0"; }

extern "C" int LLVMFuzzerInitialize(int *argc, char **argv) {
  (void)argc;
  (void)argv;

  qlex_lib_init();
  qparse_lib_init();
  qxir_lib_init();

  std::cout << "+===========================================================+\n";
  std::cout << "Initialized libquix-lexer, libquix-parser, and libquix-qxir\n";
  std::cout << "VERSIONS:\n";
  std::cout << "  - libquix-lexer: " << qlex_lib_version() << std::endl;
  std::cout << "  - libquix-parser: " << qparse_lib_version() << std::endl;
  std::cout << "  - libquix-qxir: " << qxir_lib_version() << std::endl;
  std::cout << "End of VERSIONS\n";
  std::cout << "+===========================================================+\n";
  return 0;
}

extern "C" int LLVMFuzzerFinalize() {
  qxir_lib_deinit();
  qparse_lib_deinit();
  qlex_lib_deinit();

  std::cout << "Deinitialized libquix-lexer, libquix-parser, and libquix-qxir" << std::endl;
  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  qparse_node_t *node = nullptr;
  FILE *fp = nullptr;
  qlex_t *lex = nullptr;
  std::optional<qcore_arena_t> arena;
  qparse_conf_t *conf = nullptr;
  qparse_t *parse = nullptr;
  qxir_conf_t *conf2 = nullptr;
  qmodule_t *mod = nullptr;
  bool success = false;

  if (Size == 0) {
    return 0;
  }

  if ((fp = fmemopen((void *)Data, Size, "r")) == nullptr) {
    goto cleanup;
  }

  if ((lex = qlex_new(fp, nullptr)) == nullptr) {
    goto cleanup;
  }

  if ((conf = qparse_conf_new(true)) == nullptr) {
    goto cleanup;
  }

  qparse_conf_setopt(conf, QPK_CRASHGUARD, QPV_OFF);

  if ((parse = qparse_new(lex, conf)) == nullptr) {
    goto cleanup;
  }

  arena = qcore_arena_t();
  qcore_arena_open(&*arena);

  if (!qparse_do(parse, &*arena, &node)) {
    goto cleanup;
  }

  if ((conf2 = qxir_conf_new(true)) == nullptr) {
    goto cleanup;
  }

  qxir_conf_setopt(conf2, QQK_CRASHGUARD, QQV_OFF);

  if ((mod = qxir_new(lex, conf2)) == nullptr) {
    goto cleanup;
  }

  if (!qxir_lower(mod, node, true)) {
    goto cleanup;
  }

  success = true;

cleanup:
  qxir_free(mod);
  qxir_conf_free(conf2);
  qparse_free(parse);
  qparse_conf_free(conf);
  if (arena) {
    qcore_arena_close(&*arena);
  }
  qlex_free(lex);
  if (fp) {
    fclose(fp);
  }
  node = nullptr;

  return success ? 1 : 0;
}