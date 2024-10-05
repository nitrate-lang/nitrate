#include <quix-lexer/Lib.h>
#include <quix-parser/Lib.h>
#include <quix-prep/Lib.h>
#include <quix-qxir/Lib.h>

#include <cstdio>
#include <iostream>
#include <quix-core/Classes.hh>
#include <quix-parser/Classes.hh>
#include <quix-prep/Classes.hh>
#include <quix-qxir/Classes.hh>

#include "quix-qxir/TypeDecl.h"

extern "C" const char *__asan_default_options() { return "detect_leaks=0"; }

extern "C" int LLVMFuzzerInitialize(int *argc, char **argv) {
  (void)argc;
  (void)argv;

  qlex_lib_init();
  qprep_lib_init();
  qparse_lib_init();
  qxir_lib_init();

  std::cout << "+===========================================================+\n";
  std::cout << "Initialized libquix-lexer, libquix-prep, libquix-parser, and libquix-qxir\n";
  std::cout << "VERSIONS:\n";
  std::cout << "  - libquix-lexer: " << qlex_lib_version() << std::endl;
  std::cout << "  - libquix-prep: " << qprep_lib_version() << std::endl;
  std::cout << "  - libquix-parser: " << qparse_lib_version() << std::endl;
  std::cout << "  - libquix-qxir: " << qxir_lib_version() << std::endl;
  std::cout << "End of VERSIONS\n";
  std::cout << "+===========================================================+\n";
  return 0;
}

extern "C" int LLVMFuzzerFinalize() {
  qxir_lib_deinit();
  qparse_lib_deinit();
  qprep_lib_deinit();
  qlex_lib_deinit();

  std::cout << "Deinitialized libquix-lexer, libquix-parser, and libquix-qxir" << std::endl;
  return 0;
}

class CppFILE {
  FILE *m_fp;

public:
  CppFILE(const uint8_t *data, size_t size) {
    m_fp = fmemopen((void *)data, size, "r");
    if (m_fp == nullptr) {
      throw std::runtime_error("fmemopen failed");
    }
  }

  ~CppFILE() { fclose(m_fp); }

  FILE *get() const { return m_fp; }
};

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  if (Size == 0) {
    return 0;
  }

  try {
    CppFILE fp(Data, Size);

    qcore_env env;

    qprep lex(fp.get(), nullptr, env.get());
    qparse_conf conf;
    qparse_conf_setopt(conf.get(), QPK_CRASHGUARD, QPV_OFF);

    qparser parser(lex.get(), conf.get(), env.get());

    qparse_node_t *node = nullptr;
    if (!qparse_do(parser.get(), &node)) {
      return 0;
    }

    qxir_conf conf2;
    qxir_conf_setopt(conf2.get(), QQK_CRASHGUARD, QQV_OFF);

    qmodule mod(lex.get(), conf2.get(), nullptr);

    if (!qxir_lower(mod.get(), node, true)) {
      return 0;
    }

    return 1;
  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 0;
  }
}