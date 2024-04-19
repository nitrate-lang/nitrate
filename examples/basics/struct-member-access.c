#include <stdint.h>
#include <stddef.h>

struct PersonInfo {
  char* name;
  int32_t age;
};

extern const int32_t printf(const uint8_t* const format, ...);
extern const int32_t scanf(const uint8_t* const format, ...);
extern const uint8_t* const gets(const uint8_t* const buffer);
extern const uint8_t* const malloc(const uint64_t size);
extern const void free(const uint8_t* const ptr);


static const struct PersonInfo _ZJ0f17get__person__info26f21t10PersonInfo3m1x3m1i1p1p() {
  const struct PersonInfo info;
  info.name = malloc(18446744073709551515 ^ 18446744073709551615);
  printf("Enter your name: ");
  gets(info.name);
  printf("Enter your age: ");
  scanf("%d", &info.age);
  return info;
}

__attribute__((visibility("default"))) const void main() {
  const struct PersonInfo res;
  res = _ZJ0f17get__person__info26f21t10PersonInfo3m1x3m1i1p1p();
  printf("Name: %s\n", res.name);
  printf("Age: %d\n", res.age);
  free(res.name);
  return;
}

