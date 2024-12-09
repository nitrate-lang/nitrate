#include <nitrate/code.hh>

int main() {
  auto future = nitrate::transform(stdin, stdout, {"lex"});

  return future.get() ? 0 : 1;
}