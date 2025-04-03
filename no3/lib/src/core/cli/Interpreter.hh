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

#pragma once

#include <core/cli/termcolor.hh>
#include <memory>
#include <nitrate-core/Logger.hh>
#include <no3/Interpreter.hh>
#include <vector>

namespace no3 {
  using ConstArguments = std::span<const std::string>;
  using MutArguments = std::vector<std::string>;
  using CommandFunction = std::function<bool(ConstArguments full_argv, MutArguments argv)>;

  class Interpreter::PImpl {
    friend class Interpreter;

    std::unique_ptr<detail::RCInitializationContext> m_init_rc = OpenLibrary();
    std::unordered_map<std::string, CommandFunction> m_commands;
    size_t m_log_sub_id = 0;
    std::vector<size_t> m_log_suspend_ids;

    static auto CommandBuild(ConstArguments full_argv, const MutArguments& argv) -> bool;
    static auto CommandClean(ConstArguments full_argv, const MutArguments& argv) -> bool;
    static auto CommandImpl(ConstArguments full_argv, MutArguments argv) -> bool;
    static auto CommandDoc(ConstArguments full_argv, const MutArguments& argv) -> bool;
    static auto CommandFormat(ConstArguments full_argv, const MutArguments& argv) -> bool;
    static auto CommandHelp(ConstArguments full_argv, const MutArguments& argv) -> bool;
    static auto CommandInit(ConstArguments full_argv, const MutArguments& argv) -> bool;
    static auto CommandInstall(ConstArguments full_argv, const MutArguments& argv) -> bool;
    static auto CommandFind(ConstArguments full_argv, const MutArguments& argv) -> bool;
    static auto CommandRemove(ConstArguments full_argv, const MutArguments& argv) -> bool;
    static auto CommandLSP(ConstArguments full_argv, const MutArguments& argv) -> bool;
    static auto CommandLicense(ConstArguments full_argv, const MutArguments& argv) -> bool;
    static auto CommandTest(ConstArguments full_argv, const MutArguments& argv) -> bool;
    static auto CommandVersion(ConstArguments full_argv, const MutArguments& argv) -> bool;
    static auto CommandUpdate(ConstArguments full_argv, const MutArguments& argv) -> bool;

    void SetupCommands();

  public:
    PImpl() noexcept { SetupCommands(); }

    auto Perform(const std::vector<std::string>& command) -> bool;
  };

}  // namespace no3
