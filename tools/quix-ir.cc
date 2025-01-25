#include <nitrate/code.hh>

int main() {
  auto future = nitrate::Pipeline(std::cin, std::cout, {"ir"});

  return future.Get() ? 0 : 1;
}