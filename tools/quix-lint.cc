#include <nitrate/code.hh>
#include <string>
#include <vector>

static std::vector<std::string> checks = {
    "+official-styleguide",    "+no-redundant-cast", "+memory-safety",
    "+better-data-structures", "+better-algorithms",
};

static std::string get_lint_options() {
  std::string flags;
  for (auto it = checks.begin(); it != checks.end(); ++it) {
    flags += "+" + *it;
    if (it + 1 != checks.end()) {
      flags += ' ';
    }
  }

  return flags;
}

int main() {
  return !nitrate::pipeline(stdin, stdout, {"lint", get_lint_options()}).get();
}