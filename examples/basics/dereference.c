#include <stdint.h>
#include <stddef.h>

const int32_t printf(const int8_t* const format, ...);


__attribute__((visibility("default"))) const void main() {
  const int32_t x = 10000000;
  const int32_t* const x_ptr = &x;
  printf("x = %d, &x = %p, x_ptr = %p, *x_ptr = %d\n", x, &x, x_ptr, *x_ptr);
  x = 20000000;
  printf("x = %d, &x = %p, x_ptr = %p, *x_ptr = %d\n", x, &x, x_ptr, *x_ptr);
  *x_ptr = 30000000;
  printf("x = %d, &x = %p, x_ptr = %p, *x_ptr = %d\n", x, &x, x_ptr, *x_ptr);
  return;
}

