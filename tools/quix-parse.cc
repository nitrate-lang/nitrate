#include <nitrate/code.hh>

int main() {
  auto future = nitrate::pipeline(stdin, stdout, {"parse"});

  return future.get() ? 0 : 1;
}