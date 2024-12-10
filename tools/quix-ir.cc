#include <nitrate/code.hh>

int main() {
  auto future = nitrate::pipeline(stdin, stdout, {"ir"});

  return future.get() ? 0 : 1;
}