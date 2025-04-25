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

#include <core/cli/Interpreter.hh>
#include <core/cli/termcolor.hh>
#include <memory>
#include <nitrate-core/Logger.hh>
#include <no3/Interpreter.hh>
#include <vector>

using namespace no3;
using namespace ncc;

auto Interpreter::PImpl::CommandHelp(ConstArguments, const MutArguments&) -> bool {
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
│ n, init    │ Create a new package from a template                    │
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
╰────────────┴─────────────────────────────────────────────────────────╯
)";

  Log << Raw << message;

  return true;
}

auto Interpreter::PImpl::CommandLicense(ConstArguments, const MutArguments& argv) -> bool {
  if (argv.size() != 1) {
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
<http://www.gnu.org/licenses/>.
)";

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
  m_commands["init"] = m_commands["n"] = CommandInit;
  m_commands["install"] = m_commands["i"] = CommandInstall;
  m_commands["lsp"] = m_commands["x"] = CommandLSP;
  m_commands["license"] = CommandLicense;
  m_commands["remove"] = m_commands["r"] = CommandRemove;
  m_commands["test"] = m_commands["t"] = CommandTest;
  m_commands["version"] = m_commands["--version"] = CommandVersion;
  m_commands["update"] = m_commands["u"] = CommandUpdate;
}

auto Interpreter::PImpl::Perform(const std::vector<std::string>& command) -> bool {
  if (command.size() >= 2) {
    if (auto it = m_commands.find(command[1]); it != m_commands.end()) {
      auto ok = it->second(command, MutArguments(command.begin() + 1, command.end()));

      return ok;
    }
    Log << "command not found: \"" << command[1] << "\". run \"" << command[0] << " help\" for a list of commands.";
  } else if (command.size() == 1) {
    Log << "no command provided. run \"" << command[0] << " help\" for a list of commands.";
  } else {
    Log << "no command provided. use \"help\" for a list of commands.";
  }

  return false;
}

auto GetMinimumLogLevel() -> ncc::Sev;

NCC_EXPORT Interpreter::Interpreter(OutputHandler output_handler) noexcept {
  // We need to suspend all log subscribers to prevent external loggers from
  // interfering with the interpreter's output collection.

  // Collect all suspended log subscriber IDs so we can resume them when
  // the interpreter is destroyed.
  std::vector<LogSubscriberID> log_suspend_ids;
  for (const auto& sub : Log->SubscribersList()) {
    if (sub.IsSuspended()) {
      log_suspend_ids.push_back(sub.ID());
    }
  }

  Log->SuspendAll();

  // All writes to this thread's log stream will be redirected to the output
  // handler. The implication is any code outside the interpreter
  // (running on the same thread) might garble the interpreter's output.

  // We are permitted to use the ncc::Log global logger even prior to
  // core library initialization.

  // We attach the subscriber to the global logger, prior to initializing
  // to ensure that initialization messages are captured in the interpreter's
  // output.
  const auto log_sub_id = Log->Subscribe([&](const LogMessage& m) {
    if (m.m_sev < GetMinimumLogLevel()) {
      return;
    }

    output_handler(m.m_by.Format(m.m_message, m.m_sev));
    output_handler("\n");
  });

  // The PImpl constructor will automatically initialize all required
  // runtime libraries.
  m_impl = std::make_unique<PImpl>();

  m_impl->m_log_sub_id = log_sub_id;
  m_impl->m_log_suspend_ids = std::move(log_suspend_ids);
}

NCC_EXPORT Interpreter::~Interpreter() noexcept {
  if (m_impl) {
    auto sub_id = m_impl->m_log_sub_id;
    auto suspend_ids = std::move(m_impl->m_log_suspend_ids);

    // This will destroy the interpreter's runtime environment
    // and decrement the reference count of libraries opened
    // during construction, thereby potentially deinitalizing
    // them.
    m_impl.reset();

    Log->Unsubscribe(sub_id);

    // Resume all log subscribers that were active before the interpreter
    // was created. If any if these subscriptions were removed externally
    // Log->Resume will simply ignore them.
    for (auto id : suspend_ids) {
      Log->Resume(id);
    }
  }
}

NCC_EXPORT auto Interpreter::Execute(const std::vector<std::string>& command) noexcept -> bool {
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
