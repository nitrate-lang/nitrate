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
#include <impl/Subcommands.hh>

using namespace ncc;
using namespace no3;

static const auto IMPL_SUBCOMMANDS = []() {
  std::unordered_map<std::string_view, CommandFunction> m;

  m["help"] = m["--help"] = m["-h"] = cmd_impl::subcommands::CommandImplHelp;
  m["config-check"] = cmd_impl::subcommands::CommandImplConfigParse;
  m["self-test"] = cmd_impl::subcommands::CommandImplSelfTest;
  m["parse"] = cmd_impl::subcommands::CommandImplParse;

  return m;
}();

auto no3::cmd_impl::subcommands::CommandImplHelp(ConstArguments, const MutArguments&) -> bool {
  std::string_view message =
      R"(╭───────────────┬──────────────────────────────────────────────────────────────╮
│ Subcommand    │ Brief description of the subcommand                          │
├───────────────┼──────────────────────────────────────────────────────────────┤
│ help, --help  │ Display this help message                                    │
│ -h            │ Get help: https://nitrate.dev/docs/no3/impl                  │
├───────────────┼──────────────────────────────────────────────────────────────┤
│ config-check  │ Package manifest file parsing and validation                 │
│               │ Get help: https://nitrate.dev/docs/no3/impl/config-check     │
├───────────────┼──────────────────────────────────────────────────────────────┤
│ self-test     │ Run internal test suite                                      │
│               │ Get help: https://nitrate.dev/docs/no3/impl/self-test        │
├───────────────┼──────────────────────────────────────────────────────────────┤
│ parse         │ Parse a source file into a parse tree                        │
│               │ Get help: https://nitrate.dev/docs/no3/impl/parse            │
╰───────────────┴──────────────────────────────────────────────────────────────╯)";

  Log << Raw << message << "\n";

  return true;
}

auto no3::Interpreter::PImpl::CommandImpl(ConstArguments full_argv, MutArguments argv) -> bool {
  using namespace no3::cmd_impl::subcommands;

  if (argv.size() < 2) {
    Log << "missing subcommand. run \"" << full_argv[0] << " impl help\" for a list of subcommands.";
    return false;
  }

  auto it = IMPL_SUBCOMMANDS.find(argv[1]);
  if (it == IMPL_SUBCOMMANDS.end()) {
    Log << "unknown subcommand: \"" << argv[1] << "\". run \"" << full_argv[0]
        << " impl help\" for a list of subcommands.";
    return false;
  }

  argv.erase(argv.begin(), argv.begin() + 1);

  return it->second(full_argv, argv);
}
