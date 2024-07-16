#include <iostream>
#include <quixcc/Library.h>

int main() {
  if (!quixcc_lib_init()) {
    std::cerr << "Failed to initialize Quix library" << std::endl;
    return 1;
  }

  quixcc_arena_t arena;
  if (!quixcc_arena_open(&arena)) {
    std::cerr << "Failed to open arena" << std::endl;
    return 1;
  }

  quixcc_ast_node_t *node;

  for (int i = 0; i < 1; i++) {
    node = quixcc_ast_alloc(QUIXCC_AST_NODE_U1_TY, &arena);
  }

  std::cout << "Node name: " << quixcc_ast_ntype_name(quixcc_ast_typeof(node)) << std::endl;
  std::cout << "Node size: " << quixcc_ast_sizeof(node) << std::endl;
  std::cerr << "Arena memory usage: " << quixcc_arena_used(&arena) << std::endl;

  quixcc_arena_close(&arena);
  return 0;
}