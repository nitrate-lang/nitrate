#include <stdint.h>
#include <stddef.h>

__attribute__((visibility("default"))) const int8_t main() {
  if (!true == true)
return;
  if (true)
return 10;
  if (!false)
return 20;
  return 60;
}

