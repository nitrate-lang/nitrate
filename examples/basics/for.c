#include <stdint.h>
#include <stddef.h>

extern const int32_t printf(const int8_t* const format, ...);


__attribute__((visibility("default"))) const int8_t main() {
  int8_t x;
  {
    x = 0;
    while (x < 100) {
      printf("Hello, World! i=%d\n", x);
      x += 1;
    }
  }
  return 60;
}

