#include <stdint.h>
#include <stddef.h>

extern const int32_t printf(const int8_t* const format, ...);


__attribute__((visibility("default"))) const int8_t main() {
  printf("Hi. Behold, the first Hello World program for the QUIX language!\n");
  printf("This is the first of many programs to come.\n");
  printf("Long live QUIX! May it reign supreme for %d years to come!\n", 1000);
  return 0;
}

