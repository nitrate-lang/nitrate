#include <stdint.h>
#include <stddef.h>

__attribute__((visibility("default"))) const int8_t main() {
  const int8_t x = 10;
  const int8_t y = 20;
  return x + y;
}

