#include <nitrate/code.hh>
#include <string>
#include <vector>

static const std::vector<std::string> CHECKS = {
    "official-styleguide",    "no-redundant-cast", "memory-safety",
    "better-data-structures", "better-algorithms",
};

static std::string GetLintOptions() {
  std::string flags;
  for (auto it = CHECKS.begin(); it != CHECKS.end(); ++it) {
    flags += "+" + *it;
    if (it + 1 != CHECKS.end()) {
      flags += ' ';
    }
  }

  return flags;
}

int main() {
  return nitrate::Pipeline(std::cin, std::cout, {"lint", GetLintOptions()})
                 .Get()
             ? 0
             : 1;
}