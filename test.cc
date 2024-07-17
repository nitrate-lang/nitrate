#include <iostream>
#include <quixcc/Library.h>

extern "C" quixcc_arena_t quixcc_ast_arena;

int main() {
  if (!quixcc_lib_init()) {
    std::cerr << "Failed to initialize Quix library" << std::endl;
    return 1;
  }

  quixcc_ast_node_t *node = quixcc_ast_alloc(QUIXCC_AST_NODE_FN, nullptr);

  size_t outlen = 0;
  uint8_t *buf = nullptr;
  quixcc_ast_brepr(node, true, nullptr, &buf, &outlen);

  char *serialized = (char *)quixcc_arena_alloc(&quixcc_ast_arena, outlen * 2 + 1);
  for (size_t i = 0; i < outlen; i++) {
    sprintf(serialized + i * 2, "%02x", buf[i]);
  }

  std::cout << "Serialized: " << serialized << std::endl;

  if (!quixcc_lib_deinit()) {
    quixcc_panic("Failed to deinitialize Quix library");
  }

  return 0;
}