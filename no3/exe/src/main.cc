#include <readline/history.h>
#include <readline/readline.h>
#include <sys/resource.h>

#include <filesystem>
#include <nitrate-core/CatchAll.hh>
#include <no3/Interpreter.hh>

static auto SplitCommand(const std::string& command) -> std::vector<std::string> {
  std::vector<std::string> args;
  std::string arg;
  bool in_quote = false;
  for (char c : command) {
    if (c == ' ' && !in_quote) {
      if (!arg.empty()) {
        args.push_back(arg);
        arg.clear();
      }
    } else if (c == '"') {
      in_quote = !in_quote;
    } else {
      arg += c;
    }
  }
  if (!arg.empty()) {
    args.push_back(arg);
  }
  return args;
}

static auto GetUserDirectory() -> std::filesystem::path {
  const char* home = getenv("HOME");  // NOLINT(concurrency-mt-unsafe)
  if (home != nullptr) {
    return {home};
  }

  return std::filesystem::current_path();
}

static void IncreaseStackLimit() {
  constexpr rlim_t kStackSize = 32 * 1024 * 1024;  // min stack size = 32 MB
  struct rlimit rl;
  int result;

  result = getrlimit(RLIMIT_STACK, &rl);
  if (result == 0) {
    if (rl.rlim_cur < kStackSize) {
      rl.rlim_cur = kStackSize;
      result = setrlimit(RLIMIT_STACK, &rl);
      if (result != 0) {
        fprintf(stderr, "setrlimit returned result = %d\n", result);
      }
    }
  }
}

auto main(int argc, char* argv[]) -> int {
  IncreaseStackLimit();

  no3::Interpreter interpreter;

  std::vector<std::string> args(argv, argv + argc);
  if (args.size() == 2 && args[1] == "shell") {
    using_history();

    read_history((GetUserDirectory() / ".no3_history").c_str());

    while (true) {
      char* line = readline("no3> ");
      if (line == nullptr) {
        std::cout << "Exiting..." << '\n';
        break;
      }

      std::string input(line);
      free(line);

      if (input.empty()) {
        continue;
      }

      add_history(input.c_str());

      if (input == "exit") {
        std::cout << "Exiting..." << '\n';
        break;
      }

      interpreter.Execute(SplitCommand(args[0] + " " + input));
    }

    write_history((GetUserDirectory() / ".no3_history").c_str());
  } else {
    return interpreter.Execute(args) ? 0 : 1;
  }
}
