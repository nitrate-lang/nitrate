#include <stdint.h>
#include <stddef.h>

__attribute__((visibility("default"))) const uint8_t main() {
  const uint8_t x = 10;
  const uint8_t y = 20;
  const uint8_t z = x + y;
  return z;
}

