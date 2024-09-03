#include <istream>
#include <iostream>

int main() {
  std::istream *s  = &std::cin;

  s->sync();
}