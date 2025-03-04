////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///     .-----------------.    .----------------.     .----------------.     ///
///    | .--------------. |   | .--------------. |   | .--------------. |    ///
///    | | ____  _____  | |   | |     ____     | |   | |    ______    | |    ///
///    | ||_   _|_   _| | |   | |   .'    `.   | |   | |   / ____ `.  | |    ///
///    | |  |   \ | |   | |   | |  /  .--.  \  | |   | |   `'  __) |  | |    ///
///    | |  | |\ \| |   | |   | |  | |    | |  | |   | |   _  |__ '.  | |    ///
///    | | _| |_\   |_  | |   | |  \  `--'  /  | |   | |  | \____) |  | |    ///
///    | ||_____|\____| | |   | |   `.____.'   | |   | |   \______.'  | |    ///
///    | |              | |   | |              | |   | |              | |    ///
///    | '--------------' |   | '--------------' |   | '--------------' |    ///
///     '----------------'     '----------------'     '----------------'     ///
///                                                                          ///
///   * NITRATE TOOLCHAIN - The official toolchain for the Nitrate language. ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The Nitrate Toolchain is free software; you can redistribute it or     ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The Nitrate Toolcain is distributed in the hope that it will be        ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the Nitrate Toolchain; if not, see                  ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#include <core/termcolor.hh>
#include <memory>
#include <nitrate-core/Logger.hh>
#include <no3/Interpreter.hh>
#include <vector>

namespace nitrate::testing {
  bool RunTestSuite(const std::vector<std::string>& args);
}

using namespace no3;
using namespace ncc;

class Interpreter::PImpl {
  using ArgumentSlice = std::vector<std::string_view>;
  using CommandFunction = std::function<bool(std::span<const std::string_view> full_argv, ArgumentSlice argv)>;

  std::unique_ptr<detail::RCInitializationContext> m_init_rc = OpenLibrary();
  std::unordered_map<std::string_view, CommandFunction> m_commands;

  static bool CommandHelp(std::span<const std::string_view>, ArgumentSlice) {
    std::string_view message =
        R"(╭──────────────────────────────────────────────────────────────────────╮
│   .-----------------.    .----------------.     .----------------.   │
│  | .--------------. |   | .--------------. |   | .--------------. |  │
│  | | ____  _____  | |   | |     ____     | |   | |    ______    | |  │
│  | ||_   _|_   _| | |   | |   .'    `.   | |   | |   / ____ `.  | |  │
│  | |  |   \ | |   | |   | |  /  .--.  \  | |   | |   `'  __) |  | |  │
│  | |  | |\ \| |   | |   | |  | |    | |  | |   | |   _  |__ '.  | |  │
│  | | _| |_\   |_  | |   | |  \  `--'  /  | |   | |  | \____) |  | |  │
│  | ||_____|\____| | |   | |   `.____.'   | |   | |   \______.'  | |  │
│  | |              | |   | |              | |   | |              | |  │
│  | '--------------' |   | '--------------' |   | '--------------' |  │
│   '----------------'     '----------------'     '----------------'   │
│                                                                      │
│ * Nitrate toolchain - Official toolchain for Nitrate developement    │
│ * Project URL: https://github.com/Kracken256/nitrate                 │
│ * Copyright (C) 2025 Wesley Jones                                    │
├────────────┬─────────────────────────────────────────────────────────┤
│ Subcommand │ Brief description of the subcommand                     │
├────────────┼─────────────────────────────────────────────────────────┤ 
│ b, build   │ Compile a local or remote package from source           │
│            │ Get help: https://nitrate.dev/docs/no3/build            │
├────────────┼─────────────────────────────────────────────────────────┤
│ c, clean   │ Remove package artifacts and optimize build cache       │
│            │ Get help: https://nitrate.dev/docs/no3/clean            │
├────────────┼─────────────────────────────────────────────────────────┤
│ d, doc     │ Generate package documentation in various formats       │
│            │ Get help: https://nitrate.dev/docs/no3/doc              │
├────────────┼─────────────────────────────────────────────────────────┤
│ f, find    │ Search for and list available packages                  │
│            │ Get help: https://nitrate.dev/docs/no3/find             │
├────────────┼─────────────────────────────────────────────────────────┤
│ m, format, │ Apply lexical canonicalization to package contents      │
│ fmt        │ Get help: https://nitrate.dev/docs/no3/format           │
├────────────┼─────────────────────────────────────────────────────────┤
│ h, help,   │ Display this help message                               │
│ -h, --help │ Get help: https://nitrate.dev/docs/no3                  │
├────────────┼─────────────────────────────────────────────────────────┤
│ w, impl    │ Low-level toolchain commands for maintainers            │
│            │ Not documented / Subject to change                      │
├────────────┼─────────────────────────────────────────────────────────┤
│ i, init    │ Create a new package from a template                    │
│            │ Get help: https://nitrate.dev/docs/no3/init             │
├────────────┼─────────────────────────────────────────────────────────┤
│ a, install │ Install a local or remote package                       │
│            │ Get help: https://nitrate.dev/docs/no3/install          │
├────────────┼─────────────────────────────────────────────────────────┤
│ x, lsp     │ Spawn a Language Server Protocol (LSP) server           │
│            │ Get help: https://nitrate.dev/docs/no3/lsp              │
├────────────┼─────────────────────────────────────────────────────────┤
│ license    │ Print software license and legal information            │
├────────────┼─────────────────────────────────────────────────────────┤
│ r, remove  │ Remove a local package                                  │
│            │ Get help: https://nitrate.dev/docs/remove               │
├────────────┼─────────────────────────────────────────────────────────┤
│ t, test    │ Run a package's test suite                              │
│            │ Get help: https://nitrate.dev/docs/no3/test             │
├────────────┼─────────────────────────────────────────────────────────┤
│ version    │ Print software version information                      │
├────────────┼─────────────────────────────────────────────────────────┤
│ u, update  │ Update packages, dependencies, and the toolchain        │
│            │ Get help: https://nitrate.dev/docs/no3/update           │
╰────────────┴─────────────────────────────────────────────────────────╯)";

    Log << Raw << message;

    return true;
  }

  static bool CommandBuild(std::span<const std::string_view> full_argv, ArgumentSlice argv) { return true; }
  static bool CommandClean(std::span<const std::string_view> full_argv, ArgumentSlice argv) { return true; }
  static bool CommandImpl(std::span<const std::string_view> full_argv, ArgumentSlice argv) { return true; }
  static bool CommandDoc(std::span<const std::string_view> full_argv, ArgumentSlice argv) { return true; }
  static bool CommandFormat(std::span<const std::string_view> full_argv, ArgumentSlice argv) { return true; }
  static bool CommandInit(std::span<const std::string_view> full_argv, ArgumentSlice argv) { return true; }
  static bool CommandInstall(std::span<const std::string_view> full_argv, ArgumentSlice argv) { return true; }
  static bool CommandFind(std::span<const std::string_view> full_argv, ArgumentSlice argv) { return true; }
  static bool CommandRemove(std::span<const std::string_view> full_argv, ArgumentSlice argv) { return true; }
  static bool CommandLSP(std::span<const std::string_view> full_argv, ArgumentSlice argv) { return true; }
  static bool CommandLicense(std::span<const std::string_view> full_argv, ArgumentSlice argv) { return true; }
  static bool CommandTest(std::span<const std::string_view> full_argv, ArgumentSlice argv) { return true; }
  static bool CommandVersion(std::span<const std::string_view> full_argv, ArgumentSlice argv) { return true; }
  static bool CommandUpdate(std::span<const std::string_view> full_argv, ArgumentSlice argv) { return true; }

  void SetupCommands() {
    m_commands["build"] = m_commands["b"] = CommandBuild;
    m_commands["clean"] = m_commands["c"] = CommandClean;
    m_commands["doc"] = m_commands["d"] = CommandDoc;
    m_commands["find"] = m_commands["f"] = CommandFind;
    m_commands["format"] = m_commands["m"] = m_commands["fmt"] = CommandFormat;
    m_commands["help"] = m_commands["-h"] = m_commands["h"] = m_commands["--help"] = CommandHelp;
    m_commands["impl"] = m_commands["w"] = CommandImpl;
    m_commands["init"] = m_commands["i"] = CommandInit;
    m_commands["install"] = m_commands["a"] = CommandInstall;
    m_commands["lsp"] = m_commands["x"] = CommandLSP;
    m_commands["license"] = CommandLicense;
    m_commands["remove"] = m_commands["r"] = CommandRemove;
    m_commands["test"] = m_commands["t"] = CommandTest;
    m_commands["version"] = CommandVersion;
    m_commands["update"] = m_commands["u"] = CommandUpdate;
  }

public:
  PImpl() noexcept { SetupCommands(); }

  bool Perform(const std::vector<std::string_view>& command) {
    if (command.size() >= 2) {
      if (auto it = m_commands.find(command[1]); it != m_commands.end()) {
        ArgumentSlice argv(command.begin() + 2, command.end());
        return it->second(command, argv);
      }
      Log << Error << "Command not found: " << command[1];

      CommandHelp({}, {});
    } else {
      Log << Error << "No command provided.";

      CommandHelp({}, {});
    }

    return false;
  }
};

ncc::Sev GetMinimumLogLevel();

Interpreter::Interpreter(OutputHandler output_handler) noexcept : m_impl(std::make_unique<PImpl>()) {
  Log.Subscribe([&](auto msg, auto sev, const auto& ec) {
    if (sev < GetMinimumLogLevel()) {
      return;
    }

    output_handler(ec.Format(msg, sev));
    output_handler("\n");
  });
}

Interpreter::Interpreter(Interpreter&& o) noexcept : m_impl(std::move(o.m_impl)) { o.m_impl = nullptr; }

Interpreter& Interpreter::operator=(Interpreter&& o) noexcept {
  if (this != &o) {
    m_impl = std::move(o.m_impl);
    o.m_impl = nullptr;
  }
  return *this;
}

Interpreter::~Interpreter() noexcept = default;

bool Interpreter::Execute(const std::vector<std::string_view>& command) noexcept {
  if (!m_impl) {
    return false;
  }

  std::string command_concat;
  for (auto it = command.begin(); it != command.end(); ++it) {
    command_concat += "\"" + std::string(*it) + "\"";
    if (it + 1 != command.end()) {
      command_concat += ", ";
    }
  }

  Log << Debug << "Executing command: " << command_concat;

  return m_impl->Perform(command);
}
