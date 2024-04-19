#include <stdint.h>
#include <stddef.h>

static const int8_t _ZJ0f11CallMeFirst7f1c1c1p1p(const int8_t a) {
  return a + 10;
}

static const int8_t _ZJ0f12CallMeSecond7f1c1c1p1p(const int8_t b) {
  return b * 20;
}

__attribute__((visibility("default"))) const int8_t main(const int32_t argc) {
  return 60 + _ZJ0f12CallMeSecond7f1c1c1p1p(_ZJ0f11CallMeFirst7f1c1c1p1p(10));
}

