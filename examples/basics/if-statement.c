#include <stdint.h>
#include <stddef.h>

__attribute__((visibility("default"))) const int8_t main() {
  if (2 == 2)
{
    if (3 == 3)
{
      return 10;
    } else
{
      return 20;
    }
    return 30;
  } else
{
    if (5 == 4)
{
      return 40;
    } else
{
      return 50;
    }
  }
  return 60;
}

