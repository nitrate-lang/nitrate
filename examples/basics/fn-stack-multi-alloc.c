#include <stdint.h>
#include <stddef.h>

__attribute__((visibility("default"))) const int8_t main() {
  const int8_t x;
  const int8_t y;
  const int8_t z = 6;
  return x + y + z;
}

