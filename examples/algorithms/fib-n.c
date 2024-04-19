#include <stdint.h>
#include <stddef.h>

extern const int32_t printf(const int8_t* const format, ...);


static const uint32_t _ZJ0f3fib7f1d1d1p1p(const uint32_t n) {
  if (n == 0 || n == 1) {
    return n;
  }
  return _ZJ0f3fib7f1d1d1p1p(n - 1) + _ZJ0f3fib7f1d1d1p1p(n - 2);
}

__attribute__((visibility("default"))) const int32_t main() {
  printf("fib(10) = %d\n", _ZJ0f3fib7f1d1d1p1p(10));
  printf("fib(20) = %d\n", _ZJ0f3fib7f1d1d1p1p(20));
  printf("fib(30) = %d\n", _ZJ0f3fib7f1d1d1p1p(30));
}

