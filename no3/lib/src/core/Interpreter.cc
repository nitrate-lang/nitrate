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

#include <core/InterpreterImpl.hh>
#include <core/termcolor.hh>
#include <memory>
#include <nitrate-core/Logger.hh>
#include <no3/Interpreter.hh>
#include <vector>

using namespace no3;
using namespace ncc;

bool Interpreter::PImpl::CommandHelp(ConstArguments, const MutArguments&) {
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
│ p, init    │ Create a new package from a template                    │
│            │ Get help: https://nitrate.dev/docs/no3/init             │
├────────────┼─────────────────────────────────────────────────────────┤
│ i, install │ Install a local or remote package                       │
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
│ --version  │ Get help: https://nitrate.dev/docs/no3/version          │
├────────────┼─────────────────────────────────────────────────────────┤
│ u, update  │ Update packages, dependencies, and the toolchain        │
│            │ Get help: https://nitrate.dev/docs/no3/update           │
╰────────────┴─────────────────────────────────────────────────────────╯)";

  Log << Raw << message;

  return true;
}

bool Interpreter::PImpl::CommandLSP(ConstArguments, const MutArguments& argv) {
  (void)argv;

  Log << "Not implemented";
  return false;
}

bool Interpreter::PImpl::CommandLicense(ConstArguments, const MutArguments& argv) {
  if (!argv.empty()) {
    Log << "Command 'license' does not take any arguments.";
    return false;
  }

  Log << Raw << R"(Nitrate Compiler Suite
Copyright (C) 2024 Wesley C. Jones

This software is free to use, modify, and share under the terms 
of the GNU Lesser General Public License version 2.1 or later.

It comes with no guarantees — it might work great, or not at all. 
There's no warranty for how well it works or whether it fits any 
particular purpose.

For full license details, see the included license file or visit 
<http://www.gnu.org/licenses/>.)";

  return true;
}

void Interpreter::PImpl::SetupCommands() {
  m_commands["build"] = m_commands["b"] = CommandBuild;
  m_commands["clean"] = m_commands["c"] = CommandClean;
  m_commands["doc"] = m_commands["d"] = CommandDoc;
  m_commands["find"] = m_commands["f"] = CommandFind;
  m_commands["format"] = m_commands["m"] = m_commands["fmt"] = CommandFormat;
  m_commands["help"] = m_commands["-h"] = m_commands["h"] = m_commands["--help"] = CommandHelp;
  m_commands["impl"] = m_commands["w"] = CommandImpl;
  m_commands["init"] = m_commands["p"] = CommandInit;
  m_commands["install"] = m_commands["i"] = CommandInstall;
  m_commands["lsp"] = m_commands["x"] = CommandLSP;
  m_commands["license"] = CommandLicense;
  m_commands["remove"] = m_commands["r"] = CommandRemove;
  m_commands["test"] = m_commands["t"] = CommandTest;
  m_commands["version"] = m_commands["--version"] = CommandVersion;
  m_commands["update"] = m_commands["u"] = CommandUpdate;
}

bool Interpreter::PImpl::Perform(const std::vector<std::string>& command) {
  if (command.size() >= 2) {
    if (auto it = m_commands.find(command[1]); it != m_commands.end()) {
      return it->second(command, MutArguments(command.begin() + 1, command.end()));
    }
    Log << Error << "Command not found: " << command[1];

    CommandHelp({}, {});
  } else {
    Log << Error << "No command provided.";

    CommandHelp({}, {});
  }

  return false;
}

ncc::Sev GetMinimumLogLevel();

NCC_EXPORT Interpreter::Interpreter(OutputHandler output_handler) noexcept : m_impl(std::make_unique<PImpl>()) {
  Log.Subscribe([&](auto msg, auto sev, const auto& ec) {
    if (sev < GetMinimumLogLevel()) {
      return;
    }

    output_handler(ec.Format(msg, sev));
    output_handler("\n");
  });
}

NCC_EXPORT Interpreter::Interpreter(Interpreter&& o) noexcept : m_impl(std::move(o.m_impl)) { o.m_impl = nullptr; }

NCC_EXPORT Interpreter& Interpreter::operator=(Interpreter&& o) noexcept {
  if (this != &o) {
    m_impl = std::move(o.m_impl);
    o.m_impl = nullptr;
  }
  return *this;
}

NCC_EXPORT Interpreter::~Interpreter() noexcept = default;

NCC_EXPORT bool Interpreter::Execute(const std::vector<std::string>& command) noexcept {
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
