#include <stdint.h>
#include <stddef.h>

extern const int32_t printf(const char* fmt, ...);


struct Address {
  char* street;
  char* city;
  char* state;
  char* postal_code;
};

struct Person {
  char* name;
  uint8_t age;
  struct Address address;
};

static const void _ZJ0f12init__person59f1v52m49p46t6Person28m25t7Address3m1x3m1x3m1x3m1x3m1x3m1b1p1p(struct Person* person) {
  person.name = "John Doe";
  person.age = 30;
  person.address.street = "123 Main St";
  person.address.city = "Springfield";
  person.address.state = "IL";
  person.address.postal_code = "62701";
  return;
}

static const void _ZJ0f13print__person56f1v49p46t6Person28m25t7Address3m1x3m1x3m1x3m1x3m1x3m1b1p1p(const struct Person* const person) {
  printf("Name: %s\n", person->name);
  printf("Age: %d\n", person->age);
  printf("Address: %s\n", person->address.street);
  printf("City: %s\n", person->address.city);
  printf("State: %s\n", person->address.state);
  printf("Postal Code: %s\n", person->address.postal_code);
  return;
}

__attribute__((visibility("default"))) const int32_t main() {
  const struct Person p;
  _ZJ0f12init__person59f1v52m49p46t6Person28m25t7Address3m1x3m1x3m1x3m1x3m1x3m1b1p1p(&p);
  _ZJ0f13print__person56f1v49p46t6Person28m25t7Address3m1x3m1x3m1x3m1x3m1x3m1b1p1p(&p);
  printf("&p: %p\n", &p);
  return;
}

