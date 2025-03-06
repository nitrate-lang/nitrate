#include <readline/history.h>
#include <readline/readline.h>

#include <filesystem>
#include <nitrate-core/CatchAll.hh>
#include <no3/Interpreter.hh>

static std::vector<std::string> SplitCommand(const std::string& command) {
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

static std::filesystem::path GetUserDirectory() {
  const char* home = getenv("HOME");
  if (home != nullptr) {
    return {home};
  }

  return std::filesystem::current_path();
}

int main(int argc, char* argv[]) {
  std::vector<std::string> args(argv, argv + argc);
  if (args.size() == 2 && args[1] == "shell") {
    using_history();

    read_history((GetUserDirectory() / ".no3_history").c_str());

    while (true) {
      char* line = readline("no3> ");
      if (line == nullptr) {
        std::cout << "Exiting..." << std::endl;
        break;
      }

      std::string input(line);
      free(line);

      if (input.empty()) {
        continue;
      }

      add_history(input.c_str());

      if (input == "exit") {
        std::cout << "Exiting..." << std::endl;
        break;
      }

      no3::Interpreter().Execute(SplitCommand(args[0] + " " + input));
    }

    write_history((GetUserDirectory() / ".no3_history").c_str());
  } else {
    return no3::Interpreter().Execute(args) ? 0 : 1;
  }
}
