#include <stdint.h>
#include <stddef.h>

extern const int32_t printf(const int8_t* const fmt, ...);


struct Address {
  const char* street;
  const char* city;
  const char* state;
  const char* postal_code;
};

struct Person {
  const char* name;
  const uint8_t age;
  struct Address address;
};

static const void _ZJ0f13print__person44f1v37p34t6Person20m17t7Address1x1x1x1x1x1b1p1p(const struct Person* const person) {
  printf("Name: %s\n", person->name);
  printf("Age: %d\n", person->age);
  printf("Address: %s\n", person->address.street);
  printf("City: %s\n", person->address.city);
  printf("State: %s\n", person->address.state);
  printf("Postal Code: %s\n", person->address.postal_code);
  return;
}

__attribute__((visibility("default"))) const void main() {
  const struct Person p;
  _ZJ0f13print__person44f1v37p34t6Person20m17t7Address1x1x1x1x1x1b1p1p(&p);
  printf("&p: %p\n", &p);
  return;
}

