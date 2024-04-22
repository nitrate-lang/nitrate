#include <stdint.h>
#include <stddef.h>

const int32_t printf(const int8_t* const format, ...);


__attribute__((visibility("default"))) const void main() {
  const int8_t x = 10;
  printf("address of x: %p\n", &x);
  return;
}

