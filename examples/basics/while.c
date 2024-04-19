#include <stdint.h>
#include <stddef.h>

__attribute__((visibility("default"))) const int8_t main() {
  const int8_t x = 20;
  while (1 == 1) {
    if (x == 21) {
      return 20;
    }
  }
  return 60;
}

