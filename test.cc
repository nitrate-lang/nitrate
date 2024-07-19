#include <iostream>
#include <quixcc/Library.h>

int main() {
  if (!quixcc_lib_init()) {
    std::cerr << "Failed to initialize Quix library" << std::endl;
    return 1;
  }

  if (!quixcc_lib_deinit()) {
    quixcc_panic("Failed to deinitialize Quix library");
  }

  return 0;
}