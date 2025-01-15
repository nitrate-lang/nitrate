#include <nitrate/code.hh>

int main() {
  auto future = nitrate::Pipeline(stdin, stdout, {"seq"});

  return future.get() ? 0 : 1;
}