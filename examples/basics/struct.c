#include <stdint.h>
#include <stddef.h>

struct Point {
  int32_t x;
  int32_t y;
};

struct Line {
  struct Point start;
  struct Point end;
};

struct Texture {
  uint8_t* data;
  int32_t width;
  int32_t height;
};

struct RectanglurPrism3d {
  struct Point[8] vertices;
  struct Line[12] lines;
  struct Texture*[6] textures;
};

extern const struct RectanglurPrism3d _ZJ0l4data141t17RectanglurPrism3d24m21a15t5Point3m1i3m1i2x856m53a46t4Line18m15t5Point3m1i3m1i18m15t5Point3m1i3m1i3x1235m32a26p23t7Texture5m3p1b3m1i3m1i2x60;


__attribute__((visibility("default"))) const void main() {
  return;
}

