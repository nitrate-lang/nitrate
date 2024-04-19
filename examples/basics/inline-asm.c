#include <stdint.h>
#include <stddef.h>

extern const int32_t printf(const int8_t* const format, ...);


static const int32_t _ZJ0f15do__inline__asm5f1i1p1p() {
  const int32_t x;
  __asm__ __volatile__ ("mov $$42, $0":"=r"(&x)::"esi");
  return x;
}

__attribute__((visibility("default"))) const void main() {
  const int32_t x = _ZJ0f15do__inline__asm5f1i1p1p();
  printf("x = %d\n", x);
  return;
}

